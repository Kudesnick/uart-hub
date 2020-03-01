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
 *   File:          uart.c
 *   Description:
 *
 ***************************************************************************************************
 *   History:       28.02.2020 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
    using namespace std;
#endif

#include "RTE_Device.h"
#include "rtx_os.h"
#include "Driver_USART.h"
#include "spi.h"
#include "os_chk.h"

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define QUEUE_CNT 128
#define BUF_CNT   16
#define TOUT_ERR  1000

/***************************************************************************************************
 *                                      PRIVATE TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                               PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       PRIVATE DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                       PUBLIC DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                      EXTERNAL DATA
 **************************************************************************************************/

static void usart_event(uint8_t _n, uint32_t _event);

// UART Drivers detect
#if (RTE_USART1 == 1) && (RTE_USART1_TX_ID != 0) &&  (RTE_USART1_RX_ID == 0)
    extern ARM_DRIVER_USART Driver_USART1;
    static void usart1_event(uint32_t event)
    {
        usart_event(1, event);
    }
    #undef  UART_CNT
    #define UART_CNT 1
#endif
#if (RTE_USART2 == 1) && (RTE_USART2_TX_ID != 0) &&  (RTE_USART2_RX_ID == 0)
    extern ARM_DRIVER_USART Driver_USART2;
    static void usart2_event(uint32_t event)
    {
        usart_event(2, event);
    }
    #undef  UART_CNT
    #define UART_CNT 2
#endif
#if (RTE_USART3 == 1) && (RTE_USART3_TX_ID != 0) &&  (RTE_USART3_RX_ID == 0)
    extern ARM_DRIVER_USART Driver_USART3;
    static void usart3_event(uint32_t event)
    {
        usart_event(3, event);
    }
    #undef  UART_CNT
    #define UART_CNT 3
#endif
#if (RTE_UART4  == 1) && (RTE_UART4_TX_ID  != 0) &&  (RTE_UART4_RX_ID  == 0)
    extern ARM_DRIVER_USART Driver_USART4;
    static void usart4_event(uint32_t event)
    {
        usart_event(4, event);
    }
    #undef  UART_CNT
    #define UART_CNT 4
#endif
#if (RTE_UART5  == 1) && (RTE_UART5_TX_ID  != 0) &&  (RTE_UART5_RX_ID  == 0)
    extern ARM_DRIVER_USART Driver_USART5;
    static void usart5_event(uint32_t event)
    {
        usart_event(5, event);
    }
    #undef  UART_CNT
    #define UART_CNT 5
#endif
#if (RTE_USART6 == 1) && (RTE_USART6_TX_ID != 0) &&  (RTE_USART6_RX_ID == 0)
    extern ARM_DRIVER_USART Driver_USART6;
    static void usart6_event(uint32_t event)
    {
        usart_event(6, event);
    }
    #undef  UART_CNT
    #define UART_CNT 6
#endif
#if (RTE_UART7  == 1) && (RTE_UART7_TX_ID  != 0) &&  (RTE_UART7_RX_ID  == 0)
    extern ARM_DRIVER_USART Driver_USART7;
    static void usart7_event(uint32_t event)
    {
        usart_event(7, event);
    }
    #undef  UART_CNT
    #define UART_CNT 7
#endif
#if (RTE_UART8  == 1) && (RTE_UART8_TX_ID  != 0) &&  (RTE_UART8_RX_ID  == 0)
    extern ARM_DRIVER_USART Driver_USART8;
    static void usart8_event(uint32_t event)
    {
        usart_event(8, event);
    }
    #undef  UART_CNT
    #define UART_CNT 8
#endif
#if (RTE_UART9  == 1) && (RTE_UART9_TX_ID  != 0) &&  (RTE_UART9_RX_ID  == 0)
    extern ARM_DRIVER_USART Driver_USART9;
    static void usart9_event(uint32_t event)
    {
        usart_event(9, event);
    }
    #undef  UART_CNT
    #define UART_CNT 9
#endif
#if (RTE_UART10 == 1) && (RTE_UART10_TX_ID != 0) &&  (RTE_UART10_RX_ID == 0)
    extern ARM_DRIVER_USART Driver_USART10;
    static void usart10_event(uint32_t event)
    {
        usart_event(10, event);
    }
    #undef  UART_CNT
    #define UART_CNT 10
#endif

