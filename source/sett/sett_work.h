
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
 *   Description:   ������ ��������� ������ � ����������� flash. ��������� ������������ ��� ����
 *                  ��������
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

    // ������� ��� ������ � ���������, ��������� ��� ������ � ����������������� �����������
    virtual void flash_lock(void) {};
    virtual void flash_unlock(void) {};

public:
    cpp_ro_params(const params_base_t *_base_addr);

    param_header_t *get_param_ptr(PARAM_ID_TYPE_t id);  //���������� ��������� �� �������� �� ID

    void *blob_get_data(PARAM_ID_TYPE_t _id);     // �������� ��������� �� BLOB ������
    void *blob_get_data(param_header_t *param);   // �������� ��������� �� BLOB ������
    void *blob_get_data(param_blob_t *param);     // �������� ��������� �� BLOB ������

    // ���������� ��������� �� ��������� ���������, ���� �������� ���������� � �������� ��� ������.
    // ���� �������� �� ����������, �� � ���� id ������������ PARAM_ID_NA
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

    // ��������� ��� � �������� �������� ����� ��������� ���������
    bool crc_complete(void);
    bool must_be_updated(void);
    bool updating(const bool blob_save);

    // ����� ���������� � ������� FIFO
    bool fifo_shift(PARAM_ID_TYPE_t id, uint8_t count);

    // ���������� ������� � ����-������
    void flash_lock(void);
    // ������������� ������� � ������
    void flash_unlock(void);

public:
    cpp_rw_params(params_base_t *_main, params_base_t *_mirr, params_base_t *_def);
    cpp_rw_params(cpp_params_arg_t *_arg);

    // ��������� ����������� ��������� � ������ flash, ��������������� ��������� ������ ������
    // ������ � ��������� ��������� ���������, � ����� ������ ������
    void test_and_repair(void);

    // ��������� �������� ��� ���������� false, ���� �������� �����������
    bool set(PARAM_ID_TYPE_t id, uint64_t value);
    // ���������� �������� ���� BLOB
    bool set(PARAM_ID_TYPE_t id, void *src, uint16_t len);
    // ���������� �������� ���� BLOB
    bool set(PARAM_ID_TYPE_t id, char *src);
    // ���������� ��������� � FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, uint64_t value, uint8_t count);
    // ���������� �������� ���� BLOB � FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, void *src, uint16_t len, uint8_t count);
    // ���������� �������� ���� BLOB-������ � FIFO
    bool fifo_set(PARAM_ID_TYPE_t id, char *src, uint8_t count);
    // ������ ���������� ��������
    bool clr(PARAM_ID_TYPE_t id);

    // ��������� �������� �� �������������� ���������� ��������
    bool is_def(PARAM_ID_TYPE_t id);

    // ��������� ��������
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, uint64_t value);
    // ��������� �������� BLOB
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, void *src, uint16_t len);
    // ��������� �������� BLOB-������
    flash_param_external_t external_set(PARAM_ID_TYPE_t id, char *src);
};
