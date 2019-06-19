
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 */
 
#include "mcp_23017.h"

//#define MAX_PINS 64
//#define MAX_CHIPS 8

#define pin_IN 1
#define pin_OUT 0
#define HIGH 1
#define LOW 0
#define pin_pullUP 1
#define pin_no_pullUP 0
#define pin_no_invert 0
#define pin_invert 1

/*
+--------------------------------------------------------------------+
| Иницилизация портов на Cтанке						  				 |
| соместное использование портов c разных микросхем например		 |
* в пользовательском пространстве   						         |
! MCP23017-18 напрямую 												 |
! UAPI_GPIO               											 |
| 		 															 |
+--------------------------------------------------------------------+*/

struct nFUNCTIONS {
	
	void (*set_nPIN)(uint8_t, uint8_t);	    // прямая запись значения
	void (*set_nPIN_async)(uint8_t,uint8_t);// быстрая запись значения
	
	uint8_t (*get_nPIN)(uint8_t);			// прямая чтение значения
	uint8_t (*get_nPIN_async)(uint8_t);  	// быстрое получение значения  
		
	void (*direction_nPIN)(uint8_t,uint8_t);// направление выхода	
	void (*init_nPIN)(uint8_t);				// иницилизация пина
	
};

struct nPIN_chip {
		
	void *chip;
	uint8_t id_chip;	
	struct nFUNCTIONS *pin_functions;
	};

struct nPIN {
	
	uint8_t n_chip;  //номер чипа 
	uint8_t bit;	 // n-bit 0...end for chip;

	uint8_t direction;// 1 - in, 0 -out  IODIRA

	uint8_t value;
	uint8_t default_value;

	uint8_t on;		 			 // default 1, 
	uint8_t ipol;    			 // in port, invert 0 - no, 1- yes
		
	uint8_t delay;
	uint8_t delay_default;
	uint8_t active_pin;
	//uint8_t busy_pin;
}; 


#define nGPIO_CHIP1 0b00000001
#define nGPIO_CHIP2 0b00000010
#define nGPIO_CHIP3 0b00000100
#define nGPIO_CHIP4 0b00001000
#define nGPIO_CHIP5 0b00010000
#define nGPIO_CHIP6 0b00100000
#define nGPIO_CHIP7 0b01000000
#define nGPIO_CHIP8 0b10000000
struct nGPIO{

// пины реле, датчии , сенсоры и т.д. 
 
	struct nPIN *pin;
	uint8_t n_pin;
	
// блок чипов	
	struct nPIN_chip *chip;
    uint8_t num_chip;
    uint8_t scan_chip;
    
// фунциии ngpio    
	
	void (*set)(uint8_t , uint8_t );	
	void (*set_async)(uint8_t,uint8_t);
	
	uint8_t (*get)(uint8_t);
	uint8_t (*get_async)(uint8_t);
		
	void (*direction)(uint8_t,uint8_t);		
	void(*init)(uint8_t );	  

// Поток отвечает за весь GPIO 
		
	pthread_t pthread_id;
	void (*pthread_rw_ngpio)(void *);
	
// буфео	
		
	void 	*data;         
	uint8_t command;
	uint8_t status_command;

	pthread_mutex_t lock_command;	
	
	uint8_t buffer_command[256];
	void * buffer_data[256];
_Atomic	uint8_t position_buffer;
_Atomic uint8_t num_commands;
	
	
	
};
 

extern struct nGPIO Stanok_gpio;

#define Stanok_CHIP_FUNC Stanok_gpio.chip[ Stanok_gpio.pin[pin].n_chip ].pin_functions
#define Stanok_CHIP 	 Stanok_gpio.chip[ Stanok_gpio.pin[pin].n_chip ].chip
#define Stanok_PIN 		 Stanok_gpio.pin[pin]

///////  Определеяем функции структуру GPIO станка //////////////////////////////////  

void Stanok_gpio_set_pin (uint8_t pin, uint8_t value){Stanok_CHIP_FUNC->set_nPIN(pin,value);};
void Stanok_gpio_set_async (uint8_t pin,uint8_t value){	Stanok_CHIP_FUNC->set_nPIN_async(pin, value);};
uint8_t Stanok_gpio_get_pin (uint8_t pin){ Stanok_CHIP_FUNC->get_nPIN(pin);};
uint8_t Stanok_gpio_get_async (uint8_t pin){Stanok_CHIP_FUNC->get_nPIN_async(pin);};
void Stanok_gpio_direction_pin (uint8_t pin, uint8_t value){Stanok_CHIP_FUNC->direction_nPIN(pin,value);};
void Stanok_gpio_init_pin (uint8_t pin){ Stanok_CHIP_FUNC->init_nPIN(pin);};



