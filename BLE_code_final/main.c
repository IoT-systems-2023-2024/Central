
// header includes

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

static void start_scan(void);

static struct bt_conn *default_conn;

#define MAX_DEVICE_NAME_LEN 31

// data variables
static bt_addr_le_t discovered_device_addr;

static char discovered_device_name[MAX_DEVICE_NAME_LEN + 1];

const char name1[] = "A13 van Jarno";
const char name2[] = "ABC";

bool read1 = false;
bool read2 = false;

bool auth = false;
bool read1f = false;

//timer info
uint32_t starttimer;
uint32_t current_time;

// services
#define MAX_SERVICES 2
uint16_t service_uuids[MAX_SERVICES];

uint16_t service_numbers[MAX_SERVICES];


//periph-specific service data
uint16_t service_uid_periph1;
uint16_t service_numbers_periph1;

uint16_t service_uid_periph2[MAX_SERVICES];
uint16_t service_numbers_periph2[MAX_SERVICES];


int service_index = 0;  // Index for the current service in the arrays

// expected UUID's
uint16_t uuid_sensor_0_periph2 = 0x0125;
uint16_t uuid_sensor_1_periph2 = 0x1234;

uint16_t uuid_sensor_periph1 = 0x1111;

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	//Check if there is already a default connection
	if (default_conn) {
		return;
	}

	
	
	uint8_t *data = ad->data;
  size_t len = ad->len;

  while (len > 1)
  {
    uint8_t field_len = data[0];
    uint8_t field_type = data[1];
    if (field_type == BT_DATA_NAME_COMPLETE)
    {
			// save full local name
			// Check if the field length is within the limit
            if (field_len <= MAX_DEVICE_NAME_LEN)
            {
              // Copy the device name into the variable
              memcpy(discovered_device_name, data + 2, field_len - 1);
              discovered_device_name[field_len - 1] = '\0';
              // Now 'discovered_device_name' contains the full device name
                
            }
            else
            {
                printk("Device name too long to store\n");
            }	
				

			// state selector 	
			if(!read1 && !read2){
				auth = memcmp(data + 2,name1 , strlen(name1)) == 0||memcmp(data + 2,name2 , strlen(name2)) == 0;
			}
			else if(!read1 && read2){
				auth = memcmp(data + 2,name1 , strlen(name1)) == 0;
			}
			else if(!read2 && read1){
				auth = memcmp(data + 2,name2 , strlen(name2)) == 0;
			}



			if (auth)
      {			
					
				auth = false;
				// save adress
				memcpy(&discovered_device_addr, addr, sizeof(bt_addr_le_t));
				
				if(memcmp(data + 2,name1 , strlen(name1)) == 0){
					printk("correct name 1 \n");
					read1 = true;
				}
				if(memcmp(data + 2,name2 , strlen(name2)) == 0){
					printk("correct name 2 \n");
				
					read2 = true;
				}
      }

    }
		
        
        

        if (field_type == BT_DATA_SVC_DATA16 && bt_addr_le_cmp(addr, &discovered_device_addr) == 0 )
        {
				
					
            // Assuming the service UUID is 16 bits
						//saves the uuid's in the array 
						// only the first 2 UUID's
						if(service_index<MAX_SERVICES){
        			service_uuids[service_index] = data[2] | (data[3] << 8);
							
							service_numbers[service_index] = data[4] ;
							
							service_index++;
							

						}
						else{
							printk("format error: more services than expected \n");
							service_uuids[0] = 0xFFFF;
							service_uuids[1] = 0xFFFF;

							service_numbers[0] = 0xFF;
							service_numbers[1] = 0xFF;
						}
						

            
        }
        
    


		

		
    // Add more cases to handle other types of advertising data
    data += field_len + 1;
    len -= field_len + 1;
  }


	if (bt_addr_le_cmp(addr, &discovered_device_addr) == 0) {
		/* Hier data verwerken / kijken welke van de 2 periphs je mee te maken hebt  */
		
		if(service_index== 0){
			printk("No services error");
			service_uuids[0] = 0xFFFF;
			service_uuids[1] = 0xFFFF;

			service_numbers[0] = 0xFF;
			service_numbers[1] = 0xFF;
		}

		if(service_index == 1){
			printk("To little services or periph 1 \n");
			service_uuids[1] = 0xFFFF;
			service_numbers[1] = 0xFF;
		}

		if((read1 && !read2)||(read1 && read2 && !read1f)){
			// verwerk data periph. 1

			service_index = 0;
					 
			if(read1 && !read2){
				read1f = true;
			}
			printk("data handling periph 1 \n");
			for(int i =0;i<2;i++){
				printk("Service UUID (16-bit): 0x%04X\n", service_uuids[i]);
				printk("Service data :0x%02X \n", service_numbers[i]);

			}
				// save to periph specific variable 
				//.....
				
				if(service_uuids[0] == uuid_sensor_periph1 ){
					service_uid_periph1=service_uuids[0];
					service_numbers_periph1 = service_numbers[0];
				}else{
					printk("wrong UUID for sensor periph 1 \n");
					service_uid_periph1=0xFFFF;
					service_numbers_periph1 = 0xFF;
				}
		}
		if((read2 && !read1)||(read1 && read2 && read1f)){
			// verwerk data periph. 2
			service_index = 0;
			read1f = false;
			printk("data handling periph 2 \n");
			for(int i =0;i<2;i++){
				printk("Service UUID (16-bit): 0x%04X\n", service_uuids[i]);
				printk("Service data :0x%02X \n", service_numbers[i]);

			}
			//save to periph specific variable
			//.... 
			if(service_uuids[0] == uuid_sensor_0_periph2){
				service_uid_periph2[0]=service_uuids[0];
				service_numbers_periph2[0] = service_numbers[0];
			}	
			else if(service_uuids[1] == uuid_sensor_0_periph2 ){
				service_uid_periph2[0]=service_uuids[1];
				service_numbers_periph2[0] = service_numbers[1];
			}
			else{
				printk("none of the services have correct UUID for sensor 0 , periph 2 \n");
				service_uid_periph2[0] = 0xFFFF;
				service_numbers_periph2[0] = 0xFF;
			}

			if(service_uuids[0] == uuid_sensor_1_periph2){	
				service_uid_periph2[1]=service_uuids[0];
				service_numbers_periph2[1] = service_numbers[0];
			}
			else if(service_uuids[1] == uuid_sensor_1_periph2){
				service_uid_periph2[1]=service_uuids[1];
				service_numbers_periph2[1] = service_numbers[1];
			}
			else{
				printk("none of the services have correct UUID for sensor 1 , periph 2 \n");
				service_uid_periph2[1] = 0xFFFF;
				service_numbers_periph2[1] = 0xFF;
			}

		}
			
		
	

		memset(&discovered_device_addr, 0, sizeof(discovered_device_addr));

    printk("Discovered device matches stored address!\n");

		// Convert Bluetooth address to string for printing
		bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

		printk("Discovered device name: %s\n", discovered_device_name);

		
		
    }

	default_conn=NULL;
	
	if(read1 && read2){
		bt_le_scan_stop();
		printk("both peripherals found \n");
		// data to be sent ...
		printk("------------ \n");
		printk("periph 1: \n");
		printk("Service UUID (16-bit): 0x%04X\n", service_uid_periph1);
		printk("Service data : 0x%02X \n", service_numbers_periph1);
		printk("periph2: \n");
		printk("Service UUID (16-bit): 0x%04X\n", service_uid_periph2[0]);
		printk("Service data :0x%02X \n", service_numbers_periph2[0]);
		printk("Service UUID (16-bit): 0x%04X\n", service_uid_periph2[1]);
		printk("Service data :0x%02X \n", service_numbers_periph2[1]);
		printk("------------ \n");
		//....
		// write to csv-file 

		//....
		printk("going to sleep \n");
		k_sleep(K_SECONDS(3));  // Sleep for 1 second
		printk("woke up \n");
		read1 =false;
		read2 = false;
		start_scan();
	}
}



static void start_scan(void)
{
	int err;

	/* Start scanning  */
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}



int main(void)
{
	

	int err;

	err = bt_enable(NULL);

	// if initialization fails stop program / print this out
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	return 0;

	// intitializes ble functionality
	// if succesfull call start_scan function (start scanning for BLE peripherals)
}