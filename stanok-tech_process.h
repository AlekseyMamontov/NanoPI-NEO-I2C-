
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com * 
 */

// Таймеры Posix , при компиляции gcc ...... -lrt

#include <signal.h>
#include <time.h>                                                                                   

// Количетсво включаемых устройств
 
#define MAX_tech_pin 8

// Выполняемое действие в переходе (ввиде маски)

#define TECH_PROCESS_PAUSE 				0b00000001
#define TECH_PROCESS_PAUSE_TIMER_INIT 	0b00000010
#define TECH_PROCESS_SENSORS_WAIT		0b00000100
#define TECH_PROCESS_DEVICE_ON_OFF		0b00001000
#define TECH_PROCESS_WAIT_DEPENDCY		0b00010000

// Устройства - для вкл./ выкл пин устройства и значение на нем
// Сенсоры - пин сенсора и его ожидаемое значение, если соблюденно 
// 			 то в зависимости выставить маску или тд.

struct Tech_pins{
	
	uint8_t      pin[MAX_tech_pin]; // пины устройств
	uint8_t    value[MAX_tech_pin];	// значение пинов
	uint8_t dependcy[MAX_tech_pin]; // номер зависимости 
	uint8_t 				 N_pin; // 	
};

// Зависимости -> для выполнения действия соблюсти данные правила

struct Tech_dependency{	
		
	uint8_t   pin[MAX_tech_pin]; // на данных пинах
	uint8_t value[MAX_tech_pin]; // проверить данное значение
	uint8_t 			  N_pin; // длина зависимости
	
	uint8_t get_mask_Transition; // проверить маску 
	uint8_t set_mask_transition; // выставить маску
	
	void    			  *data;	
};

/*****************************************************************************************
 *							Технологический переход	   								 	 * 
 *****************************************************************************************/
 
struct Tech_transition{

 uint8_t 				 transition_type; // Тип перехода (константа) 
 unsigned char 	 *transition_information; // Описание перехода, для вывода инфы на экран      
 
 struct Tech_pins		check_sensors; // Ожидать события на данных пинах, чтоб войти (выйти) в(из) переход
 struct Tech_pins 		device_on_off; // Включаемое и выключаемое оборудование 
 struct timespec	 transition_delay; // Время для паузы	_nanosleep, timer
 
 struct Tech_dependency *dependency; // Цепочка зависимостей 
 uint8_t	  		   N_dependency; // длина цепочки

 uint8_t 	     move_to_next_transition; // Маска событий которые долны быть выполнены в переходе
 uint8_t			   temporary_counter; // временный счетчик
 
 struct Tech_transition *next_transition; // Ccылка на следующей переход
 struct Tech_transition *alternative_next_transition; // Переход в случаи не возможночсти выполнения условия
};

/*****************************************************************************************
 *							Технологоческая операция		   					 		 * 
 *****************************************************************************************/

struct Tech_operation{

 // блок термостат
 struct Thermostat_Blocks  *thermoblock;
 uint16_t 			    *temperatura_on;
 uint16_t 	  		   *temperatura_off;
 uint8_t	 	 	 			 N_temp;
 
 // блок счетчиков
 struct ProductCounter **counters; // указатель на массив указателей счетчиков
 uint32_t			*counter_raw; // начальные значения на счетчиков
 uint32_t 		  *counter_event; // насколько за раз увеличивается счетчик
 uint8_t  			   N_counter; // количество счетчиков

 // разные данные для операции
 void *attrib;

struct Tech_transition *first_transition;
struct Tech_transition  *exit_transition;
};


/*****************************************************************************************
 * 			Основной технологический процесс		   		 							 * 
 *****************************************************************************************/

struct Tech_process{
				
	struct nGPIO 	 *gpio;
	struct nGPIO_DATA data;	

	// Операции
	
	struct Tech_operation *operation;
	uint8_t 		  	 N_operation;
	uint8_t    		current_number_operation;
	struct Tech_operation *current_operation;
	

