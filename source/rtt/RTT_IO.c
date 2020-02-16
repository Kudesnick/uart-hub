/***************************************************************************************************
 *   Project:       stm_commander
 *   Author:        Stulov Tikhon
 ***************************************************************************************************
 *   Distribution:  
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          RTT_IO.c
 *   Description:   
 *
 ***************************************************************************************************
 *   History:       02.06.2019 - file created
 *
 **************************************************************************************************/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>

#ifdef RTE_CMSIS_RTOS2
#include "cmsis_os2.h" // ARM::CMSIS:RTOS:Keil RTX
#endif

#include "bsp_cdc.h"
#include "rtx_os.h"

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

/// Консольный ввод/вывод
#define USR_PUT_RTT 1 ///< Вывод консоли в RTT
#define USR_PUT_ITM 1 ///< Вывод консоли в SWO
#define USR_PUT_VCP 1 ///< Вывод консоли в VCP
#define USR_ERR_RTT 1 ///< Отладочный вывод в RTT
#define USR_ERR_ITM 1 ///< Отладочный вывод в SWO
#define USR_ERR_VCP 1 ///< Отладочный вывод в VCP
#define USR_GET_RTT 1 ///< Консольный ввод из RTT
#define USR_GET_ITM 1 ///< Консольный ввод из SWO
#define USR_GET_VCP 1 ///< Консольный ввод из VCP

#if (USR_PUT_ITM != 0) || (USR_GET_ITM != 0)
    /* ITM registers */
    #define ITM_PORT0_U8          (*((volatile uint8_t  *)0xE0000000))
    #define ITM_PORT0_U32         (*((volatile uint32_t *)0xE0000000))
    #define ITM_TER               (*((volatile uint32_t *)0xE0000E00))
    #define ITM_TCR               (*((volatile uint32_t *)0xE0000E80))
    #ifndef ITM_TCR_ITMENA_Msk
    #define ITM_TCR_ITMENA_Msk    (1UL << 0)
    #endif
#endif

/***************************************************************************************************
 *                                      PRIVATE TYPES
 **************************************************************************************************/

typedef enum : uint8_t
{
    CON_NA,
    CON_RTT,
    CON_ITM,
    CON_VCP,
} con_channel_t;

/***************************************************************************************************
 *                                      PRIVATE DATA
 **************************************************************************************************/

con_channel_t curr_con = CON_NA;

/***************************************************************************************************
 *                                     PRIVATE FUNCTIONS
 **************************************************************************************************/

#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDOUT) || defined(RTE_Compiler_IO_STDERR)

    #if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDOUT_User) || defined(RTE_Compiler_IO_STDERR_User)
    
        int usr_put_char(int ch)
        {
            #if (USR_PUT_RTT == 0) && (USR_PUT_ITM == 0) && (USR_PUT_VCP == 0)
                void(ch);
                
                return -1;
            #else
                
                int result = -1;
                
                #if (USR_PUT_RTT != 0)
                    if ((curr_con == CON_NA) || (curr_con == CON_RTT))
                    {
                        extern unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c);
                        int tmp = SEGGER_RTT_PutChar(0, ch);
                        result = (tmp >= 0) ? tmp : result;
                    }
                #endif
                
                #if (USR_PUT_ITM != 0)
                    if ((curr_con == CON_NA) || (curr_con == CON_ITM))
                    {
                        if ((ITM_TCR & ITM_TCR_ITMENA_Msk) && /* ITM enabled */
                            (ITM_TER & (1UL << 0)        ))   /* ITM Port #0 enabled */
                        {
                            while (ITM_PORT0_U32 == 0);
                            __NOP();
                            ITM_PORT0_U8 = (uint8_t)ch;
                        }
                        result = (ch >= 0) ? 1 : result;
                    }
                #endif
                
                #if (USR_PUT_VCP != 0)
                    if ((curr_con == CON_NA) || (curr_con == CON_VCP))
                    {
                        int tmp = bsp_cdc0_put_char(ch);
                        result = (tmp >= 0) ? tmp : result;
                    }
                #endif
                
                return result;
            #endif
        }
        
        int usr_put_err_char(int ch)
        {
            #if (USR_ERR_RTT == 0) && (USR_ERR_ITM == 0) && (USR_ERR_VCP == 0)
                void(ch);
                
                return -1;
            #else
                
                int result = -1;
                
                #if (USR_ERR_RTT != 0)
                    if ((curr_con == CON_NA) || (curr_con != CON_RTT))
                    {
                        extern unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c);
                        int tmp = SEGGER_RTT_PutChar(0, ch);
                        result = (tmp >= 0) ? tmp : result;
                    }
                #endif
                
                #if (USR_ERR_ITM != 0)
                    if ((curr_con == CON_NA) || (curr_con != CON_ITM))
                    {
                        if ((ITM_TCR & ITM_TCR_ITMENA_Msk) && /* ITM enabled */
                            (ITM_TER & (1UL << 0)        ))   /* ITM Port #0 enabled */
                        {
                            while (ITM_PORT0_U32 == 0);
                            __NOP();
                            ITM_PORT0_U8 = (uint8_t)ch;
                        }
                        result = (ch >= 0) ? 1 : result;
                    }
                #endif
                
                #if (USR_ERR_VCP != 0)
                    if ((curr_con == CON_NA) || (curr_con != CON_VCP))
                    {
                        int tmp = bsp_cdc0_put_char(ch);
                        result = (tmp >= 0) ? tmp : result;
                    }
                #endif
                
                return result;
            #endif
        }
        
        #ifdef RTE_Compiler_IO_TTY_User
        void ttywrch (int ch)
        {
            usr_put_char(ch);
        }
        #endif
        
        #ifdef RTE_Compiler_IO_STDOUT_User
        int stdout_putchar(int ch)
        {    
            return usr_put_char(ch);
        }
        #endif
        
        #ifdef RTE_Compiler_IO_STDERR_User
        int stderr_putchar (int ch)
        {
            return usr_put_err_char(ch);
        }
        #endif
    #endif

