
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
 *   File:          sett_work.cpp
 *   Description:   ������ ��������� ������ � ����������� flash. ��������� ��������������� ���� ��������
 *
 ***************************************************************************************************
 *   History:       07.06.2019 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                         INCLUDED FILES
 **************************************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header

#include <stdlib.h>
#include <string.h>

#include "bsp_flash.h"

#include "fifo.h"
#include "misc_macro.h"
#include "crc.h"
#include "sett_work.h"

/***************************************************************************************************
 *                                           DEFINITIONS
 **************************************************************************************************/

#define PARAM_ID_END PARAM_ID_VERS ///< ��������, ������� ������ ����� � �������� ���������

#define BLOB_BUF_SIZE FIFO_SIZE_16 ///< ������������ ���������� BLOB ����������. ����� ��� ������������ ���������� ������

/***************************************************************************************************
 *                                          PRIVATE TYPES
 **************************************************************************************************/

// ����� ��� ���������� ������ BLOB-������
typedef __PACKED_STRUCT
{
    bsp_flash_addr_t source_addr;
    bsp_flash_addr_t target_addr;
    bsp_flash_size_t len;
} procrastinated_blob_t;

typedef FIFO_TYPEDEF(procrastinated_blob_t, uint8_t, BLOB_BUF_SIZE) procrastinated_buf_t;


// ��� ����������� ����� ��� �������� ����������� ������ 
typedef uint32_t crc_t;

/***************************************************************************************************
 *                                  PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                           PRIVATE DATA
 **************************************************************************************************/

procrastinated_buf_t procrastinated_buf;

/***************************************************************************************************
 *                                           FLASH DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                           PUBLIC DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                          EXTERNAL DATA
 **************************************************************************************************/

/***************************************************************************************************
 *                                  PRIVATE FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                  EXTERNAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                        PRIVATE FUNCTIONS
 **************************************************************************************************/

/***************************************************************************************************
 *                                  ������� ��������
 **************************************************************************************************/

/**
 *  @brief      �������� �������� �� ID
 *  @details    ���������� � ������, ���� ID ��������� �� ������ �� �������� ��������. ��������,
 *              ��� ����������� ������ �� ������������� ID, ���� ��� ������ ����� � ������������ �������
 *              ������.
 *
 *  @param[in]  id ID �������������� ���������
 *              
 *  @return     ���������, ���������� ������ ��� ��������� (� ������ � blob-�������)
 *
 *  @warning    ������� ������������� ��� ���������� � ��������������� ������, ��� �������������
 */
__WEAK flash_param_external_t user_external_get(PARAM_ID_TYPE_t id)
{
    flash_param_external_t result = {};
    
    return result;
}

/**
 *  @brief      �������� �������������� ������ ��������� � ����������� ��������
 *  @details    ������� ���������� ��� ���� �� test_and_repair.
                ������ ���, ����� test_and_repair ������������ �� ��������� ��������
                �����-�� ���������� ������ � �� ����� �� ����������������.
                � ���� ������ ������� ���������� � ���������� after_update = false 
                � ����� ���������� ������������ � ��������� ������ � ����������� ������.
                ����� ����� ��� ���������� �� test_and_repair ��� ���,
                ������ � ���������� after_update = true. ������ ��� ����� �������� ���������,
                �.�. �������� ��� ��������� � ������������ � ����� ��������.
 *              
 *  @param[in]  after_update false - ������� ���������� �� ��������� ��������� flash ������ true - �����.
 *
 *  @warning    ������� ������������� ��� ���������� � ��������������� ������, ��� �������������
 */
void convert_old_struct(bool after_update)
{
};

/***************************************************************************************************
 *                                  ����� ������ � ����-�������
 **************************************************************************************************/

/**
 *  @brief       ������������ ������ �� ������ ��������
 *
 *  @param[out]  _addr - ������������ ����� ����-������
 *
 *  @return      �����, ���������� �� ��������
 */
static bsp_flash_addr_t _flash_addr_around(bsp_flash_addr_t &_addr)
{
    _addr = bsp_flash_get_page_addr(bsp_flash_get_page_num(_addr));
    
    return _addr;
}

/**
 *  @brief      �������� ������ �������� ����-������, ������� ����������� �����
 *
 *  @param[in]  _addr - ������������ ����� ����-������
 *
 *  @return     ������ ��������
 */
static bsp_flash_size_t _flash_get_page_size(bsp_flash_addr_t &_addr)
{
    return bsp_flash_get_page_size(bsp_flash_get_page_num(_addr));
}

/**
 *  @brief      ������� ��������, ������� ����������� �����
 *
 *  @param[in]  _addr - ������������ ����� ����-������
 *
 *  @return     true - �������� ������� ������, false - ������ �������� ��������
 */
static bool _flash_erase_page(bsp_flash_addr_t &_addr)
{
    return bsp_flash_erase_sync(bsp_flash_get_page_num(_addr)) == BSP_RESULT_OK;
}

/***************************************************************************************************
 *                                  �������� �� ���������� �����
 **************************************************************************************************/

/**
 *  @brief      ��������� �������� �� ������� ����������
 *  @details    ������ �������, ��� �� �������� ���� ������, �������� �� FF
 *              
 *  @param[in]  page_addr ����� ��������
 *              
 *  @return     true - �������� ������ (������), false - ���� �����-�� ������, � �.�. �����
 */
static bool _flash_page_is_clean(bsp_flash_addr_t _page_addr)
{
    const uint32_t size = _flash_get_page_size(_page_addr) / sizeof(uint64_t);
    
    for (auto i = 0; i < size; i++)
    {
        if ((((uint64_t *)_page_addr)[i]) != UINT64_MAX)
        {
            return false;
        }
    }

    return true;
};

/**
 *  @brief      �������� ���� ������� �� ���������
 *  @details    ��������� �� ��������� ���������� ���� ������ ������
 *              
 *  @param[in]  source_addr ����� ������ ��������
 *  @param[in]  targ_addr   ����� ������ ��������
 *              
 *  @return     true, ���� ���������� ������� ���������
 */
static bool _flash_page_is_eq(bsp_flash_addr_t _src_addr, bsp_flash_addr_t _dest_addr)
{
    _flash_addr_around(_src_addr);
    _flash_addr_around(_dest_addr);

    if (_src_addr == _dest_addr)
    {
        return true;
    }

    if (_flash_get_page_size(_src_addr) != _flash_get_page_size(_dest_addr))
    {
        return false;
    }
    
    return memcmp(_src_addr, _dest_addr, _flash_get_page_size(_src_addr)) == 0;
};


/**
 *  @brief      ������� ��� ������ �� ���� ���������
 *  @details    ��������� �������� ������ �� ����, �� ���������� �� �� ������
 *              
 *  @param[in]  source_addr ����� �������� ������
 *  @param[in]  target_addr ������� ����� ������
 *  @param[in]  size        ������ ����� ��� ������
 *              
 *  @return     ���� ��������� ���� �� �������� �� �����, ���� ��������� � ���������� data
 *              � ���� ������, ���� ��������� ������� � ���������� size = 0, �� �� ����� �������� ��
 *              ����� � ������� �� ����
 */
