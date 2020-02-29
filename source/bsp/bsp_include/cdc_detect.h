/**
 *  @file       cdc_detect.h
 *
 *  @brief      Detector of USART port for CDC
 *
 *  @details
 *
 *  @author     Stulov Tikhon
 *
 *  @date       2020/02/23
 *
 *  @warning
 *
 *  @todo
 *
 */

#pragma once

/***************************************************************************************************
 *                                         INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Device.h"

/***************************************************************************************************
 *                                           DEFINITIONS                                           *
 **************************************************************************************************/

// UART Port number detect
#if !defined(UART_PORT) && (RTE_USART1 == 1) && (RTE_USART1_TX_ID != 0) &&  (RTE_USART1_RX_ID != 0)
    #define UART_PORT 1
#endif
#if !defined(UART_PORT) && (RTE_USART2 == 1) && (RTE_USART2_TX_ID != 0) &&  (RTE_USART2_RX_ID != 0)
    #define UART_PORT 2
#endif
#if !defined(UART_PORT) && (RTE_USART3 == 1) && (RTE_USART3_TX_ID != 0) &&  (RTE_USART3_RX_ID != 0)
    #define UART_PORT 3
#endif
#if !defined(UART_PORT) && (RTE_UART4  == 1) && (RTE_UART4_TX_ID  != 0) &&  (RTE_UART4_RX_ID  != 0)
    #define UART_PORT 4
#endif
#if !defined(UART_PORT) && (RTE_UART5  == 1) && (RTE_UART5_TX_ID  != 0) &&  (RTE_UART5_RX_ID  != 0)
    #define UART_PORT 5
#endif
#if !defined(UART_PORT) && (RTE_USART6 == 1) && (RTE_USART6_TX_ID != 0) &&  (RTE_USART6_RX_ID != 0)
    #define UART_PORT 6
#endif
#if !defined(UART_PORT) && (RTE_UART7  == 1) && (RTE_UART7_TX_ID  != 0) &&  (RTE_UART7_RX_ID  != 0)
    #define UART_PORT 7
#endif
#if !defined(UART_PORT) && (RTE_UART8  == 1) && (RTE_UART8_TX_ID  != 0) &&  (RTE_UART8_RX_ID  != 0)
    #define UART_PORT 8
#endif
#if !defined(UART_PORT) && (RTE_UART9  == 1) && (RTE_UART9_TX_ID  != 0) &&  (RTE_UART9_RX_ID  != 0)
    #define UART_PORT 9
#endif
#if !defined(UART_PORT) && (RTE_UART10 == 1) && (RTE_UART10_TX_ID != 0) &&  (RTE_UART10_RX_ID != 0)
    #define UART_PORT 10
#endif

/***************************************************************************************************
 *                                        END OF FILE
 **************************************************************************************************/
