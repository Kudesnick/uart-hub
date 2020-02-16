
/***************************************************************************************************
 *   Project:
 *   Author:        Stulov Tikhon
 ***************************************************************************************************
 *   Distribution:
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          sett_work.h
 *   Description:   ћодуль реализует работу с настройками flash. јлгоритмы универсальны дл€ всех
 *                  проектов
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
#include "sett_types.h"
#include "cpp_list.h"
#include "cpp_os.h"

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                         DEFINITIONS                                             *
 **************************************************************************************************/

/***************************************************************************************************
 *                                          PUBLIC TYPES                                           *
 **************************************************************************************************/

typedef struct
{
    uint16_t id;
    uint8_t secure;
    uint8_t type;
    union
    {
        uint64_t value;
        struct
        {
            uint32_t len;
            void *ptr;
        };
    };
} flash_param_external_t;

typedef struct
{
    params_base_t *main_addr;
    params_base_t *mirr_addr;
    params_base_t *def_addr;
} cpp_params_arg_t;

/***************************************************************************************************
 *                                         GLOBAL VARIABLES                                        *
 **************************************************************************************************/

/***************************************************************************************************
 *                                    PUBLIC FUNCTION PROTOTYPES                                   *
 **************************************************************************************************/

class cpp_ro_params : public cpp_list<cpp_ro_params>
{
protected:
    const params_base_t *main_addr_;
    void *blob_ptr_;
    void *blob_ptr_init(void);

    friend flash_param_external_t user_external_get(PARAM_ID_TYPE_t id);

    // ‘ункции дл€ работы с мьютексом, актуальны при работе с перезаписываемыми настройками
    virtual void flash_lock(void) {};
    virtual void flash_unlock(void) {};

public:
    cpp_ro_params(const params_base_t *_base_addr);

    param_header_t *get_param_ptr(PARAM_ID_TYPE_t id);  //¬озвращает указатель на параметр по ID

    void *blob_get_data(PARAM_ID_TYPE_t _id);     // ѕолучить указатель на BLOB данные
    void *blob_get_data(param_header_t *param);   // ѕолучить указатель на BLOB данные
    void *blob_get_data(param_blob_t *param);     // ѕолучить указатель на BLOB данные

    // ¬озвращает указатель на структуру параметра, если параметр существует и доступен дл€ чтени€.
    // ≈сли параметр не существует, то в поле id возвращаетс€ PARAM_ID_NA
    flash_param_external_t external_get(PARAM_ID_TYPE_t id);
};


class cpp_rw_params : public cpp_ro_params
{
private:
    const params_base_t *mirr_addr_;
    const params_base_t *def_addr_;

    friend void convert_old_struct(bool after_update);

protected:
    // t_s #warning used this mutex
    cpp_os_mutex flash_mutex;

    // ѕровер€ет что в основной странице лежит целостна€ структура
    bool crc_complete(void);
    bool must_be_updated(void);
    bool updating(const bool blob_save);

    // сдвиг параметров в очереди FIFO
    bool fifo_shift(PARAM_ID_TYPE_t id, uint8_t count);

    // блокировка доступа к флеш-пам€ти
    void flash_lock(void);
    // разблокировка доступа к пам€ти
    void flash_unlock(void);

public:
    cpp_rw_params(params_base_t *_main, params_base_t *_mirr, params_base_t *_def);
    cpp_rw_params(cpp_params_arg_t *_arg);

    // провер€ет целостность структуры в пам€ти flash, восстанавливает страницув случае потери
    // данных и добавл€ет дефолтные параметры, в более старые версии
    void test_and_repair(void);

    // сохран€ет параметр или возвращает false, если параметр отсутствует
    bool set(PARAM_ID_TYPE_t id, uint64_t value);
    // —охранение значени€ типа BLOB
    bool set(PARAM_ID_TYPE_t id, void *src, uint16_t len);
    // —охранение значени€ типа BLOB
    bool set(PARAM_ID_TYPE_t id, char *src);
    // ƒобавление параметра в FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, uint64_t value, uint8_t count);
    // —охранение значени€ типа BLOB в FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, void *src, uint16_t len, uint8_t count);
    // —охранение значени€ типа BLOB-строку в FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, char *src, uint8_t count);
    // «апись дефолтного значени€
    bool clr(PARAM_ID_TYPE_t id);

    // ѕровер€ет параметр на несоответствие дефолтному значению
    bool is_def(PARAM_ID_TYPE_t id);

    // сохран€ет параметр
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, uint64_t value);
    // сохран€ет параметр BLOB
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, void *src, uint16_t len);
    // сохран€ет параметр BLOB-строку
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, char *src);
};
