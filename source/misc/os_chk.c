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
 *   File:          os_chk.c
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
#include <stdbool.h>

#include "rtx_os.h"

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

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

osStatus_t os_chck(osStatus_t _status)
{
    if (_status != osOK)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _status;
};

void *os_chck_ptr(void *_ptr)
{
    if (_ptr == NULL)
    {
        fprintf(stderr, "<cpp_os> os_chck not complete.\r\n");
    };

    return _ptr;
};

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
