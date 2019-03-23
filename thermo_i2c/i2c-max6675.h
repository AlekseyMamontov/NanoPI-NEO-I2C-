/*
 * Chip MAX6675
 * 
 * Copyright 2018 Aleksey Мamontov ukrfoil@gmail.com
 * https://github.com/AlekseyMamontov
 * 
 * b0 -clk
 * b1 -cs
 * b2 -MISO 1
 * b3 -MISO 2
 * ...
 * b7 -MISO 3
 
 */
 
#include "i2c-thermo.h"

struct Thermosensors MAX6675;

//---------------- i2c (mcp23017) emulation SPI  for max6675 ---------------//
//	  BIT(2..7) - miso, BIT(0) - clk ,BIT(2) - cs							//
//--------------------------------------------------------------------------//

int max6675_read_raw(struct Thermosensors *thermosensor){

MCP23017 *chip = thermosensor->chip;
uint8_t registr = thermosensor->chip_registr; 

uint8_t buf_MISO[1]={0};
uint8_t buf_CLK[2] = {registr,0};
static uint8_t buf_RAW[16] = {0};

	struct i2c_msg msgCLK [1] = {{chip->address,0, 2,buf_CLK}};
	struct i2c_msg msgMISO [2] ={
                             {chip->address,0, 1, buf_MISO},
                             {chip->address, I2C_M_RD, 1, buf_MISO}
								};

	struct i2c_rdwr_ioctl_data CLK = {msgCLK, 1 };
	struct i2c_rdwr_ioctl_data MISO = {msgMISO, 2 };


//  в этом цикле считываются 16 бит данных сразу с нескольких MAX6675,
//  за один проход от 1 до 6шт. пины 1-CS, 0 -CLK , 2-7  MAX6675  


if (mcp23017_open_i2c (chip) < 0) {perror("I2C erorr open MAX6675");return chip->bus_open;};



		buf_MISO[0] = registr+2; // OLATA(B) значение на защелках, выходы.

//  read registr  чтение регистра OLATA(B) и создание маски из регистра
//  для _/\_ strob CLK
		
		if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr read MAX6675");
	
			chip->registr[registr] = buf_MISO[0]&0b11111100; 
   	
   	

		for(uint8_t i=0; i< 16; i++){

			buf_CLK[1]= chip->registr[registr]|0b00000001;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0) perror("I2C erorr CLK ");

			// пишем считанные данные в массив  buf_RAW[1..16}
			
			buf_MISO[0] = registr;// GPIOA(В); 
			if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr MISO");

		    buf_RAW [i] = buf_MISO[0]; 
			

			buf_CLK[1] = chip->registr[registr]&0b11111110;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0)  perror("I2C erorr CLK");

		};

// обновить данные для сенсоров на этих портах

		chip->registr[registr-2] = buf_MISO[0];
		chip->registr[registr] = chip->registr[registr]|0b00000011;
		buf_CLK[1] = chip->registr[registr];  // CS-1 выключить микросхемы

		if ( ioctl(chip->bus_open, I2C_RDWR, &CLK ) < 0 ) perror("I2C erorr CS");
  

if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() ");return -1;}



// сохранить сырые данные в массиве, согласно - N_thermal_sensors (количеству подключенных MAX6675)
// 1 - bit2 , 2- bit3 .... 6 -bit7 
// 
	
for (uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){					
	
thermosensor->thermo_raw[t] = 0;
					
		for(int i=0; i< 16; i++){
	
		thermosensor->thermo_raw[t] = thermosensor->thermo_raw[t] | ( ((buf_RAW[i]>>2)&(0x01<<t)) >> t);
			
			if (i <16) thermosensor->thermo_raw[t] = thermosensor->thermo_raw[t] << 1;
		};
};
		
		
// Обработать данные
		
for(uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){				
		
	// Проверяем подключена ли термопара бит d2 = 1 - нет термопары (no thermocouple)  
	// Если бит d15 = 1 то MAX6675 не подключена (см схему включения).
	
	if (thermosensor->thermo_raw[t]&0b110){
		
	// 4 - нет термопары, 5 - устройства нет 
		
		thermosensor->check_thermocouple[t] = thermosensor->thermo_raw[t]&0x06;
		thermosensor->thermo_data[t] = 0;

	// откорректировать температуру согласно t_correct													
	}else{thermosensor->check_thermocouple[t] = 0;
		  thermosensor->thermo_data[t] = (thermosensor->thermo_raw[t] >> 5) + thermosensor->t_correct[t];
		  };	
};
	
