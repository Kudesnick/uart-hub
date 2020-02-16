/**
 *  @file       bsp.c
 *
 *  @brief      Файл реализации
 *
 *  @details
 *
 *  @author     Zelenin Alex
 *
 *  @date       2019/10/03
 *
 *  @warning
 *
 *  @todo
 *
 */

/***************************************************************************************************
 *                                         INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "stm32f4xx_hal.h" // Device header

#include <stdio.h>

#include "bsp.h"
#include "bsp_flash.h"

#ifdef RTE_CMSIS_RTOS2
    // see https://www.keil.com/pack/doc/STM32Cube/General/html/cubemx__r_t_x.html
    #include "cmsis_os2.h"      // ARM::CMSIS:RTOS:Keil RTX
#endif

/***************************************************************************************************
 *                                           DEFINITIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                          PRIVATE TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                           PRIVATE DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                           PUBLIC DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                          EXTERNAL DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                      PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                      PRIVATE FUNCTIONS
 **************************************************************************************************/

#ifdef RTE_CMSIS_RTOS2
uint32_t HAL_GetTick(void)
{
    static uint32_t ticks = 0U;

    if (osKernelGetState() == osKernelRunning)
    {
        return osKernelGetTickCount();
    }

    // If Kernel is not running wait approximately 1 ms then increment
    // and return auxiliary tick counter value
    for (uint32_t i = (SystemCoreClock >> 14U); i > 0U; i--)
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
    return ++ticks;
}
#else
void SysTick_Handler(void)
{
    HAL_IncTick();
}
#endif

static void Error_Handler(void)
{
    fprintf(stderr, "<bsp> Error_Handler!");

    for (;;);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @return None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    // User can add his own implementation to report the file name and line number
    fprintf(stderr, "<bsp> Wrong parameters value: file %s on line %d\r\n", file, line);

    for (;;);
}
#endif

/**
  * @brief  System Clock Configuration
  * @note   This funtion is generated from CubeMX project
  *         core clock - 100 MHz
  *         clock source - external
  *         USB clock source - PLLI2S
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
#if defined(HSE_ON)
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
#elif defined(HSE_BYPASS)
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
#else
#error HSE Source not configure
#endif
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = HSE_VALUE / 1000000;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
    PeriphClkInitStruct.PLLI2S.PLLI2SM = HSE_VALUE / 2000000;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 4;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLI2SQ;
    PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

/***************************************************************************************************
 *                                      PUBLIC FUNCTIONS
 **************************************************************************************************/

void bsp_init(void)
{
    HAL_Init();

    SystemClock_Config();
    SystemCoreClockUpdate();
}

/**************************************************************************************************
 *                                        END OF FILE
 **************************************************************************************************/
