/**
 *  @file       bsp_flash.h
 *
 *  @brief      ������ � ����-�������
 *
 *  @details
 *
 *  @author     Stulov Tikhon
 *
 *  @date       2018/12/17
 *
 *  @warning
 *
 *  @todo
 *
 */

#pragma once

/***************************************************************************************************
 *                                         INCLUDED FILES
 **************************************************************************************************/

#include "bsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 *                                           DEFINITIONS                                           *
 **************************************************************************************************/

/***************************************************************************************************
 *                                          PUBLIC TYPES                                           *
 **************************************************************************************************/

/// ��� ����� ��� ������ �� ����
typedef uint16_t bsp_flash_word_t;
/// ��� ��������� �� ����� ����-������
typedef void * bsp_flash_addr_t;
/// ��� ����������, �������� ������ ����-������ ��� ��������� � �������
typedef uint32_t bsp_flash_size_t;
/// ��� ��� ���������������� ��������� ������� ����-������
typedef uint8_t  bsp_flash_page_t;

/// ��������� �� ������� �������� �� ����������� �������� ������ � ����-�������
typedef void (* bsp_flash_async_callback_t)(bsp_result_t _result);

/***************************************************************************************************
 *                                         GLOBAL VARIABLES                                        *
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PUBLIC FUNCTION PROTOTYPES                                   *
 **************************************************************************************************/

/// ������������� ������
void bsp_flash_init(void);

/// �������� ��� ���������� ����������� ��������
void bsp_flash_manager(void);

/// �������� ������ ������ ����-������ (� ������)
bsp_flash_size_t bsp_flash_get_size(void);

/// �������� ���������� �������
bsp_flash_page_t bsp_flash_page_cnt(void);

/// �������� ������ ���������� �������� (� ������)
bsp_flash_size_t bsp_flash_get_page_size(bsp_flash_page_t _page);

/// �������� ����� ������ ��������
bsp_flash_addr_t bsp_flash_get_page_addr(bsp_flash_page_t _page);

/// �������� ����� ��������, ������� ����������� �����
bsp_flash_page_t bsp_flash_get_page_num(bsp_flash_addr_t _addr);

/// ���������, ��� ����� �������� ������� ��������
bsp_result_t bsp_flash_is_start_page_addr(bsp_flash_addr_t _addr);

/// ���������� �������� ��������
bsp_result_t bsp_flash_erase_sync(bsp_flash_page_t _page);

/// ����������� �������� ��������
void bsp_flash_erase_async(bsp_flash_page_t _page, bsp_flash_async_callback_t _callback);

/// ���������� ������ �����
bsp_result_t bsp_flash_write_word_sync(bsp_flash_addr_t _addr, bsp_flash_word_t _data);

/// ����������� ������ �����
void bsp_flash_write_word_async(bsp_flash_addr_t _addr, 
                                bsp_flash_word_t _data, 
                                bsp_flash_async_callback_t _callback);

/// ���������� ������� - ���������� memcpy
bsp_result_t bsp_flash_memcpy_sync(bsp_flash_addr_t _addr, 
                                   const void * _src_ptr, 
                                   bsp_flash_size_t _len);

/// ����������� ������� - ���������� memcpy
void bsp_flash_memcpy_async(bsp_flash_addr_t _addr, 
                            const void * _src_ptr, 
                            bsp_flash_size_t _len, 
                            bsp_flash_async_callback_t _callback);

#ifdef __cplusplus
} // extern "C"
#endif

/***************************************************************************************************
 *                                        END OF FILE
 **************************************************************************************************/