////////////////// Управление оборудованием //////////////////////////////////////



extern void Stanok_init_device (){
	for (uint8_t pin=0; pin < Stanok_gpio.n_pin ; pin++){Stanok_gpio.init(pin);};
};



////////  микросхемы I2C mcp 230xx (17-18) ///////  

void mcp_set_pin(uint8_t pin ,uint8_t value){
	Stanok_PIN.value = value;	
	mcp23017_registr_set_bit (Stanok_CHIP,GPIOA,Stanok_PIN.bit,Stanok_PIN.value);
};

uint8_t mcp_get_pin (uint8_t pin){
	return Stanok_PIN.value = mcp23017_registr_read_bit(Stanok_CHIP,GPIOA,Stanok_PIN.bit);
};

void mcp_direction (uint8_t pin, uint8_t value){	
	Stanok_PIN.direction = value;
	mcp23017_registr_set_bit (Stanok_CHIP,GPPUA,Stanok_PIN.bit,Stanok_PIN.direction);
};

void mcp_init(uint8_t pin){
		
	mcp23017_set_mode(Stanok_CHIP,Stanok_PIN.bit,Stanok_PIN.direction); //0-out 1-in
	
	if(Stanok_PIN.direction&1){
		
					  mcp23017_set_pullUP(Stanok_CHIP,Stanok_PIN.bit,Stanok_PIN.default_value);
					  mcp23017_set_IPOL(Stanok_CHIP,Stanok_PIN.bit,Stanok_PIN.ipol);
					  Stanok_PIN.value = mcp23017_read_pin_bit(Stanok_CHIP,Stanok_PIN.bit); 
					  return;};
						 
	Stanok_PIN.value = Stanok_PIN.default_value;					 	
	mcp23017_set_pin_bit (Stanok_CHIP,Stanok_PIN.bit,Stanok_PIN.value);
};

uint8_t mcp_get_async (uint8_t pin){
	
	MCP23017 *chip;
	chip = Stanok_CHIP;
		
	uint8_t bit = Stanok_PIN.bit;
	uint8_t status_bit = 0b00000001 << (bit&0x7);// создаем маску бита

	// example 0...7 GPIOA  8...15 GPIOB		
	uint8_t registr = GPIOA | ((bit&0x08)>>3); 
	
	// Информацию мы получаем из регистров которые лежат в структуре сhip _mcp23017
	// при обмене данных с микросхемой, запись или чтении - эти регистры обновляются
    
	//uint8_t per = chip->registr[registr];
	return (chip->registr[registr]&status_bit) >> (bit&0x7);
	
	//return Stanok_gpio.pin[pin]->value = (per&status_bit) >> (bit&0x7);
};
void mcp_set_async (uint8_t pin, uint8_t value){
	
	MCP23017 *chip;
	chip = Stanok_CHIP;
		
	uint8_t bit = Stanok_PIN.bit;
	uint8_t registr = OLATA | ((bit&0x08)>>3);
	
	uint8_t status_bit = 0b00000001 << (bit&0x7);
	
	pthread_mutex_lock (&chip->lock_registr_OLAT);	
		
	chip->registr[registr] = (value)? chip->registr[registr] | status_bit : chip->registr[registr]&(~status_bit);

	pthread_mutex_unlock (&chip->lock_registr_OLAT);	
	
};

// функции для обработки обращений

struct nFUNCTIONS mcp230xx = {
	
	.set_nPIN = mcp_set_pin,
	.get_nPIN = mcp_get_pin,
	.direction_nPIN = mcp_direction,
	.init_nPIN = mcp_init,
	.get_nPIN_async = mcp_get_async,
	.set_nPIN_async = mcp_set_async,
	};

	
////////// UAPI gpio Linux ////////////////	
	

// добавить позже	



/*****************************************************************************************
 * отдельнный поток																		 * 
 *****************************************************************************************/
#define NOP 0 
#define GET_PIN 2
#define GET_PIN_GROUP 3
#define SET_PIN 4
#define SET_PIN_GROUP 5

#define ENABLED_DEVICE 6
#define DISABLED_DEVICE 7

#define ENABLED_DEVICE_GROUP 8
#define DISABLED_DEVICE_GROUP 9

