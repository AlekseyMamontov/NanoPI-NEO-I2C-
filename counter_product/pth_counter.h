
 /* 
/*
+-------------------------------------------------------------------------+
|				Создаем блок счетчика готовой продукции					  |
+-------------------------------------------------------------------------+
*/

struct ProductCounter{
	
// количество событий 		
	uint32_t counter_raw; 				
// значение на которое увеличвается счетчик
	uint32_t value_inc_the_counter;		
// при такой кратности должно срабатывать событие, например 1/10,1/25 и 1/1000
	uint32_t counter_event; 			
// будущие событие									
	uint32_t future_counter_raw_event;									
	
	uint8_t pin_counter;   // изменение на этом пине  увеличивает счетчик 	
	
	uint8_t *sequence_of_events;        // массив цепочки событий	(010) - например _/\_
	uint8_t N_positions_in_the_event;   // номер позиции в очереди
	uint8_t N_sequence_of_events;	    // длина массива цепочки событий 


		
	uint8_t pin_on[5];  // массив содержит пины которые реагируют 
	uint8_t  N_pin_on;			     // включением - при кратности ; 
	uint8_t pin_off[5]; // массив содержит пины которые реагируют 
	uint8_t  N_pin_off;				 // отключением - наступлении события

	struct TFT_info_block info_block;

	void (*check_event)(struct ProductCounter *);
	void (*counter_raw_to_char)(struct TFT_info_block *,uint8_t,uint32_t,uint32_t); 
	void (*activation_counter_event)(struct ProductCounter *);
	};
/*
+-------------------------------------------------------------------------+
|    Cписок счетчиков опрашиваемых в цикле потоком						  |
+-------------------------------------------------------------------------+
*/
#define MAX_COUNTER 3

	struct PthreadsCounter_cycle{
		
	struct ProductCounter *counter[MAX_COUNTER+1];	
	/* 
	* время между опросами датчиков
	* nanosleep ( struct timespec , NULL); 
	*/	
	struct itimerspec time_pause;	
	
	pthread_t pthread_id;
	
	void (*pthread_block_counters)(void *);
};
struct PthreadsCounter_cycle Stanok_counters;

/*
+-------------------------------------------------------------------------+
|    Поток опрашивающий счетчики				  						  |
+-------------------------------------------------------------------------+
*/
void check_block_counters(void *temp){
	
 struct ProductCounter *counter;
 struct PthreadsCounter_cycle *counter_cycle;
 int i;
 
 counter_cycle = (struct PthreadsCounter_cycle *) temp;
	
  while(1){ 

		for(int i=0; counter_cycle->counter[i] != NULL; i++){

		counter_cycle->counter[i]->check_event(counter_cycle->counter[i]);

		};
 // задержка опроса пинов		
 nanosleep(&counter_cycle->time_pause.it_value, NULL);
 };
 
 pthread_exit(NULL);	
};	
/*
+-------------------------------------------------------------------------+
|    Старт потока опрашивающего счетчики		  						  |
+-------------------------------------------------------------------------+
*/
void pthread_start_counters(struct PthreadsCounter_cycle *counters){

 int potok = pthread_create(&counters->pthread_id,
							NULL,
							(void*) counters->pthread_block_counters,
							(void *) counters);
													
if (potok) printf("ERROR; return code from pthread_create() is %d\n", potok);	
};


/*
+-------------------------------------------------------------------------+
|    проверить наступило ли событие для счетчика 		  				  |
+-------------------------------------------------------------------------+
*/

void counter_check_event(struct ProductCounter *counter){
	
 uint8_t verify_bit = counter->info_block.get_pin(counter->pin_counter); 
	
  // данные значение не изменилось выходим		
  if (verify_bit == counter->sequence_of_events[counter->N_positions_in_the_event]) return; 
  
  if(++(counter->N_positions_in_the_event) < counter->N_sequence_of_events) {//printf("verify_bit %d ; counter->N_positions_in_the_event %d \n",verify_bit,counter->N_positions_in_the_event);
	return;} // часть события произошла, выходим цепочка еще не закончилась, 

	
	 
	counter->N_positions_in_the_event = 0; 										// цепочка завершилась.						
	counter->counter_raw += counter->value_inc_the_counter;	//printf("countercounter_raw %d \n",counter->counter_raw);// увеличить счетчик	
	counter->counter_raw_to_char(&counter->info_block,0,10,counter->counter_raw); // преобразовать число в текст.
			 
			   if (counter->counter_event != 0  && counter->counter_raw == counter->future_counter_raw_event){ //если отслеживаем кратность
				   counter->future_counter_raw_event += (counter->counter_event);							   // запускаем реакцию на событие
				   counter->activation_counter_event(counter); 
				}; 					 		  	
};

/*
+-------------------------------------------------------------------------+
|    пустышка пока										  				  |
+-------------------------------------------------------------------------+
*/
	
void activation_counter_event(){};













