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

#include "RTE_Components.h"

#include <stdio.h>
#include <string.h>
#include "Driver_SPI.h"

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

/* SPI Driver */
extern ARM_DRIVER_SPI Driver_SPI1;

/***************************************************************************************************
 *                              EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PRIVATE FUNCTIONS
 **************************************************************************************************/

 
/* Test data buffers */
const uint8_t testdata_out[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; 
uint8_t       testdata_in [8];
 
osEventFlagsId_t spi_evt;

void spi_callback(uint32_t event)
{   
    
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        /* Success */
        printf("<spi> 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\r\n",
               testdata_in[0],
               testdata_in[1],
               testdata_in[2],
               testdata_in[3],
               testdata_in[4],
               testdata_in[5],
               testdata_in[6],
               testdata_in[7]
               );
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
    
    spi_evt = osEventFlagsNew(NULL);

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
        /* Transmit some data */
        SPIdrv->Transfer(testdata_out, testdata_in, sizeof(testdata_out));
        
        osEventFlagsWait(spi_evt, ARM_SPI_EVENT_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
    };
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
