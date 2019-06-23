/*
 * Chip MAX6675
 * 
 * Copyright 2018 Aleksey Мamontov ukrfoil@gmail.com
 * https://github.com/AlekseyMamontov
 * 
 * b6-clk
 * b7 -cs
 * b2 -MISO 1
 * b3 -MISO 2
  ...
 struct nGPIO_cycle_Data{
	uint8_t cs;
	uint8_t clk;
	uint16_t *pin_raw;
	uint8_t num_pin_raw;
	uint8_t  *pin_miso;
	uint8_t num_pin_miso;
	uint8_t status;
};			  
			  
 */
 
#include "i2c-thermo.h"

#define CHIP_MAX6675_BUSY 1

struct Thermosensors MAX6675;

//---------------- i2c (mcp23017) emulation SPI  for max6675 ---------------//
//	  BIT(0..5) - miso, BIT(6) - clk ,BIT(7) - cs							//
//--------------------------------------------------------------------------//

int max6675_read_raw(struct Thermosensors *thermosensor){

uint16_t buf[17];

uint8_t  buf_cycle[18] = 
		{thermosensor->pin_clk,
		 thermosensor->pin_cs};
		 
uint8_t status_bit = 16;

struct nGPIO_DATA max6675 = {
	.pin = 	 {.clk_cs_miso = buf_cycle},
	.value = {.raw =  buf},	
	.num_pin = (thermosensor->N_thermal_sensors) + 2,
	.data = &status_bit,
};

for(int i=0; i < thermosensor->N_thermal_sensors;i++) 
 buf_cycle[i+2] = thermosensor->pin_miso[i];

gpio_set_command (&Stanok_gpio,GET_RAW_MAX6675,&max6675);

// Дожидаемся когда пройдет цикл обмена

while(status_bit){}; /// можем не проверять есть вариант что произойдет сбой. 

// сохранить сырые данные в массиве, согласно - N_thermal_sensors (количеству подключенных MAX6675)
// 1 - bit2 , 2- bit3 .... 6 -bit7 
// 

	
	for (uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){					
	
	thermosensor->thermo_raw[t] = 0;
						
		for(int i=0; i< 16; i++){
	
		thermosensor->thermo_raw[t] |= ((buf[i]&(1 << t))>> t);	
	    if (i <16) thermosensor->thermo_raw[t] <<= 1;
	    
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
	
//char text_copy[6]={0};
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
		 
	// Сообщить что данные в этой части блоке обновились для экрана. 
	thermosensor->thermo_char_visible[t] = 1;	 
		 
	};
};



//----------------------------------------------------------------//
// for timer handler											  //
//----------------------------------------------------------------//
void max6675_timer_cycle(){MAX6675.time_cycle = 1;};



/*////////////////////////// test ////////////////////////////////////////////////////

void *max6675_reading(void *temp){
	
 struct Thermosensors *max6675;
 
 max6675=(struct Thermosensors *) temp;
	
cycle:

 max6675->temperature_raw_processing(max6675);
 max6675->temperature_data_processing(max6675);
 
 usleep(500000);
 
 goto cycle;	
	
pthread_exit(NULL);	
};
*/