#define PIN_DIRECTION 10
#define PIN_DIRECTION_GROUP 11
#define INIT_PIN 12
#define INIT_GPIO 13
#define GET_RAW_MAX6675 14




/*****************************************************************************************
 * 	Кольцевой буфер команд и данных 																					 * 
 *****************************************************************************************/
int gpio_set_command(struct nGPIO *gpio, uint8_t command, void *data){

   uint8_t res;
    
	if(pthread_mutex_lock(&gpio->lock_command)) perror("error mutex lock buffer_command \n");
	
	 res = gpio->position_buffer + (gpio->num_commands++);
	 gpio->buffer_command[res] = command;
	 gpio->buffer_data[res] = data;

	if(pthread_mutex_unlock(&gpio->lock_command)) perror("error mutex unlock buffer_command \n");
	//printf("buffer position %d num_command %d  res %d  \n",gpio->position_buffer,gpio->num_commands,res);	
};

int gpio_get_command(struct nGPIO *gpio){

uint8_t res = 0;

  if(gpio->num_commands) { 
	
  	if(!pthread_mutex_trylock(&gpio->lock_command)){
		
		gpio->data = gpio->buffer_data[gpio->position_buffer];
		res = gpio->buffer_command[gpio->position_buffer++];
		gpio->command = res;
		gpio->num_commands--;	
	
	pthread_mutex_unlock(&gpio->lock_command);};	

  };
return res;
};


//////////////////////////////////////////////
struct nGPIO_Data{
	uint8_t pin;	
	uint8_t value;
	uint8_t *pin_group;
	uint8_t *value_group;
	uint8_t num_pin;
};
struct nGPIO_cycle_Data{
	uint8_t cs;
	uint8_t clk;
	uint16_t *pin_raw;
	uint8_t num_pin_raw;
	uint8_t  *pin_miso;
	uint8_t num_pin_miso;
	uint8_t status;
};			  
			  
			  
			  
			  
/*****************************************************************************************
 * главнный поток																		 * 
 *****************************************************************************************/

