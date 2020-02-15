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
 *   File:          bsp_vcp.h
 *   Description:
 *
 ***************************************************************************************************
 *   History:       <30.10.2019> - file created
 *
 **************************************************************************************************/

#pragma once

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include "bsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                      PUBLIC TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/

/***************************************************************************************************
 *                                PUBLIC FUNCTION PROTOTYPES
 **************************************************************************************************/

/**
 *  @brief     ������� ������������� VCP
 *  @details   �������������� USB Device Controller Core, ������ USB. ��������� PULL-UP �� ������
 *             �������� D+ � D- (������ ����������� ���������� � ���� USB)
 *
 *  @return    ���������� ���������� �������
 */
bsp_result_t bsp_vcp_init();

/**
 *  @brief     ������� ��-�������������� VCP
 *  @details   ������� ������ ���������� �� ���� USB, ��-�������������� USB Device Controller Core
 *             � ������ USB
 *
 *  @return    ���������� ���������� �������
 */
bsp_result_t bsp_vcp_deinit();

/**
 *  @brief     ������� �������� ������� VCP
 *  @details
 *
 *  @return    true - ���������� ���������������� � ������ ��� ������������
 *             false - ���������� �� ���������������� � �� ������ ��� ������������
 */
bool bsp_vcp_is_configured();

/**
 *  @brief     ������� ��� ���������� ���� �� ��������� ������
 *  @details
 *
 *  @param[in] ��������� �� �����
 *  @param[in] ���-�� ���� ��� ����������
 *
 *  @return    ���-�� ��������� ����. ������������� �������� ������������ ��� ������
 */
int32_t bsp_vcp_read_data(uint8_t *buf, int32_t len);

/**
 *  @brief     ������� ��� �������� ���� � ��������� �����
 *  @details
 *
 *  @param[in] ��������� �� �����
 *  @param[in] ���-�� ���� ��� ��������
 *
 *  @return    ���-�� ���� ������������ � ��������� �����. ������������� �������� ������������
 *             ��� ������
 */
int32_t bsp_vcp_write_data(const uint8_t *buf, int32_t len);

int bsp_vcp_put_char(int ch);

int bsp_vcp_get_char(void);

#ifdef __cplusplus
} // extern "C"
#endif

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
