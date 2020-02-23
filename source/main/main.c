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
 *   File:          main.c
 *   Description:
 *
 ***************************************************************************************************
 *   History:       13.04.2019 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdio.h>
#include "RTE_Components.h"

#include "misc_macro.h"
#include "bsp.h"
#include "cdc.h"

#if defined(RTE_CMSIS_RTOS2)
  #include "cmsis_os2.h"
#if defined(RTE_CMSIS_RTOS2_RTX5)
  #include "rtx_os.h"
#endif
#endif
#if defined(RTE_CMSIS_RTOS)
  #include "cmsis_os.h"
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

/***************************************************************************************************
 *                              EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

static osStatus_t os_chck(osStatus_t _status)
{
    if (_status != osOK)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _status;
};

static void *os_chck_ptr(void *_ptr)
{
    if (_ptr == NULL)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _ptr;
};

static void create_os(const bool _printinfo)
{
    if (_printinfo)
    {
        osVersion_t vers;

        os_chck(osKernelGetInfo(&vers, NULL, 0));

        fprintf(stderr, "<cpp_os> Operation system info:\r\n"
                "  API version: %d.%d.%d\r\n"
                "  kernel version: %d.%d.%d\r\n"
                "  kernel id: " osRtxKernelId "\r\n",
                vers.api    / 10000000, (vers.api    % 10000000) / 10000, vers.api    % 10000,
                vers.kernel / 10000000, (vers.kernel % 10000000) / 10000, vers.kernel % 10000);

#ifdef __ARMCC_VERSION
        fprintf(stderr, "<cpp_os> Compiller version: %d.%d.%d\r\n",
                __ARMCC_VERSION / 1000000,
                (__ARMCC_VERSION % 1000000) / 10000,
                __ARMCC_VERSION % 10000);
#endif
        fprintf(stderr, "<cpp_os> compilation date and time: " __DATE__ " [" __TIME__ "]\r\n");

    }

    os_chck(osKernelInitialize()); // initialize RTX

    os_chck_ptr(osThreadNew(cdc_thread, NULL, NULL));

    os_chck(osKernelStart()); // start RTX kernel
};

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

#if !defined(__CC_ARM) && defined(__ARMCC_VERSION) && !defined(__OPTIMIZE__)
    /*
    Without this directive, it does not start if -o0 optimization is used and the "main"
    function without parameters.
    see http://www.keil.com/support/man/docs/armclang_mig/armclang_mig_udb1499267612612.htm
    */
    __asm(".global __ARM_use_no_argv\n\t" "__ARM_use_no_argv:\n\t");
#endif

int main(void)
{
    fprintf(stderr, "\033[31mC\033[32mO\033[33mL\033[34mO\033[35mR\033[42m \033[0m"
            "\033[36mT\033[37mE\033[30m\033[47mS\033[0mT\r\n"); // Color test
    fprintf(stderr, "Runing main function..\r\n");

    fprintf(stderr, "BSP init..\r\n");
    bsp_init();

    fprintf(stderr, "Starting OS..\r\n");

    create_os(true);

    BRK_PTR("Main function terminated.");

    return 0;
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
