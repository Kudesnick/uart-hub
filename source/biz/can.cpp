#include "RTE_Components.h"
#include CMSIS_device_header

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "Driver_CAN.h"

#include "can.h"
#include "cpp_os.h"
#include "misc_macro.h"
#include "sett_def.h"

#ifdef __cplusplus
    using namespace std;
#endif

extern ARM_DRIVER_CAN Driver_CAN1;
extern ARM_DRIVER_CAN Driver_CAN2;
extern ARM_DRIVER_CAN Driver_CAN3;

const ARM_DRIVER_CAN (&CAN)[] =
{
    Driver_CAN1,
    Driver_CAN2,
    Driver_CAN3,
};

typedef enum: uint8_t
{
    MSG_PRIO_DELAYED, // Сообщение отложено
    MSG_PRIO_SEND   , // Сообщение ожидает отправки
} msg_priority_t;

cpp_os_queue<can_rx_msg_t, 128> rx_q;
cpp_os_event tx_event;
class can_thread can_threads[3] = {can_1, can_2, can_3};

//------------------------------------------------------------------------------
//  CAN Interface Signal Object Event Callback
//------------------------------------------------------------------------------
static void CAN_SignalObjectEvent(can_t _can, uint32_t obj_idx, uint32_t event)
{

    if (event == ARM_CAN_EVENT_RECEIVE) // If receive event
    {
        ARM_CAN_OBJ_CAPABILITIES can_obj_cap = CAN[_can].ObjectGetCapabilities(obj_idx);
        if (can_obj_cap.rx == 1) // If receive object event
        {
            ARM_CAN_MSG_INFO rx_msg_info;
            can_rx_msg_t msg = {.can = _can, .echo = false};
            msg.timestamp = cpp_os::get_tick_count();
        
            CAN[_can].MessageRead(obj_idx, &rx_msg_info, msg.data, sizeof(msg.data));
            
            msg.id  = rx_msg_info.id;
            msg.len = rx_msg_info.dlc;
            
            rx_q.put(&msg, NULL, 0);
        }
    }
    if (event == ARM_CAN_EVENT_SEND_COMPLETE) // If send completed event
    {
        ARM_CAN_OBJ_CAPABILITIES can_obj_cap = CAN[_can].ObjectGetCapabilities(obj_idx);
        if (can_obj_cap.tx == 1) // If transmit object event
        {
            // Send signal to user interface thread
            tx_event.set(TO_FLAG(_can));
        }
    }
}

//------------------------------------------------------------------------------
//  CAN Interface Signal Unit Event Callback
//------------------------------------------------------------------------------
static void CAN_SignalUnitEvent(can_t _can, uint32_t event)
{

    switch (event)
    {
        case ARM_CAN_EVENT_UNIT_ACTIVE:
            fprintf(stderr, "<CAN%d> ARM_CAN_EVENT_UNIT_ACTIVE\r\n", _can + 1);
            break;
        case ARM_CAN_EVENT_UNIT_WARNING:
            fprintf(stderr, "<CAN%d> ARM_CAN_EVENT_UNIT_WARNING\r\n", _can + 1);
            break;
        case ARM_CAN_EVENT_UNIT_PASSIVE:
            fprintf(stderr, "<CAN%d> ARM_CAN_EVENT_UNIT_PASSIVE\r\n", _can + 1);
            break;
        case ARM_CAN_EVENT_UNIT_BUS_OFF:
            fprintf(stderr, "<CAN%d> ARM_CAN_EVENT_UNIT_BUS_OFF\r\n", _can + 1);
            break;
    }
}

void (*obj_event[])(uint32_t obj_idx, uint32_t event) =
{
    [](uint32_t obj_idx, uint32_t event){CAN_SignalObjectEvent(can_1, obj_idx, event);},
    [](uint32_t obj_idx, uint32_t event){CAN_SignalObjectEvent(can_2, obj_idx, event);},
    [](uint32_t obj_idx, uint32_t event){CAN_SignalObjectEvent(can_3, obj_idx, event);},
};

