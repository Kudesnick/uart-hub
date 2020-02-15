/***************************************************************************************************
 *   Project:       
 *   Author:        Stulov Tikhon (kudesnick@inbox.ru)
 ***************************************************************************************************
 *   Distribution:  
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          cpp_os.h
 *   Description:   Object wrapper for RTX-5 primitives.
 *                  Classes provide static memory allocation for stacks, queues and so on.
 *                  Сlasses provide an interface for using OS functions within the C ++ paradigm. 
 *
 ***************************************************************************************************
 *   History:       27.05.2019 - file created
 *                  08.12.2019 - v0.1.0 - added all primitives and Debug Access Port (DAP) support
 *                  05.01.2020 - v0.1.1 - added priority field into cpp_os_event class
 *
 **************************************************************************************************/

#pragma once

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdlib.h>
#include <stdint.h>

#include "cpp_list.h"
#include "rtx_os.h"
#include "RTX_Config.h"

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

/**
 *  @brief      Location of control block memory into object structure
 *
 *  @warning    If STATIC_CBM is undefined, you must up size of dynamic memory in RTX_Config.h,
 *              because control blocks located in it.
 */
//  #define STATIC_CBM

#define ALIGNED_LEN(_data, _type) (((_data) + sizeof(_type) - 1) / sizeof(_type))

/***************************************************************************************************
 *                                      PUBLIC TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                PUBLIC FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

void thread_run(void * argument);
void timer_run(void * argument);

/***************************************************************************************************
 *                               PUBLIC CLASS
 **************************************************************************************************/

//-- os_elements

// Абстрактный класс, реализующий общие функции примитивов ОС и обертки над глобальными функциями ОС
class cpp_os : public cpp_list<cpp_os>
{
private:
    // Произвести инициализацию всех примитивов ОС
    static void all_elements_create(void);
    // Инициализация примитива ОС
    virtual void * create(void) = 0;

protected:
    // Проверить возвращаемое значение (используется для отладки)
    static osStatus_t os_chck(osStatus_t);
    static void * os_chck(void *);
    const char * name;

public:
    cpp_os(void):
        cpp_list(),
        name(NULL)
    {};
    
    cpp_os(const char * _name):
        cpp_list(),
        name(_name)
    {};

    // Первичная инициализация ОС
    static void create_os(const bool _printinfo = false);

    // for other functions see https://www.keil.com/pack/doc/CMSIS/RTOS2/html/index.html

    static osStatus_t delay(const uint32_t _ticks)
    {
        return osDelay(_ticks);
    };
    
    static osStatus_t delay_until(const uint32_t _ticks)
    {
        return osDelayUntil(_ticks);
    };

    static uint32_t get_tick_count(void)
    {
        return osKernelGetTickCount();
    };
    
    static uint32_t get_tick_freq(void)
    {
        return osKernelGetTickFreq();
    }
    
    static constexpr  int32_t ok              = osOK;
    static constexpr  int32_t error           = osError;
    static constexpr  int32_t error_timeout   = osErrorTimeout;
    static constexpr  int32_t error_resource  = osErrorResource;
    static constexpr  int32_t error_parameter = osErrorParameter;
    static constexpr  int32_t error_nomemory  = osErrorNoMemory;
    static constexpr  int32_t error_isr       = osErrorISR;    