	// Переходы
	struct Tech_transition *current_transition;
	uint8_t		       move_to_next_transition;
	uint8_t            temporary_counter;
				
	// Буфер команд			
	pthread_mutex_t 	lock_command;	
_Atomic	uint8_t 	 position_buffer;
_Atomic uint8_t 		num_commands;	
	uint8_t  	 buffer_command[256];
	void   	   	   *buffer_data[256];	
	uint8_t 		status_work_flow;

	// TIMER		
	timer_t 		  	  timer_tech_process; // таймер
	struct itimerspec set_pause_tech_process; // временной интервал таймера
		
	// SIGNAL
	sigset_t 		  sig_mask_tech_process;  // маска_сигналов
	struct sigaction  	   act_tech_process;  // настроить сигнал
	struct sigevent   		   signal_timer;  // то что передает сигнал в функцию;
						
	// Запустить поток, обслуживающий техпроцесс;
	pthread_t 				  pthread_id;
	void (*pthread_Tech_process)(void *);	
		
};


/*****************************************************************************************
 *  Инициализация при первом запуске переход.											 *
 *  init_transition - маска должна быть определена при иницилизации						 *
 *  init_transition_check_parameter - построить исходя из данных в переходе	  		     * 
 *****************************************************************************************/

void init_transition (struct Tech_process *tech_process){
tech_process->move_to_next_transition = tech_process->current_transition->move_to_next_transition;
}	
/*--------------------------------------------------------------------------------------*/

void init_transition_check_parameter(struct Tech_process *tech_process){	
struct Tech_transition *transition = tech_process->current_transition;
uint8_t status_transition = 0;

// Включить и выключить устройства		
if(transition->device_on_off.N_pin) 
	status_transition |= TECH_PROCESS_DEVICE_ON_OFF;
			
// Установить таймер задержки или clock_nanosleep 		
if(transition->transition_delay.tv_sec || transition->transition_delay.tv_nsec)
	status_transition |= TECH_PROCESS_PAUSE;	
	
// Установить что надо дождатся какого то пина.	
if(transition->check_sensors.N_pin) 
	status_transition |= TECH_PROCESS_SENSORS_WAIT; 	
	
tech_process->move_to_next_transition = status_transition | transition->move_to_next_transition;
	
};


/****************************************************************************************
 * Паузы																				*
 * tech_pause_nanosleep 		-на основе nanosleep									*
 * tech_pause_clock_nanosleep	-типа более точная согласно Linux API 					*		
 * tech_pause_timer 			-на основе timer_create,								*
 ****************************************************************************************/
 
void tech_pause_nanosleep(struct Tech_process *tech_process){
 
 if(nanosleep(&tech_process->current_transition->transition_delay,NULL)) perror ("Erorr nanosleep tech_pause_request()") ;
 
  tech_process->move_to_next_transition &= ~TECH_PROCESS_PAUSE;
};
/*-------------------------------------------------------------------------------------*/
 
void tech_pause_clock_nanosleep(struct Tech_process *tech_process){
	
	static struct timespec request;
	int erorr ;
	
	if(clock_gettime(CLOCK_REALTIME,&request)) perror ("Erorr clock_gettime tech_pause_request()");
	
	request.tv_sec +=  tech_process->current_transition->transition_delay.tv_sec;
	request.tv_nsec += tech_process->current_transition->transition_delay.tv_nsec;
	
	if (request.tv_nsec >= 1000000000) {request.tv_nsec = request.tv_nsec-1000000000;
									  request.tv_sec ++;};
	erorr = clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&request,NULL);

	
	if(erorr){ 
	printf ("erorr  %d \n", erorr);
	perror ("Erorr clock_nanosleep tech_pause_request()"); };
	
	tech_process->move_to_next_transition &= ~TECH_PROCESS_PAUSE;
	
};
/*------------------------------------------------------------------------------------*/

