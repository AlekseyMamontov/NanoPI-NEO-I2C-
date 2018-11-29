


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


typedef struct {
		char *i2_bus;
 int16_t 	 address;
 int      	 chip_open;
 int       	 bus_open;

uint16_t    data_key;
}TTP229L;

TTP229L chip = {"/dev/i2c-0",0x57};




//-------------------------------------  open i2C bus------------------------------------------------------//

int TTP229L_open_i2c(TTP229L *chip){

        chip->bus_open = open( chip->i2_bus , O_RDWR);

        return chip->bus_open; };

//-------------------------------------  close i2c bus   ---------------------------------------------------//

int TTP229L_close_i2c (TTP229L  *chip){

    int chip_close = close(chip->bus_open);
        if (chip_close <0) perror("TTP229L  error close() I2C  ");

    return chip_close;};

//-------------------------------------- check open chip for  read , write --------------------------------//

int ttp229_open_read_write (TTP229L *chip){

    if ( TTP229L_open_i2c (chip) < 0) {perror("TTP229L error  open() I2C; ");
                                                            return chip->bus_open;}            // шины нет, выходим с ошибкой

        chip->chip_open = ioctl( chip->bus_open, I2C_SLAVE,chip->address);
            if ( chip->chip_open < 0 ) {perror("Устройство на шине I2C не найдено: ");
                                                    TTP229L_close_i2c (chip);
                                                    };

    return chip->chip_open;                                                              // устройство есть на шине, выходим
};


///----------------------------------------read  bytes  ------------------------------------------------//

int16_t TTP229L_read_bytes (TTP229L *chip){

   if (TTP229L_open_i2c (chip) < 0) return chip->bus_open;

  // printf("chip open /n");

    static uint8_t buf[2];
                                                 

    struct i2c_msg msg [1] = {
                               //{chip->address, 0, 1, buf },          //выставить адрес (методом записи)
                               {chip->address, I2C_M_RD, 2, buf }    //прочитать данные в буфер
                                            };

   struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };                    
                                                                   

        if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ){ perror("I2C erorr rdwr");}
                                      //  при ок сохранить даннные в массив

  if ( TTP229L_close_i2c (chip) < 0){ perror("I2C erorr close() ");
                                                           return -1;}
   //printf("chip close /n");                                                        
                                                           
                                                           
                                                           
chip->data_key = 0;                                                          
chip->data_key = (chip->data_key | buf[0])<<8;
chip->data_key = chip->data_key | buf[1];


return chip->data_key; };


//////-------------------------------------------------------------------



int main(int argc, char** argv) {

sleep(1);
test:




if (TTP229L_read_bytes(&chip)==0) goto test;



for(int i=0;i<16;i++){ 
printf("%d", ((chip.data_key&1<<i)>>i));
};

printf ("\n %x\n",chip.data_key );




	





goto test;






return (EXIT_SUCCESS);}