void (*unit_event[])(uint32_t event) =
{
    [](uint32_t event){CAN_SignalUnitEvent(can_1, event);},
    [](uint32_t event){CAN_SignalUnitEvent(can_2, event);},
    [](uint32_t event){CAN_SignalUnitEvent(can_3, event);},
};


bool can_obj::set_clock(void)
{
    int32_t status;

    uint32_t clock = driver.GetClock(); // Get CAN bas clock
    
    if ((clock % (8U * sett.baud.data)) == 0U) // If CAN base clock is divisible by 8 * nominal bitrate without remainder
    {
        status = driver.SetBitrate(
            ARM_CAN_BITRATE_NOMINAL,         // Set nominal bitrate
            sett.baud.data,                  // Set nominal bitrate to configured constant value
            ARM_CAN_BIT_PROP_SEG(5U) |       // Set propagation segment to 5 time quanta
            ARM_CAN_BIT_PHASE_SEG1(1U) |     // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
            ARM_CAN_BIT_PHASE_SEG2(1U) |     // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
            ARM_CAN_BIT_SJW(sett.sjw.data)); // Resynchronization jump width is same as phase segment 2
    }
    else if ((clock % (10U * sett.baud.data)) == 0U) // If CAN base clock is divisible by 10 * nominal bitrate without remainder
    {
        status = driver.SetBitrate(
            ARM_CAN_BITRATE_NOMINAL,         // Set nominal bitrate
            sett.baud.data,                  // Set nominal bitrate to configured constant value
            ARM_CAN_BIT_PROP_SEG(7U) |       // Set propagation segment to 7 time quanta
            ARM_CAN_BIT_PHASE_SEG1(1U) |     // Set phase segment 1 to 1 time quantum (sample point at 90% of bit time)
            ARM_CAN_BIT_PHASE_SEG2(1U) |     // Set phase segment 2 to 1 time quantum (total bit is 10 time quanta long)
            ARM_CAN_BIT_SJW(sett.sjw.data)); // Resynchronization jump width is same as phase segment 2
    }
    else if ((clock % (12U * sett.baud.data)) == 0U) // If CAN base clock is divisible by 12 * nominal bitrate without remainder
    {
        status = driver.SetBitrate(
            ARM_CAN_BITRATE_NOMINAL,         // Set nominal bitrate
            sett.baud.data,                  // Set nominal bitrate to configured constant value
            ARM_CAN_BIT_PROP_SEG(7U) |       // Set propagation segment to 7 time quanta
            ARM_CAN_BIT_PHASE_SEG1(2U) |     // Set phase segment 1 to 2 time quantum (sample point at 83.3% of bit time)
            ARM_CAN_BIT_PHASE_SEG2(2U) |     // Set phase segment 2 to 2 time quantum (total bit is 12 time quanta long)
            ARM_CAN_BIT_SJW(sett.sjw.data)); // Resynchronization jump width is same as phase segment 2
    }
    else
    {
        return false;
    }
    if (status != ARM_DRIVER_OK)
    {
        return false;
    }

    return true;
}


can_obj::can_obj(can_t _can):
    can(_can),
    sett(sett_usr_param->can_sett[_can]),
    driver(CAN[_can])
{
};