    static constexpr osPriority_t priority_none           = osPriorityNone        ; ///< No priority (not initialized).
    static constexpr osPriority_t priority_idle           = osPriorityIdle        ; ///< Reserved for Idle thread.
    static constexpr osPriority_t priority_low            = osPriorityLow         ;
    static constexpr osPriority_t priority_low_1          = osPriorityLow1        ;
    static constexpr osPriority_t priority_low_2          = osPriorityLow2        ;
    static constexpr osPriority_t priority_low_3          = osPriorityLow3        ;
    static constexpr osPriority_t priority_low_4          = osPriorityLow4        ;
    static constexpr osPriority_t priority_low_5          = osPriorityLow5        ;
    static constexpr osPriority_t priority_low_6          = osPriorityLow6        ;
    static constexpr osPriority_t priority_low_7          = osPriorityLow7        ;
    static constexpr osPriority_t priority_below_normal   = osPriorityBelowNormal ;
    static constexpr osPriority_t priority_below_normal_1 = osPriorityBelowNormal1;
    static constexpr osPriority_t priority_below_normal_2 = osPriorityBelowNormal2;
    static constexpr osPriority_t priority_below_normal_3 = osPriorityBelowNormal3;
    static constexpr osPriority_t priority_below_normal_4 = osPriorityBelowNormal4;
    static constexpr osPriority_t priority_below_normal_5 = osPriorityBelowNormal5;
    static constexpr osPriority_t priority_below_normal_6 = osPriorityBelowNormal6;
    static constexpr osPriority_t priority_below_normal_7 = osPriorityBelowNormal7;
    static constexpr osPriority_t priority_normal         = osPriorityNormal      ;
    static constexpr osPriority_t priority_normal_1       = osPriorityNormal1     ;
    static constexpr osPriority_t priority_normal_2       = osPriorityNormal2     ;
    static constexpr osPriority_t priority_normal_3       = osPriorityNormal3     ;
    static constexpr osPriority_t priority_normal_4       = osPriorityNormal4     ;
    static constexpr osPriority_t priority_normal_5       = osPriorityNormal5     ;
    static constexpr osPriority_t priority_normal_6       = osPriorityNormal6     ;
    static constexpr osPriority_t priority_normal_7       = osPriorityNormal7     ;
    static constexpr osPriority_t priority_above_normal   = osPriorityAboveNormal ;
    static constexpr osPriority_t priority_above_normal_1 = osPriorityAboveNormal1;
    static constexpr osPriority_t priority_above_normal_2 = osPriorityAboveNormal2;
    static constexpr osPriority_t priority_above_normal_3 = osPriorityAboveNormal3;
    static constexpr osPriority_t priority_above_normal_4 = osPriorityAboveNormal4;
    static constexpr osPriority_t priority_above_normal_5 = osPriorityAboveNormal5;
    static constexpr osPriority_t priority_above_normal_6 = osPriorityAboveNormal6;
    static constexpr osPriority_t priority_above_normal_7 = osPriorityAboveNormal7;
    static constexpr osPriority_t priority_high           = osPriorityHigh        ;
    static constexpr osPriority_t priority_high_1         = osPriorityHigh1       ;
    static constexpr osPriority_t priority_high_2         = osPriorityHigh2       ;
    static constexpr osPriority_t priority_high_3         = osPriorityHigh3       ;
    static constexpr osPriority_t priority_high_4         = osPriorityHigh4       ;
    static constexpr osPriority_t priority_high_5         = osPriorityHigh5       ;
    static constexpr osPriority_t priority_high_6         = osPriorityHigh6       ;
    static constexpr osPriority_t priority_high_7         = osPriorityHigh7       ;
    static constexpr osPriority_t priority_realtime       = osPriorityRealtime    ;
    static constexpr osPriority_t priority_realtime_1     = osPriorityRealtime1   ;
    static constexpr osPriority_t priority_realtime_2     = osPriorityRealtime2   ;
    static constexpr osPriority_t priority_realtime_3     = osPriorityRealtime3   ;
    static constexpr osPriority_t priority_realtime_4     = osPriorityRealtime4   ;
    static constexpr osPriority_t priority_realtime_5     = osPriorityRealtime5   ;
    static constexpr osPriority_t priority_realtime_6     = osPriorityRealtime6   ;
    static constexpr osPriority_t priority_realtime_7     = osPriorityRealtime7   ;
    static constexpr osPriority_t priority_isr            = osPriorityISR         ; ///< Reserved for ISR deferred thread.
    static constexpr osPriority_t priority_error          = osPriorityError       ; ///< System cannot determine priority or illegal priority.
    static constexpr osPriority_t priority_reserved       = osPriorityReserved    ; ///< Prevents enum down-size compiler optimization.

    static constexpr osThreadState_t thread_inactive   = osThreadInactive  ;
    static constexpr osThreadState_t thread_ready      = osThreadReady     ;
    static constexpr osThreadState_t thread_running    = osThreadRunning   ;
    static constexpr osThreadState_t thread_blocked    = osThreadBlocked   ;
    static constexpr osThreadState_t thread_terminated = osThreadTerminated;
    static constexpr osThreadState_t thread_error      = osThreadError     ;
    static constexpr osThreadState_t thread_reserved   = osThreadReserved  ;
    