#endif



#if defined(RTE_Compiler_IO_TTY) || defined(RTE_Compiler_IO_STDIN)

    #if defined(RTE_Compiler_IO_TTY_User) || defined(RTE_Compiler_IO_STDIN_User)
    
        #if (!defined(RTE_Compiler_IO_TTY_ITM)    && \
             !defined(RTE_Compiler_IO_STDIN_ITM)  && \
             !defined(RTE_Compiler_IO_STDOUT_ITM) && \
             !defined(RTE_Compiler_IO_STDERR_ITM))
            
            volatile int32_t ITM_RxBuffer;
            volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;
        
        #endif
    
        int stdin_getchar (void)
        {
            #if (USR_GET_RTT == 0) && (USR_GET_ITM == 0) && (USR_GET_VCP == 0)
                return -1;
            #else
                int result = -1;
                
                // Чтобы не читать данные из двух конкурирующих источников
                con_channel_t tmp_con = CON_NA;
                
                do
                {

                #if (USR_GET_RTT != 0)
                    if ((tmp_con == CON_NA) || (tmp_con == CON_RTT))
                    {
                        extern int SEGGER_RTT_GetKey(void);
                        result = SEGGER_RTT_GetKey();
                        
                        if (result >= 0)
                        {
                            tmp_con = CON_RTT;
                        }
                    }
                #endif
                
                #if (USR_GET_ITM != 0)
                    if ((tmp_con == CON_NA) || (tmp_con == CON_ITM))
                    {
                        if (result < 0)
                        {
                            extern volatile int32_t ITM_RxBuffer;
                            
                            if (ITM_RxBuffer != ITM_RXBUFFER_EMPTY)
                            {
                                result = ITM_RxBuffer;
                                ITM_RxBuffer = ITM_RXBUFFER_EMPTY;  /* ready for next character */
                            }
                            
                            if (result >= 0)
                            {
                                tmp_con = CON_ITM;
                            }
                        }
                    }
                #endif
                
                #if (USR_GET_VCP != 0)
                    if ((tmp_con == CON_NA) || (tmp_con == CON_VCP))
                    {
                        if (result < 0)
                        {
                            result = bsp_cdc0_get_char();
                        }
                        
                        if (result >= 0)
                        {
                            tmp_con = CON_VCP;
                        }
                    }
                #endif
            
                #ifdef RTE_CMSIS_RTOS2
                    if (result == -1)
                    {
                        osThreadYield();
                    }
                #endif
                }
                while (result == -1);
                
                // Некоторые терминалы посылают только один символ. Для работы gets требуется именно \n
                if (result == '\r') result = '\n';
                
                // Фиксируем источник-приемник
                if (tmp_con != CON_NA)
                {
                    curr_con = tmp_con;
                }
                
                return result;
            #endif   
        }
    #endif

#endif

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