void tech_pause_timer(struct Tech_process *tech_process){
		
	tech_process->set_pause_tech_process.it_interval.tv_sec = 0;
	tech_process->set_pause_tech_process.it_interval.tv_nsec = 0;
	tech_process->set_pause_tech_process.it_value.tv_sec =  tech_process->current_transition->transition_delay.tv_sec;
	tech_process->set_pause_tech_process.it_value.tv_nsec = tech_process->current_transition->transition_delay.tv_nsec;
	 
	tech_process->move_to_next_transition |= TECH_PROCESS_PAUSE; 
	if(timer_settime( tech_process->timer_tech_process,0,&tech_process->set_pause_tech_process,NULL)
	   ) perror("timer Tech_transition  no start");
};

/* Обработка сигнала от таймера для потока*/

 static void handler_timer_tech_process(int sig, siginfo_t *info, void *uc){	 
	uint8_t *move_to_next_transition = (uint8_t*) info->si_value.sival_ptr ;	
	*move_to_next_transition &= ~TECH_PROCESS_PAUSE;
}; 

/* Иницилизация таймера и назначенмя сигнала */

void init_timer_and_signal_tech_process(struct Tech_process *tech_process){

 // определяем тип доставки, сигнал должен быть определен во время иницилизации
 tech_process->signal_timer.sigev_notify = SIGEV_SIGNAL;
 tech_process->signal_timer.sigev_value.sival_ptr = (void *)&tech_process->move_to_next_transition;

 //создаем маску сигнала 
 sigemptyset(&tech_process->sig_mask_tech_process); 
 sigaddset(&tech_process->sig_mask_tech_process,tech_process->signal_timer.sigev_signo);

 // определяем возрат siginfo_t,функцию обработки и маску временной блокировки сигнала
 tech_process->act_tech_process.sa_flags = SA_SIGINFO;
 tech_process->act_tech_process.sa_sigaction = handler_timer_tech_process;
 tech_process->act_tech_process.sa_mask = tech_process->sig_mask_tech_process;
 
 
 // иницилизация структуры сигнала
 sigaction(tech_process->signal_timer.sigev_signo, // номер сигнал
		   &tech_process->act_tech_process,		   // структура сигнала
		   NULL);
		   
 // создаем таймер 
 timer_create(CLOCK_REALTIME,
			  &tech_process->signal_timer,		 // передается в сигнал
			  &tech_process->timer_tech_process);// вернуть id таймера в эту переменную.
			  
 // разблокируем сигнал для данного потока			  			  
 pthread_sigmask(SIG_UNBLOCK,&tech_process->sig_mask_tech_process,NULL); 	
};




/****************************************************************************************
 * Простые Переходы																		*
 *   																					*
 * Device_on_off_transition-включение или отключение устройств							*
 * Pause_transition		   -пауза на основе nanosleep, увеличивает время отклика в меню	*
 * Pause_timer_transition  -простая пауза на основе таймера, уменьшает время отклика	*
 * Check_sensors_transition-дождатся следующих значений от датчиков	и перейти			*
 ****************************************************************************************/
 /*
 Мысли )) 
Возможно рассмотреть проверку счетчика, хотя он пока сам включает их
Возможно ветление переходов от результатов счетчика, сенсора и температуры.
Проверку на сбой отличие переходов от количество продукции  

*/

/* Простой переход вкл/выкл устройств */

#define DEVICE_ON_OFF_TRANSITION 1

void Device_on_off_transition(struct Tech_process *tech_process){
 struct Tech_transition *transition = tech_process->current_transition;
 struct nGPIO_DATA *data = &tech_process->data;
 struct nGPIO  	   *gpio = tech_process->gpio;

 static uint8_t   value_group[MAX_tech_pin]={0};
 static uint8_t   status = 1;
 
  if (transition->device_on_off.N_pin){
		for (uint8_t i=0 ; i< transition->device_on_off.N_pin;i++){
			
			value_group[i] = transition->device_on_off.value[i]? 
				  gpio->pin[transition->device_on_off.pin[i]].on: 
				!(gpio->pin[transition->device_on_off.pin[i]].on);
				
//printf("device pin %d value %d \n",transition->device_on_off.pin[i],value_group[i]);
			};		
	data->pin.group = transition->device_on_off.pin;
	data->num_pin = transition->device_on_off.N_pin;
	data->value.group = value_group;
	data->data = &status;  
	gpio_set_command(&Stanok_gpio,SET_PIN_GROUP,data);
	
   }else status = 0;
 
 while(status){}; // не одназначно, может не стоит проверять
  
 tech_process->move_to_next_transition &= ~TECH_PROCESS_DEVICE_ON_OFF;
};