static bool _flash_memcpy(const bsp_flash_addr_t _src_addr,
                          const bsp_flash_addr_t _dest_addr,
                          const bsp_flash_size_t _size)
{
    static bsp_flash_word_t data;
    static bsp_flash_addr_t last_addr = NULL;
    
    bool result = true;
    
    if (   _size == 0
        && last_addr != NULL
        && ((uint32_t)last_addr & 1) == 0
        )
    {
        return (bsp_flash_write_word_sync(last_addr, data | 0xFF00) == BSP_RESULT_OK);
    }
    else
    {
        for (uint32_t i = 0; i < _size; i++)
        {
            if ((((uint32_t)_dest_addr + i) & 1) != 0)
            {
                data |= *(uint8_t *)((uint32_t)_src_addr + i) << 8;
                
                result = (bsp_flash_write_word_sync(
                    (bsp_flash_addr_t)((uint32_t)_dest_addr + i - 1),
                     data) == BSP_RESULT_OK);
            }
            else
            {
                data = *(uint8_t *)((uint32_t)_src_addr + i);
            }
        }
        
        last_addr = (bsp_flash_addr_t)((uint32_t)_dest_addr + _size - 1);
    }
    
    return result;
}


/**
 *  @brief      ����������� ��������
 *  @details    �������� ���� �������� � ������. � ������� �������� �������� ��� ������.
 *              
 *  @param[in]  source_addr ����� �������� ��������
 *  @param[in]  targ_addr   ����� ������� ��������
 *              
 *  @return     true - ������� ������� �����������, false - � �������� ����������� ��������� ������
 */
static bool _flash_page_copy(bsp_flash_addr_t _src_addr, bsp_flash_addr_t _dest_addr)
{
    bool result = true;
    
    _flash_addr_around(_src_addr);
    _flash_addr_around(_dest_addr);
    
    if (   _src_addr != _dest_addr
        && !_flash_page_is_eq(_src_addr, _dest_addr)
        )
    {
        if (_flash_page_is_clean(_dest_addr) == false)
        {
            result = _flash_erase_page(_dest_addr);
        }

        if (result && !_flash_page_is_clean(_src_addr))
        {
            _flash_memcpy(_src_addr, _dest_addr, _flash_get_page_size(_dest_addr));
        }
        
        result = _flash_page_is_eq(_src_addr, _dest_addr);
    }
    
    return result;
};

/**
 *  @brief      ������� ����������� ����� ��� ���� ��������
 *  @details    ����������� ����� �������� � ��������� ������� ��������
 *              ��������� ��� CRC-32/POSIX
 *              
 *  @param[in]  page_addr ����� ��������
 *              
 *  @return     �������� ����������� �����
 */
static crc_t _flash_param_crc_calc(bsp_flash_addr_t _addr)
{
    static const crc::crc_sett_t<crc_t> crc_set = // ��������� ��� CRC-32/POSIX
    {
        .poly   = 0x04C11DB7, 
        .init   = 0x00000000, 
        .refin  = false,
        .refout = false,
        .xorout = 0xFFFFFFFF,
    };
    
    crc_t crc = crc_set.init,
             result = 0;
    
    _flash_addr_around(_addr);

    for (auto i = 0; i <= _flash_get_page_size(_addr) - sizeof(crc_t) - 1; i++)
    {
        result = crc_calc(*(uint8_t *)((uint32_t)_addr + i), crc_set, crc);
    }
    
    return result;
};

/**
 *  @brief      ��������� CRC ��������
 *  @details    Dsxbckztn CRC � ���������� � ����������� � ����� �������� ���������
 *              
 *  @param[in]  page_addr ����� ��������
 *              
 *  @return     true - ��������� CRC ��������� � �����������
 */
static bool _crc_is_valid(const bsp_flash_addr_t _addr)
{
    void * addr = _addr;
    
    _flash_addr_around(addr);
    
    crc_t crc = _flash_param_crc_calc(addr);
    
    return (crc == *(crc_t *)((uint32_t)addr + _flash_get_page_size(addr) - sizeof(uint32_t)));
};

/***************************************************************************************************
 *                            �������� �� ����������, ���������� �� ����
 **************************************************************************************************/

/**
 *  @brief      ����������� ����� ��������� � ������ (����� + ������)
 *              
 *  @param[in]  *hdr ��������� �� ����� ��������� � ���������
 *              
 *  @return     ���������� ����, ���������� � ��������� ����������
 */
static uint8_t _param_size_get(const param_header_t * hdr)
{
    uint8_t size;

    switch(hdr->state)
    {
        case PARAM_DATA_BLOB  : size = sizeof(param_blob_t  ) ; break;
        case PARAM_DATA_1_BYTE: size = sizeof(param_uint8_t ) ; break;
        case PARAM_DATA_2_BYTE: size = sizeof(param_uint16_t) ; break;
        case PARAM_DATA_4_BYTE: size = sizeof(param_uint32_t) ; break;
        case PARAM_DATA_8_BYTE: size = sizeof(param_uint64_t) ; break;
        default               : size = sizeof(param_trigger_t); break;
    }

    return size;
}


/**
 *  @brief      ���������� ��������� �� �������� �� ID
 *  @details    ���� � ������ ������� ID. ������ ���������� � ��������� ������, ������, ������� �� ���� ����� �� ��������������� ������� ���������
 *              
 *  @param[in]  params_addr ����� ��������� ����������
 *  @param[in]  id          ID ���������
 *              
 *  @return     ��������� �� �������� � ������ flash
 */
static param_header_t * _get_param_ptr(const __PACKED_STRUCT params_base_t * _addr, PARAM_ID_TYPE_t id)
{
    param_header_t * result = NULL;
    
    bsp_flash_size_t page_size = _flash_get_page_size((bsp_flash_addr_t &)_addr);
    
    for(auto i = 0; i < page_size; )
    {
        param_header_t * curr_hdr = (param_header_t *)((uint32_t)_addr + i);
        
        // if (!is_valid_id(curr_hdr->id) break;
        /* ����� �� ���� ����� ��������� ���������� id, �� ����� �������� ������������� �� �������
         * ����������. �.�. ��������� �������� ����� �� ������ ��������.
         */

        if (curr_hdr->id == id)
        {
            result = curr_hdr;
            break;
        }
        else if (curr_hdr->id == PARAM_ID_END || curr_hdr->id == PARAM_ID_NA)
        {
            break;
        }
        else
        {
            i += _param_size_get(curr_hdr);
        }
    }

    return result;
};


/**
 *  @brief      �������� ���������� ID
 *  @details    ���������, ���� �� ID � ���������� ������ ID
 *              
 *  @param[in]  id �������� ID ��� ����� ������� (������ ���� ����� ��������)
 *              
 *  @return     true - ID ���� � ���������� ������ ID
 */
#define is_valid_id(id) ((bool)(((id) & ((1 << HEADER_FIELD_LEN_ID) - 1)) < PARAM_ID_COUNT))


/**
 *  @brief      ���������� ����� ������ BLOB ������ �� ������ ��������
 *  @details    BLOB ������ ���������� ����� ����� ��������� ��������, �� ��������� ����� �� ��������������� ��������� �� ����� ����������
 *              
 *  @param[in]  page_addr ����� ������� ��������
 *              
 *  @return     ����� ������ BLOB ������
 */