struct
{ 
    ARM_DRIVER_USART              *drv;
    const ARM_USART_SignalEvent_t  evt;
    uint8_t rx_buf;
} uart[UART_CNT] =
{
#if (UART_CNT >= 1)
#if (RTE_USART1 == 1) && (RTE_USART1_TX_ID != 0) &&  (RTE_USART1_RX_ID == 0)
    {&Driver_USART1, usart1_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 2)
#if (RTE_USART2 == 1) && (RTE_USART2_TX_ID != 0) &&  (RTE_USART2_RX_ID == 0)
    {&Driver_USART2, usart2_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 3)
#if (RTE_USART3 == 1) && (RTE_USART3_TX_ID != 0) &&  (RTE_USART3_RX_ID == 0)
    {&Driver_USART3, usart3_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 4)
#if (RTE_UART4  == 1) && (RTE_UART4_TX_ID  != 0) &&  (RTE_UART4_RX_ID  == 0)
    {&Driver_USART4, usart4_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 5)
#if (RTE_UART5  == 1) && (RTE_UART5_TX_ID  != 0) &&  (RTE_UART5_RX_ID  == 0)
    {&Driver_USART5, usart5_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 6)
#if (RTE_USART6 == 1) && (RTE_USART6_TX_ID != 0) &&  (RTE_USART6_RX_ID == 0)
    {&Driver_USART6, usart6_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 7)
#if (RTE_UART7  == 1) && (RTE_UART7_TX_ID  != 0) &&  (RTE_UART7_RX_ID  == 0)
    {&Driver_USART7, usart7_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 8)
#if (RTE_UART8  == 1) && (RTE_UART8_TX_ID  != 0) &&  (RTE_UART8_RX_ID  == 0)
    {&Driver_USART8, usart8_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 9)
#if (RTE_UART9  == 1) && (RTE_UART9_TX_ID  != 0) &&  (RTE_UART9_RX_ID  == 0)
    {&Driver_USART9, usart9_event},
#else
    {NULL, NULL},
#endif
#endif
#if (UART_CNT >= 10)
#if (RTE_UART10 == 1) && (RTE_UART10_TX_ID != 0) &&  (RTE_UART10_RX_ID == 0)
    {&Driver_USART10, usart10_event},
#else
    {NULL, NULL},
#endif
#endif
};

osMessageQueueId_t uart_msg_q;
uint32_t           mask_valid;
osThreadId_t       uart_thr_id;

/***************************************************************************************************
 *                              EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

bool spi_get_data(spi_msg_t _data)
{
    return (osMessageQueuePut(uart_msg_q, &_data, 0, 0) == osOK);
}

static void usart_event(uint8_t _n, uint32_t _event)
{
    if (_event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        osThreadFlagsSet(uart_thr_id, 1U << _n);
    }
 
    if (_event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        spi_set_data((1U << (_n + 8)) | uart[_n].rx_buf);
        uart[_n].drv->Receive(&uart[_n].rx_buf, 1);
    }
}

void uart_thread(void *arg)
{
    (void)arg;
    
    uart_thr_id = osThreadGetId();
    uart_msg_q = os_chck_ptr(osMessageQueueNew(QUEUE_CNT, sizeof(uint16_t), NULL));
    
    // Drivers init
    for (uint8_t i = 0; i < UART_CNT; i++)
    {
        ARM_DRIVER_USART *drv = uart[i].drv;
        
        if (drv != NULL)
        {
            mask_valid |= 1U << i;
            
            uint32_t usart_err;
            
            usart_err = drv->Initialize(uart[i].evt);
            if (usart_err != ARM_DRIVER_OK)
            {
                printf("<uart> usart %d Initialize error: %x08", i, usart_err);
                osThreadExit();
            }
            
            usart_err = drv->PowerControl(ARM_POWER_FULL);
            if (usart_err != ARM_DRIVER_OK)
            {
                printf("<uart> usart %d PowerControl error: %x08", i, usart_err);
                osThreadExit();
            }
    
            usart_err = drv->Control(ARM_USART_MODE_SINGLE_WIRE |
                                     ARM_USART_DATA_BITS_8      |
                                     ARM_USART_PARITY_NONE      |
                                     ARM_USART_STOP_BITS_1, 9600);
            if (usart_err != ARM_DRIVER_OK)
            {
                printf("<uart> usart %d Control error: %x08", i, usart_err);
                osThreadExit();
            }
            
            usart_err = drv->Control(ARM_USART_CONTROL_TX, 1);
            if (usart_err != ARM_DRIVER_OK)
            {
                printf("<uart> usart %d TX Control error: %x08", i, usart_err);
                osThreadExit();
            }
        }
    }
    printf("<uart> ARM_DRIVER_OK\r\n");

    osThreadFlagsSet(uart_thr_id, mask_valid);

    // Main cycle
    for(;;)
    {
        
        spi_msg_t data;
        
        if (false
            || data != 0
            || osMessageQueueGet(uart_msg_q, &data, NULL, osWaitForever) == osOK
           )
        {
            const uint32_t mask = (data >> 8) & mask_valid;
            
            if (osThreadFlagsWait(mask, osFlagsWaitAll, TOUT_ERR) == mask)
            {
                // TO_DO sending data
            }
            else
            {
                printf("<uart> Sending timeout error.\r\n");
            }
        }
    }
};

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
