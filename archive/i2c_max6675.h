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
 
 */
 
#include "i2c-thermo.h"

#define CHIP_MAX6675_BUSY 1

struct Thermosensors MAX6675;

//---------------- i2c (mcp23017) emulation SPI  for max6675 ---------------//
//	  BIT(0..5) - miso, BIT(6) - clk ,BIT(7) - cs							//
//--------------------------------------------------------------------------//

int max6675_read_raw(struct Thermosensors *thermosensor){


MCP23017 *chip = thermosensor->gpio->chip[thermosensor->gpio->pin[thermosensor->pin_clk].n_chip].chip;

uint8_t registr_GPIO = GPIOA|((0b00001000&thermosensor->gpio->pin[thermosensor->pin_clk].bit)>>3); 
uint8_t registr_OLAT = registr_GPIO + 2;
uint8_t bit_clk = 1 << (0b00000111&thermosensor->gpio->pin[thermosensor->pin_clk].bit);
uint8_t bit_cs = 1 << (0b00000111&thermosensor->gpio->pin[thermosensor->pin_cs].bit);
uint8_t bit_miso ;

uint8_t buf_MISO[1]={0};
uint8_t buf_CLK[2] = {registr_OLAT,0};
static uint8_t buf_RAW[16] = {0};

	struct i2c_msg msgCLK [1] = {{chip->address,0, 2,buf_CLK}};
	struct i2c_rdwr_ioctl_data CLK = {msgCLK, 1 };
		
	struct i2c_msg msgMISO [2] ={{chip->address,0, 1, buf_MISO},
								 {chip->address, I2C_M_RD, 1, buf_MISO}};
	struct i2c_rdwr_ioctl_data MISO = {msgMISO, 2 };

		buf_MISO[0] = registr_OLAT; // OLATA(B) значение на защелках, выходы.

//  read registr  чтение регистра OLATA(B) и создание маски из регистра
//  для _/\_ strob CLK

// Lock	 -----------------------------------------------------	
		pthread_mutex_lock (&chip->lock_registr_OLAT);			
	if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr read MAX6675");		
		chip->registr[registr_OLAT] = buf_MISO[0]&(~(bit_clk|bit_cs)); 
		pthread_mutex_unlock (&chip->lock_registr_OLAT);	
//unlock-------------------------------------------------------	
	
// Цикл считывание данных с микросхем
   	
	for(uint8_t i=0; i< 16; i++){

// Lock	 -----------------------------------------------------
			pthread_mutex_lock (&chip->lock_registr_OLAT);

			chip->registr[registr_OLAT]|= bit_clk;			
			buf_CLK[1]= chip->registr[registr_OLAT];
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0) perror("I2C erorr CLK ");
			
			pthread_mutex_unlock (&chip->lock_registr_OLAT);
//unlock-------------------------------------------------------	

						
			buf_MISO[0] = registr_GPIO;// GPIOA(В); 

// Lock	 -----------------------------------------------------		    
		    pthread_mutex_lock (&chip->lock_registr_GPIO);	
		    					
			if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr MISO");
			chip->registr[registr_GPIO] = buf_MISO[0];

			pthread_mutex_unlock (&chip->lock_registr_GPIO);
//unlock-------------------------------------------------------	
	
			// пишем считанные данные в массив  buf_RAW[1..16}
			buf_RAW [i] = buf_MISO[0];

// Lock	 -----------------------------------------------------		 
			pthread_mutex_lock (&chip->lock_registr_OLAT);

			chip->registr[registr_OLAT]&=(~bit_clk);		
			buf_CLK[1]= chip->registr[registr_OLAT];			
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0)  perror("I2C erorr CLK");

			pthread_mutex_unlock (&chip->lock_registr_OLAT);
//unlock-------------------------------------------------------	

	};
// Lock	 -----------------------------------------------------
		pthread_mutex_lock (&chip->lock_registr_OLAT);

		chip->registr[registr_OLAT] |= bit_clk|bit_cs;					
		buf_CLK[1] = chip->registr[registr_OLAT];  // CS-1 выключить микросхемы
		if ( ioctl(chip->bus_open, I2C_RDWR, &CLK ) < 0 ) perror("I2C erorr CS");

		pthread_mutex_unlock (&chip->lock_registr_OLAT);		
//unlock-------------------------------------------------------

// сохранить сырые данные в массиве, согласно - N_thermal_sensors (количеству подключенных MAX6675)
// 1 - bit2 , 2- bit3 .... 6 -bit7 
// 

	
	for (uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){					
	
	thermosensor->thermo_raw[t] = 0;
	bit_miso = 0b00000111 & thermosensor->gpio->pin[thermosensor->pin_miso[t]].bit;
					
		for(int i=0; i< 16; i++){
	
		thermosensor->thermo_raw[t] = thermosensor->thermo_raw[t] | ((buf_RAW[i]&(1 << bit_miso))>> bit_miso);	
			
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














