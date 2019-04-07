
 /*
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 */

////////////// 8bit mode (IOCON.BANK = 1) ////////////////////////////

#define IODIRA_8bit 0x00
#define IODIRB_8bit 0x10
#define IPOLA_8bit  0x01
#define IPOLB_8bit  0x11
#define GPINTENA_8bit 0x02
#define GPINTENB_8bit 0x12
#define DEFVALA_8bit 0x03
#define DEFVALB_8bit 0x13
#define INTCONA_8bit 0x04
#define INTCONB_8bit 0x14
#define IOCON_8bit 0x05
#define IOCON2_8bit 0x15
#define GPPUA_8bit 0x06
#define GPPUB_8bit 0x016
#define INTFA_8bit 0x07
#define INTFB_8bit 0x017
#define INTCAPA_8bit 0x08
#define INTCAPB_8bit 0x18
#define GPIOA_8bit 0x09
#define GPIOB_8bit 0x19
#define OLATA_8bit 0x0a
#define OLATB_8bit 0x1a


///////////////////// 16 bit mode  (IOCON.BANK = 0) ////////////////////

#define IODIRA 0x00 //  регистры указывают навправление входа и выхода для 16 портов
#define IODIRB 0x01 //  bit =1 - вход , bit = 0 выход, IODIRA - порты А0 - А7, ODIRB порты B0-В7

#define IPOLA  0x02 //  регистры отвечают за инверсию входных данных,  для портов  GPIOA - GPIOB
#define IPOLB  0x03 //  которые выставлены как вход (bit=1)  1 -> 0 или 0 ->1 :   (bit=0) 1->1 или 0->0

#define GPINTENA 0x04 //    Выставив  bit=1 мы определяем что данный порт может быть внешним прерыванием
#define GPINTENB 0x05 //    Прерывание происходит при смене напряжения.

#define DEFVALA 0x06 // В данных регистрах мы указываем при каком условии происходит прерывание
#define DEFVALB 0x07 // например bit=0 - то прерывание произойдет при  появление 1 (+3.3 до +5V)  на данном порту
                                    //  или bit=1 - тогда когда на порту появится 0 (GND,VSS) произойдет прерывание

#define INTCONA 0x08 // Если в этом регистре мы выставляем bit = 1 , то  определения для прерываний
#define INTCONB 0x09 // заданные в DEFVALA , DEFVAB игнорируются и прерывание происходит при условии
                                    // смены уровней тоесть 0 -> 1 прерывание  1-> 0 прерывание.

#define GPPUA 0x0c  // Подтягивать порты которые как входы к питанию через 100 кОм резистор
#define GPPUB 0x0d  // 1 - подтягивать , 0 - не подтягивать

#define INTFA 0x0e // Помогает определить на какой ножке произошло прерывание,  по умолчанию во всем
#define INTFB 0x0f  // регистре 0, произошло прерывание на порту в соответствующем разряде  появится 1

#define INTCAPA 0x10 // Регистр защелка, после прерывания происходит фиксация всех значений на портах на момент
#define INTCAPB 0x11 // прерывания,

#define GPIOA 0x12 //0b00010010 Это двухнаправленный регистр здесь хранятся значения на портах выставленных как входы
#define GPIOB 0x13 //0b00010011 c помощью регистров IODIRA(B), и при записи значения попадают фактически в OLATA(B)

/*
*Из-за особенностей внутренней реализации MCP23017 и MCP23018 (или бага в последней), они ведут себя по разному при записи в один регистр 
*по кругу (режим bit5 SEQOP = 1) , если писать напрямую в защелки регистр OLATA(B), то mcp23017 ведет себя адекватно и ожидаемо,
*пишет по кругу в данные регистры, но mcp23018 - сходит с ума, записав первую пару значений, перескакивает на следующую пару регистров и 
* пишет все остальные данные в эти регистры, а так как это последния парв регистров OLATA(B) - то все остальные данные попадают в первыую пару 
* регистров IODIRA(B) - отвечающая за выставление направление портов - и в этоге становится весело и этот баг нигде не описан.          

*Поэтому наверно не стоит писать напрямую в регистры  OLATA(B) по кругу, для этого лучше работать c GPIOA(B), в итоге получается примерно адекватная реакция
*от MCP23018 он исходя из логики описанной работы , записывает первую пару значений GPIOA(B), и затем все остальное в OLATA(B). 

как то так

*/

