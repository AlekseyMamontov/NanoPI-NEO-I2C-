
 /*
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 * 

 * 	mcp23017 GPIOA7,GPIOB7 желательно выставлять на выход, так как из-за аппаратной ошибки чипа mcp23017
	при выборе данных пинов как входы - если в момент чтения из него происходит 
	смена уровня на нем, происходит сбой с выставлением сигнала STOP на линии.
	проблема не решена. errata mcp23017
  
*	Из-за особенностей внутренней реализации MCP23017 и MCP23018 (или бага в последней), они ведут себя по разному при записи в один регистр 
	по кругу (режим bit5 SEQOP = 1) , если писать напрямую в защелки регистр OLATA(B), то mcp23017 ведет себя адекватно и ожидаемо,
	пишет по кругу в данные регистры, но mcp23018 - сходит с ума, записав первую пару значений, перескакивает на следующую пару регистров и 
	пишет все остальные данные в эти регистры, а так как это последния парв регистров OLATA(B) - то все остальные данные попадают в первыую пару 
	регистров IODIRA(B) - отвечающая за выставление направление портов - и в этоге становится весело и этот баг нигде не описан.          
	Поэтому наверно не стоит писать напрямую в регистры  OLATA(B) по кругу, для этого лучше работать c GPIOA(B), в итоге получается примерно адекватная реакция
	от MCP23018 он исходя из логики описанной работы , записывает первую пару значений GPIOA(B), и затем все остальное в OLATA(B). 
	
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

//16 bit mode  (IOCON.BANK = 0) 

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



#define OLATA 0x14 //0b00010100 В этом регистре выставляются значения на портах выбранными как выходы
#define OLATB 0x15 //0b00010101 c помощью регистров IODIRA , IODIRB,

#define IOCON 0x0A    // Управляющие регистры дублируют друг друга.
#define IOCON2 0x0B  //
/*
bit 7 (BANK)   0 - 16 bit  представление регистров
               1 -  8 bit представление регистров

bit 6 (Mirror) 0 -  выходы микросхемы INTA и INTB отвечают каждая за свои 8 портов
               1-   выходы дублируют друг друга

bit5 (SEQOP)   0 -  при записи данных -  счетчик адресс  регистров увеличивается
               1 -  при записи данных - адресс регистров не изменяется
                  //8 битном режиме// - в один регистр
                  //16 битном режиме// - в два регистра последовательно
                                         оесть -> OLATA -> OLATB -> OLATA ->OLTAB

bit4 (DISSLW)  0 -  помочь ведущему на линии опускать и поднимать ногу на линии
               1 - не помогать ))

bit3         	 -  для микросхемы MCP23S17 (SPI)

bit2 (ODR)     0  - ножки микросхемы INTA и INTB - цифровые   0 или 1
               1 -  ножки микросхемы INTA и INTB - транзисторный выход (открытый коллетор)
                    ногу к резистору 10кОМ и к + (VDD),

bit1 (iNTPOL)  0 -  вслучаи прерывания активный сигнал 0, простой 1
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


#define MAX_msg 10
#define IODIR_OUT 0
#define IODIR_IN 1

	
typedef struct {
char 	 *i2_bus;
uint16_t address;
int      chip_open;
int      bus_open; 
pthread_mutex_t lock_registr_GPIO;
pthread_mutex_t lock_registr_OLAT;
pthread_mutex_t lock_registr; 
uint8_t registr[0x16];

struct i2c_msg msg [MAX_msg];
struct i2c_rdwr_ioctl_data rdwr;

}MCP23017;



//--------------------------------------------------------------------------------------//
//									open i2C bus 										//
//--------------------------------------------------------------------------------------//
int mcp23017_open_i2c(MCP23017 *chip){	
	chip->bus_open = open( chip->i2_bus , O_RDWR);
	if(chip->bus_open < 0) perror("mcp23017 error open() I2C;");					
return chip->bus_open; };       
//--------------------------------------------------------------------------------------//
//									close i2c bus 										//
//--------------------------------------------------------------------------------------//
int mcp23017_close_i2c (MCP23017 *chip){
    int chip_close = close(chip->bus_open);    
    if (chip_close <0) perror("mcp23017 error close() I2C");				
return chip_close;};    
//--------------------------------------------------------------------------------------//
//	Check open chip for  read , write - проверить есть устройство на шине и сама шина	//
//--------------------------------------------------------------------------------------//
int mcp23017_check_device (MCP23017 *chip){
	if(mcp23017_open_i2c(chip) < 0){
		perror("mcp23017 error open() I2C; ");
        return chip->bus_open;} // шины нет, выходим с ошибкой
                                                				
    chip->chip_open = ioctl( chip->bus_open, I2C_SLAVE,chip->address);        
    if ( chip->chip_open < 0 ){
		perror("Устройство на шине I2C не найдено: ");
        mcp23017_close_i2c (chip);}        
return chip->chip_open;                                                              // устройство есть на шине, выходим
};
//--------------------------------------------------------------------------------------//
//				Write bytes 8bit - записать байт в регистр 								//
//--------------------------------------------------------------------------------------//
int mcp23017_write_byte (MCP23017 *chip , uint8_t registr, uint8_t value){
pthread_mutex_lock (&chip->lock_registr);	

	uint8_t buf[2] = {registr,value};
	struct i2c_msg msg [1] = {{chip->address, 0, 2, buf }};  //выставить адрес и записать
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };		

	chip->registr[registr] = value;

	int  error = ioctl(chip->bus_open, I2C_RDWR,&rdwr);
	if ( error < 0 ) perror("I2C mcp23017_write_byte erorr rdwr");

pthread_mutex_unlock (&chip->lock_registr);	
return error;
};	
//--------------------------------------------------------------------------------------//
//	write Array 8bit - отправка буфера  buf[0] - registr ; buf[1] ....buf[n] - data		//
// buf- max 8192 byte ( страница 4096 байт)												//
//--------------------------------------------------------------------------------------//

int mcp23017_write_array (MCP23017 *chip , uint8_t *buf, uint32_t size_buf){    
pthread_mutex_lock (&chip->lock_registr);

	struct i2c_msg msg [1] = {{chip->address, 0, size_buf, buf }}; 
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };
									
	int chip_write =ioctl(chip->bus_open, I2C_RDWR, &rdwr );	
	if ( chip_write < 0 )perror("I2C erorr mcp23017_write_array");
						 
pthread_mutex_unlock (&chip->lock_registr);						 							
return  chip_write;
};
//--------------------------------------------------------------------------------------//
//				Read bytes 8bit - прочитать данные из регистра   (8 бит) 				//
//--------------------------------------------------------------------------------------//

uint8_t mcp23017_read_byte (MCP23017 *chip, uint8_t registr){

pthread_mutex_lock (&chip->lock_registr);

	uint8_t buf[1] = {registr};										//выставить регистр который читаем
	struct i2c_msg msg [2] = {{chip->address, 0, 1, buf },          //выставить адрес (методом записи)
                             {chip->address, I2C_M_RD, 1, buf }};   //прочитать данные в буфер
	struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                    //поместить массив  блоков (не больше 32)																//структуру (на все дается 8192 байт)
    
    if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ){perror("I2C erorr mcp23017_read_byte");
	 }else chip->registr[registr] = buf[0];													  

pthread_mutex_unlock (&chip->lock_registr);
									   
return chip->registr[registr];
};

//------------------------------------------------------------------------------------//
//	Registr set bit - установить бит в регистре на чипе								  //
//------------------------------------------------------------------------------------//
uint8_t mcp23017_registr_set_bit(MCP23017 *chip,uint8_t registr, uint8_t bit,uint8_t value){
	
pthread_mutex_lock (&chip->lock_registr);

	int error = -1;
	uint8_t buf[3];
	uint8_t status_bit = 0b00000001 << (bit&0x7);// выставить бит на данном порту
	struct i2c_msg msg [2] = {
		{chip->address,0,1,buf}, 			//выставить адрес (методом записи)
		{chip->address,I2C_M_RD,1,buf}		//прочитать данные в буфер
	};   
	struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };
		
	registr = (registr == GPIOA)? registr + 2 : registr ;
	registr = registr|((bit&0b00001000)>>3);		// регистр A или B (OLAT..)
	buf[0] = registr;								//выставить регистр который читаем
		
	if(ioctl(chip->bus_open,I2C_RDWR,&rdwr) < 0){
		perror("I2C erorr rdwr set_bit");
	}else{chip->registr[registr] = buf[0];}

	buf[0]=registr;
	buf[1] = (value)? chip->registr[registr]|status_bit: 
					  chip->registr[registr]&(~status_bit);					
	msg[0].len = 2;
	rdwr.nmsgs = 1;
			
	if(ioctl(chip->bus_open, I2C_RDWR, &rdwr)< 0){
		perror("I2C erorr rdwr set_bit");
	}else{chip->registr[registr] =buf[1]; 
		  error = 0;}
																			     
pthread_mutex_unlock (&chip->lock_registr);																													
};

//------------------------------------------------------------------------------------//
//	Registr set bit - установить бит в регистре на чипе								  //
//------------------------------------------------------------------------------------//

uint8_t mcp23017_registr_set_bit2(MCP23017 *chip, uint8_t registr, uint8_t bit,uint8_t value){
	
	registr = (registr == GPIOA)? registr + 2 : registr ;
	registr = registr|((bit&0b00001000)>>3);		// регистр A или B (OLAT..)
	uint8_t status_bit = 0b00000001 << (bit&0x7);	// выставить бит на данном порту
			
	mcp23017_read_byte(chip,registr);		// Если это регисты портов - то считать данные
													// с регистра OLATA(B) - защелки на выход.
	
	chip->registr[registr] = (value)? chip->registr[registr]|status_bit:// 1
									  chip->registr[registr]&(~status_bit); // 0
			
	mcp23017_write_byte(chip,registr,chip->registr[registr]); // и записать 		
};





//-------------------------------------------------------------------------------------//
//	Registr read bit - считать значение  бита в регистре на чипе					   //
//-------------------------------------------------------------------------------------//
uint8_t mcp23017_registr_read_bit (MCP23017 *chip, uint8_t registr,uint8_t bit){

pthread_mutex_lock (&chip->lock_registr);

	uint8_t b;
	registr = registr | ((bit&0x08)>>3);			// регистр A или B (OLAT..)
	uint8_t status_bit = 0b00000001 << (bit&0x7);	// выставить бит на данном порту

	uint8_t buf[1] = {registr};										//выставить регистр который читаем
	struct i2c_msg msg [2] = {{chip->address, 0, 1, buf },          //выставить адрес (методом записи)
                             {chip->address, I2C_M_RD, 1, buf }};   //прочитать данные в буфер
	struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                    //поместить массив  блоков (не больше 32)																//структуру (на все дается 8192 байт)
    
    if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 ){perror("I2C erorr mcp23017_read_byte");
	 }else chip->registr[registr] = buf[0];													  

	bit = (chip->registr[registr]&status_bit)>>(bit&0x7);

pthread_mutex_unlock (&chip->lock_registr);
									   
return b;
};

//------------------------------------------------------------------------------------//
//      scan_chip - обновление данных портов - регистры GPIOA(B)..OLATA(B)	 		  //										  //
//------------------------------------------------------------------------------------//

int mcp23017_get_registrs_chip (MCP23017 *chip){ 

	
	uint8_t buf[2];
	buf[0]= GPIOA;                                                 	//выставить регистр который читаем
	struct i2c_msg msg [2] = {{chip->address, 0, 1, buf },           //выставить адрес (методом записи)
                             {chip->address, I2C_M_RD, 2, buf}};    //прочитать данные в буфер
	struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                     //поместить массив  блоков (не больше 32 байт в блоке)

	pthread_mutex_lock (&chip->lock_registr);
	
    if(ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 )perror("mcp23017_scan_sensors_chip I2C erorr rdwr");																	
		chip->registr[GPIOA] = buf[0];
		chip->registr[GPIOB] = buf[1];
		
	pthread_mutex_unlock (&chip->lock_registr);	
	return 0;
};
//------------------------------------------------------------------------------------//
//      re-build_chip - обновление данных портов - регистры GPIOA(B)..OLATA(B)	 	  //										  //
//------------------------------------------------------------------------------------//
int mcp23017_set_registrs_chip (MCP23017 *chip){ 

	static uint8_t buf[3];
	struct i2c_msg msg [1] = {{chip->address, 0, 3, buf }}; 
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };
	
	pthread_mutex_lock (&chip->lock_registr);
	
	buf[0]= OLATA;                                                 	
	buf[1]= chip->registr[OLATA];
	buf[2]= chip->registr[OLATB];
	int ret = ioctl(chip->bus_open, I2C_RDWR, &rdwr );
	
	pthread_mutex_unlock (&chip->lock_registr);
	
    if ( ret < 0 ) perror("mcp23017_scan_sensors_chip I2C erorr rdwr");

return ret;
};

//------------------------------------------------------------------------------------//
//			      RESTORE default MCP23017 - сброс микросхемы в default  		      //
//------------------------------------------------------------------------------------//

void mcp23017_restore_default (MCP23017 *chip){	
	uint8_t buf[0x16] = {0xff,0xff,0};	
	mcp23017_write_byte(chip,0x05,0);
	mcp23017_write_byte(chip,0x0A,0);
	for (int i=0 ; i <0x16 ; i++){mcp23017_write_byte(chip,i,buf[i]);};
};



























	
	
	
	
	
	
	
	
	
	
	
	

 
  
 