static void * _blob_start_ptr(const __PACKED_STRUCT params_base_t * _addr)
{
    param_header_t * param_end_ptr = _get_param_ptr(_addr, PARAM_ID_END);
    
    return (void *)((uint32_t)param_end_ptr + _param_size_get(param_end_ptr));
}


/**
 *  @brief      ����������� �������� � �������� ��������
 *  @details    ������������ ��� ������ �� ����� ����������, �.�. ��� ���� ����� ��� ���������� - ������� ���������� ��������� � �������� ��� ������
 *              
 *  @param[in]  val �������� ���������
 *              
 *  @return     ���������� ��������� �������� PARAM_NA, PARAM_ACTIV ��� PARAM_NOT_ACTIV
 */
__INLINE static PARAM_TRIGGER_TYPE_t _value_to_state(uint64_t val)
{
    switch(val)
    {
        case PARAM_TRIGGER_ON : 
        case PARAM_TRIGGER_OFF: return (PARAM_TRIGGER_TYPE_t)val;
        default               : return PARAM_TRIGGER_NA;
    }
};


/**
 *  @brief      ��������� ������ ��� ����� ���������� ������
 *  @details    �������� � ���� ������ � �������������� ��.
 *              
 *  @return     ��������� �� �����
 */
static procrastinated_buf_t * _procrastinate_buf_callock(void)
{
    procrastinated_buf_t * buf = NULL;
    
    buf = (procrastinated_buf_t *)calloc(sizeof(procrastinated_buf_t), 1);
    
    if (buf == NULL)
    {
        // ���� � ���� �� ������� ����� - �������� �����
        BRK_PTR("heap overflow");
    }
    else
    {
        memset(buf, 0, sizeof(procrastinated_buf_t));
    }
    
    return buf;
}


/**
 *  @brief      ������ ���������� ������
 *              
 *  @param[in]  buf ��������� �� ����� ���������� ������
 */
static bool _procrastinate_write(procrastinated_buf_t * buf)
{
    bool result;
    
    if (buf != NULL)
    {
        // ��������� ���������� ������
        while (FIFO_IS_EMPTY(*buf) == false)
        {
            procrastinated_blob_t tmp = FIFO_EXTRACT(*buf);
            
            result = _flash_memcpy(tmp.source_addr, tmp.target_addr, tmp.len);
            
            if (!result) return false;
        }
        
        // ����������� ������
        free(buf);
    }

    // ����������� ������, ���� ����
    result = _flash_memcpy(NULL, NULL, 0);
    
    return result;
}


/**
 *  @brief      ������ � ������ CRC
 *  @details    ������� CRC � ���������� � ����� �������� ��������
 *              ������ ����� ����� �������� ��������� ��������
 *              
 *  @param[in]  page_addr ����� ��������, ��� ������� ������������ CRC
 */
static void _param_crc_write(bsp_flash_addr_t _addr)
{
    _flash_addr_around(_addr);
    
    crc_t crc = _flash_param_crc_calc(_addr);

    _flash_memcpy(&crc, (uint8_t *)_addr + _flash_get_page_size(_addr) - sizeof(crc_t), sizeof(crc_t));
};


__INLINE uint8_t _sizeofdata(uint64_t &_data)
{
    uint8_t size;

    for (size = sizeof(_data); size > 1; size--)
    {
        if (_data & (0xFFUL << ((size - 1) * 8)))
        {
            break;
        }
    }
    
    return size;
}

/***************************************************************************************************
 *                            ���������� ������ cpp_ro_params
 **************************************************************************************************/


/**
 *  @brief      ����������� ������� ��������, ��������� ������ ��� ������
 *  @details    ���������� ��������� �������� �� ���������
 *              
 *  @param[in]  _base_addr - ����� ��������� ��������
 */
cpp_ro_params::cpp_ro_params(const params_base_t * _base_addr):
    main_addr_(_base_addr),
    blob_ptr_(NULL)
{
};

/**
 *  @brief      ������������� �������� ���� blob_ptr_ - ��������� �� ������ BLOB ������
 *  @details    ���������� ��������� �������� �� ���������
 *              
 *  @return     ���������� ��������� �� ������ BLOB ������ ��� NULL, ���� �� ������� ����������
 *              ����� (��������� ����������)
 */
void * cpp_ro_params::blob_ptr_init(void)
{
    if (blob_ptr_ == NULL)
    {
        param_header_t * param = get_param_ptr(PARAM_ID_END);
        
        if (param != NULL)
        {
            blob_ptr_ = (void *)((uint32_t)param + _param_size_get(param));
        }
        else
        {
            return NULL;
        }
    }
    
    return blob_ptr_;
};


/**
 *  @brief      ���������� ��������� �� �������� �� ID
 *  @details    ���� � ������ ������� ID. ������ ���������� � ��������� ������, ������, ������� �� ���� ����� �� ��������������� ������� ���������
 *              
 *  @param[in]  id          ID ���������
 *              
 *  @return     ��������� �� �������� � ������ flash
 */
param_header_t * cpp_ro_params::get_param_ptr(PARAM_ID_TYPE_t _id)
{
    flash_lock();
    param_header_t * result = _get_param_ptr(main_addr_, _id);
    flash_unlock();
    
    return result;
};

/**
 *  @brief      �������� ��������� �� BLOB ������
 *  @details    ���������� ��������� �� ������ ������
 *              
 *  @param[in]  _id ������������� ���������

 *  @return     ��������� �� ������ ������ ��� NULL, ���� ������ ��� ��� ������������� �� ������
 *              ��� ������ �������� ����� ������ ���
 */
void * cpp_ro_params::blob_get_data(PARAM_ID_TYPE_t _id)
{
    void *result;
    
    flash_lock();
    result = blob_get_data(get_param_ptr(_id));
    flash_unlock();
    
    return result;
};

/**
 *  @brief      �������� ��������� �� BLOB ������
 *  @details    ���������� ��������� �� ������ ������
 *              
 *  @param[in]  param ��������� �� ������� �������, ��� �������� �� ����� �������� ��������� ��
 *              ������

 *  @return     ��������� �� ������ ������ ��� NULL, ���� ������ ��� ��� ������ �������� �����
 *              ������ ���
 */
void * cpp_ro_params::blob_get_data(param_header_t * param)
{
    void *result = NULL;
    
    if (   true
        && param != NULL
        && param->state == PARAM_DATA_BLOB
        )
    {
        flash_lock();
        result = blob_get_data((param_blob_t *)param);
        flash_unlock();
    }
    
    return result;
}

/**
 *  @brief      �������� ��������� �� BLOB ������
 *  @details    ���������� ��������� �� ������ ������
 *              
 *  @param[in]  param ��������� �� ������� �������, ��� �������� �� ����� �������� ��������� ��
 *              ������

 *  @return     ��������� �� ������ ������ ��� NULL, ���� ������ ���
 */
void * cpp_ro_params::blob_get_data(param_blob_t * param)
{
    void *result = NULL;
    
    flash_lock();
    
    if (blob_ptr_init() != NULL)
    {
        result = (param->len > 0) ? (void *)((uint32_t)blob_ptr_ + param->shift) : NULL;
    }
    
    flash_unlock();
    
    return result;
}