#define OLATA 0x14 //0b00010100 В этом регистре выставляются значения на портах выбранными как выходы
#define OLATB 0x15 //0b00010101 c помощью регистров IODIRA , IODIRB,

#define IOCON 0x0A    // Управляющие регистры дублируют друг друга.
#define IOCON2 0x0B  //
/*
bit 7  (BANK)  0 - 16 bit  представление регистров
                       1 -   8 bit представление регистров

bit 6 (Mirror)  0 -  выходы микросхемы INTA и INTB отвечают каждая за свои 8 портов
                       1-   выходы дублируют друг друга

bit5 (SEQOP)  0 -  при записи данных -  счетчик адресс  регистров увеличивается
                        1 -  при записи данных - адресс регистров не изменяется
                        //8 битном режиме// - в один регистр
                        //16 битном режиме// - в два регистра последовательно
                                                             тоесть -> OLATA -> OLATB -> OLATA ->OLTAB

bit4 (DISSLW)  0 -  помочь ведущему на линии опускать и поднимать ногу на линии
                       1 - не помогать ))

bit3                -  для микросхемы MCP23S17 (SPI)

bit2 (ODR)      0  - ножки микросхемы INTA и INTB - цифровые   0 или 1
                      1 -  ножки микросхемы INTA и INTB - транзисторный выход (открытый коллетор)
                                ногу к резистору 10кОМ и к + (VDD),

bit1  (iNTPOL)  0 -  вслучаи прерывания активный сигнал 0, простой 1
                      1 -  вслучаи прерывания активный сигнал 1 , простой  0
 */

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
//#include <threads.h>

#define MAX_msg 10
#define IODIR_OUT 0
#define IODIR_IN 1


	
typedef struct {
char 		*i2_bus;
uint16_t 	address;

 int      	chip_open;
 int       	bus_open;
 uint8_t 	lock_chip;
 
uint8_t 	registr[0x16];
uint8_t 	buf[8192];

struct i2c_msg msg [MAX_msg];
struct i2c_rdwr_ioctl_data rdwr;

}MCP23017;

// example
//MCP23017 chip1 = {"/dev/i2c-0",0x20};
//MCP23017 chip2 = {"/dev/i2c-0",0x24};



//--------------------------------------------------------------------------------------//
/*	struct MCP23017_PIN  pinblock[] ={
{ *chip, bit, modeInOut, default_value,on,ipol},
{ *chip2,bit, modeInOut, default_value,on,ipol},
{ *chip, bit, modeInOut, default_value,on,ipol},
......
{ *chip3,bit, modeInOut, default_value,on,ipol}
};

*/
//--------------------------------------------------------------------------------------//

struct MCP23017_PIN{
MCP23017 *chip;
uint8_t bit;	  // n-bit 0...15 for chip;
uint8_t modeInOut;// 1 - in, 0 -out  IODIRA
uint8_t default_value;
uint8_t on;		  // default 1, 
uint8_t ipol;     // in port, invert 0 - no, 1- yes
uint8_t value;

};



//--------------------------------------------------------------------------------------//
//									open i2C bus 										//
//--------------------------------------------------------------------------------------//


int mcp23017_open_i2c(MCP23017 *chip){
	
open_lock:	if(chip->lock_chip ) return -1;

        chip->bus_open = open( chip->i2_bus , O_RDWR);

		if(chip->bus_open < 0) {perror("mcp23017 error  open() I2C; ");
			}else {chip->lock_chip = 1;}
		
        return chip->bus_open; };


//--------------------------------------------------------------------------------------//
//									close i2c bus 										//
//--------------------------------------------------------------------------------------//


int mcp23017_close_i2c (MCP23017 *chip){

    int chip_close = close(chip->bus_open);
    
        if (chip_close <0) perror("mcp23017 error close() I2C  ");
		
	if (chip->lock_chip == 1) chip->lock_chip = 0;
		
    return chip_close;};


//--------------------------------------------------------------------------------------//
//	Check open chip for  read , write - проверить есть устройство на шине и сама шина	//
//--------------------------------------------------------------------------------------//


int mcp23017_open_read_write (MCP23017 *chip){

    if ( mcp23017_open_i2c (chip) < 0) {perror("mcp23017 error  open() I2C; ");
                                        return chip->bus_open;}            				// шины нет, выходим с ошибкой

        chip->chip_open = ioctl( chip->bus_open, I2C_SLAVE,chip->address);
        
            if ( chip->chip_open < 0 ) {perror("Устройство на шине I2C не найдено: ");
                                        mcp23017_close_i2c (chip);};

    return chip->chip_open;                                                              // устройство есть на шине, выходим
};