    static constexpr uint32_t wait_forever   = osWaitForever;
    
    static constexpr uint32_t flags_wait_any        = osFlagsWaitAny;
    static constexpr uint32_t flags_wait_all        = osFlagsWaitAll;
    static constexpr uint32_t flags_no_clear        = osFlagsNoClear;
    static constexpr uint32_t flags_wait_any_no_clr = osFlagsWaitAny | osFlagsNoClear;
    static constexpr uint32_t flags_wait_all_no_clr = osFlagsWaitAll | osFlagsNoClear;

    static constexpr uint32_t flags_error_unknown   = osFlagsErrorUnknown;
    static constexpr uint32_t flags_error_timeout   = osFlagsErrorTimeout;
    static constexpr uint32_t flags_error_resource  = osFlagsErrorResource;
    static constexpr uint32_t flags_error_parameter = osFlagsErrorParameter;
    static constexpr uint32_t flags_error_ISR       = osFlagsErrorISR;

    static constexpr uint32_t mutex_recursive    = osMutexRecursive;
    static constexpr uint32_t mutex_prio_inherit = osMutexPrioInherit;
    static constexpr uint32_t mutex_robust       = osMutexRobust;
    };

//-- thread

// Абстрактный класс потока
/* Параметризация шаблона:
 * - T_stack_size - размер стека
 */
template<uint32_t T_stack_size = OS_STACK_SIZE> class cpp_os_thread : public cpp_os
{
private:
    // Выравнивание по uint64_t обязательно
    uint64_t stack[ALIGNED_LEN(T_stack_size, uint64_t)]; 
#ifdef STATIC_CBM
    osRtxThread_t tcb;
#endif
    bool must_run;
    const osPriority_t priority;

    friend void thread_run(void * argument);
    
    // Виртуальная функция тела потока. Должна быть реализована в дочернем классе.
    virtual void thread_func(void) = 0;
    
    // Инициализация и запуск потока
    osThreadId_t create(void)
    {
        if (must_run)
        {
            id_ = run();
        }
        
        return id_;
    };
    
protected:
    osThreadId_t id_;
    
    osStatus_t yeld(void)
    {        
        return osThreadYield();
    };
    
    void exit(void)
    {
        osThreadExit();
    };
    
    uint32_t flags_clear(uint32_t _flags)
    {
        return osThreadFlagsClear(_flags);	
    };
    
    uint32_t flags_get(void)
    {
        return osThreadFlagsGet();	
    };
    
    uint32_t flags_wait(uint32_t _flags, uint32_t _options, uint32_t _timeout)
    {
        return osThreadFlagsWait(_flags, _options, _timeout);
    };

public:
    // если true - инициализация и запуск потока выполняется на этапе инициализации ОС
    cpp_os_thread(const bool _auto_run,
                  const osPriority_t _priority,
                  const char * _name):
        cpp_os(_name),
        must_run(_auto_run),
        priority(_priority)
    {};
    
    cpp_os_thread(const bool _auto_run, const char * _name):
        cpp_os_thread(_auto_run, priority_normal, _name)
    {};
    
    cpp_os_thread(const char * _name):
        cpp_os_thread(true, priority_normal, _name)
    {};

    osThreadId_t run(void)
    {
        const osThreadAttr_t attr =
        {
            .name       = name,
        #ifdef STATIC_CBM
            .cb_mem     = &tcb,
            .cb_size    = sizeof(tcb),
        #endif
            .stack_mem  = stack,
            .stack_size = sizeof(stack),
            .priority   = priority,
        };

        if (id_ != NULL)
        {
            if (get_state() == thread_error)
            {
                id_ = NULL;
            }
        }

        if (id_ == NULL)
        {
            id_ = os_chck(osThreadNew(thread_run, this, &attr));
        }
        
        return id_;
    };
    
    const char * get_name(void)
    {
        return osThreadGetName(id_);
    };
    
    osThreadState_t get_state(void)
    {
        return osThreadGetState(id_);
    };
    
    osStatus_t set_priority(osPriority_t _priority)
    {
        return osThreadSetPriority(id_, _priority);
    };
    
    osPriority_t get_priority(void)
    {        
        return osThreadGetPriority(id_);
    };

    osStatus_t suspend(void)
    {        
        return osThreadSuspend(id_);
    };
    
    osStatus_t resume(void)
    {        
        return osThreadResume(id_);
    };
    
    osStatus_t detach(void)
    {        
        return osThreadDetach(id_);
    };
    
    osStatus_t join(void)
    {        
        return osThreadJoin(id_);
    };
    
    osStatus_t terminate(void)
    {        
        return osThreadTerminate(id_);
    };
    
    uint32_t get_stack_size(void)
    {
        return osThreadGetStackSize(id_);
    };

    uint32_t get_stack_space(void)
    {
        // Stack watermark recording during execution needs to be enabled.
        return osThreadGetStackSize(id_);
    };
    
    static uint32_t get_count(void)	
    {
        return osThreadGetCount();
    };
    
    static uint32_t enumerate(osThreadId_t *& _thread_array, uint32_t _array_items)
    {
        return osThreadEnumerate(_thread_array, _array_items);
    };

    uint32_t flags_set(uint32_t _flags)
    {
        return osThreadFlagsSet(id_, _flags);
    };
};

