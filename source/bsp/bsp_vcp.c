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

#include "bsp_vcp.h"
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

static CDC_LINE_CODING _cdc_acm_line_coding;

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



// Перегрузка callback'ов
// Called during USBD_Initialize to initialize the USB CDC class instance (ACM).
void USBD_CDC0_ACM_Initialize (void)
{
    // Add code for initialization
}

// Called during USBD_Uninitialize to de-initialize the USB CDC class instance (ACM).
void USBD_CDC0_ACM_Uninitialize (void)
{
    // Add code for de-initialization
}

// Called upon USB Bus Reset Event.
void USBD_CDC0_ACM_Reset (void)
{
    // Add code for reset
}

// Callback function called upon reception of request send encapsulated command sent by the USB Host.
// \param[in]   buf           buffer that contains send encapsulated command request.
// \param[in]   len           length of send encapsulated command request.
// \return      true          send encapsulated command request processed.
// \return      false         send encapsulated command request not supported or not processed.
bool USBD_CDC0_ACM_SendEncapsulatedCommand (const uint8_t *buf, uint16_t len)
{
    return true;
}

// Callback function called upon reception of request to get encapsulated response sent by the USB Host.
// \param[in]   max_len       maximum number of data bytes that USB Host expects to receive
// \param[out]  buf           pointer to buffer containing get encapsulated response to be returned to USB Host.
// \param[out]  len           pointer to number of data bytes to be returned to USB Host.
// \return      true          get encapsulated response request processed.
// \return      false         get encapsulated response request not supported or not processed.
bool USBD_CDC0_ACM_GetEncapsulatedResponse (uint16_t max_len, uint8_t **buf, uint16_t *len)
{
    return true;
}

// Called upon USB Host request to change communication settings.
// \param[in]   line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          set line coding request processed.
// \return      false         set line coding request not supported or not processed.
bool USBD_CDC0_ACM_SetLineCoding (const CDC_LINE_CODING *line_coding)
{
    // Фиктивные настройки бодрейта необходимы для некоторых эмуляторов терминала
    _cdc_acm_line_coding = *line_coding;

    return true;
}

// Called upon USB Host request to retrieve communication settings.
// \param[out]  line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          get line coding request processed.
// \return      false         get line coding request not supported or not processed.
bool USBD_CDC0_ACM_GetLineCoding (CDC_LINE_CODING *line_coding)
{
    // Фиктивные настройки бодрейта необходимы для некоторых эмуляторов терминала
    *line_coding = _cdc_acm_line_coding;

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

bsp_result_t bsp_vcp_init()
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

bsp_result_t bsp_vcp_deinit()
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

bool bsp_vcp_is_configured()
{
    return USBD_Configured(USBD_CDC0_DEV);
}

int32_t bsp_vcp_read_data(uint8_t *buf, int32_t len)
{
    return (USBD_CDC_ACM_DataAvailable(USBD_CDC_VCP)) ? 
            USBD_CDC_ACM_ReadData(USBD_CDC_VCP, buf, len) : 0;
}

int32_t bsp_vcp_write_data(const uint8_t *buf, int32_t len)
{
    return USBD_CDC_ACM_WriteData(USBD_CDC_VCP, buf, len);
}

int bsp_vcp_put_char(int ch)
{
    return USBD_CDC_ACM_PutChar(USBD_CDC_VCP, ch);
}

int bsp_vcp_get_char(void)
{
    return USBD_CDC_ACM_GetChar(USBD_CDC_VCP);
}

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
