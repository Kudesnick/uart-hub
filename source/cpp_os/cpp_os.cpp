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
 *   File:          cpp_os.cpp
 *   Description:   see cpp_os.h
 *
 ***************************************************************************************************
 *   History:       27.05.2019 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "cpp_list.h"
#include "cpp_os.h"

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

//-- os_elements

void cpp_os::all_elements_create(void)
{
    enumerate([](cpp_os *&_el_os)
    {
        _el_os->create();

        return true;
    });
};

osStatus_t cpp_os::os_chck(osStatus_t _status)
{
    if (_status != osOK)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _status;
};

void *cpp_os::os_chck(void *_ptr)
{
    if (_ptr == NULL)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _ptr;
};

void thread_run(void *argument)
{
    static_cast<cpp_os_thread<> *>(argument)->thread_func();
}

void timer_run(void *argument)
{
    static_cast<cpp_os_timer *>(argument)->timer_func();
}

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

//-- os_elements

void cpp_os::create_os(const bool _printinfo)
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

    all_elements_create();

    os_chck(osKernelStart()); // start RTX kernel
};

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
