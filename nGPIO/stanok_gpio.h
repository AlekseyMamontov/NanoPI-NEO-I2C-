
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 Сделать;
 * отработку программного дребезга на сенсорах 
 * добавить функций GPIO
 * UAPI GPIO
 * обработку клавиш TTP229L
 
 */

#include "mcp23017_nGPIO.h"

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
	
}; 


#define nGPIO_CHIP1 0b00000001
#define nGPIO_CHIP2 0b00000010
#define nGPIO_CHIP3 0b00000100
#define nGPIO_CHIP4 0b00001000
#define nGPIO_CHIP5 0b00010000
#define nGPIO_CHIP6 0b00100000
#define nGPIO_CHIP7 0b01000000
#define nGPIO_CHIP8 0b10000000

union _pin{
uint8_t        n_pin;
uint8_t       *group;
uint8_t *clk_cs_miso;
};
union _value{
uint8_t   	    data;
uint8_t  	  *group;
uint16_t 		*raw;	
};
			  
struct nGPIO_DATA{ 	
 union _pin      pin;
 union _value  value;
 void    	   *data;
 uint8_t     num_pin;
};


struct nGPIO{

// пины реле, датчии , сенсоры и т.д. 
 
	struct nPIN *pin;
	uint8_t n_pin;
	
// блок чипов	
	struct nPIN_chip *chips;
    uint8_t num_chip;
   // uint8_t scan_chip;
    
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
	
// буфер	

		//uint8_t status_command;

		pthread_mutex_t lock_command;	
_Atomic	uint8_t position_buffer;
_Atomic uint8_t num_commands;	
		uint8_t buffer_command[256];
		struct nGPIO_DATA buffer_data[256];	
};
 

extern struct nGPIO Stanok_gpio;

#define Stanok_CHIP_FUNC Stanok_gpio.chips[ Stanok_gpio.pin[pin].n_chip ].pin_functions
#define Stanok_CHIP 	 Stanok_gpio.chips[ Stanok_gpio.pin[pin].n_chip ].chip
#define Stanok_PIN 		 Stanok_gpio.pin[pin]

///////  Определеяем функции структуру GPIO станка //////////////////////////////////  

void Stanok_gpio_set_pin (uint8_t pin, uint8_t value){Stanok_CHIP_FUNC->set_nPIN(pin,value);};
void Stanok_gpio_set_async (uint8_t pin,uint8_t value){	Stanok_CHIP_FUNC->set_nPIN_async(pin, value);};
uint8_t Stanok_gpio_get_pin (uint8_t pin){ Stanok_CHIP_FUNC->get_nPIN(pin);};
uint8_t Stanok_gpio_get_async (uint8_t pin){Stanok_CHIP_FUNC->get_nPIN_async(pin);};
void Stanok_gpio_direction_pin (uint8_t pin, uint8_t value){Stanok_CHIP_FUNC->direction_nPIN(pin,value);};
void Stanok_gpio_init_pin (uint8_t pin){ Stanok_CHIP_FUNC->init_nPIN(pin);};



////////////////// Управление оборудованием //////////////////////////////////////

uint8_t Stanok_gpio_get(uint8_t pin){return Stanok_PIN.value;};

////////  микросхемы I2C mcp 230xx (17-18) ///////  

void mcp_set_pin(uint8_t pin ,uint8_t value){
Stanok_PIN.value = value;	
mcp23017_registr_set_bit(Stanok_CHIP,
						 GPIOA,
						 Stanok_PIN.bit,
						 Stanok_PIN.value);
};

uint8_t mcp_get_pin (uint8_t pin){

return Stanok_PIN.value = 
mcp23017_registr_read_bit(Stanok_CHIP,
						  GPIOA,
						  Stanok_PIN.bit);
};

void mcp_direction (uint8_t pin, uint8_t value){	
Stanok_PIN.direction = value;
mcp23017_registr_set_bit (Stanok_CHIP,
						  IODIRA,
						  Stanok_PIN.bit,
						  Stanok_PIN.direction);
};

void mcp_init(uint8_t pin){
	
mcp23017_registr_set_bit(Stanok_CHIP,
						 IODIRA,
						 Stanok_PIN.bit,
						 Stanok_PIN.direction); //0-out 1-in

if(Stanok_PIN.direction&1){
mcp23017_registr_set_bit (Stanok_CHIP,
						  GPPUA,
						  Stanok_PIN.bit,
						  Stanok_PIN.default_value);
mcp23017_registr_set_bit (Stanok_CHIP,
						  IPOLA,
						  Stanok_PIN.bit,
						  Stanok_PIN.ipol);
Stanok_PIN.value = 
mcp23017_registr_read_bit(Stanok_CHIP,
						  GPIOA,
						  Stanok_PIN.bit); 
return;};

Stanok_PIN.value = Stanok_PIN.default_value;					 	
mcp23017_registr_set_bit (Stanok_CHIP,
						  GPIOA,
						  Stanok_PIN.bit,
						  Stanok_PIN.value);
};