//-- Timer

// Класс таймера
class cpp_os_timer : public cpp_os
{
private:

#ifdef STATIC_CBM
    osRtxTimer_t  tcb;
#endif
    const bool repeat;
    const uint32_t interval;

    friend void timer_run(void * argument);
    
    // Виртуальная функция тела таймера. Должна быть реализована в дочернем классе.
    virtual void timer_func(void) = 0;

    // Инициализация таймера
    osTimerId_t create(void)
    {
        const osTimerAttr_t attr =
        {
            .name    = name,
#ifdef STATIC_CBM
            .cb_mem  = &tcb,
            .cb_size = sizeof(tcb),
#endif
        };
        
        const osTimerType_t type = (repeat) ? osTimerPeriodic : osTimerOnce;
        
        id_ = osTimerNew(timer_run, type, this, &attr);
        
        return id_;
    };

protected:
    osTimerId_t id_;

public:
    // Constructor
    cpp_os_timer(const uint32_t _interval, const bool _repeat, const char * _name):
        cpp_os(_name),
        interval(_interval),
        repeat(_repeat)
    {};
    
    const char * get_name(void)
    {
        return osTimerGetName(id_);
    };
    
    osStatus_t start(void)
    {
        return osTimerStart(id_, interval);	
    };
    
    osStatus_t start(const uint32_t _interval)
    {
        return osTimerStart(id_, _interval);	
    };
    
    osStatus_t stop(void)
    {
        return osTimerStop(id_);
    };
    
    bool is_runing(void)
    {
        return static_cast<bool>(osTimerIsRunning(id_));
    };
};

//-- queue

// Абстрактный класс очереди
/* Параметризация шаблона:
 * - T_queue_elment_t - тип элемента очереди
 * - T_queue_count - количество элементов в очереди
 */