return 0;
};


//--------------------------------------------------------------------------//
//	 Преобразования числа в строку  										//
//--------------------------------------------------------------------------//

void max6675_data_to_char(struct Thermosensors *thermosensor){
	
char text_copy[6]={0};
uint8_t symbol,numeral,i;
int16_t data_to_char, del; 
 
	for (int t=0; t< thermosensor->N_thermal_sensors;t++){
 
 	// сформировать строковое представление температуры 65535 .... 0
	 	 
		if(!thermosensor->check_thermocouple[t]){
		
			symbol = 0x20; del = 10000;	
			data_to_char = thermosensor->thermo_data[t];

			for(i=0;i<5;i++){
			
				if(data_to_char >= del){numeral=data_to_char/del; 
									data_to_char %=del;
									symbol = 0x30;
				}else{numeral = 0;};
			
				
						    
			thermosensor->thermo_char[t][i] = symbol + numeral;			
			if ((del /= 10) == 1) symbol = 0x30;	
			};

		}else{
		
	// Если не подключено выдает 
	// - нет MAX6675 (no MAX6675)
	// # нет термопары (no thermocouple)	
		
		thermosensor->thermo_char[t][0] = 0x20; // 
		thermosensor->thermo_char[t][1] = 0x20; // 
		thermosensor->thermo_char[t][2] = 0x20; // 
		thermosensor->thermo_char[t][3] = (thermosensor->check_thermocouple[t] == 4)? 0x23 : 0x2D;
		thermosensor->thermo_char[t][4] = 0x20; // #			
		 };
	};
};



//----------------------------------------------------------------//
// for timer handler											  //
//----------------------------------------------------------------//
void max6675_timer_cycle(){MAX6675.timer_cycle = 1;};



/////////////////////////// test ////////////////////////////////////////////////////



















/*  на удаление -->	

		 // sprintf лучше переделать в свою функцию, позже ))
	 
		number = sprintf (text_copy,"%d", thermosensor->thermo_data[t]);

			if(number){ 
					for(int i=0;i<number;i++){
					thermosensor->thermo_char[t][(5-number)+i] = text_copy[i];
						};
					};


//--------------------------------------------------------------------------//
//	 Преобразования числа в строку  										//
//--------------------------------------------------------------------------//

void max6675_data_to_char(struct Thermosensors *thermosensor){
	
char text_copy[6]={0};
uint8_t symbol,numeral;
int16_t data_to_char, del; 
 
for (int t=0; t< thermosensor->N_thermal_sensors;t++){
 
 	// сформировать строковое представление температуры
	 	 
	if(!thermosensor->check_thermocouple[t]){
		
		symbol = 0x20; numeral = 0;
		thermosensor->thermo_char[t][0] = symbol;		
		data_to_char = thermosensor->thermo_data[t];

// Первая цифра 
		
		if(data_to_char >=1000){numeral = 1; 
							    data_to_char /=1000;
							    symbol = 0x30;
			}else{numeral = 0;};
							    
		thermosensor->thermo_char[t][1] = symbol + numeral;

// Вторая цифра
							   
		if(data_to_char >=100){numeral=data_to_char/100;
							   data_to_char %= 100;
							   symbol = 0x30;
			}else{numeral = 0;};
							  
		thermosensor->thermo_char[t][2] = symbol + numeral;					  
							  
// Третья цифра							  
							  							  
		if(data_to_char >=10){numeral=data_to_char/10;
							   data_to_char %= 10;
							   symbol = 0x30;
			}else{numeral = 0;};
							  
		thermosensor->thermo_char[t][3] = symbol + numeral;	

// Четвертая цифра		
		
		thermosensor->thermo_char[t][4] = 0x30 + data_to_char;	

	// Если не подключено выдает 
	// - нет MAX6675 (no MAX6675)
	// # нет термопары (no thermocouple)
	
 	}else{ 
		thermosensor->thermo_char[t][0] = 0x20; // 
		thermosensor->thermo_char[t][1] = 0x20; // 
		thermosensor->thermo_char[t][2] = 0x20; // 
		thermosensor->thermo_char[t][3] = (thermosensor->check_thermocouple[t] == 4)? 0x23 : 0x2D;
		thermosensor->thermo_char[t][4] = 0x20; // -   			
		 };
};
 
};

*/