//--------------------------------------------------------------------------------------//
//				Write bytes 8bit - записать байт в регистр 								//
//--------------------------------------------------------------------------------------//


uint8_t mcp23017_write_byte (MCP23017 *chip , uint8_t registr, uint8_t value){

if (mcp23017_open_i2c (chip) < 0) {perror("I2C erorr open() write byte()");
										return chip->bus_open;}

static uint8_t buf[2];

buf[0] = registr;                                           // buf[0]  - номер  регистра
buf[1] = value;                                             // buf[1]  - что писать в этот регистр

	struct i2c_msg msg [1] = {{chip->address, 0, 2, buf }};  //выставить адрес и записать
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };

	
		int chip_write = ioctl(chip->bus_open, I2C_RDWR, &rdwr );
	
			if ( chip_write < 0 ) {perror("I2C erorr rdwr");
							}else{chip->registr[registr] = value;}


 return  mcp23017_close_i2c (chip);}	//if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() write_byte()"); 
										//return -1;};

//--------------------------------------------------------------------------------------//
//	write Array 8bit - отправка буфера  buf[0] - registr ; buf[1] ....buf[n] - data		//
// buf- max 8192 byte ( страница 4096 байт)												//
//--------------------------------------------------------------------------------------//


uint8_t mcp23017_write_array (MCP23017 *chip , uint8_t *buf, uint32_t size_buf){

	if (mcp23017_open_i2c (chip) < 0){perror("I2C erorr open() write byte()");
										return chip->bus_open;}


	struct i2c_msg msg [1] = {{chip->address, 0, size_buf, buf }}; 
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };

									
	int chip_write =ioctl(chip->bus_open, I2C_RDWR, &rdwr );
	
		if ( chip_write < 0 ) {perror("I2C erorr rdwr");
						 }else{//chip->registr[buf[0]] = buf[value-1];
								}

	if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() write_byte()"); 
										return -1;};

 return  chip_write;}

//--------------------------------------------------------------------------------------//
//				Read bytes 8bit - прочитать данные из регистра   (8 бит) 				//
//--------------------------------------------------------------------------------------//


uint8_t mcp23017_read_byte (MCP23017 *chip, uint8_t registr){

	if (mcp23017_open_i2c (chip) < 0) {perror("I2C erorr open() read byte()");
									   return chip->bus_open;}

static uint8_t buf[1];
buf[0]= registr;                                                 	//выставить регистр который читаем

   struct i2c_msg msg [2] = {{chip->address, 0, 1, buf },           //выставить адрес (методом записи)
                             {chip->address, I2C_M_RD, 1, buf }};   //прочитать данные в буфер
   struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                     //поместить массив  блоков (не больше 32 байт в блоке)
																	// структуру (на все дается 8192 байт)

                                                                    


    if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ){perror("I2C erorr rdwr read byte()");
									    }else {chip->registr[registr] = buf[0];}  //  при ок сохранить даннные в массив

	if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() read byte()");
                                         return -1;}


return chip->registr[registr];
};



//-------------------------------------------------------------------------------------//
//	Registr read bit - считать значение  бита в регистре на чипе					   //
//-------------------------------------------------------------------------------------//

uint8_t mcp23017_registr_read_bit (MCP23017 *chip, uint8_t registr,uint8_t bit){

	registr = registr | ((bit&0x08)>>3);			// регистр A или B (OLAT..)
	uint8_t status_bit = 0b00000001 << (bit&0x7);	// выставить бит на данном порту
	
	return (mcp23017_read_byte(chip,registr)& status_bit) >> (bit&0x7);
};

//------------------------------------------------------------------------------------//
//	Registr set bit - установить бит в регистре на чипе								  //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_registr_set_bit (MCP23017 *chip, uint8_t registr, uint8_t bit,uint8_t value){
	
	uint8_t correct = (registr == GPIOA)? 2 : 0;
	registr = registr|((bit&0b00001000)>>3);		// регистр A или B (OLAT..)
	uint8_t status_bit = 0b00000001 << (bit&0x7);	// выставить бит на данном порту
			
	mcp23017_read_byte(chip,registr+correct);		// Если это регисты портов - то считать данные
													// с регистра OLATA(B) - защелки на выход.
	
	if(value){value = chip->registr[registr]|status_bit;     // если больше единицы то выставить бит
		}else{value = chip->registr[registr]&(~status_bit);} // если меньше сбросить
			
	mcp23017_write_byte(chip,registr,value); // и записать 	
};