template<typename T_queue_elment_t, uint32_t T_queue_count> class cpp_os_queue : public cpp_os
{
private:
    uint32_t qdata[osRtxMessageQueueMemSize(T_queue_count, sizeof(T_queue_elment_t)) / sizeof(uint32_t)];

#ifdef STATIC_CBM
    osRtxMessageQueue_t qcb;
#endif

    T_queue_elment_t msg_;
    uint8_t          msg_priority_;

    // Инициализация очереди
    osMessageQueueId_t create(void)
    {
        const osMessageQueueAttr_t attr =
        {
            .name    = name,
#ifdef STATIC_CBM
            .cb_mem  = &qcb,
            .cb_size = sizeof(qcb),
#endif
            .mq_mem  = &qdata,
            .mq_size = sizeof(qdata),
        };

        id_ = os_chck(osMessageQueueNew(T_queue_count, sizeof(T_queue_elment_t), &attr));
        return id_;
    };

protected:
    osMessageQueueId_t id_;

public:
    cpp_os_queue(const char * _name = NULL):
        cpp_os(_name)
    {};

    osStatus_t put(const void * _msg_ptr, uint8_t _msg_prio, uint32_t _timeout)
    {
        if (_msg_ptr == NULL)
        {
            _msg_ptr = &msg_;
        }
    
        return osMessageQueuePut(id_, _msg_ptr, _msg_prio, _timeout);
    };
    
    osStatus_t put_now(void)
    {
        return put(NULL, 0, 0);
    }
    
    osStatus_t put_wait_forever(void)
    {
        return put(NULL, 0, cpp_os::wait_forever);
    }

    osStatus_t get(void * _msg_ptr, uint8_t * _msg_prio, uint32_t _timeout)
    {
        if (_msg_ptr == NULL)
        {
            _msg_ptr = &msg_;
        }
        if (_msg_prio == NULL)
        {
            _msg_prio = &msg_priority_;
        }
    
        return osMessageQueueGet(id_, _msg_ptr, _msg_prio, _timeout);
    };
    
    osStatus_t get_now(void)
    {
        return get(NULL, NULL, 0);
    }
    
    osStatus_t get_wait_forever(void)
    {
        return get(NULL, NULL, cpp_os::wait_forever);
    }

    uint32_t get_capacity(void)
    {
        return osMessageQueueGetCapacity(id_);
    };

    uint32_t get_msg_size(void)
    {
        return osMessageQueueGetMsgSize(id_);
    };

    uint32_t get_count(void)
    {
        return osMessageQueueGetCount(id_);
    };  

    uint32_t get_space(void)
    {
        return osMessageQueueGetSpace(id_);
    };

    osStatus_t reset(void)   
    {
        return osMessageQueueReset(id_);
    };
    
    const T_queue_elment_t &msg = msg_;
    const uint8_t &msg_priority = msg_priority_;
};

//-- event_flags

// Класс флаговых событий
class cpp_os_event : public cpp_os
{
private:

#ifdef STATIC_CBM
    osRtxEventFlags_t ecb;
#endif

    // Первичная инициализация
    osEventFlagsId_t create(void)
    {
        const osEventFlagsAttr_t attr =
        {
            .name    = name,
#ifdef STATIC_CBM
            .cb_mem  = &ecb,
            .cb_size = sizeof(ecb),
#endif
        };

        id_ = os_chck(osEventFlagsNew(&attr));
        return id_;
    };
    
protected:
    osEventFlagsId_t id_;

public:
    cpp_os_event(const char * _name = NULL):
        cpp_os(_name)
    {};

    uint32_t set(uint32_t _flags)
    {
        return osEventFlagsSet(id_, _flags);
    };

    uint32_t clear(uint32_t _flags)
    {
        return osEventFlagsClear(id_, _flags);
    };

    uint32_t get(void)
    {
        return osEventFlagsGet(id_);
    };

    uint32_t wait(uint32_t _flags, uint32_t _options, uint32_t _timeout)
    {
        return osEventFlagsWait(id_, _flags, _options, _timeout);
    };
};

//-- mutex

class cpp_os_mutex : public cpp_os
{
private:
#ifdef STATIC_CBM
    osRtxMutex_t mcb;
#endif

    // Первичная инициализация
    osMutexId_t create(void)
    {
        const osMutexAttr_t attr =
        {
            .name      = name,
            .attr_bits = flags,
        #ifdef STATIC_CBM
            .cb_mem    = &mcb,
            .cb_size   = sizeof(mcb),
        #endif
        };
        
        id_ = os_chck(osMutexNew(&attr));
        return id_;
    };

protected:
    osMutexId_t id_;

    const uint8_t flags;

public:
    cpp_os_mutex(uint32_t _flags = mutex_recursive | mutex_robust | mutex_prio_inherit,
                 const char * _name = NULL):
        cpp_os(_name),
        flags(_flags)
    {};
    
    cpp_os_mutex(const char * _name):
        cpp_os_mutex(mutex_recursive | mutex_robust | mutex_prio_inherit, _name)
    {};

    osStatus_t acquire(uint32_t _timeout)
    {
        return osMutexAcquire(id_, _timeout);
    };

    osStatus_t release(void)
    {
        return osMutexRelease(id_);
    };

    osThreadId_t get_owner(void)
    {
        return osMutexGetOwner(id_);
    };
};

/***************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/
    
/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