static void _param_header_to_external(param_header_t *& _hdr, flash_param_external_t &_ext)
{
    if (_hdr != NULL)
    {
        if (   (   _hdr->secure_flag != PARAM_ACCESS_RO
                && _hdr->secure_flag != PARAM_ACCESS_RW
                )
            )
        {
            _hdr = NULL;
        }
        else
        {
            _ext.id = _hdr->id;
            _ext.secure = _hdr->secure_flag;
            
            _ext.type = _hdr->state;

            switch (_hdr->state)
            {
                case PARAM_DATA_BLOB  : _ext.len   = ((param_blob_t   *)_hdr)->len ; break;
                case PARAM_DATA_1_BYTE: _ext.value = ((param_uint8_t  *)_hdr)->data; break;
                case PARAM_DATA_2_BYTE: _ext.value = ((param_uint16_t *)_hdr)->data; break;
                case PARAM_DATA_4_BYTE: _ext.value = ((param_uint32_t *)_hdr)->data; break;
                case PARAM_DATA_8_BYTE: _ext.value = ((param_uint64_t *)_hdr)->data; break;
                default: _ext.type = 0; _ext.value = _hdr->state; break;
            }
        }
    }
}

/**
 *  @brief      �������� �������� �� ID
 *  @details    ������������ ��� ��������� ������ �������� ������������.
 *
 *  @param[in]  id ID �������������� ���������
 *              
 *  @return     ���������, ���������� ������ ��� ��������� �� ������ � �� �����,
 *              � ������ � blob-�������.
 *              ���� ID ���������� ��� ������, �� � ���� id ������������ ��������� ����� PARAM_ID_NA
 */
flash_param_external_t cpp_ro_params::external_get(PARAM_ID_TYPE_t _id)
{
    flash_param_external_t result = {};
    
    flash_lock();
    
    param_header_t * param_header = get_param_ptr(_id);
    
    if (param_header != NULL)
    {
        _param_header_to_external(param_header, result);
        
        // ���� ������ - blob, �� ������� ���������
        if (   true
            && result.id != PARAM_ID_NA
            && result.type == PARAM_DATA_BLOB
            && result.len > 0
            )
        {
            result.ptr = blob_get_data(param_header);
        }
    }
    else
    {
        // �������� ����� �� ���� ������������������ ����������
        struct lambda_arg
        {
            cpp_ro_params * obj;
            PARAM_ID_TYPE_t id;
            param_header_t * param;
        } arg = {this, _id, NULL};
        
        cpp_list<cpp_ro_params>::enumerate(&arg, [](cpp_ro_params *& _el, void * _arg)
        {
            struct lambda_arg * arg = static_cast<struct lambda_arg *>(_arg);
            
            if (_el != arg->obj)
            {
                arg->param = _el->get_param_ptr(arg->id);
                
                if (arg->param != NULL)
                {
                    arg->obj = _el;
                    
                    return false;
                }
            }
            
            return true;
        });
        
        if (arg.param != NULL)
        {
            _param_header_to_external(arg.param, result);
            
            // ����������� ��������� �� ������ �������
            if (result.type == PARAM_DATA_BLOB)
            {
                result.ptr = (result.len) ? arg.obj->blob_get_data(arg.param) : NULL;
            }
        }
        
        param_header = arg.param;
    }

    // ���� � ��� ����� - ��������� ���������������� �������
    if (param_header == NULL)
    {
        result = user_external_get(_id);
    }
    
    flash_unlock();
    return result;
};


/***************************************************************************************************
 *                            ���������� ������ cpp_rw_params
 **************************************************************************************************/


/**
 *  @brief      ����������� ������� ��������, ��� ������ � ������
 *  @details    ���������� ��������� �������� �� ���������
 *              
 *  @param[in]  _main - ����� �������� ��������� ��������
 *  @param[in]  _mirr - ����� ��������� ��������
 *  @param[in]  _def - ����� ��������� �������� �� ���������
 */
cpp_rw_params::cpp_rw_params(params_base_t * _main, params_base_t * _mirr, params_base_t * _def):
    cpp_ro_params(_main),
    mirr_addr_(_mirr),
    def_addr_(_def)
{
    if (   false
        || main_addr_ == NULL
        || mirr_addr_ == NULL
        || def_addr_  == NULL
        || main_addr_ == mirr_addr_
        || mirr_addr_ == def_addr_
        || main_addr_ == def_addr_
        || _flash_get_page_size((bsp_flash_addr_t &)main_addr_) != _flash_get_page_size((bsp_flash_addr_t &)mirr_addr_)
        || _flash_addr_around((bsp_flash_addr_t &)main_addr_) != _main
        || _flash_addr_around((bsp_flash_addr_t &)mirr_addr_) != _mirr
        )
    {
        // �������� ����������������� ������
        BRK_PTR("cpp_rw_params creating error");
    }
};


/**
 *  @brief      ����������� ������� ��������, ��� ������ � ������
 *  @details    ���������� ��������� �������� �� ���������
 *              
 *  @param[in]  _arg - ��������� � �������� ��������
 */
cpp_rw_params::cpp_rw_params(cpp_params_arg_t * _arg):
    cpp_rw_params(_arg->main_addr, _arg->mirr_addr, _arg->def_addr)
{
};


/**
 *  @brief      ��������� ��� � �������� �������� ����� ��������� ���������
 *  @details    �������� ������ ������������ crc. ������������ ��� ����������� ������ ������
 *
 *  @return     true - ����������� ����� �������� �������� �������� �����
 */
bool cpp_rw_params::crc_complete(void)
{
    return _crc_is_valid((bsp_flash_addr_t)(main_addr_));
}


/**
 *  @brief      ����� �� ��������� ��������� �� flash
 *  @details    ��������� ������� ��������� � ��������� �������:
 *                  - ��������� ����� ��������
 *                  - ������ ������ ��������
 *                  - ���������� ������������ ����������
 *                  - ��������� ��� ��������� (��������, � uint8_t �� uint16_t)
 *                  - ��������� ���� ���� ������� ���������
 *
 *  @return     true - ��������� ������� ��������
 */
bool cpp_rw_params::must_be_updated(void)
{
    bool result = false;
    
    for (auto i = 0; i < PARAM_ID_COUNT; i++)
    {
        param_header_t * param_hdr_dflt = _get_param_ptr(def_addr_, (PARAM_ID_TYPE_t)i);
        param_header_t * param_hdr_flash = _get_param_ptr(main_addr_, (PARAM_ID_TYPE_t)i);
        
        // ��������� � ����� id ��� � ����������
        if (   param_hdr_dflt == NULL
            && param_hdr_flash == NULL
            )
        {
            continue;
        }

        // �������� �������� ��� ������
        else if (   param_hdr_dflt == NULL
                 || param_hdr_flash == NULL
                 )
        {
            result = true;
            break;
        }

        else if (   (uint32_t)param_hdr_dflt - (uint32_t)def_addr_ != (uint32_t)param_hdr_flash - (uint32_t)main_addr_ // ���������� ����������������� ����������
                 || param_hdr_dflt->secure_flag != param_hdr_flash->secure_flag // ��������� ���� ���� ������� ���������
                 || (   param_hdr_dflt->state != param_hdr_flash->state
                     && (param_hdr_dflt->state | param_hdr_flash->state) > PARAM_TRIGGER_OFF
                     ) // ��������� ��� ���������
                 )
        {
            result = true;
            break;
        }
    }
    
    return result;
}