//------------------------------------------------------------------------------------//
// 						PIN_read - Чтение бита из порта 		         			  //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_read_pin_bit (MCP23017 *chip, uint8_t bit){

	return  mcp23017_registr_read_bit (chip,GPIOA,bit);
};
//------------------------------------------------------------------------------------//
// 						PIN_write - Запись бита в порт						          //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_set_pin_bit (MCP23017 *chip, uint8_t bit,uint8_t value){

	return mcp23017_registr_set_bit (chip,GPIOA,bit,value);
};

//-----------------------------------------------------------------------------------//
//			SET_modeInOut - Установить порт на 0 - выход, 1  - вход		   	 	     //
//-----------------------------------------------------------------------------------//

uint8_t mcp23017_set_mode(MCP23017 *chip, uint8_t bit,uint8_t value){

	return mcp23017_registr_set_bit (chip,IODIRA,bit,value);
};

//-----------------------------------------------------------------------------------//
//			 	set_pullUP  0 - no, yes - 1  подтягивать к питанию 100кОм			 //
//-----------------------------------------------------------------------------------//

uint8_t mcp23017_set_pullUP(MCP23017 *chip, uint8_t bit,uint8_t value){

	return mcp23017_registr_set_bit (chip,GPPUA,bit,value);
};

//-----------------------------------------------------------------------------------//
//			 	set_IPOL 0 - no , yes - 1   Инвертитровать входной сигнал            //
//-----------------------------------------------------------------------------------//

uint8_t mcp23017_set_IPOL(MCP23017 *chip, uint8_t bit,uint8_t value){

	return mcp23017_registr_set_bit (chip,IPOLA,bit,value);
};

//------------------------------------------------------------------------------------//
// 			Enabled device (Rele,pneumocylinder ...) включить устройство 			  //
//------------------------------------------------------------------------------------//

int mcp23017_enable_device (struct MCP23017_PIN *pin){
	pin->value = pin->on;
	return mcp23017_set_pin_bit (pin->chip,pin->bit,pin->value);
};

//------------------------------------------------------------------------------------//
//		    Disabled device (rele, pneumocylinder ...)	выключить устройство      	  //
//------------------------------------------------------------------------------------//

int mcp23017_disable_device (struct MCP23017_PIN *pin){
	pin->value = !(pin->on);
	return mcp23017_set_pin_bit (pin->chip,pin->bit,pin->value);
};
//------------------------------------------------------------------------------------//
//						      init device rele, pnevmocylinder...			  		  //
//------------------------------------------------------------------------------------//

int mcp23017_init_device (struct MCP23017_PIN *pin){
	
	mcp23017_set_mode(pin->chip,pin->bit,pin->modeInOut); //0-out 1-in
	
	if(pin->modeInOut&1){mcp23017_set_pullUP(pin->chip,pin->bit,pin->default_value);
						 mcp23017_set_IPOL(pin->chip,pin->bit,pin->ipol);
						 return pin->value = mcp23017_read_pin_bit(pin->chip,pin->bit); 
						 };
						 
	pin->value = pin->default_value;					 	

	return mcp23017_set_pin_bit (pin->chip,pin->bit,pin->value);
};

//------------------------------------------------------------------------------------//
//			      		      	Init_block_pin 										  //
//------------------------------------------------------------------------------------//

void mcp23017_init_block_device(struct MCP23017_PIN *pin, uint8_t i, uint8_t n){

	n = n + i;
	
	for (i; i < n; i++){
				
		mcp23017_init_device(&pin[i]);
	};
};

//------------------------------------------------------------------------------------//
//      				read Sensor - чтение значение с концевика				  //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_read_sensor (struct MCP23017_PIN *pin){

return pin->value = mcp23017_read_pin_bit(pin->chip,pin->bit);
};
//------------------------------------------------------------------------------------//
//      scan_sensors_chip - обновление данных портов - регистры GPIOA(B)..OLATA(B)	  //										  //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_scan_sensors_chip (MCP23017 *chip){ 

	if (mcp23017_open_i2c (chip) < 0) {perror("mcp23017_scan_sensors_chip_byte I2C erorr open()");
									   return chip->bus_open;}

	static uint8_t buf[4];
	buf[0]= GPIOA;                                                 	//выставить регистр который читаем

   struct i2c_msg msg [2] = {{chip->address, 0, 1, buf },           //выставить адрес (методом записи)
                             {chip->address, I2C_M_RD, 2, buf}};    //прочитать данные в буфер
   struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                     //поместить массив  блоков (не больше 32 байт в блоке)
																	//структуру (на все дается 8192 байт)


    if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ){perror("mcp23017_scan_sensors_chip I2C erorr rdwr");
		}else {chip->registr[GPIOA] = buf[0];
			   chip->registr[GPIOB] = buf[1];
			   //chip->registr[OLATA] = buf[2];
			   //chip->registr[OLATB] = buf[3];
			}  //при ок сохранить даннные в массив
    if ( mcp23017_close_i2c (chip) < 0){ perror("mcp23017_scan_sensors_chip I2C erorr close() ");
                                         return -1;}