/* Простая пауза */

#define PAUSE_TRANSITION 2

void Pause_transition(struct Tech_process *tech_process){
 struct Tech_transition *transition = tech_process->current_transition;
 if(transition->transition_delay.tv_sec || transition->transition_delay.tv_nsec)
	tech_pause_clock_nanosleep(tech_process);
 tech_process->move_to_next_transition &= ~TECH_PROCESS_PAUSE;
};


/* Простая пауза на основе таймера */

#define PAUSE_TIMER_TRANSITION 3

void Pause_timer_transition(struct Tech_process *tech_process){

  struct Tech_transition *transition = tech_process->current_transition;
  // выйти если иницилизация была
  if(!(tech_process->move_to_next_transition&TECH_PROCESS_PAUSE_TIMER_INIT)) return;   
  if(transition->transition_delay.tv_sec || transition->transition_delay.tv_nsec)
	 tech_pause_timer(tech_process);
  
  tech_process->move_to_next_transition &= ~(TECH_PROCESS_PAUSE_TIMER_INIT);	 
};

/* Проверить датчики */

#define CHECK_SENSORS_TRANSITION 4

void Check_sensors_transition(struct Tech_process *tech_process){
 
 struct Tech_transition *transition = tech_process->current_transition;
 struct nGPIO *gpio = tech_process->gpio;
 uint8_t error = 0;
	
	for (int i=0;i< transition->check_sensors.N_pin ;i++){	
	   if(gpio->pin[(transition->check_sensors.pin[i])].value != 
		  transition->check_sensors.value[i] ) error = 1;		
	};
		
 if(!error) tech_process->move_to_next_transition &= ~TECH_PROCESS_SENSORS_WAIT;			

};

/*Дождатся рабочих температур для продолжения работы */

#define WAIT_TEMP_FOR_COUNTINUE 5

void Wait_temperature_for_countinue(struct Tech_process *tech_process){
	
	uint8_t status =			  tech_process->move_to_next_transition;
	struct Tech_operation *operation =	tech_process->current_operation;
	struct Thermostat *thermostat=  *operation->thermoblock->thermostat;
 	
 	if(!(status&TECH_PROCESS_WAIT_DEPENDCY)) return;
 	
	status &= ~TECH_PROCESS_WAIT_DEPENDCY;
	 
	for(int i = 0; i< operation->N_temp;i++){
		
		if (*(thermostat+i)->temperatura_data < *(operation->temperatura_on + i)){
		status |=TECH_PROCESS_WAIT_DEPENDCY;
		break;}	
	};
    
    tech_process->move_to_next_transition = status;
    printf("6675 temp  %d  on %d  res %d \n",*(thermostat)->temperatura_data,*(operation->temperatura_on),status);

};


/****************************************************************************************
 * Объединенные простые переходы														*
 ****************************************************************************************/

/*Включить и выключить устройства, дождатся события от датчиков*/
#define DEVICE_AND_SENSORS_TRANSITION 6
void Device_on_off_Check_sensors(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;
  if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);	
  if(status&TECH_PROCESS_SENSORS_WAIT) Check_sensors_transition(tech_process);		
};

/*Сначала дождаться показаний от сенсоров, а потом включить устройства */
#define SENSORS_AND_DEVICE_TRANSITION 7
void Check_sensors_Device_on_off(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;
  if(status&TECH_PROCESS_SENSORS_WAIT){ Check_sensors_transition(tech_process);
  }else if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);	  			
};