uint8_t mcp_get_async (uint8_t pin){
	
	MCP23017 *chip = Stanok_CHIP;
	uint8_t bit = Stanok_PIN.bit;
	uint8_t status_bit = 0b00000001 << (bit&0x7);		
	uint8_t registr = GPIOA | ((bit&0x08)>>3); 	
	return (chip->registr[registr]&status_bit) >> (bit&0x7);
};
void mcp_set_async (uint8_t pin, uint8_t value){
//pthread_mutex_lock(&((MCP23017*)Stanok_CHIP)->lock_registr);	
					
	MCP23017 *chip = Stanok_CHIP;			
	uint8_t bit = Stanok_PIN.bit;
	uint8_t registr = OLATA | ((bit&0x08)>>3);
	uint8_t status_bit = 0b00000001 << (bit&0x7);	
	//pthread_mutex_lock(&chip->lock_registr);
	chip->registr[registr] = 
	(value)? chip->registr[registr] | status_bit :
			 chip->registr[registr]&(~status_bit);	
	//pthread_mutex_unlock (&chip->lock_registr);		
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
 * Кольцевой буфер команд и данных 													 	 *
 * Cтруктура которая передается в поток должна быть static, 							 *
 * например  static struct nGPIO_DATA device 											 *
 * иначе не успеет скопироватся в буфер и будет  Segmentation fault 					 *															 *
 *****************************************************************************************/
 
int gpio_set_command(struct nGPIO *gpio, uint8_t command, void *data){

   uint8_t res;
    
	if(pthread_mutex_lock(&gpio->lock_command)) perror("error mutex lock buffer_command \n");
	
	 res = gpio->position_buffer + (gpio->num_commands++);
	 gpio->buffer_command[res] = command;
	 memcpy(&gpio->buffer_data[res],data,sizeof(struct nGPIO_DATA));
  
	if(pthread_mutex_unlock(&gpio->lock_command)) perror("error mutex unlock buffer_command \n");
	//printf("buff-data %d = %d, data %d,  sizeof %d  \n",res,(int)&gpio->buffer_data[res],(int) data, sizeof(struct nGPIO_DATA));	
};

			  
/*****************************************************************************************
 * главный поток																		 * 
 *****************************************************************************************/

#define MAX_PINs 255
#define MAX_MISO 6

void pthread_Ngpio (void *Ngpio){
	
struct nGPIO *gpio;
gpio = (struct nGPIO *) Ngpio;

//MCP23017 *mcp_chip;

struct nGPIO_DATA *data;
struct nGPIO_DATA data_cycle;

uint8_t pin[256] = {0};
uint8_t command = 0;

uint8_t status_write   = 0;  //mask chip write
uint8_t status_read    = 0;  //mask chip read
uint8_t status_sensors = 0;  //mask_chip read

uint8_t pin_sensors [MAX_PINs] = {0};
uint8_t n_pin_sensors 		= 0;

// для обработки цикла
uint8_t n_bit_cycle			= 0;
uint8_t clk 				= 0;
uint8_t cs 					= 0;
uint8_t cycle_switch 		= 0;
uint8_t cycle_counter 		= 0;
uint8_t mask_miso_read 		= 0;
uint8_t mask_clk_cs_write	= 0;
uint16_t miso_raw 			= 0;
 
 
while(1){
	
if(gpio->num_commands){
	if(pthread_mutex_trylock(&gpio->lock_command)) {
		command = 0;
	}else{	
	
		data = &gpio->buffer_data[gpio->position_buffer];
		command = gpio->buffer_command[(gpio->position_buffer)++];
		(gpio->num_commands)--;						
		pthread_mutex_unlock(&gpio->lock_command);
	};		 
}else command = 0;

status_read = status_sensors;
		
switch(command){
	
case 0:
	break;
		
case GET_PIN:				 

	if(n_pin_sensors < MAX_PINs){
		pin_sensors[n_pin_sensors++] = data->pin.n_pin;
		// прочитать данные с этого чипа		
		status_read |= gpio->chips[gpio->pin[data->pin.n_pin].n_chip].id_chip;								
	 }else perror("stanok_gpio GET_PIN n_pin_sensors > MAX_PINS");
	 
	if(data->data != NULL) *(uint8_t*)data->data = 0;	
	break;
	
case GET_PIN_GROUP:		  

	if(!data->num_pin) break;
	if((n_pin_sensors + data->num_pin) < MAX_PINs){	 	   	
		for (int i=0; i < data->num_pin ; i++){
		status_read |= gpio->chips[gpio->pin[data->pin.group[i]].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = data->pin.group[i];
		};
	}else perror("stanok_gpio GET_PIN_GROUP n_pin_sensors > MAX_PINS");			
	if(data->data != NULL) *(uint8_t*)data->data = 0;
	break;
		
case SET_PIN:
		
_set_pin:	 
     if(n_pin_sensors < MAX_PINs){		 
	 gpio->set_async(data->pin.n_pin,data->value.data);	
	 // дать знать, что данные должны быть записаны в микросхему     
	 status_write |= gpio->chips[gpio->pin[data->pin.n_pin].n_chip].id_chip;
	 // и потом прочитаны чтоб убедится что он выставлен
     status_read  |= gpio->chips[gpio->pin[data->pin.n_pin].n_chip].id_chip;
	 pin_sensors[n_pin_sensors++] = data->pin.n_pin;
	 	 
	 }else perror("stanok_gpio__erorr SET_PIN n_pin_sensors > MAX_PINS"); 
	         
	if(data->data != NULL) *(uint8_t*)data->data = 0;
	break;

case SET_PIN_GROUP:		

 	 if(!data->num_pin) break;	
_set_pin_group:	 	  	   		
	 if((n_pin_sensors + data->num_pin) < MAX_PINs){
		for (int i=0; i < data->num_pin ; i++){
			gpio->set_async(data->pin.group[i],data->value.group[i]);	 
			status_write |= gpio->chips[gpio->pin[data->pin.group[i]].n_chip].id_chip;
			status_read  |= gpio->chips[gpio->pin[data->pin.group[i]].n_chip].id_chip;
			pin_sensors[n_pin_sensors++] = data->pin.group[i];		
		};
	}else perror("stanok_gpio__erorr SET_PIN_GROUP n_pin_sensors > MAX_PINS");

	if(data->data != NULL) *(uint8_t*)data->data = 0;
	break;
	
case ENABLED_DEVICE: 	

	data->value.data = gpio->pin[data->pin.n_pin].on; 	 
	goto _set_pin;
	
case DISABLED_DEVICE:	 

	data->value.data = !(gpio->pin[data->pin.n_pin].on);
	goto _set_pin;

case ENABLED_DEVICE_GROUP:	 
	 
	if(!data->num_pin) break;
	
	data->value.group = pin;
	
	  for (int i=0; i<data->num_pin;i++)
	  data->value.group[i] = gpio->pin[data->pin.group[i]].on;
	  						 	
	goto _set_pin_group;
	 	
case DISABLED_DEVICE_GROUP:	 

	if(!data->num_pin) break;
	
	data->value.group = pin;
	
   	 for (int i=0; i<data->num_pin;i++)	
		data->value.group[i] = !(gpio->pin[data->pin.group[i]].on);
	goto _set_pin_group;
	 
case INIT_PIN:

	gpio->init(data->pin.n_pin);		 
	if(data->data != NULL) *(uint8_t*)data->data = 0;
	break;

case INIT_GPIO:
	 	
     for(int i=0; i< gpio->n_pin; i++){	
	 gpio->init(i);	
	// Если пин на вход (1) то сохранить опрашиваемый на чтение чип 
	// в pin_sensors

	 if(gpio->pin[i].direction && gpio->pin[i].active_pin) {
		status_sensors |= gpio->chips[gpio->pin[i].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = i;
		};
	 };
	 if(data->data != NULL) *(uint8_t*)data->data = 0;
	break;
	
// Запуск опроса чипа температуры

case GET_RAW_MAX6675:

     // cохраняем данные для цикла во внутреннию переменные цикла
      
     memcpy(&data_cycle,data,sizeof(struct nGPIO_DATA));          
	 mask_clk_cs_write 	= 0;
	 mask_miso_read 	= 0;
	 cycle_switch 		= 1;
	 cycle_counter 		= 0;
	 // количество бит
	 n_bit_cycle = *(uint8_t*)data_cycle.data;
	 // первые два пина clk cs, потом пины данных.
	 clk = data_cycle.pin.clk_cs_miso[0];
	 cs  = data_cycle.pin.clk_cs_miso[1];	 
	 // маска цикла записи 
	 mask_clk_cs_write |= gpio->chips[gpio->pin[clk].n_chip].id_chip | gpio->chips[gpio->pin[cs].n_chip].id_chip;	 	 	 	 
	 for (int i = 2; i< data_cycle.num_pin; i++){
	 //маска для чипов с каких читать пины которые miso	 
		mask_miso_read |= gpio->chips[gpio->pin[data_cycle.pin.clk_cs_miso[i]].n_chip].id_chip;
		pin_sensors[n_pin_sensors++] = data_cycle.pin.clk_cs_miso[i];		
	 };
	 gpio->set_async(cs,0);
	 gpio->set_async(clk,0);
	 
	 status_write |= mask_clk_cs_write ;
	 status_read |= mask_miso_read;	 
		 
	break;

	default: break;
};

// Обработать запись
	
if(status_write & nGPIO_CHIP1) mcp23017_set_registrs_chip((MCP23017 *) gpio->chips[0].chip);

status_write = 0;
	
// Обработать чтение и сохранить value

if(status_read & nGPIO_CHIP1) mcp23017_get_registrs_chip(gpio->chips[0].chip);	

for(int i=0; i<n_pin_sensors;i++) 
	gpio->pin[pin_sensors[i]].value = gpio->get_async(pin_sensors[i]);
		  
status_read = 0; 
     
// Восстановить значение  

if ( command == GET_PIN || command == SET_PIN)  n_pin_sensors --;
if ( command == GET_PIN_GROUP || command == SET_PIN_GROUP)  n_pin_sensors -= data->num_pin;	
		
command = 0;
  
// Отбработать цикл 
 
if(n_bit_cycle){
	  
	if(cycle_switch){
		
		cycle_switch --;		   
		gpio->set_async(clk,0);
		status_write |=mask_clk_cs_write;		
		miso_raw = 0;

		for(int i=2; i < data_cycle.num_pin;i++)			   		    
		miso_raw |= ((gpio->pin[data_cycle.pin.clk_cs_miso[i]].value)&1) << (i-2);			   
		
		data_cycle.value.raw[cycle_counter++] = miso_raw;

       //  printf("raw-> %d, counter-> %d \n", miso_raw,cycle_counter);
         
         
	}else{
				  
		cycle_switch ++;		
		gpio->set_async(clk,1);	
		status_write |=mask_clk_cs_write;		  

		if(--n_bit_cycle) {			  
			status_read |= mask_miso_read;			  
		}else{ 
			gpio->set_async(cs,1);
			for (int i=2;i < data_cycle.num_pin;i++) n_pin_sensors--;			  			  
		   if(data_cycle.data != NULL )*(uint8_t*)data_cycle.data = 0;
		};		  
	};	  
};  

//end while
};
			
//end potok
};

/*****************************************************************************************
 * включение устройств с ожиданием выполнения команды 		   							 * 
 *****************************************************************************************/

extern void Stanok_enabled_device (uint8_t pin){	
   struct nGPIO_DATA t;
	static __thread uint8_t status = 1; 
	t.pin.n_pin = pin;
	t.data = &status; 
	if (t.data == NULL) printf("NULL \n");
	gpio_set_command(&Stanok_gpio,ENABLED_DEVICE,&t);
	while(status){};
	printf("enabled pin %d\n",pin);
};

extern void Stanok_disabled_device (uint8_t pin){	
	struct nGPIO_DATA t;
	static __thread uint8_t status = 1; 
	t.pin.n_pin = pin;
	t.data = &status;
	if (t.data == NULL) printf("NULL \n");	
	gpio_set_command(&Stanok_gpio,DISABLED_DEVICE,&t);
	while(status){};
	printf("disabled pin %d\n",pin);
};




/*****************************************************************************************
 * Cтарт потока который отслеживает порты  		   								 		 * 
 *****************************************************************************************/

void pthread_start_nGPIO(struct nGPIO *gpio){
 
 int potok = pthread_create(&gpio->pthread_id,
							NULL,
							(void*) pthread_Ngpio,
							(void *) gpio);
 if (potok)printf("ERROR; return code from pthread_create() is %d\n", potok);
 printf("enabled gpio \n");
 
};




/*
 
	 if(per_value != GPIN.value){ 		
					            
		if (!(GPIN.delay)){				// задержка для избежания дребезга 0 - без, 
										// 1-255 задержка в проходах             			
			GPIN.value = per_value;
			GPIN.delay = GPIN.delay_default;
						
		}else (GPIN.delay)--;

// если во время задержки пин стал равен - то задержку в начало (значит есть дребезг).
						  
		       }else{GPIN.delay = GPIN.delay_default;};
										
	}else{	// пин на выход (0) 
			

 * 
 */

















