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
 *   File:          cdc_test.cpp
 *   Description:
 *
 ***************************************************************************************************
 *   History:       16.02.2020 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "cpp_os.h"
#include "bsp_cdc.h"
#include "Driver_USART.h"

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

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

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_USART Driver_USART3;
extern ARM_DRIVER_USART Driver_USART4;
extern ARM_DRIVER_USART Driver_USART5;
extern ARM_DRIVER_USART Driver_USART6;
extern ARM_DRIVER_USART Driver_USART7;
extern ARM_DRIVER_USART Driver_USART8;
extern ARM_DRIVER_USART Driver_USART9;
extern ARM_DRIVER_USART Driver_USART10;

/***************************************************************************************************
 *                              EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

static void _usart_event(uint32_t _event)
{
    printf("<cdc> usart event %x08", _event);
}


class : public cpp_os_timer
{
private:
    bool last_result = false;

    void timer_func()
    {
        bool result = bsp_cdc_is_configured();

        if (!result)
        {
            if (last_result)
            {
                fprintf(stderr, "<cdc> USB VCP stopped\r\n");

                bsp_cdc_deinit();
            }
            else
            {
                bsp_cdc_init();
            }
        }
        else if (!last_result)
        {
            fprintf(stderr, "<cdc> USB CDC runing\r\n");
            fprintf(stderr, "If you use Putty, you must select this settings:\r\n"
                    "  - in bookmark 'connection/serial' disable 'flow control';\r\n"
                    "  - in bookmark 'Terminal' check 'implict CR in every LF',\r\n"
                    "    set 'Force on' in 'Local echo' and 'Local line ending'.\r\n"
                   );

        }

        last_result = result;
    };

public:
    using cpp_os_timer::cpp_os_timer;
} cdc = {1000, true, "cdc"};

class : public cpp_os_thread<>
{
private:

    void thread_func(void)
    {
        cdc.start();
        
        uint32_t usart_err;
        
        usart_err = Driver_USART1.Initialize(_usart_event);
        if (usart_err != ARM_DRIVER_OK)
        {
            printf("<cdc> usart Initialize error: %x08", usart_err);
        }
        
        usart_err = Driver_USART1.PowerControl(ARM_POWER_FULL);
        if (usart_err != ARM_DRIVER_OK)
        {
            printf("<cdc> usart PowerControl error: %x08", usart_err);
        }
 
        usart_err = Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
                              ARM_USART_DATA_BITS_8       |
                              ARM_USART_PARITY_NONE       |
                              ARM_USART_STOP_BITS_1       |
                              ARM_USART_MODE_SINGLE_WIRE  |
                              ARM_USART_FLOW_CONTROL_NONE, 9600);
        if (usart_err != ARM_DRIVER_OK)
        {
            printf("<cdc> usart Control error: %x08", usart_err);
        }

        for (;;)
        {
            static char buf[256];

            memset(buf, 0, sizeof(buf));

            while (gets(buf) != buf);

            uint16_t len = strlen(buf);

            if (false
                || len < 1
                || len >= sizeof(buf)
               )
            {
                continue;
            }

            printf("<cdc> echo: %s\r\n", buf);
            
            Driver_USART1.Send(buf, len);
        }
    }

public:
    using cpp_os_thread::cpp_os_thread;
} cdc0 = {"cdc0"};

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/