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

	struct i2c_msg msgCLK [1] = {
								{chip->address,0, 2,buf_CLK}
								};
	struct i2c_msg msgMISO [2] ={
                             {chip->address,0, 1, buf_MISO},
                             {chip->address, I2C_M_RD, 1, buf_MISO}
								};

	struct i2c_rdwr_ioctl_data CLK = {msgCLK, 1 };
	struct i2c_rdwr_ioctl_data MISO = {msgMISO, 2 };


//  в этом цикле считываются 16 бит данных сразу с нескольких MAX6675,
//  за один проход от 1 до 6шт. пины 1-CS, 0 -CLK , 2-7  MAX6675  


if (mcp23017_open_i2c (chip) < 0) {perror("I2C erorr open MAX6675");return chip->bus_open;};



		buf_MISO[0] = registr;

//  read registr  чтение регистра OLATA(B) и создание маски из регистра
//  для _/\_ strob CLK
		
		if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr read MAX6675");
	
			chip->registr[registr] = buf_MISO[0]&0b11111100; 
   	
   	

		for(uint8_t i=0; i< 16; i++){

			buf_CLK[1]= chip->registr[registr]|0b00000001;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0) perror("I2C erorr CLK ");

			// пишем считанные данные в массив  buf_RAW[1..16}
			
			buf_MISO[0] = registr-2;// GPIOA(В) = OLATA(В) -2 ; 
			if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr MISO");

		    buf_RAW [i] = buf_MISO[0]; 

			buf_CLK[1] = chip->registr[registr]&0b11111110;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0)  perror("I2C erorr CLK");

		};

		buf_CLK[1] = chip->registr[registr]|0b00000011;  // CS-1 выключить микросхемы
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
uint8_t number,i;
 
for (int t=0; t< thermosensor->N_thermal_sensors;t++){
 
	// записать в строку пробелы 5 шт.
	for(i=0;i<5;i++){thermosensor->thermo_char[t][i]=0x20;};// записать в строку пробелы 5 шт.
	
	// сформировать строковое представление температуры
	 
	if(!thermosensor->check_thermocouple[t]){

		 // sprintf лучше переделать в свою функцию, позже ))
		 
		number = sprintf (text_copy,"%d", thermosensor->thermo_data[t]);

			if(number){ 
					for(int i=0;i<number;i++){
					thermosensor->thermo_char[t][(5-number)+i] = text_copy[i];
						};
					};
	// Если не подключено выдает 
	// - нет MAX6675 (no MAX6675)
	// # нет термопары (no thermocouple)
	
 	}else{ thermosensor->thermo_char[t][3] = 0x2d; // - 
		 if(thermosensor->check_thermocouple[t] == 4) thermosensor->thermo_char[t][3] = 0x23; // #			
		 };
};
 
};
//----------------------------------------------------------------//
//	 Bin to BCD 												  //			
//----------------------------------------------------------------//



//----------------------------------------------------------------//
// for timer handler											  //
//----------------------------------------------------------------//
void max6675_timer_cycle(){MAX6675.timer_cycle = 1;};