/*Включить или выключить устройства и запустить паузу на nanosleep */
#define DEVICE_AND_PAUSE_TRANSITION 8
void Device_on_off_Pause(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;	
  if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);
  if(status&TECH_PROCESS_PAUSE)Pause_transition(tech_process);		
};

/*Включить или выключить устройства и запустить паузу на основе timer*/
#define DEVICE_AND_PAUSE_TIMER TRANSITION 9
void Device_on_off_Pause_timer(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;	
  if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);	  
  if(status&TECH_PROCESS_PAUSE_TIMER_INIT) Pause_timer_transition(tech_process);	  			
};
/*Запустить паузу на nanosleep и включить или выключить устройства и  */
#define PAUSE_AND_DEVICE_TRANSITION 10
void Pause_Device_on_off(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;	
  if(status&TECH_PROCESS_PAUSE)Pause_transition(tech_process);		
  if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);  
};

/*Запустить паузу на основе timer и по окончания паузы включить устройства*/
#define PAUSE_TIMER_AND_DEVICE_TRANSITION 11
void Pause_timer_Device_on_off(struct Tech_process *tech_process){
	
  uint8_t status = tech_process->move_to_next_transition;	  	  
  if(status&TECH_PROCESS_PAUSE_TIMER_INIT) Pause_timer_transition(tech_process);  
  if(!(status&TECH_PROCESS_PAUSE)) 
     if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);  	  			
};


/*Проверить сенсоры,выставить устройства по умолчанию и дождатся рабочих температур*/
#define INIT_SENSOR_DEVICE_TEMPWORK_PAUSE 12

void Init_Sensor_Device_TempWork_Pause(struct Tech_process *tech_process){

  uint8_t status = tech_process->move_to_next_transition; 
  if(status&TECH_PROCESS_SENSORS_WAIT)  Check_sensors_transition(tech_process);	  
  if(status&TECH_PROCESS_DEVICE_ON_OFF) Device_on_off_transition(tech_process);	
  if(status&TECH_PROCESS_WAIT_DEPENDCY) Wait_temperature_for_countinue(tech_process);
  if(status&TECH_PROCESS_PAUSE_TIMER_INIT) Pause_timer_transition(tech_process);	  			
};

/* Программный анти-дребезг - опрос несколько раз с временным интервалом  */

#define WAIT_SENSORS_ANTI_RATTLING 13

void Wait_sensors_anti_rattling(struct Tech_process *tech_process){
 
  if (tech_process->temporary_counter){	
    if (tech_process->move_to_next_transition&TECH_PROCESS_SENSORS_WAIT) Check_sensors_transition(tech_process);
  		if(tech_process->move_to_next_transition & TECH_PROCESS_SENSORS_WAIT){
			/* значения не повторяются,восстановить счетчик */
			tech_process->temporary_counter = 
			tech_process->current_transition->temporary_counter;	
		}else{
			/* значения совпали, вычесть счетчики выставить проверять сенсор*/
			tech_process->temporary_counter--;			
			tech_process->move_to_next_transition |= TECH_PROCESS_SENSORS_WAIT;
		};
	
   }else {tech_process->move_to_next_transition = 0; /* 0 - Выходим дождалисчь события*/			
		  return;
		  }; // &= ~(TECH_PROCESS_SENSORS_WAIT | TECH_PROCESS_PAUSE);

  /* затем выдержать паузу 10ms и пойти на новый круг для опроса*/ 	
	if(tech_process->move_to_next_transition&TECH_PROCESS_PAUSE){ 
		Pause_transition(tech_process);
		tech_process->move_to_next_transition |= TECH_PROCESS_PAUSE;
		};
	
};
	
// Добавить 

// Включить млм выключить  устройства, выдержать паузу. вернуть обратно. 





/****************************************************************************************
 * Простые переходы с зависимостями														*
 ****************************************************************************************/






/****************************************************************************************
 * Сложные переходы со многоуровневыми зависимостями									*
 ****************************************************************************************/