#define MAX_PINs 255
#define MAX_MISO 6
void pthread_Ngpio (void *Ngpio){
	
struct nGPIO *gpio;
gpio = (struct nGPIO *) Ngpio;

MCP23017 *mcp_chip;

struct nGPIO_Data *data;
struct nGPIO_cycle_Data *cycle_data;

uint8_t pin;
uint8_t command;


uint8_t status_write = 0; //mask chip
uint8_t status_read = 0;  //mask chip 
uint8_t status_sensors = 0; // mask_chip

uint8_t pin_sensors [MAX_PINs];
uint8_t n_pin_sensors ;

// для обработки цикла
uint8_t n_bit_cycle = 0;
uint8_t clk = 0;
uint8_t cs = 0;
uint8_t cycle_switch = 0;
uint8_t cycle_counter = 0;
uint8_t mask_miso_read = 0;
uint8_t mask_clk_cs_write = 0;
uint16_t miso_raw = 0;

 
  while(1){
	
	
	if(gpio->num_commands){
	
	if(pthread_mutex_trylock(&gpio->lock_command)) {
		
		command = 0;
		
		}else{	
			gpio->data = gpio->buffer_data[gpio->position_buffer];
			command = gpio->buffer_command[(gpio->position_buffer)++];
			(gpio->num_commands)--;						
			pthread_mutex_unlock(&gpio->lock_command);
		};
		 
	}else command = 0;

	status_read = status_sensors;	
			
	switch (command){
		
	gpio->status_command = command;	
		
		
/////// Чтение пина
	case NOP:
	break;
		
	case GET_PIN:	
	 
	 data = (struct nGPIO_Data *) gpio->data;	

	 // записать в массив пин который будем читать
	 if(++n_pin_sensors <= MAX_PINs){
			pin_sensors[n_pin_sensors-1] = data->pin;
			// прочитать данные с этого чипа		
			status_read |= gpio->chip[gpio->pin[data->pin].n_chip].id_chip;					
	 }else perror("stanok_gpio__erorr get n_pin_sensors > MAX_PINS");
	break;

//////// Чтение группы пинов
	
	case GET_PIN_GROUP:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 
	 if(!data->num_pin) break;
	 // если массив больше то срезать его )
	 if(MAX_PINs < (n_pin_sensors + (data->num_pin))) data->num_pin = (MAX_PINs-n_pin_sensors)+1;
	 	   	
	 for (int i=0; i < data->num_pin ; i++){
		status_read |= gpio->chip[gpio->pin[data->pin_group[i]].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = data->pin_group[i];
	 };	
	break;

/////// Записать значение в пин

	case SET_PIN:		
	 data = (struct nGPIO_Data *) gpio->data;	
	// выставить бит в регистре (в памяти)
_set_pin:
	 
     if(++n_pin_sensors <= MAX_PINs){
		 
	 gpio->set_async(data->pin,data->value);	
	 // дать знать, что регистр должен быть записан в микросхему     
	 status_write |= gpio->chip[gpio->pin[data->pin].n_chip].id_chip;
	 // и прочитан чтоб убедится что он выставлен
     status_read  |= gpio->chip[gpio->pin[data->pin].n_chip].id_chip;
	 pin_sensors[n_pin_sensors-1] = data->pin;
	 
	 }else perror("stanok_gpio__erorr set n_pin_sensors > MAX_PINS");          
	break;

////// Записать группу значений в пины

	case SET_PIN_GROUP:		
	 
	 data = (struct nGPIO_Data *) gpio->data;	 		

	 if(!data->num_pin) break;
	 	   	
_set_pin_group:	
	// если массив больше то срезать его )
	 if(MAX_PINs < (n_pin_sensors + (data->num_pin))) data->num_pin = (MAX_PINs-n_pin_sensors)+1;

	 for (int i=0; i < data->num_pin ; i++){
		gpio->set_async(data->pin_group[i],data->value_group[i]);	 
		status_write |= gpio->chip[gpio->pin[data->pin_group[i]].n_chip].id_chip;
		status_read  |= gpio->chip[gpio->pin[data->pin_group[i]].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = data->pin_group[i];		
	 };
	break;
	
	case ENABLED_DEVICE:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 data->value = gpio->pin[data->pin].on;
	goto _set_pin;
	
	case DISABLED_DEVICE:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 data->value = ~(gpio->pin[data->pin].on);
	goto _set_pin;

	case ENABLED_DEVICE_GROUP:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 if(!data->num_pin) break;
   	 for (int i=0; i<data->num_pin;i++){	
		data->value = gpio->pin[data->pin_group[i]].on;
	 };	 
	goto _set_pin_group;
	 	
	case DISABLED_DEVICE_GROUP:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 if(!data->num_pin) break;
   	 for (int i=0; i<data->num_pin;i++){	
		data->value = ~(gpio->pin[data->pin_group[i]].on);
	 };	 
	goto _set_pin_group;
	 
	case INIT_PIN:
	 
	 data = (struct nGPIO_Data *) gpio->data;
	 gpio->init(data->pin);	
	break;

// Иницилизация блока gpio

	case INIT_GPIO:
	 	
     for(int i=0; i< gpio->n_pin; i++){
		 
	printf ("gpio pin %d  \n",i);	 
	 gpio->init(i);	
	// Если пин на вход (1) то сохранить опрашиваемый на чтение чип 
	// в pin_sensors
	 if(gpio->pin[i].direction && gpio->pin[i].active_pin) {
		status_sensors |= gpio->chip[gpio->pin[i].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = i;
		};
	 };
	break;
	
// Запуск опроса чипа температуры

	case GET_RAW_MAX6675:

	 cycle_data = (struct nGPIO_cycle_Data *) gpio->data;

	 mask_clk_cs_write = 0;
	 mask_miso_read = 0;
	 cycle_switch = 0;
	 cycle_counter = 0;
	 clk = cycle_data->clk;
	 cs  = cycle_data->cs;
	 mask_clk_cs_write |= gpio->chip[gpio->pin[clk].n_chip].id_chip | gpio->chip[gpio->pin[cs].n_chip].id_chip;	 	 
	
	 	 
	 for (int i =0; i< cycle_data->num_pin_miso; i++){
		 
		mask_miso_read |= gpio->chip[gpio->pin[cycle_data->pin_miso[i]].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = cycle_data->pin_miso[i];
		
	 };
	 
	 gpio->set_async(cs,0);
	 gpio->set_async(clk,0);
	 cycle_switch = 1;
	 status_write |= mask_clk_cs_write ;
	 status_read |= mask_miso_read;	 
	 n_bit_cycle = (cycle_data->num_pin_raw) ;
	break;
		
};





// Обработать запись
	
	if(status_write & nGPIO_CHIP1) mcp23017_set_registrs_chip((MCP23017 *) gpio->chip[0].chip);
		
	status_write = 0;
	
// Обработать чтение 	
		
	if(status_read & nGPIO_CHIP1) mcp23017_get_registrs_chip(gpio->chip[0].chip);
	
// Cохранить value	
	
	for(int i=0; i<n_pin_sensors;i++) 
	gpio->pin[pin_sensors[i]].value = gpio->get_async(pin_sensors[i]);
   
    status_read = 0; 
     
// Восстановить значение  

  if ( command == GET_PIN || command == SET_PIN)  n_pin_sensors --;
  if ( command == GET_PIN_GROUP || command == SET_PIN_GROUP)  n_pin_sensors -= data->num_pin;	
		
  gpio->status_command = 0;
  command = 0;
  
// Обработать цикл 

  if(n_bit_cycle){
	  
	if(cycle_switch){

		cycle_switch --;
		   
		gpio->set_async(clk,0);
		status_write |=mask_clk_cs_write;
		
		miso_raw = 0;
		
		for(int i=0; i<cycle_data->num_pin_miso;i++){			   		    
		 miso_raw |= ((gpio->pin[cycle_data->pin_miso[i]].value)&1) << (i);			   
		};
		 //printf ("miso_raw %d \n",miso_raw);  
		cycle_data->pin_raw[cycle_counter++] = miso_raw;
		   
	
	  }else{
		  
		  cycle_switch ++;
		  //printf ("test 1\n");
		  gpio->set_async(clk,1);
		  status_write |=mask_clk_cs_write;		  
		  if(--n_bit_cycle) {			  
			 status_read |= mask_miso_read;			  
		  }else{ gpio->set_async(cs,1);
			    for (int i=0;i < cycle_data->num_pin_miso;i++){
					n_pin_sensors--;
					};			  			  
				cycle_data->status = 0;
			  };
		  
		  };
	  
  };  

//end while
};
			
//end potok
};

/*****************************************************************************************
 * 		   								 		 * 
 *****************************************************************************************/

extern void Stanok_enabled_device (uint8_t pin){	
 struct nGPIO_Data t;
	t.pin = pin; 
	//gpio_set_command(&Stanok_gpio,ENABLED_DEVICE,&t);
	printf("enabled pin %d\n",pin);
};

extern void Stanok_disabled_device (uint8_t pin){	
struct nGPIO_Data t;
	t.pin = pin; 
	//gpio_set_command(&Stanok_gpio,DISABLED_DEVICE,&t);
	printf("disabled pin %d\n",pin);
};




/*****************************************************************************************
 * Cтарт потока который отслеживает порты  		   								 		 * 
 *****************************************************************************************/

void pthread_start_nGPIO(struct nGPIO *gpio){
 
 //gpio->command = INIT_GPIO;
 
 int potok = pthread_create(&gpio->pthread_id,
							NULL,
							(void*) pthread_Ngpio,
							(void *) gpio);
 if (potok)printf("ERROR; return code from pthread_create() is %d\n", potok);
 		
};




/*
 *mcp23017_get_registrs_chip(gpio->chip[0].chip);			
				
 for (uint8_t n_pin=0; n_pin < gpio->n_pin; n_pin++){
	
	if(GPIN.busy_pin) continue;
		
	per_value = gpio->get_async(n_pin);	
	
	if (GPIN.direction){				// пин как вход	;
		
	 if(per_value != GPIN.value){ 		
					            
		if (!(GPIN.delay)){				// задержка для избежания дребезга 0 - без, 
										// 1-255 задержка в проходах             			
			GPIN.value = per_value;
			GPIN.delay = GPIN.delay_default;
						
		}else (GPIN.delay)--;

// если во время задержки пин стал равен - то задержку в начало (значит есть дребезг).
						  
		       }else{GPIN.delay = GPIN.delay_default;};
										
	}else{	// пин на выход (0) 
			
		if(per_value != GPIN.value){
			
			
			//if(!GPIN.busy_pin) gpio->set_async(n_pin,per_value);			
			gpio->set_async(n_pin,GPIN.value);
			
			printf ("-per_value %d  -GPIN.value %d  -n_pin %d\n",per_value,GPIN.value,gpio->get_async(n_pin));
						
			status_write |= (gpio->chip[GPIN.n_chip].id_chip);	
		};							
	};	
 };		

// здесь все данные по пинам сохраняются одновременно в зависимости от id
// переделать в цикл

	if(status_write&MCP_GPIO_SET){
	
};
	 mcp23017_set_registrs_chip((MCP23017 *) gpio->chip[0].chip);
	 printf ("-per_valu \n");
	 
	};	
 status_write = 0;   
};	
     
 * 
 * 
 */

