return 0;
};


//------------------------------------------------------------------------------------//
//			      RESTORE default MCP23017 - сброс микросхемы в default  		      //
//------------------------------------------------------------------------------------//

void mcp23017_restore_default (MCP23017 *chip){
	
	uint8_t buf[0x16] = {0xff,0xff,0};
	
	mcp23017_write_byte(chip,0x05,0);
	mcp23017_write_byte(chip,0x0A,0);
	
	for (int i=0 ; i <0x16 ; i++){
		
		mcp23017_write_byte(chip,i,buf[i]);
		
	};
};



////////////////////// experiment function ////////////////////////////////////////////											  


//registr set bit v2	 		              

uint8_t mcp23017_set_bit_v2 (MCP23017 *chip,uint8_t registr, uint8_t bit,uint8_t value){
	
	registr = registr | ((bit&0x08)>>3);
	chip->buf[0]=registr;
	uint8_t status_bit = 0b00000001 << (bit&0x7);

	chip->buf[0]=registr;
	chip->msg[0].addr = chip->address;							
	chip->msg[0].flags = 0;
	chip->msg[0].buf = chip->buf;
	chip->msg[0].len = 1;
	
	chip->msg[1].addr = chip->address;							
	chip->msg[1].flags = I2C_M_RD;
	chip->msg[1].buf = chip->buf;
	chip->msg[1].len = 1;
	
	chip->rdwr.msgs = chip->msg;
	chip->rdwr.nmsgs = 2;		
		
	if (mcp23017_open_i2c (chip) < 0){return chip->bus_open;};
	
	int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
			
		if ( chip_write < 0 ){perror("I2C erorr rdwr set_bit");
							  return chip_write;
							  } else {chip->registr[registr]= chip->buf[0];}

			chip->buf[0]=registr;
						
			if(value){value = chip->registr[registr]|status_bit;
				}else{value = chip->registr[registr]&(~status_bit);}
			
			chip->buf[1]= value;			
			chip->msg[0].len = 2;
			chip->rdwr.nmsgs = 1;
			
			chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));	
			if ( chip_write < 0 ){perror("I2C erorr rdwr set_bit");
								  return chip_write;
								 } else {chip->registr[registr]= chip->buf[1];}
											
								     
	if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() set_bit ");
										return -1;};
																														
	
return 0;
};



//	registr read_bit	

uint8_t mcp23017_read_bit_v2 (MCP23017 *chip,uint8_t registr, uint8_t bit){
	
	 if (mcp23017_open_i2c (chip) < 0) return chip->bus_open;
	 
	registr = registr | ((bit&0x08)>>3);
	
	uint8_t status_bit = 0b00000001 << (bit&0x7);
	
		
	chip->buf[0]=registr;	
	
	chip->msg[0].addr = chip->address;							
	chip->msg[0].flags = 0;
	chip->msg[0].buf = chip->buf;
	chip->msg[0].len = 1;
	
	chip->msg[1].addr = chip->address;							
	chip->msg[1].flags = I2C_M_RD;
	chip->msg[1].buf = chip->buf;
	chip->msg[1].len = 1;
	
	chip->rdwr.msgs = chip->msg;
	chip->rdwr.nmsgs = 2;			
	
	
	
	int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));		
		if ( chip_write < 0 ){perror("I2C erorr rdwr read_bit");
		    } else {chip->registr[registr]= chip->buf[0];}
								     
	if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() read_bit ");
										return -1;}
										

	
	status_bit = (chip->registr[registr] & status_bit) >> (bit&0x7);
	
	return status_bit;
			
};

/*
 *printf ("registr chip  %x value %x ",chip->registr[registr],value);
 *  
 printf ("registr %x bit %x ,value %x\n",registr,status_bit,value);
 * 
 */
 
  
 



