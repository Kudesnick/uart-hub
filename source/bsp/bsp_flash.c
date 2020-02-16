/***************************************************************************************************
 *   Project:
 *   Author:        Zelenin Alex
 ***************************************************************************************************
 *   Distribution:
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          bsp_flash.c
 *   Description:
 *
 ***************************************************************************************************
 *   History:       <02.04.2019> - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "bsp_flash.h"
#include "stm32f4xx_hal.h"

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define MEMORY_SIZE_REGISTR ((uint32_t)(*(uint32_t *)0x1FFF7A22))
#define FLASH_START_ADDR 0x08000000
#define MEMORY_0_3_SECTOR_SIZE  (16 * 1024)
#define MEMORY_4_SECTOR_SIZE    (64 * 1024)
#define MEMORY_5_11_SECTOR_SIZE (128 * 1024)
#define MEMORY_PAGE_CNT 11

/***************************************************************************************************
 *                                      PRIVATE TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                               PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       PRIVATE DATA
 **************************************************************************************************/

static bsp_flash_async_callback_t callback;
static bsp_flash_addr_t addr;
static const void *src_ptr;
static bsp_flash_size_t len;

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

bsp_result_t check_alight_addres_and_data_len(uint32_t _addr, bsp_flash_size_t _len)
{
    if (_addr % 2 || _len % 2)
    {
        return BSP_RESULT_ERR;
    }
    return BSP_RESULT_OK;
}

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

bsp_flash_size_t bsp_flash_get_size(void)
{
    uint32_t temp;

    temp = MEMORY_SIZE_REGISTR;
    temp &= 0xFFFF;

    return temp * 1024;
}

bsp_flash_page_t bsp_flash_page_cnt(void)
{
    return MEMORY_PAGE_CNT;
}

bsp_flash_size_t bsp_flash_get_page_size(bsp_flash_page_t _page)
{
    const uint32_t page_size_arr[] =
    {
        MEMORY_0_3_SECTOR_SIZE,
        MEMORY_0_3_SECTOR_SIZE,
        MEMORY_0_3_SECTOR_SIZE,
        MEMORY_0_3_SECTOR_SIZE,
        MEMORY_4_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE
    };

    if (_page > MEMORY_PAGE_CNT)
    {
        return 0;
    }

    return page_size_arr[_page];
}

bsp_flash_addr_t bsp_flash_get_page_addr(bsp_flash_page_t _page)
{
    uint32_t temp = FLASH_START_ADDR;
    const uint32_t page_addr_arr[] =
    {
        NULL,
        MEMORY_0_3_SECTOR_SIZE,
        MEMORY_0_3_SECTOR_SIZE * 2,
        MEMORY_0_3_SECTOR_SIZE * 3,
        MEMORY_4_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE,
        MEMORY_5_11_SECTOR_SIZE * 2,
        MEMORY_5_11_SECTOR_SIZE * 3,
        MEMORY_5_11_SECTOR_SIZE * 4,
        MEMORY_5_11_SECTOR_SIZE * 5,
        MEMORY_5_11_SECTOR_SIZE * 6,
        MEMORY_5_11_SECTOR_SIZE * 7
    };

    if (_page > MEMORY_PAGE_CNT)
    {
        return (bsp_flash_addr_t)NULL;
    }

    temp += page_addr_arr[_page];
    return (bsp_flash_addr_t)temp;
}

bsp_flash_page_t bsp_flash_get_page_num(bsp_flash_addr_t _addr)
{
    uint8_t page = MEMORY_PAGE_CNT + 1;
    uint32_t temp;

    for (uint8_t i = 0; i <= MEMORY_PAGE_CNT; i++)
    {
        temp = (uint32_t)bsp_flash_get_page_addr(i) + bsp_flash_get_page_size(i);
        if (temp > (uint32_t)_addr)
        {
            page = i;
            break;
        }
    }

    return page;
}

bsp_result_t bsp_flash_is_start_page_addr(bsp_flash_addr_t _addr)
{
    bsp_result_t res = BSP_RESULT_ERR;

    for (uint8_t i = 0; i <= MEMORY_PAGE_CNT; i++)
    {
        if (bsp_flash_get_page_addr(i) == _addr)
        {
            res = BSP_RESULT_OK;
            break;
        }
    }

    return res;
}

bsp_result_t bsp_flash_erase_sync(bsp_flash_page_t _page)
{
    bsp_result_t res;
    uint32_t sector_error;
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init_struct =
    {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .NbSectors = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };

    erase_init_struct.Sector = _page;

    HAL_FLASH_Unlock();
    status = HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
    HAL_FLASH_Lock();

    res = status == HAL_OK ? BSP_RESULT_OK : BSP_RESULT_ERR;

    return res;
}

bsp_result_t bsp_flash_write_word_sync(bsp_flash_addr_t _addr, bsp_flash_word_t _data)
{
    bsp_result_t res;
    HAL_StatusTypeDef status;

    res = check_alight_addres_and_data_len((uint32_t)_addr, 2);
    if (res == BSP_RESULT_ERR)
    {
        return res;
    }

    HAL_FLASH_Unlock();
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)_addr, _data);
    HAL_FLASH_Lock();

    res = status == HAL_OK ? BSP_RESULT_OK : BSP_RESULT_ERR;

    return res;
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
