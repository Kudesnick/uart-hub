/**
 *  @file       bsp_flash.h
 *
 *  @brief      Работа с флеш-памятью
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

/// Тип слова для записи во флеш
typedef uint16_t bsp_flash_word_t;
/// Тип указателя на адрес флеш-памяти
typedef void * bsp_flash_addr_t;
/// Тип переменной, хранящей размер флеш-памяти или отдельных её страниц
typedef uint32_t bsp_flash_size_t;
/// Тип для последовательной нумерации страниц флеш-памяти
typedef uint8_t  bsp_flash_page_t;

/// Указатель на функцию возврата из асинхронных процедур работы с флеш-памятью
typedef void (* bsp_flash_async_callback_t)(bsp_result_t _result);

/***************************************************************************************************
 *                                         GLOBAL VARIABLES                                        *
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PUBLIC FUNCTION PROTOTYPES                                   *
 **************************************************************************************************/

/// Инициализация модуля
void bsp_flash_init(void);

/// Менеджер для выполнения асинхронных опериций
void bsp_flash_manager(void);

/// Получить полный размер флеш-памяти (в байтах)
bsp_flash_size_t bsp_flash_get_size(void);

/// Получить количество страниц
bsp_flash_page_t bsp_flash_page_cnt(void);

/// Получить размер конкретной страницы (в байтах)
bsp_flash_size_t bsp_flash_get_page_size(bsp_flash_page_t _page);

/// Получить адрес начала страницы
bsp_flash_addr_t bsp_flash_get_page_addr(bsp_flash_page_t _page);

/// Получить номер страницы, которой принадлежит адрес
bsp_flash_page_t bsp_flash_get_page_num(bsp_flash_addr_t _addr);

/// Проверить, что адрес является началом страницы
bsp_result_t bsp_flash_is_start_page_addr(bsp_flash_addr_t _addr);

/// синхронное стирание страницы
bsp_result_t bsp_flash_erase_sync(bsp_flash_page_t _page);

/// асинхронное стирание страницы
void bsp_flash_erase_async(bsp_flash_page_t _page, bsp_flash_async_callback_t _callback);

/// Синхронная запись слова
bsp_result_t bsp_flash_write_word_sync(bsp_flash_addr_t _addr, bsp_flash_word_t _data);

/// асинхронная запись слова
void bsp_flash_write_word_async(bsp_flash_addr_t _addr, 
                                bsp_flash_word_t _data, 
                                bsp_flash_async_callback_t _callback);

/// Синхронная функция - эквивалент memcpy
bsp_result_t bsp_flash_memcpy_sync(bsp_flash_addr_t _addr, 
                                   const void * _src_ptr, 
                                   bsp_flash_size_t _len);

/// Асинхронная функция - эквивалент memcpy
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