/**
 *  @brief      ���������� ����������� ��������� �� ���� (� ��������������� ���������)
 *  @details    � ������� �������� ������������ ����������� ���������. ������, �� ����������� ������� �� ���������-���������.
 *              ���� ��� ����������� � ������� �� ��������� ���������, �� �� ������������, ��� � ����� ������� ��������,
 *              ������������� ��� ��������� ���������. ��� ���� ������, �� �����������, �����������.
 *              
 *  @param[in]  blob_save   true - blob ������, ��� �������������� � ��� ������������� ����� ����������� (���������� �� �����),
 *                          false - ��������� ��������
 */
bool cpp_rw_params::updating(const bool _blob_save)
{
    procrastinated_buf_t * blob_buf = _procrastinate_buf_callock();
    
    if (!_flash_erase_page((bsp_flash_addr_t &)mirr_addr_))
    {
        return false;
    }
    
    param_blob_t blob_target; // ��������� �������� ��������� �� BLOB ������
    blob_target.len = 0;
    blob_target.shift = 0;
    
    const auto i_max = (uint32_t)_blob_start_ptr(def_addr_) - (uint32_t)def_addr_;

    for(auto i = 0; i < i_max; )
    {
        param_header_t * param_hdr_dflt = (param_header_t *)((uint8_t *)def_addr_ + i);
        param_header_t * param_hdr_source = _get_param_ptr(main_addr_, (PARAM_ID_TYPE_t)(param_hdr_dflt->id));
        
        // ��������� BLOB
        if (param_hdr_dflt->state == PARAM_DATA_BLOB)
        {
            blob_target.header = ((param_blob_t *)param_hdr_dflt)->header;
            blob_target.shift += blob_target.len;
            uint32_t blob_src_addr;
            
            // � �������� ��������� ��� ����� ���� - ���������� ��������� ��������
            if (param_hdr_source == NULL)
            {
                blob_target.len = ((param_blob_t *)param_hdr_dflt)->len;
                
                blob_src_addr = (uint32_t)_blob_start_ptr(def_addr_) + ((param_blob_t *)param_hdr_dflt)->shift;
            }
            // � ��������� ���� ���� � ��� ����� ��� PARAM_DATA_BLOB
            else if (param_hdr_source->state == PARAM_DATA_BLOB)
            {
                blob_target.len = ((param_blob_t *)param_hdr_source)->len;
                
                blob_src_addr = (uint32_t)_blob_start_ptr(main_addr_) + ((param_blob_t *)param_hdr_source)->shift;
            }
            // � ��������� ���� ����, �� ��� ������������� �����
            else
            {
                blob_target.len = _param_size_get(param_hdr_dflt) - sizeof(param_header_t);
                
                blob_src_addr = (uint32_t)param_hdr_dflt + sizeof(param_header_t);
            }
            
            if(!_flash_memcpy(&blob_target, (uint8_t *)mirr_addr_ + i, sizeof(param_blob_t)))
            {
                return false;
            }
            
            i += sizeof(param_blob_t);
            
            // �� ���������� ���� ������, �.�. ������ � ������ ������ ���� ���������������
            // ����� ����� �������� ������������� �� ������ ������
            if (blob_target.len > 0)
            {
                procrastinated_blob_t tmp =
                {
                    .source_addr = (void *)blob_src_addr,
                    .target_addr = (void *)((uint32_t)_blob_start_ptr(def_addr_) - (uint32_t)def_addr_ + (uint32_t)mirr_addr_ + blob_target.shift),
                    .len         = blob_target.len,
                };
                if (FIFO_IS_FULL(*blob_buf) == false)
                {
                    FIFO_ADD(*blob_buf, tmp);
                }
                else
                {
                    // �������� �����, ���� ������� ������ �� �������
                    BRK_PTR("blob_buf is overflow.");
                }
            }
            // flash_memcpy(blob_src_addr, target_addr + sizeof(params_t) + blob_target.shift, blob_target.len);

        }
        // �������� � ������ ������������� �����
        else
        {
            uint64_t tmp = 0;

            // ���������� �������� ������ � �������� �� � ���� uint64_t
            param_header_t * tmp_hdr = param_hdr_source;
            
            if (   param_hdr_source == NULL
                || (param_hdr_source->state == PARAM_DATA_BLOB && _blob_save == false
                    )
                )
            {
                tmp_hdr = param_hdr_dflt;
            }

            switch (tmp_hdr->state)
            {
                case PARAM_DATA_1_BYTE: tmp = ((param_uint8_t  *)tmp_hdr)->data; break;
                case PARAM_DATA_2_BYTE: tmp = ((param_uint16_t *)tmp_hdr)->data; break;
                case PARAM_DATA_4_BYTE: tmp = ((param_uint32_t *)tmp_hdr)->data; break;
                case PARAM_DATA_8_BYTE: tmp = ((param_uint64_t *)tmp_hdr)->data; break;
                case PARAM_DATA_BLOB:
                {
                    tmp = 0;
                    memcpy((void *)((uint32_t)_blob_start_ptr(main_addr_) + ((param_blob_t *)param_hdr_source)->shift),
                           &tmp,
                           (((param_blob_t *)param_hdr_source)->len < sizeof(tmp)) ? ((param_blob_t *)param_hdr_source)->len : sizeof(tmp));
                    break;
                }
                default: tmp = tmp_hdr->state; break;
            }
            
            // ���������� ������ �������� ��������� ����
            param_header_t valid_hdr = * param_hdr_dflt;
            uint8_t len = _param_size_get(&valid_hdr);
            
            if (len == sizeof(param_header_t))
            {
                valid_hdr.state = _value_to_state(tmp);
            }
            
            if(!_flash_memcpy(&valid_hdr, (uint8_t *)mirr_addr_ + i, sizeof(param_header_t)))
            {
                return false;
            }
            
            i += sizeof(param_header_t);
            
            if (len > sizeof(param_header_t))
            {
                len -= sizeof(param_header_t);
                
                if(!_flash_memcpy(&tmp, (uint8_t *)mirr_addr_ + i, len))
                {
                    return false;
                }
                
                i += len;
            }
        }
    }
    
    return _procrastinate_write(blob_buf);
};


/**
 *  @brief      ��������� ����������� ��������� � ������ flash
 *  @details    ��������������� �������� � � ������ ������ ������ � ��������� ��������� ���������, � ����� ������ ������
 *
 *  @warning    ������ ���������� ������ ��� ����� ������ ������� ������ � �����������, ����� ����������� �������� �� �������������
 */
void cpp_rw_params::test_and_repair(void)
{
    if (   !_flash_page_is_clean((bsp_flash_addr_t)main_addr_)
        || !_flash_page_is_clean((bsp_flash_addr_t)mirr_addr_)
        )
    {
        // ��������� ������ �� ��������� ���� � �������� ������
        if (!_crc_is_valid((bsp_flash_addr_t)main_addr_))
        {
            if (_crc_is_valid((bsp_flash_addr_t)mirr_addr_))
            {
                _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
            }
        }
    }
    
    // ��������� ��������� ���������� �������, ���� ��� �������
    convert_old_struct(false); // ��. �������� �������

    // ��������� ������ �� ������������ ��������� ��������� � ��������� ��������� �� ����
    if (must_be_updated())
    {
        _flash_erase_page((bsp_flash_addr_t &)mirr_addr_);
        
        updating(true);
        
        _param_crc_write((bsp_flash_addr_t &)mirr_addr_);
        
        _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
    }
    
    // ��������������� ���������, ���������� �� ��������� ���������
    convert_old_struct(true); // ��. �������� �������
};


