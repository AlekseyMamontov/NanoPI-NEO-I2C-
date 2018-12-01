


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>  // struct i2c_msg
#include <sys/ioctl.h>

#include <linux/i2c.h> // struct i2c_rdwr_ioctl_data

#define MAX_key_TTP229 16



typedef struct {
		char *i2_bus;
 uint16_t 	 address;
 int      	 chip_open;
 int       	 bus_open;

 uint16_t   data_key;
 uint8_t    n_bit_mask[MAX_key_TTP229]; // номер бита нажатой клавиши в data_key
 uint8_t  	bufer[MAX_key_TTP229+1];    // буфер нажатых клавиш одновременно
 
 uint8_t    size_bufer;
 uint8_t   	*table_key;

}TTP229L;


static char table_key_TTP229[MAX_key_TTP229] = {
	16,15,14,13,
	12,11,10,9,
	8,7,6,5,
	4,3,2,1
};

TTP229L chip_keys = { .i2_bus = "/dev/i2c-0",
					  .address = 0x57,
					  .table_key = table_key_TTP229,
					};




//-------------------------------------  open i2C bus------------------------------------------------------//

int TTP229L_open_i2c(TTP229L *chip){

        chip->bus_open = open( chip->i2_bus , O_RDWR);

        return chip->bus_open; };

//-------------------------------------  close i2c bus   ---------------------------------------------------//

int TTP229L_close_i2c (TTP229L  *chip){

    int chip_close = close(chip->bus_open);
        if (chip_close <0) perror("TTP229L  error close() I2C  ");

    return chip_close;};




//-------------------------------------- read  2 bytes (16 keys) -------------------------------------------//

uint16_t TTP229L_read_bytes (TTP229L *chip){

static uint8_t buf[2] = {0};
 struct i2c_msg msg [1] = {{chip->address, I2C_M_RD, 2, buf }};     //прочитать данные в буфер
 struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };                                              
                                            
                                            
   if (TTP229L_open_i2c (chip) < 0) return chip->bus_open;
             
        if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ) perror("I2C erorr rdwr");

  if ( TTP229L_close_i2c (chip) < 0) perror("I2C erorr close() ");
                                                                                                                   

//Data_0 : B7~B0 is TP0~TP7 on/off status. 0 is key off, 1 is key on.
//Data_1 : B7~B0 is TP8~TP15 on/off status. 0 is key off, 1 is key on.
                                                           
int data_key = 0;                                                          
data_key = (data_key | buf[0])<<8;
data_key = data_key | buf[1];


return data_key; };


///------------------------------ Read keys to buffer--------------------------------//

void TTP229L_read_key (TTP229L *chip){
uint16_t data_key;
uint8_t s_bufer = 0;
	
 data_key = TTP229L_read_bytes(chip);
	
	for(int i=0; i<16; i++){
		
		if(data_key&1<<i){			
							chip->bufer[s_bufer]= chip->table_key[i];
							chip->n_bit_mask[s_bufer] = i;
							s_bufer ++;
						  };						  
	};
	
chip->bufer[s_bufer]= 0;
chip->size_bufer = s_bufer;
chip->data_key = data_key;
};
///////////////////////////////////////////////////////////////////////





/////////////////// TEST  chip 229L

int main(int argc, char** argv) {



sleep(1);

test:

TTP229L_read_key(&chip_keys);

if (chip_keys.size_bufer == 0) goto test;

	for(int i=0; i < chip_keys.size_bufer; i++ ){
								
								printf("byte %d bit %d +", chip_keys.bufer[i],chip_keys.n_bit_mask[i]);
	};

printf ("\n");


goto test ;


return (EXIT_SUCCESS);}








