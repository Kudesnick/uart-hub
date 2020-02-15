/***************************************************************************************************
 *   Project:      
 *   Author:       
 ***************************************************************************************************
 *   Distribution: 
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          can.h
 *   Description:  
 *
 ***************************************************************************************************
 *   History:       07.01.2020 - file created
 *
 **************************************************************************************************/
 
#pragma once
 
/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdlib.h>
#include <stdint.h>

#include "Driver_CAN.h"

#include "cpp_os.h"
#include "sett_def.h"


#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define DATA_LEN (8U)
#define TO_FLAG(n) (1U << n)

/***************************************************************************************************
 *                                      PUBLIC TYPES
 **************************************************************************************************/

typedef enum: uint8_t
{
    can_1,
    can_2,
    can_3,
} can_t;

typedef __PACKED_STRUCT
{
    can_t    can           ;
    uint32_t id            ;
    uint8_t  len           ;
    uint8_t  data[DATA_LEN];
    uint32_t timestamp     ;
    bool     echo          ;
} can_rx_msg_t;

typedef __PACKED_STRUCT
{
    uint32_t id            ;
    uint8_t  len           ;
    uint8_t  data[DATA_LEN];
    uint32_t timestamp     ;
    uint16_t interval      ;
    uint16_t repeat_cnt    ;
} can_tx_msg_t;

/***************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/

class can_obj
{
private:
    const can_sett_t &sett;
    const can_t can;
    
    uint8_t tx_num_;
        
    bool set_clock(void);
    
public:
    const uint8_t &tx_num = tx_num_;
    const ARM_DRIVER_CAN &driver;

    can_obj(can_t _can);    
    bool init(void);
};


class can_thread : public cpp_os_thread<>
{
private:
    cpp_os_queue<can_tx_msg_t, 128> tx_q;
    cpp_os_event new_msg_evnt;
    can_obj can;
    
    void thread_func(void);
public:
    
    const can_t can_num;

    can_thread(can_t _can);
    osStatus_t put(const can_tx_msg_t &msg);
    void reset(void);
};

extern class can_thread can_threads[3];

/***************************************************************************************************
 *                                PUBLIC FUNCTION PROTOTYPES
 **************************************************************************************************/
 
/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/