/**
 *  @brief      ��������� �������� ��� ���������� false, ���� �������� �����������
 *  @details    
 *              
 *  @param[in]  id    ID ��������� ��� ����������
 *  @param[in]  value �������� ���������
 *              
 *  @return     true - �������� ��������, false - ���������� ���������� (�������� �����������)
 */
bool cpp_rw_params::set(PARAM_ID_TYPE_t id, uint64_t value)
{
    flash_lock();
    
    param_header_t * param_header = get_param_ptr(id);
    
    bool result = (param_header != NULL);
    
    if (result)
    {
        if (param_header->state == PARAM_DATA_BLOB)
        {
            result = set(id, &value, _sizeofdata(value));
        }
        
        else
        {
            bool val_not_eq = false;
            
            switch (param_header->state)
            {
                case PARAM_DATA_BLOB  : break; // �� ����� ������, ���� ������������ ���
                case PARAM_DATA_1_BYTE: val_not_eq = ((param_uint8_t  *)param_header)->data != (uint8_t )value; break;
                case PARAM_DATA_2_BYTE: val_not_eq = ((param_uint16_t *)param_header)->data != (uint16_t)value; break;
                case PARAM_DATA_4_BYTE: val_not_eq = ((param_uint32_t *)param_header)->data != (uint32_t)value; break;
                case PARAM_DATA_8_BYTE: val_not_eq = ((param_uint64_t *)param_header)->data != (uint64_t)value; break;
                default               : val_not_eq = param_header->state != _value_to_state(value); break;
            }
            
            if (val_not_eq)
            {
                // ������� ��������� ��������
                _flash_erase_page((bsp_flash_addr_t &)mirr_addr_);
        
                param_header_t header = *param_header;
                
                // ����� ��� ������ �� �����, ������� ���� ��������
                uint32_t len = (uint32_t)param_header - (uint32_t)main_addr_;
                
                _flash_memcpy((void *)main_addr_, (void *)mirr_addr_, len);
                
                // ����� ����� ������
                if (_param_size_get(&header) == sizeof(param_header_t))
                {
                    header.state = _value_to_state(value);
                }
                
                _flash_memcpy(&header, (uint8_t *)mirr_addr_ + len, sizeof(header));
                len += sizeof(header);
                
                if (_param_size_get(&header) != sizeof(param_header_t))
                {
                    auto tmp = _param_size_get(param_header) - sizeof(param_header_t);
                    _flash_memcpy(&value, (uint8_t *)mirr_addr_ + len, tmp);
                    len += tmp;
                }
                
                // ���������� ���, ��� ��������
                _flash_memcpy(
                    (uint8_t *)main_addr_ + len,
                    (uint8_t *)mirr_addr_ + len,
                    _flash_get_page_size((bsp_flash_addr_t &)main_addr_) - len - sizeof(uint32_t));
                // ����� ������� �� ������������� �� �����, ������ ��� ����� �� ����� ��������
                
                _param_crc_write((bsp_flash_addr_t)mirr_addr_);
                        
                _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
            }
        }
    }

    flash_unlock();
    
    return result;
};


/**
 *  @brief      ���������� �������� ���� BLOB
 *  @details    ��������� � ��������� ������ ������������ �����
 *              
 *  @param[in]  id  ID ���������
 *  @param[in]  src ��������� �� ������
 *  @param[in]  len ����� ������� ������. ���� len �������������� � ����, �� ������ ���������.
 *              � ���� ������ src ����� ����� �������� NULL
 *              
 *  @return     true  - ������ ������ �������.
 *              false - ��� ��������� � ����� ID;
 *                    - ������ �� ������� � �������� ��������;
 *                    - �������� ����������.
 */
bool cpp_rw_params::set(PARAM_ID_TYPE_t id, void * src, uint16_t len)
{
    flash_lock();
    
    if (blob_ptr_init() == NULL)
    {
        flash_unlock();
        
        return false;
    }
    
    param_header_t * header = get_param_ptr(id);
    
    bool result = (header != NULL);
    
    if (result)
    {
        if (header->state != PARAM_DATA_BLOB)
        {
            uint8_t size = (len < sizeof(uint64_t)) ? len : sizeof(uint64_t);
            uint64_t val = 0;
            
            memcpy(&val, src, size);
            
            result = set(id, val);
        }
        else
        {
            param_header_t * curr_header = NULL;
            param_blob_t curr_blob;
            curr_blob.len = 0;
            curr_blob.shift = 0;
            
            _flash_erase_page((bsp_flash_addr_t &)mirr_addr_);
            
            procrastinated_buf_t * blob_buf = _procrastinate_buf_callock();
            
            for (uint32_t i = 0; curr_header->id != PARAM_ID_END; )
            {
                curr_header = (param_header_t *)((uint8_t*)main_addr_ + i);
                
                if (curr_header->state != PARAM_DATA_BLOB)
                {
                    uint8_t tmp = _param_size_get(curr_header);
                    _flash_memcpy((uint8_t *)main_addr_ + i, (uint8_t *)mirr_addr_ + i, tmp);
                    i += tmp;
                }
                else
                {
                    curr_blob.header = *curr_header;
                    curr_blob.shift += curr_blob.len;
                    void * data_ptr;
                    
                    if (curr_header->id != id)
                    {
                        curr_blob.len = ((param_blob_t *)curr_header)->len;
                        data_ptr = (void *)((uint8_t *)blob_ptr_ + ((param_blob_t *)curr_header)->shift);
                    }
                    else
                    {
                        curr_blob.len = len;
                        data_ptr = src;
                    }
                    
                    _flash_memcpy(&curr_blob, (uint8_t *)mirr_addr_ + i, sizeof(param_blob_t));
                    i += sizeof(param_blob_t);
    
                    if (  ((uint32_t)blob_ptr_ + curr_blob.shift + curr_blob.len + sizeof(crc_t)) 
                        > (uint32_t)mirr_addr_ + (uint32_t)_flash_get_page_size((bsp_flash_addr_t &)mirr_addr_))
                    {
                        // ���� �� ��������� �����, ������, �� ����� ���-�� ���� ����������� � �������� ������
                        header = NULL;
                        break;
                    }
                    else if (curr_blob.len > 0)
                    {
                        // �� ���������� ���� ������, �.�. ������ � ������ ������ ���� ���������������
                        // ����� ����� �������� ������������� �� ������ ������
                        procrastinated_blob_t tmp =
                        {
                            .source_addr = data_ptr,
                            .target_addr = (uint8_t *)mirr_addr_ + (uint32_t)blob_ptr_ - (uint32_t)main_addr_ + curr_blob.shift,
                            .len         = curr_blob.len,
                        };
                        FIFO_ADD(*blob_buf, tmp); // �� ��������� �� ������������, ���������� �������� � param_updating
                        // flash_memcpy((uint32_t)data_ptr, (uint32_t)&params_reserve + sizeof(params_t) + curr_blob.shift, curr_blob.len);
                    }
                }
            }
            
            _procrastinate_write(blob_buf);
            
            // ���������� ����������� ����� � �������� ��������� �������� � ��������
            if (header != NULL)
            {
                _param_crc_write((bsp_flash_addr_t)mirr_addr_);
                _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
            }
            else
            {
                result = false;
            }
        }
    }
    
    flash_unlock();

    return result;
}


