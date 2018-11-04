/*
 * 
 * Copyright 2018 Aleksey <aleksey@aleksey-P5QL-E>
 * b0 -clk
 * b1 -cs
 * b2 -MISO 1
 * b3 -MISO 2
 * ...
 * b7 -MISO 3
 * 
 * 
 * 
 */
struct MCP23017_thermosensor{
	MCP23017 *chip;
	uint16_t thermo_raw[6];
	int16_t  thermo_data[6];
	int8_t   t_correct[6];
	uint8_t  check_thermocouple[6];
	uint8_t  N_thermal_sensors;  // <= 6
	uint8_t  chip_registr;
	char     char_number[6][6];
};

	
	

//---------------- i2c (mcp23017) emulation SPI  for max6675 ---------------//
//	  BIT(2..7) - miso, BIT(0) - clk ,BIT(2) - cs							//
//--------------------------------------------------------------------------//

int Read_raw_MAX6675(struct MCP23017_thermosensor *thermosensor){

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


if (mcp23017_open_i2c (chip) < 0) return chip->bus_open;

chip->lock_chip = 1;

		buf_MISO[0] = registr;//  read mask registr  чтение маски из регистра
		if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr read MAX6675");
	
			chip->registr[registr] = buf_MISO[0]&0b11111100; // CS- 0 enabled
   
		for(uint8_t i=0; i< 16; i++){

			buf_CLK[1]= chip->registr[registr]|0b00000001;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0) perror("I2C erorr CLK ");

			buf_MISO[0] = registr-2;// GPIOA(В) = OLATA(В) -2 ; 
			if ( ioctl(chip->bus_open, I2C_RDWR, &MISO) < 0) perror("I2C erorr MISO");
		    buf_RAW [i] = buf_MISO[0]; 

			buf_CLK[1] = chip->registr[registr]&0xfe;
			if ( ioctl(chip->bus_open, I2C_RDWR, &CLK) < 0)  perror("I2C erorr CLK");

		};

		buf_CLK[1] = chip->registr[registr]|0b00000010;  // CS-1 выключить микросхемы
		if ( ioctl(chip->bus_open, I2C_RDWR, &CLK ) < 0 ) perror("I2C erorr CS");



if ( mcp23017_close_i2c (chip) < 0){perror("I2C erorr close() ");return -1;}

chip->lock_chip = 0;///!!!!!!
		

for (uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){					// сохранить сырые данные в структуру
	
thermosensor->thermo_raw[t] = 0;
					
		for(int i=0; i< 16; i++){
	
		thermosensor->thermo_raw[t] = thermosensor->thermo_raw[t] | ( ((buf_RAW[i]>>2)&(0x01<<t)) >> t);
			
			if (i <16) thermosensor->thermo_raw[t] = thermosensor->thermo_raw[t] << 1;
		};
};
		
		
for(uint8_t t=0; t < thermosensor->N_thermal_sensors; t++){				// Обработать данные
	
	thermosensor->thermo_data[t] = (thermosensor->thermo_raw[t] >> 5) + thermosensor->t_correct[t];
		if(thermosensor->thermo_data[t] < 0 ) thermosensor->thermo_data[t] = 0;
	thermosensor->check_thermocouple[t] = !(0x01 & (thermosensor->thermo_raw[t] >> 2)); //on - of 
	};
	
return 0;
};



//--------------------------------------------------------------------------//
//	 Преобразования числа в строку  										//
//--------------------------------------------------------------------------//

void max6675_data_to_char(struct MCP23017_thermosensor *thermosensor){
	
 char text_copy[6];
 uint8_t number,i;
 
 
	for (int t=0; t< thermosensor->N_thermal_sensors;t++){
 
		for(i=0;i<5;i++){
			text_copy[i]=0x30;
			thermosensor->char_number[t][i]=0x20;
			};

			number = sprintf (text_copy,"%d", thermosensor->thermo_data[t]);

				if(number){ 
							for(int i=0;i<number;i++){
								thermosensor->char_number[t][(5-number)+i] = text_copy[i];
								};
						};  
	};
 
};











/*
  fprintf(disp, "temperatura %hd  ; temperatura1 %hd \n" ,  temperatura,temperatura1);
fclose(disp);



 */
  