bool can_obj::init(void)
{
    // Initialize CAN driver
    if (driver.Initialize(unit_event[can], obj_event[can]) != ARM_DRIVER_OK) 
    {
        return false;
    }

    // Power-up CAN controller
    if (driver.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
    {
        return false;
    }

    // Activate initialization mode
    if (driver.SetMode(ARM_CAN_MODE_INITIALIZATION) != ARM_DRIVER_OK)
    {
        return false;
    }

    // Set bitrate
    if(!set_clock())
    {
        return false;
    }

    // Get CAN driver capabilities
    ARM_CAN_CAPABILITIES can_cap = driver.GetCapabilities(); 
    // Number of receive/transmit objects
    uint32_t num_objects = can_cap.num_objects; 
    // Find first available object for receive and transmit
    for (uint8_t i = 0U; i < num_objects; i++) 
    {
        // Get object capabilities
        ARM_CAN_OBJ_CAPABILITIES can_obj_cap = driver.ObjectGetCapabilities(i); 
        
        if (can_obj_cap.rx == 1U)
        {
            // Configure receive object
            if (driver.ObjectConfigure(i, ARM_CAN_OBJ_RX) != ARM_DRIVER_OK)
            {
                return false;
            }
            // Set filter to receive messages with extended ID 0x12345678
            if (driver.ObjectSetFilter(i, ARM_CAN_FILTER_ID_MASKABLE_ADD, 0U, 0U) != ARM_DRIVER_OK)
            {
                return false;
            }
        }
        else if (can_obj_cap.tx == 1U)
        {
            // Configure transmit object
            if (driver.ObjectConfigure(i, ARM_CAN_OBJ_TX) != ARM_DRIVER_OK)
            {
                return false;
            }
            tx_num_ = i;
        }
    }

    if (   true
        && sett.loop.header.state == PARAM_DATA_TRIGGER_ON
        && sett.slnt.header.state == PARAM_DATA_TRIGGER_ON
        )
    {
        if (   false
            || can_cap.internal_loopback != 1U
            || can_cap.monitor_mode != 1U
            )
        {
            return false;
        }
        if (driver.SetMode(ARM_CAN_MODE_LOOPBACK_INTERNAL) != ARM_DRIVER_OK)
        {
            return false;
        }
    }
    else if(sett.loop.header.state == PARAM_DATA_TRIGGER_ON)
    {
        if (can_cap.external_loopback != 1U)
        {
            return false;
        }
        if (driver.SetMode(ARM_CAN_MODE_LOOPBACK_EXTERNAL) != ARM_DRIVER_OK)
        {
            return false;
        }
    }
    else if(sett.slnt.header.state == PARAM_DATA_TRIGGER_ON)
    {
        if (can_cap.monitor_mode != 1U)
        {
            return false;
        }
        if (driver.SetMode(ARM_CAN_MODE_MONITOR) != ARM_DRIVER_OK)
        {
            return false;
        }
    }
    else
    {
        if (driver.SetMode(ARM_CAN_MODE_NORMAL) != ARM_DRIVER_OK)
        {
            return false;
        }
    }

    return true;
}




void can_thread::thread_func(void)
{
    bool can_init = can.init();
    
    if(can_init)
    {
        fprintf(stderr, "<can%d> init complete!\r\n", can_num + 1);
    }
    else
    {
        fprintf(stderr, "<can%d> init error!\r\n", can_num + 1);
        exit();
    }
    
    uint32_t sleep_time = UINT32_MAX;
    uint32_t delayed_cnt = 0;
    tx_q.reset();
    new_msg_evnt.clear(1);
    
    for (;;)
    {
        if (tx_q.get_wait_forever() != cpp_os::ok)
        {
            continue;
        }
    
        // Sending messages from queue
        switch (tx_q.msg_priority)
        {
            case MSG_PRIO_SEND: // Messages for sending
                sleep_time = UINT32_MAX;
                delayed_cnt = 0;
            
                ARM_CAN_MSG_INFO msg_info;
                msg_info.id  = tx_q.msg.id;
                msg_info.dlc = tx_q.msg.len;
                msg_info.id |= (tx_q.msg.id > 0x7FF) ? 0x80000000 : 0;
                
                can.driver.MessageSend(can.tx_num, &msg_info, tx_q.msg.data, tx_q.msg.len);
                                    
                if (tx_event.wait(TO_FLAG(can_num), cpp_os::flags_wait_all_no_clr, 500) & TO_FLAG(can_num))
                {
                    tx_event.clear(TO_FLAG(can_num));
                
                    can_rx_msg_t msg = 
                    {
                        .can       = can_num,
                        .id        = tx_q.msg.id,
                        .len       = tx_q.msg.len,
                        .timestamp = cpp_os::get_tick_count(),
                        .echo      = true,
                    };
                    memcpy(msg.data, tx_q.msg.data, (tx_q.msg.len <= sizeof(msg.data)) ? tx_q.msg.len : sizeof(msg.data));
                    
                    rx_q.put(&msg, NULL, 0);
                    
                    if (tx_q.msg.repeat_cnt != 1)
                    {
                        can_tx_msg_t msg = tx_q.msg;
                        if (msg.timestamp == 0)
                        {
                            msg.timestamp = cpp_os::get_tick_count();
                        }
                        else
                        {
                            msg.timestamp += msg.interval;
                        }
                        if (msg.repeat_cnt != 0) msg.repeat_cnt--;
                        tx_q.put(&msg, MSG_PRIO_DELAYED, 0);
                    }
                }
                else
                {
                    fprintf(stderr, "<can%d> sending timeout error!\r\n", can_num + 1);
                }
                
                new_msg_evnt.clear(1);
                
            break;
                
            case MSG_PRIO_DELAYED: // Delayed messages
                uint32_t tick_count = cpp_os::get_tick_count();
            
                if ((tick_count - tx_q.msg.timestamp) > tx_q.msg.interval)
                {
                    tx_q.put(&tx_q.msg, MSG_PRIO_SEND, 0);
                }
                else
                {
                    tx_q.put(&tx_q.msg, MSG_PRIO_DELAYED, 0);
                    delayed_cnt++;
                    
                    uint32_t tmp_sleep_time = tx_q.msg.timestamp + tx_q.msg.interval - tick_count;
                    if (tmp_sleep_time < sleep_time)
                    {
                        sleep_time = tmp_sleep_time;
                    }
                    
                    if (delayed_cnt >= tx_q.get_count()) // Were are delayed messages in queue only
                    {
                        delayed_cnt = 0;
                    
                        new_msg_evnt.wait(1, cpp_os::flags_wait_all, sleep_time);
                    }
                }
            break;
        }
    }
};

can_thread::can_thread(can_t _can):
    can_num(_can),
    can(_can),
    cpp_os_thread(false, cpp_os::priority_high, "can_thread")
{};

osStatus_t can_thread::put(const can_tx_msg_t &msg)
{
    return tx_q.put(&msg, MSG_PRIO_SEND, 0);
    new_msg_evnt.set(1);
};

void can_thread::reset(void)
{
    tx_q.reset();
};

#if (0)
class : public cpp_os_thread<>
{
private:

    void thread_func(void)
    {
        for (uint8_t i = 0; i < countof(can_threads); i++)
        {
            can_threads[i].run();
            
            can_tx_msg_t msg = 
            {
                .id         = 0x201,
                .len        = 8,
                .data       = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88},
                .timestamp  = 0,
                .interval   = 1000,
                .repeat_cnt = 0,
            };
            msg.id += i;
            
            can_threads[i].put(msg);
            
            msg.id |= 0x3000;
            msg.repeat_cnt = 3;
            
            can_threads[i].put(msg);
        }

        for(;;)
        {
            if (rx_q.get_wait_forever() == cpp_os::ok)
            {
                const can_rx_msg_t &msg = rx_q.msg;
            
                printf("%c CAN%d 0x%08X %d",
                       (msg.echo) ? '<' : '>',
                       msg.can + 1,
                       msg.id/* & ((1 << 29) - 1)*/,
                       msg.len);
                for (uint8_t i = 0; i < msg.len; i++)
                {
                    printf(" %02X", msg.data[i]);
                }
                printf(" %10d\r\n", msg.timestamp);
            }
        }
    }

public:
    using cpp_os_thread::cpp_os_thread;
} can_test = {false, cpp_os::priority_normal, "can_test"};
#endif