/**
 *  @brief      ���������� �������� ���� BLOB
 *  @details    ��������� � ���������  ������ ������
 *              
 *  @param[in]  id  ID ���������
 *  @param[in]  src ��������� �� ������
 *              
 *  @return     true - ������ ������ �������, false - �������� ��� ��������� � ������ ID, ��� ��������� � ����� ID, ������ �� ������� � �������� ��������
 */
bool cpp_rw_params::set(PARAM_ID_TYPE_t id, char * src)
{
    return set(id, src, strlen(src) + 1);
}

/**
 *  @brief      ����� ���������� � ������� FIFO
 *  @details    ���������� �������� ��������� � fifo ������� � �������������� ������� � ������ �����
 *              ������, ������� ������ �� ������ � ������� ����� ������ ��������. ��� ���� ��������
 *              ���������� ���������� �� ID ���� �������. ��������� ������ ���� ������ ����.
 *              ������������ ��������� � fifo_set
 *
 *  @param[in]  id    ID ������� ��������� � �������
 *  @param[in]  count ����� �������, �.�. ���������� ���������� � ���������, ��������� �� �������
 *              ����, ������� � ID ������� ���������, ������� ��������� ��� �������
 *
 *  @return     ����� ������� ��������
 *  @todo       ������� �������� ������������ � �� ������� FIFO (count > 2) ���� ������� ����������
 *              ������ ��������/������. ���� ����� ���������� � �� ����� ������ ������, �� �
 *              ��������� ������ ����������, �.�. �� �������������� � �������� ������������� 
 *              (������ � �������� count = 2) ��� ������������ ���.
 */
bool cpp_rw_params::fifo_shift(PARAM_ID_TYPE_t id, uint8_t count)
{
    param_header_t *param_hdr_start = NULL;
    param_header_t *param_hdr_end = NULL;
    
    if (count == 0)
    {
        return false; // ������� ������� �����
    }
    
    param_hdr_end = param_hdr_start = get_param_ptr(id);
    
    if (param_hdr_start == NULL)
    {
        return false; // �������� ID
    }
    
    uint8_t param_size = _param_size_get(param_hdr_start);
    
    for (auto i = count - 1; i > 0; i--)
    {
        param_hdr_end = (param_header_t *)((uint8_t *)param_hdr_end + param_size);
        
        if (   true
            && param_hdr_start->state != param_hdr_end->state
            && (   false
                || param_hdr_start->state >= PARAM_DATA_BLOB
                || param_hdr_end->state >= PARAM_DATA_BLOB
                )
            )
            {
                return false; // ���� � ���������� ��������� ����� �� ���������
            }
    }
    
    // �������� ��� ��������� ����
    for (; param_hdr_end > param_hdr_start;
         param_hdr_end = (param_header_t *)((uint8_t *)param_hdr_end - param_size))
    {
        param_header_t *param_hdr_prev = (param_header_t *)((uint8_t *)param_hdr_end - param_size);
        
        bool result = true;
        
        switch (param_hdr_end->state)
        {
            case PARAM_DATA_BLOB:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id,
                    blob_get_data(param_hdr_prev),
                    ((param_blob_t *)param_hdr_prev)->len);
                break;
            case PARAM_DATA_1_BYTE:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id,
                    ((param_uint8_t *)param_hdr_prev)->data);
                break;
            case PARAM_DATA_2_BYTE:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id,
                    ((param_uint16_t *)param_hdr_prev)->data);
                break;
            case PARAM_DATA_4_BYTE:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id,
                    ((param_uint32_t *)param_hdr_prev)->data);
                break;                
            case PARAM_DATA_8_BYTE:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id,
                    ((param_uint64_t *)param_hdr_prev)->data);
                break;
            default:
                result = set((PARAM_ID_TYPE_t)param_hdr_end->id, param_hdr_prev->state);
                break;
        }
        
        if (!result) return false;
    }
    
    return true;
}

/**
 *  @brief      ���������� ������� � ����-������
 *  @details    ������� ������������ ������, ����������� �������
 */
void cpp_rw_params::flash_lock(void)
{
    while(flash_mutex.acquire(cpp_os::wait_forever) != cpp_os::ok);
};

/**
 *  @brief      ������������� ������� � ������
 *  @details    ����������� �������
 */
void cpp_rw_params::flash_unlock(void)
{
    flash_mutex.release();
};

/**
 *  @brief      ���������� ��������� � fifo �������
 *  @details    ���������� �������� ��������� � fifo ������� � ��������� ����� �������� �� ������,
 *              ������� ����� ������. ��� ���� �������� ���������� ���������� �� ID ���� �������.
 *              ��������� ������ ���� ������ ����.
 *              
 *  @param[in]  id    ID ������� ��������� � �������
 *  @param[in]  value �������� ���������
 *  @param[in]  count ����� �������, �.�. ���������� ���������� � ���������, ��������� �� �������
 *              ����, ������� � ID ������� ���������, ������� ��������� ��� �������
 *              
 *  @return     true - ������� ������� �������
 *
 *  @warning    ���� ������������ ������ ��� HL_ID, �� ����� �� ������ ������������ ��� ������
 *              ��������� ���� �� param_triiger_t � �� param_blob_t
 */
bool cpp_rw_params::fifo_set(PARAM_ID_TYPE_t id, uint64_t value, uint8_t count)
{
    bool result = false;
    
    flash_lock();
    
    if (fifo_shift(id, count))
    {
        set(id, value);
        
        result = true;
    }
    
    flash_unlock();
    
    return result;
};

/**
 *  @brief      ���������� �������� ���� BLOB � FIFO
 *  @details    ���������� �������� ��������� � fifo ������� � ��������� ����� �������� �� ������,
 *              ������� ����� ������. ��� ���� �������� ���������� ���������� �� ID ���� �������.
 *              ��������� ������ ���� ������ ����.
 *              
 *  @param[in]  id    ID ������� ��������� � �������
 *  @param[in]  src   ��������� �� ������
 *  @param[in]  len   ����� ������� ������. ���� len �������������� � ����, �� ������ ���������.
 *  @param[in]  count ����� �������, �.�. ���������� ���������� � ���������, ��������� �� �������
 *              ����, ������� � ID ������� ���������, ������� ��������� ��� �������
 *              
 *  @return     true - ������� ������� �������
 *
 *  @warning    ���� ������������ ������ ��� HL_ID, �� ����� �� ������ ������������ ��� ������
 *              ��������� ���� �� param_triiger_t � �� param_blob_t
 */
bool cpp_rw_params::fifo_set(PARAM_ID_TYPE_t id, void * src, uint16_t len, uint8_t count)
{
    bool result = false;
    
    flash_lock();
    
    if (fifo_shift(id, count))
    {
        set(id, src, len);
        
        result = true;
    }
    flash_unlock();
    
    return result;
};

