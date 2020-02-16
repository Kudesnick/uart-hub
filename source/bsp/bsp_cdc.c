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
 *   File:          bsp_vcp.c
 *   Description:
 *
 ***************************************************************************************************
 *   History:       <30.10.2019> - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "stm32f4xx_hal.h"

#include "bsp_cdc.h"
#include "rl_usb.h"
#include "USBD_Config_CDC_0.h"
#include "rtx_os.h"
#include "bsp_gpio_macro.h"
#include "bsp_types.h"

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define USBD_P_GPIO PORTA_12
#define USBD_CDC_VCP 0

/***************************************************************************************************
 *                                      PRIVATE TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                               PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       PRIVATE DATA
 **************************************************************************************************/

static CDC_LINE_CODING _cdc0_acm_line_coding;

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
 *                            PRIVATE MISCELLANEOUS FUNCTIONS
 **************************************************************************************************/

/**
 *  @brief     Connect USB-PD pin to groung.
 *  @details   This action detected by host system as reconnect USB device
 */
static void _manual_push_disconnect(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN(USBD_P_GPIO);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIO_PORT(USBD_P_GPIO), &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIO_PORT(USBD_P_GPIO), GPIO_PIN(USBD_P_GPIO), GPIO_PIN_RESET);
}

/***************************************************************************************************
 *                          PRIVATE FUNCTIONS CDC0
 **************************************************************************************************/

// Called upon USB Host request to change communication settings.
// \param[in]   line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          set line coding request processed.
// \return      false         set line coding request not supported or not processed.
bool USBD_CDC0_ACM_SetLineCoding (const CDC_LINE_CODING *line_coding)
{
    // Фиктивные настройки бодрейта необходимы для некоторых эмуляторов терминала
    _cdc0_acm_line_coding = *line_coding;

    return true;
}

// Called upon USB Host request to retrieve communication settings.
// \param[out]  line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          get line coding request processed.
// \return      false         get line coding request not supported or not processed.
bool USBD_CDC0_ACM_GetLineCoding (CDC_LINE_CODING *line_coding)
{
    // Фиктивные настройки бодрейта необходимы для некоторых эмуляторов терминала
    *line_coding = _cdc0_acm_line_coding;

    return true;
}

// Called upon USB Host request to set control line states.
// \param [in]  state         control line settings bitmap.
//                - bit 0: DTR state
//                - bit 1: RTS state
// \return      true          set control line state request processed.
// \return      false         set control line state request not supported or not processed.
bool USBD_CDC0_ACM_SetControlLineState (uint16_t state)
{
    // Add code for set control line state

    (void)(state);

    return true;
}

// Called when new data was received.
// \param [in]  len           number of bytes available for reading.
void USBD_CDC0_ACM_DataReceived (uint32_t len)
{
    // Add code for handling new data reception
}

// Called when when all data was sent.
void USBD_CDC0_ACM_DataSent (void)
{
    // Add code for handling new data send
}

/***************************************************************************************************
 *                                    PUBLIC FUNCTIONS
 **************************************************************************************************/

bsp_result_t bsp_cdc_init()
{
    usbStatus _res;

    _manual_push_disconnect();
    osDelay(10);
    _res = USBD_Initialize(USBD_CDC0_DEV);
    if (_res == usbOK)
    {
        _res = USBD_Connect(USBD_CDC0_DEV);
    }

    return _res == usbOK ? BSP_RESULT_OK : BSP_RESULT_ERR;
}

bsp_result_t bsp_cdc_deinit()
{
    usbStatus _res;

    _res = USBD_Disconnect(USBD_CDC0_DEV);
    if (_res == usbOK)
    {
        _res = USBD_Uninitialize(USBD_CDC0_DEV);
    }
    _manual_push_disconnect();

    return _res == usbOK ? BSP_RESULT_OK : BSP_RESULT_ERR;
}

bool bsp_cdc_is_configured()
{
    return USBD_Configured(USBD_CDC0_DEV);
}

int32_t bsp_cdc0_read_data(uint8_t *buf, int32_t len)
{
    return (USBD_CDC_ACM_DataAvailable(USBD_CDC_VCP)) ? 
            USBD_CDC_ACM_ReadData(USBD_CDC_VCP, buf, len) : 0;
}

int32_t bsp_cdc0_write_data(const uint8_t *buf, int32_t len)
{
    return USBD_CDC_ACM_WriteData(USBD_CDC_VCP, buf, len);
}

int bsp_cdc0_put_char(int ch)
{
    return USBD_CDC_ACM_PutChar(USBD_CDC_VCP, ch);
}

int bsp_cdc0_get_char(void)
{
    return USBD_CDC_ACM_GetChar(USBD_CDC_VCP);
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