/****************************************************************************************
 * NOP_transition - пустой переход никуда не ведет										*
 ****************************************************************************************/
void NOP_transition (struct Tech_process *tech_process){};

/*****************************************************************************************
 * Кольцевой буфер отправка команд и данных в поток тех.процесса						 *
 *****************************************************************************************/
 
int Tech_set_command(struct Tech_process *Tech, uint8_t command, void *data){

   uint8_t res;
    
	if(pthread_mutex_lock(&Tech->lock_command)) perror("error mutex lock buffer_command \n");
	
	 res = Tech->position_buffer + (Tech->num_commands++);
	 Tech->buffer_command[res] = command;
	 Tech->buffer_data[res] = data;
  
	if(pthread_mutex_unlock(&Tech->lock_command)) perror("error mutex unlock buffer_command \n");
};

/*****************************************************************************************
 * Поток который работает с тех.процессом												 * 
 *****************************************************************************************/
#define NULL_OPERATION			0
#define START_OPERATION 		1
#define PAUSE_OPERATION 		2
#define COUNTINUE_OPERATION 	3
#define END_OPERATION			4
#define PAUSE_KEY_TO_COUNTINUE  5
#define SELECTION_OPERATION		6

 
 
void pthread_Tech_process(void *per_tech){
	
 struct Tech_process  *tech_process = (struct Tech_process*)per_tech;
 struct Tech_transition *transition ; //tech_process->current_transition;
 struct Tech_operation 	 *operation ; //tech_process->current_operation;
 struct Thermostat 		*thermostat ; // *operation->thermoblock->thermostat;
 struct ProductCounter     *counter ; // счетчик в операции 
 uint8_t *number_operation = &tech_process->current_number_operation;
 uint8_t temp_var;
 uint8_t command;
 void      *data;
  
 struct timespec pause_delay = {
	.tv_sec = 0,
	.tv_nsec = 50000000,	 
  };	 
 
 uint8_t status_work_flow = 1;
 static uint8_t status_process   =	NULL_OPERATION;
 
printf("enabled Tech_process ... \n");
 
 void (*launch_transition[])(struct Tech_process *)={
	NOP_transition,					//
	// простые переходы 1-5
	Device_on_off_transition,		//DEVICE_ON_OFF_TRANSITION
	Pause_transition,				//PAUSE_TRANSITION
	Pause_timer_transition,			//PAUSE_TIMER_TRANSITION
	Check_sensors_transition,		//CHECK_SENSORS_TRANSITION
	Wait_temperature_for_countinue, //WAIT_TEMP_FOR_COUNTINUE

	// обьедененные переходы 6-12
	Device_on_off_Check_sensors,		
	Check_sensors_Device_on_off,
	Device_on_off_Pause,
	Device_on_off_Pause_timer,
	Pause_Device_on_off,
	Pause_timer_Device_on_off,
	Init_Sensor_Device_TempWork_Pause,
	Wait_sensors_anti_rattling,
 };
	  
// перестраиваем и разрешаем сигнал  для таймера;

  init_timer_and_signal_tech_process(tech_process);		  
		  			  
// основной цикл			  
 
 while (status_work_flow){
	
  if(tech_process->num_commands){
		
	if(pthread_mutex_trylock(&tech_process->lock_command)) {
		command = 0;
    }else{		
		data = &tech_process->buffer_data[tech_process->position_buffer];
		command = tech_process->buffer_command[(tech_process->position_buffer)++];
		(tech_process->num_commands)--;						
		pthread_mutex_unlock(&tech_process->lock_command);
  };		 
	
 }else command = 0;
		
 switch(command){
	 
	 case 0:
	 break;

/* 
 * сбросить счетчик
 * Выставить температуру на термостатах..
 * Возможно рассмотреть  запуск процесса только
 * когда температура достигнет заданных значений
 */

	case START_OPERATION:
 	 
 	 if(status_process != NULL_OPERATION) break; 
     //printf("start Tech_transition \n");
 	 	 
	 if(*number_operation >= tech_process->N_operation) break;
	 //printf("number Tech_transition %d - %d \n",*number_operation ,tech_process->N_operation);

	/* Выбираем операцию*/	 	 	
	 operation = 
	 tech_process->current_operation = 
	 (tech_process->operation)+(*number_operation);
	 
	 /* Обновляем температуру для операции */
	 
	 thermostat = *operation->thermoblock->thermostat;
	 
	 for(int i = 0; i< operation->N_temp;i++){

	  
		(thermostat+i)->temperatura_on = *(operation->temperatura_on + i);
		(thermostat+i)->temperatura_off = *(operation->temperatura_off + i);
		
		change_temp_on_off_thermostat(thermostat+i);


	  printf("Temp on %d  off %d \n",(thermostat+i)->temperatura_on,(thermostat+i)->temperatura_off);
	  };
	  
	/* Сбрасываем счетчик  - это костыль надо сделать правильно*/  
	 
	 counter = *operation->counters;

	for(int i=0 ; i< operation->N_counter; i++){
	
	(counter + i)->counter_raw = *(operation->counter_raw + i);
	
	(counter + i)->counter_event = *(operation->counter_raw + i);
	/* обновляем будущие событие  согласно  >counter_event*/
	(counter + i)->future_counter_raw_event = 
		(*(operation->counter_raw + i)) + (*(operation->counter_raw + i));
    /*очередь цепочки событий в ноль*/
	(counter + i)->N_positions_in_the_event = 0;
	/* сбрасываем счетчик в default и сообщаем tft чтоб обновил счетчик на экране*/
	(counter + i)->counter_raw_to_char(
			&(counter + i)->info_block,0,10,(counter + i)->counter_raw);
	};
	
	  
	/* Выбераем первый переход из операции */  
	transition =
	tech_process->current_transition = 
	operation->first_transition; 
	
	/* Копируем начальные условия для начала работы перехода */
    tech_process->move_to_next_transition = transition->move_to_next_transition;
	tech_process->temporary_counter		  = transition->temporary_counter;
    status_process 						  = COUNTINUE_OPERATION;
    
	//printf("transition  %d current_transition %d \n",(int*)transition,(int*)tech_process->current_transition);
			
	break;

	/* Операцию в режим паузы */	 	 

	 case PAUSE_OPERATION:
		if(status_process != COUNTINUE_OPERATION) break; 
		status_process = PAUSE_OPERATION;
	 break;

	/* Операцию продолжить */

     case COUNTINUE_OPERATION: 
		if(status_process != PAUSE_OPERATION) break;     
		status_process = COUNTINUE_OPERATION;     
     break;
     
    /*
	Остановить процесс и не продолжать:
	обнулить все данные и выставить значение 
	по умолчанию.
	*/ 
	case END_OPERATION:
	if(status_process == NULL_OPERATION) break;
	
	transition = 
	tech_process->current_transition 	  = operation->exit_transition; 
	tech_process->move_to_next_transition = transition->move_to_next_transition;	 	 
	status_process = END_OPERATION;	
	printf("end Tech_transition \n");		
	break;
	  
	/* 
	Остановится и дождаться нажатия 
    клавиши и выполнить один переход,
    и обратно дождаться клавиши.
    */ 
	case PAUSE_KEY_TO_COUNTINUE:
	if(status_process != PAUSE_OPERATION) break;
 	status_process = PAUSE_KEY_TO_COUNTINUE;
	break;

	/*Выбрать номер операции для выполнения,
	  выбор идет в меню.
	*/
	case SELECTION_OPERATION:
	
	if(status_process != NULL_OPERATION) break;	
	temp_var = *((uint8_t*)data);	
	//if(temp_var < tech_process->N_operation) 
		*number_operation = temp_var;
	
	//printf("select %d  void %d  number %d  \n",temp_var,data,*number_operation);		
		
	break;
	
	 
	default:	
	break;
};
	
command =0;

/*---- Обработка режима перехода,останова или окончания операции---*/ 

 switch(status_process){
		
  case COUNTINUE_OPERATION:
	
	// Запуск перехода
	launch_transition[transition->transition_type](tech_process);

	if(!tech_process->move_to_next_transition){	
	// Условие соблюденно следющий переход	
		transition = 
		tech_process->current_transition 	  = transition->next_transition;	
		tech_process->move_to_next_transition = transition->move_to_next_transition;
		tech_process->temporary_counter		  = transition->temporary_counter;
		printf("%s \n", transition->transition_information);		
	};		
  break;
  
  case PAUSE_KEY_TO_COUNTINUE:
  
    launch_transition[transition->transition_type](tech_process);

	
	if(!tech_process->move_to_next_transition){	
		transition =
		tech_process->current_transition 	  = transition->next_transition;	
		tech_process->move_to_next_transition = transition->move_to_next_transition;			

		printf("%s \n", transition->transition_information);
	/*
	  Не продолжаем дальше, а ждем нажатия клавиши 
	  для начало следующего перехода 
	*/
		status_process = PAUSE_OPERATION;
	};		 
  break;
  
  case END_OPERATION: 
    launch_transition[transition->transition_type](tech_process);
    if(!tech_process->move_to_next_transition) status_process = NULL_OPERATION;
  break;
	
/*	
  case PAUSE_OPERATION:  
  // в ожидании команды разблокировки
  nanosleep(&pause_delay,NULL);
  break;

  case NULL_OPERATION:
  nanosleep(&pause_delay,NULL);
  break;
*/  
  default:
  nanosleep(&pause_delay,NULL);  
  break;
 };

	
};//while	
pthread_exit(NULL);		
};	
/*****************************************************************************************
 * Cтарт потока Техпроцесс	   								 		 * 
 *****************************************************************************************/