/**
 *  @brief      ���������� �������� ���� BLOB � FIFO
 *  @details    ���������� �������� ��������� � fifo ������� � ��������� ����� �������� �� ������,
 *              ������� ����� ������. ��� ���� �������� ���������� ���������� �� ID ���� �������.
 *              ��������� ������ ���� ������ ����.
 *              
 *  @param[in]  id    ID ������� ��������� � �������
 *  @param[in]  src   ��������� �� ������
 *  @param[in]  count ����� �������, �.�. ���������� ���������� � ���������, ��������� �� �������
 *              ����, ������� � ID ������� ���������, ������� ��������� ��� �������
 *              
 *  @return     true - ������� ������� �������
 *
 *  @warning    ���� ������������ ������ ��� HL_ID, �� ����� �� ������ ������������ ��� ������
 *              ��������� ���� �� param_triiger_t � �� param_blob_t
 */
bool cpp_rw_params::fifo_set(PARAM_ID_TYPE_t id, char * src, uint8_t count)
{
    bool result = false;
    
    flash_lock();
    
    if (fifo_shift(id, count))
    {
        set(id, src);
        
        result = true;
    }
    flash_unlock();
    
    return result;
};

/**
 *  @brief      ���������� � �������� ��������� ��������
 *              
 *  @param[in]  id ID ���������
 *              
 *  @return     false �������� �� ��� ������� � ��������� ��������
 */
bool cpp_rw_params::clr(PARAM_ID_TYPE_t _id)
{    
    if (is_def(_id))
    {
        return true;
    }
    
    flash_lock();

    param_header_t * curr = _get_param_ptr(main_addr_, _id);
    param_header_t * def  = _get_param_ptr(def_addr_, _id);
    
    bool result = (curr != NULL && def  != NULL);

    if (result)
    {
        switch (curr->state)
        {
            case PARAM_DATA_BLOB:
            {
                void *blob_ptr = (uint8_t *)_blob_start_ptr(def_addr_) +
                                 ((param_blob_t *)def)->shift;
                
                result = set(_id, blob_ptr, ((param_blob_t *)def)->len);
            }
            break;
            
            case PARAM_DATA_1_BYTE: result = set(_id, ((param_uint8_t  *)def)->data); break;
            case PARAM_DATA_2_BYTE: result = set(_id, ((param_uint16_t *)def)->data); break;
            case PARAM_DATA_4_BYTE: result = set(_id, ((param_uint32_t *)def)->data); break;
            case PARAM_DATA_8_BYTE: result = set(_id, ((param_uint64_t *)def)->data); break;
            default               : result = set(_id, def->state);
        }
    }
    
    flash_unlock();
    
    return result;
};


/**
 *  @brief      ��������� �������� �� ������������ ���������� ��������
 *  @details    ����� ������� - ���������, ��� �������� �� ��� �������
 *              
 *  @param[in]  id ID ���������
 *              
 *  @return     true �������� �� ��� ������� � �������� ������ ���������� (������������� ����������)
 */
bool cpp_rw_params::is_def(PARAM_ID_TYPE_t id)
{
    bool result = true;
    
    flash_lock();
    
    param_header_t * param_def  = _get_param_ptr(def_addr_, id);
    param_header_t * param_main = get_param_ptr(id);
    
    if (param_def->state != PARAM_DATA_BLOB)
    {
        result = (memcmp(param_def, param_main, _param_size_get(param_def)) == 0);
    }
    else
    {
        #define param_blob_def   ((param_blob_t *)param_def)
        #define param_blob_main  ((param_blob_t *)param_main)
        
        result = (param_blob_def->len == param_blob_main->len);
        
        if (   result
            && param_blob_def->len > 0)
        {
            result = (memcmp((uint8_t *)def_addr_ + param_blob_def->shift,
                             (uint8_t *)main_addr_ + param_blob_main->shift,
                             param_blob_def->len) == 0);
        }
        
        #undef param_blob_def
        #undef param_blob_main
    }
    
    flash_unlock();

    return result;
};


/**
 *  @brief      ��������� ��������
 *  @details    ��������� ��������
 *              
 *  @param[in]  id    ID ���������
 *  @param[out] value �������� ���������
 *              
 *  @return     ���������, ���������� ������ ��� ��������� �� ������ � �� �����,
 *              � ������ � blob-�������.
 *              ���� ID ���������� ��� ������, �� � ���� id ������������ ��������� ����� PARAM_ID_NA
 *              ��������� �������� ������, ���������� ������������� � ������ flash � ��������� 
 *              ������� external_get
 */
flash_param_external_t cpp_rw_params::external_set(PARAM_ID_TYPE_t id, uint64_t value)
{
    flash_param_external_t result = {};

    flash_lock();
    
    param_header_t * param_header = get_param_ptr(id);
    
    if (param_header != NULL)
    {
        if (param_header->secure_flag == PARAM_ACCESS_RW)
        {
            if (!set(id, value))
            {
                param_header = NULL;
            }
        }
        else
        {
            param_header = NULL;
        }
    }
    flash_unlock();
    
    return external_get(id);
};


/**
 *  @brief      �������� BLOB ������
 *  @details    ���������� BLOB ������
 *              
 *  @param[in]  id  ID ���������
 *  @param[in]  src ��������� �� ������ ������ ��� ������
 *  @param[in]  len ����� ������ ��� ������
 *              
 *  @return     ���������, ���������� ������ ��� ��������� �� ������ � �� �����,
 *              � ������ � blob-�������.
 *              ���� ID ���������� ��� ������, �� � ���� id ������������ ��������� ����� PARAM_ID_NA
 *              ��������� �������� ������, ���������� ������������� � ������ flash � ��������� 
 *              ������� external_get
 */
flash_param_external_t cpp_rw_params::external_set(PARAM_ID_TYPE_t id, void * src, uint16_t len)
{
    flash_param_external_t result = {};

    flash_lock();
    
    param_header_t * param_header = get_param_ptr(id);
    
    if (param_header != NULL)
    {
        if (param_header->secure_flag == PARAM_ACCESS_RW)
        {
            if (!set(id, src, len))
            {
                param_header = NULL;
            }
        }
        else
        {
            param_header = NULL;
        }
    }
    flash_unlock();
    
    return external_get(id);
}

/**
 *  @brief      �������� BLOB-������
 *  @details    ���������� BLOB ������ � � ������ �������� ������ ���������� ���������� ��������
 *              
 *  @param[in]  id  ID ���������
 *  @param[in]  src ��������� �� ������
 *              
 *  @return     ���������, ���������� ������ ��� ��������� �� ������ � �� �����,
 *              � ������ � blob-�������.
 *              ���� ID ���������� ��� ������, �� � ���� id ������������ ��������� ����� PARAM_ID_NA
 *              ��������� �������� ������, ���������� ������������� � ������ flash � ��������� 
 *              ������� external_get
 */
flash_param_external_t cpp_rw_params::external_set(PARAM_ID_TYPE_t id, char * src)
{
    return external_set(id, src, strlen(src));
}

/***************************************************************************************************
 *                                        END OF FILE
 **************************************************************************************************/
