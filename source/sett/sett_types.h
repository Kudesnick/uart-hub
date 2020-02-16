
/***************************************************************************************************
 *   Project:
 *   Author:    Stulov Tikhon
 ***************************************************************************************************
 *   Distribution:
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          sett_types.h
 *   Description:   ћодуль должен обновл€тьс€ во всех проектах при добавлении новых id параметров
 *
 ***************************************************************************************************
 *   History:       07.06.2019 - file created
 *
 **************************************************************************************************/

#pragma once

/***************************************************************************************************
 *                                         INCLUDED FILES                                          *
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header

#include <stdlib.h>

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                         DEFINITIONS                                             *
 **************************************************************************************************/

// длины полей структуры заголовка
#define HEADER_FIELD_LEN_ID      11
#define HEADER_FIELD_LEN_SECURE   2
#define HEADER_FIELD_LEN_STATE    3

#if ((HEADER_FIELD_LEN_ID + HEADER_FIELD_LEN_SECURE + HEADER_FIELD_LEN_STATE) > 16)
    #error param_header_t length overflow!
#endif

#define FIELD_SHIFT_SECURE  (HEADER_FIELD_LEN_ID)
#define FIELD_SHIFT_STATE   (FIELD_SHIFT_SECURE + HEADER_FIELD_LEN_SECURE)

// ƒефолтные (непроинициализированные) значени€ параметров
#define PARAM_BLOB_NA 0,0

#define STM_SETT_PREAMBLE 0xDEADBEEF

/***************************************************************************************************
 *                                          PUBLIC TYPES                                           *
 **************************************************************************************************/

typedef enum : uint16_t
{
    PARAM_ID_VERS,
    PARAM_ID_PREAMBLE,
    PARAM_ID_REV,

    /*
    id of user settings
    */

    PARAM_ID_COUNT,
    PARAM_ID_NA = 0x07FF  // ‘ейковый параметр - неопределенный ID. ¬сегда равен максимально возможному значению ID.
} PARAM_ID_TYPE_t;

/**
 @brief:
 @detail:
 */
typedef enum : uint8_t
{
    PARAM_TRIGGER_NA    = 0x0000, //
    PARAM_TRIGGER_ON    = 0x0001, //
    PARAM_TRIGGER_OFF   = 0x0002, //
    PARAM_TRIGGER_ERR   = 0x0003  //
} PARAM_TRIGGER_TYPE_t;

/**
 @brief:
 @detail:
 */
typedef enum : uint8_t
{
    PARAM_ACCESS_NO = 0x0000, //
    PARAM_ACCESS_RO = 0x0001, //
    PARAM_ACCESS_RW = 0x0002  //
} PARAM_ACCESS_TYPE_t;

/**
 @brief:
 @detail:
 */
typedef enum : uint8_t
{
    PARAM_DATA_TRIGGER_NA   = 0x0000, //
    PARAM_DATA_TRIGGER_ON   = 0x0001, //
    PARAM_DATA_TRIGGER_OFF  = 0x0002, //
    PARAM_DATA_BLOB         = 0x0003, //
    PARAM_DATA_1_BYTE       = 0x0004, //
    PARAM_DATA_2_BYTE       = 0x0005, //
    PARAM_DATA_4_BYTE       = 0x0006, //
    PARAM_DATA_8_BYTE       = 0x0007  //
} PARAM_DATA_TYPE_t;

// структура заголовка параметра
__PACKED_STRUCT param_header_t
{
uint16_t id          :
    HEADER_FIELD_LEN_ID     ; // id параметра (PARAM_ID_TYPE_t)
uint16_t secure_flag :
    HEADER_FIELD_LEN_SECURE ; // ѕрава доступа (param_secure_flag_t)
uint16_t state       :
    HEADER_FIELD_LEN_STATE  ; // ѕоле состо€ни€ / размера данных (PARAM_DATA_TYPE_t)
};

// параметр-триггер
__PACKED_STRUCT param_trigger_t
{
    param_header_t header;
};

// ѕараметр BLOB (binare large object)
__PACKED_STRUCT param_blob_t
{
    param_header_t header;
    uint16_t       shift; // —мещение данных, задаваемое относительно нулевого байта после самой структуры настроек
    uint16_t       len;   // –азмер данных в байтах
};


__PACKED_STRUCT param_uint8_t
{
    param_header_t header;
    uint8_t        data;
}; // параметр uint8_t

__PACKED_STRUCT param_uint16_t
{
    param_header_t header;
    uint16_t       data;
}; // параметр uint16_t

__PACKED_STRUCT param_uint32_t
{
    param_header_t header;
    uint32_t       data;
}; // параметр uint32_t

__PACKED_STRUCT param_uint64_t
{
    param_header_t header;
    uint64_t       data;
}; // параметр uint64_t


__PACKED_STRUCT params_base_t
{
    const param_uint32_t preamble = {{PARAM_ID_PREAMBLE, PARAM_ACCESS_RO, PARAM_DATA_4_BYTE}, STM_SETT_PREAMBLE}; // преамбула (константа дл€ определени€ того, что это начало структуры настроек)
};

/***************************************************************************************************
 *                                         GLOBAL VARIABLES                                        *
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PUBLIC FUNCTION PROTOTYPES                                   *
 **************************************************************************************************/
