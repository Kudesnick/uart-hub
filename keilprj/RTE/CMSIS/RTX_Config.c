/*
 * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * $Revision:   V5.1.0
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Configuration
 *
 * -----------------------------------------------------------------------------
 */
 
#include "cmsis_compiler.h"
#include "rtx_os.h"
#include "misc_macro.h"
 
// OS Idle Thread
__WEAK __NO_RETURN void osRtxIdleThread (void *argument)
{
    (void)argument;

    for (;;)
    {
        __WFE();
    }
}
 
/// OS Error Callback function (weakly function)
uint32_t osRtxErrorNotify (uint32_t code, void *object_id)
{
    const char * ptr = "strange error";
    
    switch (code)
    {
        case osRtxErrorStackUnderflow:
            // Stack overflow detected for thread (thread_id=object_id)
            ptr = "Stack overflow detected for thread";
            break;
        case osRtxErrorISRQueueOverflow:
            // ISR Queue overflow detected when inserting object (object_id)
            ptr = "ISR Queue overflow detected when inserting object";
            break;
        case osRtxErrorTimerQueueOverflow:
            // User Timer Callback Queue overflow detected for timer (timer_id=object_id)
            ptr = "User Timer Callback Queue overflow detected for timer";
            break;
        case osRtxErrorClibSpace:
            // Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM
            ptr = "Standard C/C++ library libspace not available";
            break;
        case osRtxErrorClibMutex:
            // Standard C/C++ library mutex initialization failed
            ptr = "Standard C/C++ library mutex initialization failed";
            break;
        default:
            break;
    }
    
    if (object_id != NULL)
    {
        fprintf(stderr, "<main> OS %s. ID = 0x%08X.", ptr, (uint32_t)object_id);

        const char *name = osThreadGetName(object_id);
        
        if (name != NULL)
        {
             fprintf(stderr, " Name = '%s'.", name);
        }
    }
    else
    {
        fprintf(stderr, "\r\n");
    }
    
    return 0U;
}

/// System hard fault
void HardFault_Handler(void)
{
    fprintf(stderr, "<main> HardFault!\r\n");
    
    return;
}