void pthread_start_Tech_process(struct Tech_process *Tech){
 
 int potok = pthread_create(&Tech->pthread_id,
							NULL,
							(void*) pthread_Tech_process,
							(void *) Tech);
 if (potok)printf("ERROR; return code from pthread_create() is %d\n", potok);
	
 
};


/*   
 
 Функции для меню - 
 
 Старт Штамповки
 * 
   Закончить
   Остановить
   Продолжить
   Пошагово
   
*/


void Tech_process_set_command (struct TFT_menu *menu){
		
struct Tech_process *process = (struct Tech_process *) menu->selected_line->data;	
uint8_t set_command = menu->selected_line->number;

if(set_command == START_OPERATION ) tft_menu_submenu (menu);
if(set_command == END_OPERATION ) tft_menu_exit(menu);
		
Tech_set_command(process,set_command,NULL);	
};

/*   
 Выбор операции станка 
 *  
 меню - 
 Программы станка
   
*/
void Tech_process_set_operation(struct TFT_menu *menu){
		
struct Tech_process *process = (struct Tech_process *) menu->selected_line->data;
struct Menu_line *menu_line = menu->selected_line;	
uint8_t num_new = menu_line->number;
uint8_t num_old = process->current_number_operation;
uint8_t the_end = 2;

// Находим О строку подменю 
while(menu_line->prev != NULL) menu_line = menu_line->prev;

// Ищем какую строку выбрать, а в какой удалить  выделение
for(int i = 0; menu_line != NULL; i++){ 
	
	menu_line = menu_line->next;
	
	if(i==num_new){
		*menu_line->text 	 = SET_background;
		*(menu_line->text+1) = 0x3a;
		*(menu_line->text+2) = 0x07;
		the_end --;
		};
		
	if(i==num_old){		
		*menu_line->text 	 = 0x1f;
		*(menu_line->text+1) = 0x1f;
		*(menu_line->text+2) = 0x1f;
		the_end --;
		};		
	if(!the_end) break;		
};


tft_menu_visible(menu);

		//printf("select %d \n",menu->selected_line->number);
Tech_set_command(process,
				 SELECTION_OPERATION,
				 (void*)(unsigned int) menu->selected_line->number
				 );	
};



