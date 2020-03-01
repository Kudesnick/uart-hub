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
 *   File:          spi.c
 *   Description:
 *
 ***************************************************************************************************
 *   History:       24.02.2020 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "cmsis_compiler.h"
#include "rtx_os.h"
#include "Driver_SPI.h"

#include "os_chk.h"
#include "spi.h"

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define QUEUE_CNT 128

/***************************************************************************************************
 *                                      PRIVATE TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                               PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       PRIVATE DATA
 **************************************************************************************************/

osEventFlagsId_t   spi_evt;
osMessageQueueId_t spi_msg_q;

spi_msg_t data_in, data_out;

/***************************************************************************************************
 *                                       PUBLIC DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                      EXTERNAL DATA
 **************************************************************************************************/

/* SPI Driver */
extern ARM_DRIVER_SPI Driver_SPI1;

/***************************************************************************************************
 *                              EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

__WEAK bool spi_get_data(spi_msg_t _data)
{
    printf("<spi> get data 0x%08X.\r\n", _data);
    
    return true;
}

void spi_callback(uint32_t event)
{   
    
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        spi_get_data(data_in);
        break;
    }

    osEventFlagsSet(spi_evt, event);
}

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

void spi_thread(void* arg)
{
    (void)arg;
    
    spi_evt   = os_chck_ptr(osEventFlagsNew(NULL));
    spi_msg_q = os_chck_ptr(osMessageQueueNew(QUEUE_CNT, sizeof(spi_msg_t), NULL));

    ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

    uint32_t result = ARM_DRIVER_OK;
    
    /* Initialize the SPI driver */
    result = SPIdrv->Initialize(spi_callback);
    if (result != ARM_DRIVER_OK)
    {
        printf("<spi> Error Initialize\r\n");
        osThreadExit();
    }
    
    /* Power up the SPI peripheral */
    result = SPIdrv->PowerControl(ARM_POWER_FULL);
    if (result != ARM_DRIVER_OK)
    {
        printf("<spi> Error PowerControl\r\n");
        osThreadExit();
    }
    
    /* Configure the SPI to Slave, 8-bit mode */
    result = SPIdrv->Control(ARM_SPI_MODE_SLAVE  |
                    ARM_SPI_CPOL0_CPHA0 |
                    ARM_SPI_MSB_LSB     |
                    ARM_SPI_SS_SLAVE_HW |
                    ARM_SPI_DATA_BITS(8), 1000000);
    if (result != ARM_DRIVER_OK)
    {
        printf("<spi> Error Control\r\n");
        osThreadExit();
    }
    else
    {
        printf("<spi> ARM_DRIVER_OK\r\n");
    }
    
    /* thread loop */
    for (;;)
    {
        spi_msg_t data;
        
        if (osMessageQueuePut(spi_msg_q, &data, NULL, 0) != osOK)
        {
            data = 0;
        }
        
        SPIdrv->Transfer(&data, &data_in, sizeof(spi_msg_t));
        osEventFlagsWait(spi_evt, ARM_SPI_EVENT_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
    };
}

bool spi_set_data(spi_msg_t _data)
{
    if (osMessageQueuePut(spi_msg_q, &_data, 0, 0) != osOK)
    {
        printf("<spi> msg put error.\r\n");
        
        return false;
    }
    
    return true;
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
