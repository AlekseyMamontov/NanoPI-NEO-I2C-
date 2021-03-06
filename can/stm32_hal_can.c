/ * *
  ************************************************ ****************************
  * @ файл stm32f1xx_hal_can.c
  * @author Команда разработчиков MCD
  * @brief драйвер модуля CAN HAL.
  * Этот файл предоставляет функции прошивки для управления следующими
  * функциональные возможности периферийного устройства Controller Area Network (CAN):
  * + Функции инициализации и деинициализации
  * + Функции настройки
  * + Функции управления
  * + Управление прерываниями
  * + Функции обратного вызова
  * + Периферийное состояние и функции ошибок
  *
  @verbatim
  ================================================== ============================
                        ##### Как использовать этот драйвер #####
  ================================================== ============================
    [..]
      (#) Инициализируйте ресурсы низкого уровня CAN, реализуя
          HAL_CAN_MspInit ():
         (++) Включите часы интерфейса CAN с помощью __HAL_RCC_CANx_CLK_ENABLE ()
         (++) Настроить контакты CAN
             (+++) Включить часы для CAN GPIO
             (+++) Настройте контакты CAN как альтернативную функцию с открытым стоком
         (++) В случае использования прерываний (например, HAL_CAN_ActivateNotification ())
             (+++) Настройте приоритет прерывания CAN с помощью
                   HAL_NVIC_SetPriority ()
             (+++) Включите обработчик CAN IRQ с помощью HAL_NVIC_EnableIRQ ()
             (+++) В обработчике CAN IRQ вызовите HAL_CAN_IRQHandler ()
      (#) Инициализировать периферийное устройство CAN с помощью функции HAL_CAN_Init (). Этот
          функция обращается к HAL_CAN_MspInit () для низкоуровневой инициализации.
      (#) Настройте фильтры приема, используя следующую конфигурацию
          функции:
            (++) HAL_CAN_ConfigFilter ()
      (#) Запускаем модуль CAN с помощью функции HAL_CAN_Start (). На этом уровне
          узел активен на шине: он принимает сообщения и может отправлять
          Сообщения.
      (#) Для управления передачей сообщений используются следующие функции управления Tx
          может быть использован:
       
            (++) HAL_CAN_AddTxMessage () для запроса передачи нового
                 сообщение.
       
            (++) HAL_CAN_AbortTxRequest () для прерывания передачи ожидающего
                 сообщение.
       
            (++) HAL_CAN_GetTxMailboxesFreeLevel (), чтобы получить количество бесплатных Tx
                 почтовые ящики.
       
            (++) HAL_CAN_IsTxMessagePending (), чтобы проверить, ожидает ли сообщение
                 в почтовом ящике Tx.
       
            (++) HAL_CAN_GetTxTimestamp () для получения отметки времени сообщения Tx
                 отправляется, если включен режим связи по времени.
     
      (#) Когда сообщение получено в FIFO CAN Rx, его можно получить
          с помощью функции HAL_CAN_GetRxMessage (). Функция
          HAL_CAN_GetRxFifoFillLevel () позволяет узнать, сколько сообщений Rx
          хранится в Rx Fifo.
      (#) Вызов функции HAL_CAN_Stop () останавливает модуль CAN.
      (#) Деинициализация достигается с помощью функции HAL_CAN_DeInit ().
      *** Работа в режиме опроса ***
      ==============================
    [..]
      (#) Прием:
            (++) Мониторинг приема сообщения с помощью HAL_CAN_GetRxFifoFillLevel ()
                 пока не будет получено хотя бы одно сообщение.
            (++) Затем получите сообщение с помощью HAL_CAN_GetRxMessage ().
      (#) Передача инфекции:
            (++) Контролируйте доступность почтовых ящиков Tx, пока не появится хотя бы один Tx
                 почтовый ящик бесплатный, используя HAL_CAN_GetTxMailboxesFreeLevel ().
            (++) Затем запросите передачу сообщения, используя
                 HAL_CAN_AddTxMessage ().
      *** Работа в режиме прерывания ***
      ================================
    [..]
      (#) Уведомления активируются с помощью HAL_CAN_ActivateNotification ()
          функция. Затем процессом можно управлять с помощью
          доступные обратные вызовы пользователей: HAL_CAN_xxxCallback () с использованием тех же API
          HAL_CAN_GetRxMessage () и HAL_CAN_AddTxMessage ().
      (#) Уведомления можно отключить с помощью
          Функция HAL_CAN_DeactivateNotification ().
      (#) Особое внимание следует уделять CAN_IT_RX_FIFO0_MSG_PENDING и
          Уведомления CAN_IT_RX_FIFO1_MSG_PENDING. Эти уведомления запускают
          обратные вызовы HAL_CAN_RxFIFO0MsgPendingCallback () и
          HAL_CAN_RxFIFO1MsgPendingCallback (). У пользователя есть два возможных варианта
          здесь.
            (++) Непосредственно получить сообщение Rx в обратном вызове, используя
                 HAL_CAN_GetRxMessage ().
            (++) Или отключите уведомление в обратном вызове без
                 получение сообщения Rx. Сообщение Rx может быть получено позже.
                 используя HAL_CAN_GetRxMessage (). После того, как сообщение Rx было
                 прочитал, уведомление можно активировать снова.
      *** Спящий режим ***
      ==================
    [..]
      (#) Периферийное устройство CAN можно перевести в спящий режим (низкое энергопотребление), используя
          HAL_CAN_RequestSleep (). Спящий режим будет включен, как только
          текущая активность CAN (передача или прием кадра CAN) будет
          будет завершена.
      (#) Можно активировать уведомление, чтобы получать информацию о переходе в спящий режим.
          будет введен.
      (#) Проверить, включен ли спящий режим, можно с помощью
          HAL_CAN_IsSleepActive ().
          Обратите внимание, что состояние CAN (доступно из API HAL_CAN_GetState ())
          HAL_CAN_STATE_SLEEP_PENDING, как только запрос спящего режима
          отправлено (в спящий режим еще не вошел) и станет
          HAL_CAN_STATE_SLEEP_ACTIVE, когда активен спящий режим.
      (#) Пробуждение из спящего режима может быть вызвано двумя способами:
            (++) Использование HAL_CAN_WakeUp (). При возврате из этой функции
                 выход из спящего режима (если статус возврата HAL_OK).
            (++) Когда начало кадра Rx CAN обнаружено периферийным устройством CAN,
                 если включен автоматический режим пробуждения.
  *** Регистрация обратного звонка ***
  =============================================
  Компиляция определяет USE_HAL_CAN_REGISTER_CALLBACKS при значении 1
  позволяет пользователю динамически настраивать обратные вызовы драйвера.
  Используйте функцию @ref HAL_CAN_RegisterCallback (), чтобы зарегистрировать обратный вызов прерывания.
  Функция @ref HAL_CAN_RegisterCallback () позволяет регистрировать следующие обратные вызовы:
    (+) TxMailbox0CompleteCallback: Tx Mailbox 0 Полный обратный вызов.
    (+) TxMailbox1CompleteCallback: Tx Mailbox 1 Complete Callback.
    (+) TxMailbox2CompleteCallback: Tx Mailbox 2 Complete Callback.
    (+) TxMailbox0AbortCallback: Tx Mailbox 0 Прервать обратный вызов.
    (+) TxMailbox1AbortCallback: Tx Mailbox 1 Abort Callback.
    (+) TxMailbox2AbortCallback: Tx Mailbox 2 Abort Callback.
    (+) RxFifo0MsgPendingCallback: Rx Fifo 0 Message Pending Callback.
    (+) RxFifo0FullCallback: Rx Fifo 0 Полный обратный вызов.
    (+) RxFifo1MsgPendingCallback: Rx Fifo 1 Message Pending Callback.
    (+) RxFifo1FullCallback: Полный обратный вызов Rx Fifo 1.
    (+) SleepCallback: обратный вызов сна.
    (+) WakeUpFromRxMsgCallback: пробуждение от обратного вызова сообщения Rx.
    (+) ErrorCallback: Обратный вызов ошибки.
    (+) MspInitCallback: CAN MspInit.
    (+) MspDeInitCallback: CAN MspDeInit.
  Эта функция принимает в качестве параметров дескриптор периферийного устройства HAL, идентификатор обратного вызова.
  и указатель на функцию обратного вызова пользователя.
  Используйте функцию @ref HAL_CAN_UnRegisterCallback (), чтобы сбросить обратный вызов до значения по умолчанию
  слабая функция.
  @ref HAL_CAN_UnRegisterCallback принимает в качестве параметров дескриптор периферийного устройства HAL,
  и идентификатор обратного вызова.
  Эта функция позволяет сбросить следующие обратные вызовы:
    (+) TxMailbox0CompleteCallback: Tx Mailbox 0 Полный обратный вызов.
    (+) TxMailbox1CompleteCallback: Tx Mailbox 1 Complete Callback.
    (+) TxMailbox2CompleteCallback: Tx Mailbox 2 Complete Callback.
    (+) TxMailbox0AbortCallback: Tx Mailbox 0 Прервать обратный вызов.
    (+) TxMailbox1AbortCallback: Tx Mailbox 1 Abort Callback.
    (+) TxMailbox2AbortCallback: Tx Mailbox 2 Abort Callback.
    (+) RxFifo0MsgPendingCallback: Rx Fifo 0 Message Pending Callback.
    (+) RxFifo0FullCallback: Rx Fifo 0 Полный обратный вызов.
    (+) RxFifo1MsgPendingCallback: Rx Fifo 1 Message Pending Callback.
    (+) RxFifo1FullCallback: Полный обратный вызов Rx Fifo 1.
    (+) SleepCallback: обратный вызов сна.
    (+) WakeUpFromRxMsgCallback: пробуждение от обратного вызова сообщения Rx.
    (+) ErrorCallback: Обратный вызов ошибки.
    (+) MspInitCallback: CAN MspInit.
    (+) MspDeInitCallback: CAN MspDeInit.
  По умолчанию после @ref HAL_CAN_Init () и когда состояние равно HAL_CAN_STATE_RESET,
  все обратные вызовы устанавливаются на соответствующие слабые функции:
  пример @ref HAL_CAN_ErrorCallback ().
  Исключение сделано для функций MspInit и MspDeInit, которые
  сбросить на устаревшую слабую функцию в @ref HAL_CAN_Init () / @ref HAL_CAN_DeInit () только когда
  эти обратные вызовы равны нулю (не зарегистрированы заранее).
  в противном случае MspInit или MspDeInit не равны нулю, @ref HAL_CAN_Init () / @ref HAL_CAN_DeInit ()
  сохранять и использовать пользовательские обратные вызовы MspInit / MspDeInit (зарегистрированные заранее)
  Обратные вызовы можно зарегистрировать / отменить регистрацию только в состоянии HAL_CAN_STATE_READY.
  Исключение сделано MspInit / MspDeInit, который можно зарегистрировать / отменить
  в состоянии HAL_CAN_STATE_READY или HAL_CAN_STATE_RESET,
  таким образом зарегистрированные (пользовательские) обратные вызовы MspInit / DeInit могут использоваться во время Init / DeInit.
  В этом случае сначала зарегистрируйте обратные вызовы пользователей MspInit / MspDeInit.
  используя @ref HAL_CAN_RegisterCallback () перед вызовом @ref HAL_CAN_DeInit ()
  или функция @ref HAL_CAN_Init ().
  Когда определение компиляции USE_HAL_CAN_REGISTER_CALLBACKS установлено в 0 или
  не определено, функция регистрации обратного вызова недоступна и все обратные вызовы
  установлены на соответствующие слабые функции.
  @endverbatim
  ************************************************ ****************************
  * @внимание
  *
  * <h2> <center> & copy; Авторские права (c) STMicroelectronics, 2016 г.
  * Все права защищены. </center> </h2>
  *
  * Этот программный компонент лицензирован ST в соответствии с лицензией BSD 3-Clause,
  * «Лицензия»; Вы не можете использовать этот файл, кроме как в соответствии с
  * Лицензия. Вы можете получить копию лицензии по адресу:
  * opensource.org/licenses/BSD-3-Clause
  *
  ************************************************ ****************************
  * /

/ * Включает ----------------------------------------------- ------------------- * /
# включить  " stm32f1xx_hal.h "

/ * * @addtogroup STM32F1xx_HAL_Driver
  * @ {
  * /

# если определено (CAN1)

/ * * @defgroup CAN CAN
  * @brief модули драйвера CAN
  * @ {
  * /

# ifdef HAL_CAN_MODULE_ENABLED

# ifdef HAL_CAN_LEGACY_MODULE_ENABLED
  # error «Драйвер CAN не может использоваться с его устаревшей версией, пожалуйста, активируйте только один модуль CAN одновременно»
# endif

/ * Частное определение типа ---------------------------------------------- ------------- * /
/ * Частное определение ---------------------------------------------- -------------- * /
/ * * @defgroup CAN_Private_Constants Частные константы CAN
  * @ {
  * /
# определить  CAN_TIMEOUT_VALUE  10U
/ * *
  * @}
  * /
/ * Частный макрос ---------------------------------------------- --------------- * /
/ * Частные переменные ---------------------------------------------- ----------- * /
/ * Прототипы частных функций --------------------------------------------- - * /
/ * Экспортированные функции ---------------------------------------------- ---------- * /

/ * * @defgroup CAN_Exported_Functions Экспортированные функции CAN
  * @ {
  * /

/ * * @defgroup CAN_Exported_Functions_Group1 Функции инициализации и деинициализации
 * @brief функции инициализации и настройки
 *
@verbatim
  ================================================== ============================
              ##### Функции инициализации и деинициализации #####
  ================================================== ============================
    [..] В этом разделе представлены функции, позволяющие:
      (+) HAL_CAN_Init: Инициализировать и настроить CAN.
      (+) HAL_CAN_DeInit: деинициализировать CAN.
      (+) HAL_CAN_MspInit: инициализировать CAN MSP.
      (+) HAL_CAN_MspDeInit: деинициализировать CAN MSP.
@endverbatim
  * @ {
  * /

/ * *

================================================== ============================
                      ##### Функции управления #####
  ================================================== ============================
    [..] В этом разделе представлены функции, позволяющие:
      (+) HAL_CAN_Start: Запуск CAN модуля
      (+) HAL_CAN_Stop: Остановить модуль CAN
      (+) HAL_CAN_RequestSleep: запрос входа в спящий режим.
      (+) HAL_CAN_WakeUp: Выход из спящего режима.
      (+) HAL_CAN_IsSleepActive: Проверить, активен ли спящий режим.
      (+) HAL_CAN_AddTxMessage: Добавить сообщение в почтовые ящики Tx
                                             и активируйте соответствующий
                                             запрос на передачу
      (+) HAL_CAN_AbortTxRequest: Отменить запрос передачи
      (+) HAL_CAN_GetTxMailboxesFreeLevel: Возвращает свободный уровень почтовых ящиков Tx
      (+) HAL_CAN_IsTxMessagePending: проверяет, является ли запрос передачи
                                             ожидает в выбранном почтовом ящике Tx
      (+) HAL_CAN_GetRxMessage: Получить CAN-кадр из Rx FIFO
      (+) HAL_CAN_GetRxFifoFillLevel: вернуть уровень заполнения Rx FIFO
@endverbatim
  * @ {
  * /
  
   ================================================== ============================
                       ##### Управление прерываниями #####
  ================================================== ============================
    [..] В этом разделе представлены функции, позволяющие:
      (+) HAL_CAN_ActivateNotification: разрешить прерывания
      (+) HAL_CAN_DeactivateNotification: отключить прерывания
      (+) HAL_CAN_IRQHandler: обрабатывает запрос прерывания CAN
@endverbatim
  * @ {
  * /

/ * *
  * @brief Разрешить прерывания.
  * @param hcan указатель на структуру CAN_HandleTypeDef, содержащую
  * информация о конфигурации для указанной CAN.
  * @param ActiveITs указывает, какие прерывания будут разрешены.
  * Этот параметр может быть любой комбинацией @arg CAN_Interrupts.
  * @retval статус HAL
  * /
  
  
  
  
  ================================================== ============================
            ##### Периферийное состояние и функции ошибок #####
  ================================================== ============================
    [..]
    В этом подразделе представлены функции, позволяющие:
      (+) HAL_CAN_GetState (): вернуть состояние CAN.
      (+) HAL_CAN_GetError (): Возвращает коды ошибок CAN, если они есть.
      (+) HAL_CAN_ResetError (): Сбросить коды ошибок CAN, если они есть.
@endverbatim
  * @ {
  * /

/ * *
  * @brief Возвращает состояние CAN.
  * @param hcan указатель на структуру CAN_HandleTypeDef, содержащую
  * информация о конфигурации для указанной CAN.
  * @retval состояние HAL
  * /
