
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 */
 
#include "mcp_23017.h"
#define MCP_GPIO_SET 0b00000001
//#define MAX_PINS 64
//#define MAX_CHIPS 8



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
}; 



struct nGPIO{

	struct nPIN *pin;
	uint8_t n_pin;
	struct nPIN_chip *chip;
    uint8_t n_chip;
	
	void (*set)(uint8_t , uint8_t );	
	void (*set_async)(uint8_t,uint8_t);
	
	uint8_t (*get)(uint8_t);
	uint8_t (*get_async)(uint8_t);
		
	void (*direction)(uint8_t,uint8_t);		
	void(*init)(uint8_t );	  
	
	struct nPIN *((*info_pin)(uint8_t));
	
	pthread_t pthread_id;
	void (*pthread_rw_ngpio)(void *);
	

};


extern struct nGPIO Stanok_gpio;

#define Stanok_CHIP_FUNC Stanok_gpio.chip[ Stanok_gpio.pin[pin].n_chip ].pin_functions
#define Stanok_CHIP 	 Stanok_gpio.chip[ Stanok_gpio.pin[pin].n_chip ].chip
#define Stanok_PIN 		 Stanok_gpio.pin[pin]

///////  Определеяем структуру GPIO станка //////////////////////////////////  

void Stanok_gpio_set_pin (uint8_t pin, uint8_t value){
	Stanok_CHIP_FUNC->set_nPIN(pin,value);
};
void Stanok_gpio_set_async (uint8_t pin,uint8_t value){	
	Stanok_CHIP_FUNC->set_nPIN_async(pin, value);
};
uint8_t Stanok_gpio_get_pin (uint8_t pin){ 
	Stanok_CHIP_FUNC->get_nPIN(pin);
};
uint8_t Stanok_gpio_get_async (uint8_t pin){
	Stanok_CHIP_FUNC->get_nPIN_async(pin);
};
void Stanok_gpio_direction_pin (uint8_t pin, uint8_t value){
	Stanok_CHIP_FUNC->direction_nPIN(pin,value);
};
void Stanok_gpio_init_pin (uint8_t pin){ 
	Stanok_CHIP_FUNC->init_nPIN(pin);
};
//struct nPIN *Stanok_gpio_info_pin(uint8_t pin){return Stanok_PIN;};


////////////////// Управление оборудованием //////////////////////////////////////

extern void Stanok_enabled_device (uint8_t pin){	
	return Stanok_gpio.set(pin, Stanok_PIN.on);
};

extern void Stanok_disabled_device (uint8_t pin){	
	return Stanok_gpio.set(pin,!(Stanok_PIN.on));
};

extern void Stanok_init_device (){
	for (uint8_t pin=0; pin < Stanok_gpio.n_pin ; pin++){Stanok_gpio.init(pin);};
};



////////  микросхемы I2C mcp 230xx (17-18) ///////  

void mcp_set_pin(uint8_t pin ,uint8_t value){
Stanok_PIN.value = value;	
mcp23017_registr_set_bit (Stanok_CHIP,
			  GPIOA,
			  Stanok_PIN.bit,
			  Stanok_PIN.value);
};

uint8_t mcp_get_pin (uint8_t pin){
return Stanok_PIN.value = mcp23017_registr_read_bit(Stanok_CHIP,GPIOA,Stanok_PIN.bit);
};

void mcp_direction (uint8_t pin, uint8_t value){	
Stanok_PIN.direction = value;
mcp23017_registr_set_bit (Stanok_CHIP,
			  GPPUA,
			  Stanok_PIN.bit,
			  Stanok_PIN.direction);
};

void mcp_init(uint8_t pin){
		
mcp23017_set_mode(Stanok_CHIP,
		  Stanok_PIN.bit,
		  Stanok_PIN.direction); //0-out 1-in

if(Stanok_PIN.direction&1){		
	mcp23017_set_pullUP(Stanok_CHIP,
			    Stanok_PIN.bit,
			    Stanok_PIN.default_value);
	mcp23017_set_IPOL(Stanok_CHIP,
			  Stanok_PIN.bit,
			  Stanok_PIN.ipol);
			  Stanok_PIN.value = mcp23017_read_pin_bit(Stanok_CHIP,
								   Stanok_PIN.bit); 
return;};
						 
Stanok_PIN.value = Stanok_PIN.default_value;					 	
mcp23017_set_pin_bit (Stanok_CHIP,
		      Stanok_PIN.bit,
		      Stanok_PIN.value);
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
	
	uint8_t status_bit = (value)? 0b00000001 << (bit&0x7): ~(0b00000001 << (bit&0x7));
	
	pthread_mutex_lock (&chip->lock_registr_OLAT);	
	chip->registr[registr] = (value)?chip->registr[registr]|status_bit:chip->registr[registr]&status_bit;
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


void pthread_Ngpio (void *Ngpio){
	

	struct nGPIO *gpio;
	gpio  = (struct nGPIO *) Ngpio;
	uint8_t per_value;
	uint8_t status_write = 0;
 
    // чип
    
    MCP23017 *mcp_chip;


		
  while(1){
				
	mcp23017_get_registrs_chip(gpio->chip[0].chip);			
				
	for (uint8_t n_pin=0; n_pin < gpio->n_pin; n_pin++){	

		per_value = gpio->get_async(n_pin);	

			if (gpio->pin[n_pin].direction){
// вход	1														
				if(per_value != gpio->pin[n_pin].value){ 

// задержка для избежания дребезга 0 - без, 1-255 задержка в проходах
                         
					if (!(gpio->pin[n_pin].delay)){					
						gpio->pin[n_pin].value = per_value;
						gpio->pin[n_pin].delay = gpio->pin[n_pin].delay_default;
					}else (gpio->pin[n_pin].delay)--;

// если во время задержки пин стал равен - то задержку в начало (значит есть дребезг).
						  
				}else{gpio->pin[n_pin].delay = gpio->pin[n_pin].delay_default;};
										
			}else{	
// выход 0   
					if(per_value != gpio->pin[n_pin].value){
			  
						gpio->set_async(n_pin,gpio->pin[n_pin].value);
						status_write |= (gpio->chip[gpio->pin[n_pin].n_chip].id_chip);	
					};							
		};	
	 };		

// здесь все данные по пинам сохраняются одновременно в зависимости от id
// переделать в цикл
	if(status_write&MCP_GPIO_SET) mcp23017_set_registrs_chip((MCP23017 *) gpio->chip[0].chip);
	
	
  status_write = 0;   
  };	
  
  
  
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
		
};






















