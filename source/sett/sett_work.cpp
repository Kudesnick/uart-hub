
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
 *   Description:   Модуль реализует работу с настройками flash. Алгоритмы универсальныдля всех проектов
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

#define PARAM_ID_END PARAM_ID_VERS ///< Параметр, который всегда лежит в стуктуре последним

#define BLOB_BUF_SIZE FIFO_SIZE_16 ///< максимальное количество BLOB параметров. Нужно для формирования отложенной записи

/***************************************************************************************************
 *                                          PRIVATE TYPES
 **************************************************************************************************/

// Буфер для отложенной записи BLOB-данных
typedef __PACKED_STRUCT
{
    bsp_flash_addr_t source_addr;
    bsp_flash_addr_t target_addr;
    bsp_flash_size_t len;
} procrastinated_blob_t;

typedef FIFO_TYPEDEF(procrastinated_blob_t, uint8_t, BLOB_BUF_SIZE) procrastinated_buf_t;


// Тип контрольной суммы для проверки челостности данных 
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
 *                                  Функции коллбеки
 **************************************************************************************************/

/**
 *  @brief      Получить параметр по ID
 *  @details    Вызывается в случае, если ID параметра не найден на странице настроек. Например,
 *              для возвращения данных по специфическим ID, если эти данные лежат в произвольной области
 *              памяти.
 *
 *  @param[in]  id ID запрашиваемого параметра
 *              
 *  @return     Структура, содержащая данные или указатель (в случае с blob-данными)
 *
 *  @warning    Функция предназначена для перегрузки в высокоуровневом модуле, при необходимости
 */
__WEAK flash_param_external_t user_external_get(PARAM_ID_TYPE_t id)
{
    flash_param_external_t result = {};
    
    return result;
}

/**
 *  @brief      Пытаемся конвертировать старую структуру с сохранением настроек
 *  @details    Функция вызывается два раза из test_and_repair.
                Первый раз, когда test_and_repair обнаруживает на страницах настроек
                какие-то непонятные данные и не может их идентифицировать.
                В этом случае функция вызывается с параметром after_update = false 
                и может попытаться расшифровать и сохранить данные в оперативной памяти.
                После этого она вызывается из test_and_repair еще раз,
                теперь с параметром after_update = true. Теперь она может обновить настройки,
                т.к. страницы уже приведены в соответствие с новым форматом.
 *              
 *  @param[in]  after_update false - функция вызывается до изменения состояния flash памяти true - после.
 *
 *  @warning    Функция предназначена для перегрузки в высокоуровневом модуле, при необходимости
 */
void convert_old_struct(bool after_update)
{
};

/***************************************************************************************************
 *                                  фасад работы с флеш-памятью
 **************************************************************************************************/

/**
 *  @brief       Выравнивание адреса по началу страницы
 *
 *  @param[out]  _addr - Произвольный адрес флеш-памяти
 *
 *  @return      Адрес, выровненый по странице
 */
static bsp_flash_addr_t _flash_addr_around(bsp_flash_addr_t &_addr)
{
    _addr = bsp_flash_get_page_addr(bsp_flash_get_page_num(_addr));
    
    return _addr;
}

/**
 *  @brief      Получить размер страницы флеш-памяти, которой принадлежит адрес
 *
 *  @param[in]  _addr - Произвольный адрес флеш-памяти
 *
 *  @return     Размер страницы
 */
static bsp_flash_size_t _flash_get_page_size(bsp_flash_addr_t &_addr)
{
    return bsp_flash_get_page_size(bsp_flash_get_page_num(_addr));
}

/**
 *  @brief      Стереть страницу, которой принадлежит адрес
 *
 *  @param[in]  _addr - Произвольный адрес флеш-памяти
 *
 *  @return     true - страница успешно стерта, false - ошибка стирания страницы
 */
static bool _flash_erase_page(bsp_flash_addr_t &_addr)
{
    return bsp_flash_erase_sync(bsp_flash_get_page_num(_addr)) == BSP_RESULT_OK;
}

/***************************************************************************************************
 *                                  Работаем со страницами флеша
 **************************************************************************************************/

/**
 *  @brief      Проверяет страницу на наличие информации
 *  @details    Просто смотрим, что на странице есть данные, отличные от FF
 *              
 *  @param[in]  page_addr Адрес страницы
 *              
 *  @return     true - страница чистая (пустая), false - есть какие-то данные, в т.ч. левые
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
 *  @brief      Проверка двух страниц на равенство
 *  @details    проверяет на равенство содержимое двух сраниц памяти
 *              
 *  @param[in]  source_addr Адрес первой страницы
 *  @param[in]  targ_addr   Адрес второй страницы
 *              
 *  @return     true, если содержимое страниц одинаково
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
 *  @brief      функция для записи во флеш побайтово
 *  @details    Позволяет записать данные во флеш, не выравнивая их по словам
 *              
 *  @param[in]  source_addr Адрес исходных данных
 *  @param[in]  target_addr Целевой адрес записи
 *  @param[in]  size        Размер блока для записи
 *              
 *  @return     Если последний байт не выровнен по слову, байт останется в переменной data
 *              в этом случае, если запустить функцию с параметром size = 0, то он будет дополнен до
 *              слова и записан во флеш
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
 *  @brief      Скопировать страницу
 *  @details    Копирует одну страницу в друкую. в целевой странице затирает все данные.
 *              
 *  @param[in]  source_addr Адрес исходной страницы
 *  @param[in]  targ_addr   Адрес целевой страницы
 *              
 *  @return     true - станица успешно скопирована, false - в процессе копирования произошла ошибка
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
 *  @brief      Считает контрольную сумму для всей страницы
 *  @details    Контрольная сумма хранится в последних ячейках страницы
 *              считается как CRC-32/POSIX
 *              
 *  @param[in]  page_addr Адрес страницы
 *              
 *  @return     Значение контрольной суммы
 */
static crc_t _flash_param_crc_calc(bsp_flash_addr_t _addr)
{
    static const crc::crc_sett_t<crc_t> crc_set = // считается как CRC-32/POSIX
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
 *  @brief      Проверяет CRC страницы
 *  @details    Dsxbckztn CRC и сравнивает с сохраненным в конце страницы значением
 *              
 *  @param[in]  page_addr Адрес страницы
 *              
 *  @return     true - расчетная CRC совпадает с сохраненной
 */
static bool _crc_is_valid(const bsp_flash_addr_t _addr)
{
    void * addr = _addr;
    
    _flash_addr_around(addr);
    
    crc_t crc = _flash_param_crc_calc(addr);
    
    return (crc == *(crc_t *)((uint32_t)addr + _flash_get_page_size(addr) - sizeof(uint32_t)));
};

/***************************************************************************************************
 *                            Работаем со структурой, записанной во флеш
 **************************************************************************************************/

/**
 *  @brief      Фактическая длина параметра в памяти (хедер + данные)
 *              
 *  @param[in]  *hdr указатель на хедер параметра в структуре
 *              
 *  @return     количество байт, занимаемых в структуре параметром
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
 *  @brief      Возвращает указатель на параметр по ID
 *  @details    Ищет в памяти нуждный ID. Просто обратиться к структуре нельзя, данные, лежащие во флеш могут не соответствовать текущей структуре
 *              
 *  @param[in]  params_addr Адрес структуры параметров
 *  @param[in]  id          ID параметра
 *              
 *  @return     указатель на параметр в памяти flash
 */
static param_header_t * _get_param_ptr(const __PACKED_STRUCT params_base_t * _addr, PARAM_ID_TYPE_t id)
{
    param_header_t * result = NULL;
    
    bsp_flash_size_t page_size = _flash_get_page_size((bsp_flash_addr_t &)_addr);
    
    for(auto i = 0; i < page_size; )
    {
        param_header_t * curr_hdr = (param_header_t *)((uint32_t)_addr + i);
        
        // if (!is_valid_id(curr_hdr->id) break;
        /* здесь по идее нужно проверять валидность id, но тогда теряется совместимость со старыми
         * прошивками. Т.е. перестает работать откат на старые прошивки.
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
 *  @brief      Проверка валидности ID
 *  @details    Проверяем, есть ли ID в глобальном списке ID
 *              
 *  @param[in]  id Значение ID или хедер целиком (лишние поля будут обрезаны)
 *              
 *  @return     true - ID есть в глобальном списке ID
 */
#define is_valid_id(id) ((bool)(((id) & ((1 << HEADER_FIELD_LEN_ID) - 1)) < PARAM_ID_COUNT))


/**
 *  @brief      Возвращает адрес начала BLOB данных на нужной странице
 *  @details    BLOB данные начинаются сразу после структуры настроек, но структура может не соответствовать дефолтной на этапе обновления
 *              
 *  @param[in]  page_addr Адрес страниц настроек
 *              
 *  @return     Адрес начала BLOB данных
 */
static void * _blob_start_ptr(const __PACKED_STRUCT params_base_t * _addr)
{
    param_header_t * param_end_ptr = _get_param_ptr(_addr, PARAM_ID_END);
    
    return (void *)((uint32_t)param_end_ptr + _param_size_get(param_end_ptr));
}


/**
 *  @brief      Преобразует параметр в значение триггера
 *  @details    Используется для защиты от смены типаданных, т.к. это поле имеет два назначения - хранить триггерные параметры и задавать тип данных
 *              
 *  @param[in]  val значение параметра
 *              
 *  @return     возвращает состояние триггера PARAM_NA, PARAM_ACTIV или PARAM_NOT_ACTIV
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
 *  @brief      Выделение памяти под буфер отложенной записи
 *  @details    Выделяет в куче память и инициализирует ее.
 *              
 *  @return     указатель на буфер
 */
static procrastinated_buf_t * _procrastinate_buf_callock(void)
{
    procrastinated_buf_t * buf = NULL;
    
    buf = (procrastinated_buf_t *)calloc(sizeof(procrastinated_buf_t), 1);
    
    if (buf == NULL)
    {
        // Если в куче не хватает места - вешаемся здесь
        BRK_PTR("heap overflow");
    }
    else
    {
        memset(buf, 0, sizeof(procrastinated_buf_t));
    }
    
    return buf;
}


/**
 *  @brief      Запись отложенных данных
 *              
 *  @param[in]  buf указатель на буфер отложенной записи
 */
static bool _procrastinate_write(procrastinated_buf_t * buf)
{
    bool result;
    
    if (buf != NULL)
    {
        // Реализуем отложенную запись
        while (FIFO_IS_EMPTY(*buf) == false)
        {
            procrastinated_blob_t tmp = FIFO_EXTRACT(*buf);
            
            result = _flash_memcpy(tmp.source_addr, tmp.target_addr, tmp.len);
            
            if (!result) return false;
        }
        
        // Освобождаем память
        free(buf);
    }

    // Выравниваем данные, если надо
    result = _flash_memcpy(NULL, NULL, 0);
    
    return result;
}


/**
 *  @brief      расчет и запись CRC
 *  @details    считает CRC и записывает в конец страницы настроек
 *              Только после этого страница считается валидной
 *              
 *  @param[in]  page_addr Адрес страницы, для которой рассчитываем CRC
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
 *                            реализация класса cpp_ro_params
 **************************************************************************************************/


/**
 *  @brief      Конструктор объекта настроек, доступных только для чтения
 *  @details    Агрегирует структуру настроек по указателю
 *              
 *  @param[in]  _base_addr - Адрес структуры настроек
 */
cpp_ro_params::cpp_ro_params(const params_base_t * _base_addr):
    main_addr_(_base_addr),
    blob_ptr_(NULL)
{
};

/**
 *  @brief      Актуализирует значение поля blob_ptr_ - указателя на начало BLOB данных
 *  @details    Агрегирует структуру настроек по указателю
 *              
 *  @return     Возвращает указатель на начало BLOB данных или NULL, если не удалось определить
 *              адрес (структура повреждена)
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
 *  @brief      Возвращает указатель на параметр по ID
 *  @details    Ищет в памяти нуждный ID. Просто обратиться к структуре нельзя, данные, лежащие во флеш могут не соответствовать текущей структуре
 *              
 *  @param[in]  id          ID параметра
 *              
 *  @return     указатель на параметр в памяти flash
 */
param_header_t * cpp_ro_params::get_param_ptr(PARAM_ID_TYPE_t _id)
{
    flash_lock();
    param_header_t * result = _get_param_ptr(main_addr_, _id);
    flash_unlock();
    
    return result;
};

/**
 *  @brief      Получить указатель на BLOB данные
 *  @details    Возвращает указатель на начало данных
 *              
 *  @param[in]  _id Идентификатор параметра

 *  @return     Указатель на начало данных или NULL, если данных нет или идентификатор не найден
 *              или элемнт настроек имеет другой тип
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
 *  @brief      Получить указатель на BLOB данные
 *  @details    Возвращает указатель на начало данных
 *              
 *  @param[in]  param Указатель на элемент настрек, для которого мы хотим получить указатель на
 *              данные

 *  @return     Указатель на начало данных или NULL, если данных нет или элемнт настроек имеет
 *              другой тип
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
 *  @brief      Получить указатель на BLOB данные
 *  @details    Возвращает указатель на начало данных
 *              
 *  @param[in]  param Указатель на элемент настрек, для которого мы хотим получить указатель на
 *              данные

 *  @return     Указатель на начало данных или NULL, если данных нет
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
 *  @brief      Получить параметр по ID
 *  @details    Используется для получения данных внешними приложениями.
 *
 *  @param[in]  id ID запрашиваемого параметра
 *              
 *  @return     Структура, содержащая данные или указатель на данные и их длину,
 *              в случае с blob-данными.
 *              если ID недоступен для чтения, то в поле id возвращаемой структуры будет PARAM_ID_NA
 */
flash_param_external_t cpp_ro_params::external_get(PARAM_ID_TYPE_t _id)
{
    flash_param_external_t result = {};
    
    flash_lock();
    
    param_header_t * param_header = get_param_ptr(_id);
    
    if (param_header != NULL)
    {
        _param_header_to_external(param_header, result);
        
        // Если данные - blob, то находим указатель
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
        // пытаемся найти во всех зарегистрированных структурах
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
            
            // Вытаскиваем указатель из чужого объекта
            if (result.type == PARAM_DATA_BLOB)
            {
                result.ptr = (result.len) ? arg.obj->blob_get_data(arg.param) : NULL;
            }
        }
        
        param_header = arg.param;
    }

    // если и тут пусто - запускаем пользовательскую функцию
    if (param_header == NULL)
    {
        result = user_external_get(_id);
    }
    
    flash_unlock();
    return result;
};


/***************************************************************************************************
 *                            реализация класса cpp_rw_params
 **************************************************************************************************/


/**
 *  @brief      Конструктор объекта настроек, для чтения и записи
 *  @details    Агрегирует структуру настроек по указателю
 *              
 *  @param[in]  _main - адрес основной структуры настроек
 *  @param[in]  _mirr - фдрес резервной страницы
 *  @param[in]  _def - адрес структуры настроек по умолчанию
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
        // неверные инициализационные данные
        BRK_PTR("cpp_rw_params creating error");
    }
};


/**
 *  @brief      Конструктор объекта настроек, для чтения и записи
 *  @details    Агрегирует структуру настроек по указателю
 *              
 *  @param[in]  _arg - Структура с базовыми адресами
 */
cpp_rw_params::cpp_rw_params(cpp_params_arg_t * _arg):
    cpp_rw_params(_arg->main_addr, _arg->mirr_addr, _arg->def_addr)
{
};


/**
 *  @brief      Проверяет что в основной странице лежит целостная структура
 *  @details    Проверяе только соответствие crc. Используется для конвертации старых версий
 *
 *  @return     true - контрольная сумма основной страницы настроек верна
 */
bool cpp_rw_params::crc_complete(void)
{
    return _crc_is_valid((bsp_flash_addr_t)(main_addr_));
}


/**
 *  @brief      Нужно ли обновлять структуру во flash
 *  @details    Структуру следует обновлять в следующих случаях:
 *                  - добавился новый параметр
 *                  - удален старый параметр
 *                  - изменилось расположение параметров
 *                  - изменился тип параметра (например, с uint8_t на uint16_t)
 *                  - изменился флаг прав доступа параметра
 *
 *  @return     true - структуру следует обновить
 */
bool cpp_rw_params::must_be_updated(void)
{
    bool result = false;
    
    for (auto i = 0; i < PARAM_ID_COUNT; i++)
    {
        param_header_t * param_hdr_dflt = _get_param_ptr(def_addr_, (PARAM_ID_TYPE_t)i);
        param_header_t * param_hdr_flash = _get_param_ptr(main_addr_, (PARAM_ID_TYPE_t)i);
        
        // параметра с таким id нет в устройстве
        if (   param_hdr_dflt == NULL
            && param_hdr_flash == NULL
            )
        {
            continue;
        }

        // параметр добавлен или удален
        else if (   param_hdr_dflt == NULL
                 || param_hdr_flash == NULL
                 )
        {
            result = true;
            break;
        }

        else if (   (uint32_t)param_hdr_dflt - (uint32_t)def_addr_ != (uint32_t)param_hdr_flash - (uint32_t)main_addr_ // изменилось месторасположение параметров
                 || param_hdr_dflt->secure_flag != param_hdr_flash->secure_flag // изменился флаг прав доступа параметра
                 || (   param_hdr_dflt->state != param_hdr_flash->state
                     && (param_hdr_dflt->state | param_hdr_flash->state) > PARAM_TRIGGER_OFF
                     ) // изменился тип параметра
                 )
        {
            result = true;
            break;
        }
    }
    
    return result;
}


/**
 *  @brief      Записываем обновленную структуру во флеш (с предварительным стиранием)
 *  @details    В целевую страницу записывается обновленная структура. Данные, по возможности берутся из структуры-источника.
 *              Если они конфликтуют с данными из дефолтной структуры, то их расположение, тип и права доступа меняются,
 *              Подстраиваясь под дефолтную структуру. при этом данные, по возможности, сохраняются.
 *              
 *  @param[in]  blob_save   true - blob данные, при преобразовании в тип фиксированной длины сохраняются (обрезаются по длине),
 *                          false - дефолтное значение
 */
bool cpp_rw_params::updating(const bool _blob_save)
{
    procrastinated_buf_t * blob_buf = _procrastinate_buf_callock();
    
    if (!_flash_erase_page((bsp_flash_addr_t &)mirr_addr_))
    {
        return false;
    }
    
    param_blob_t blob_target; // Последние значения указателя на BLOB данные
    blob_target.len = 0;
    blob_target.shift = 0;
    
    const auto i_max = (uint32_t)_blob_start_ptr(def_addr_) - (uint32_t)def_addr_;

    for(auto i = 0; i < i_max; )
    {
        param_header_t * param_hdr_dflt = (param_header_t *)((uint8_t *)def_addr_ + i);
        param_header_t * param_hdr_source = _get_param_ptr(main_addr_, (PARAM_ID_TYPE_t)(param_hdr_dflt->id));
        
        // Формируем BLOB
        if (param_hdr_dflt->state == PARAM_DATA_BLOB)
        {
            blob_target.header = ((param_blob_t *)param_hdr_dflt)->header;
            blob_target.shift += blob_target.len;
            uint32_t blob_src_addr;
            
            // В исходной структуре нет этого поля - записываем дефолтные значения
            if (param_hdr_source == NULL)
            {
                blob_target.len = ((param_blob_t *)param_hdr_dflt)->len;
                
                blob_src_addr = (uint32_t)_blob_start_ptr(def_addr_) + ((param_blob_t *)param_hdr_dflt)->shift;
            }
            // В структуре есть поле и оно имеет тип PARAM_DATA_BLOB
            else if (param_hdr_source->state == PARAM_DATA_BLOB)
            {
                blob_target.len = ((param_blob_t *)param_hdr_source)->len;
                
                blob_src_addr = (uint32_t)_blob_start_ptr(main_addr_) + ((param_blob_t *)param_hdr_source)->shift;
            }
            // В структуре есть поле, но оно фиксированной длины
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
            
            // Не записываем сами данные, т.к. запись в память должна идти последовательно
            // иначе можно похерить невыровненные по словам данные
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
                    // Вешаемся здесь, если размера буфера не хватает
                    BRK_PTR("blob_buf is overflow.");
                }
            }
            // flash_memcpy(blob_src_addr, target_addr + sizeof(params_t) + blob_target.shift, blob_target.len);

        }
        // Работаем с типами фиксированной длины
        else
        {
            uint64_t tmp = 0;

            // Определяем валидные данные и приводим их к типу uint64_t
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
            
            // Записываем данные согласно истинному типу
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
 *  @brief      Проверяет целостность структуры в памяти flash
 *  @details    Восстанавливает страницу и в случае потери данных и добавляет дефолтные параметры, в более старые версии
 *
 *  @warning    Должна вызываться раньше чем любые другие функции работы с настройками, иначе целостность настроек не гарантируется
 */
void cpp_rw_params::test_and_repair(void)
{
    if (   !_flash_page_is_clean((bsp_flash_addr_t)main_addr_)
        || !_flash_page_is_clean((bsp_flash_addr_t)mirr_addr_)
        )
    {
        // проверяем данные на возможные сбои в процессе записи
        if (!_crc_is_valid((bsp_flash_addr_t)main_addr_))
        {
            if (_crc_is_valid((bsp_flash_addr_t)mirr_addr_))
            {
                _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
            }
        }
    }
    
    // Сохраняем настройки архаичного формата, если они имеются
    convert_old_struct(false); // см. описание функции

    // проверяем данные на соответствие дефолтной структуре и обновляем структуру во флеш
    if (must_be_updated())
    {
        _flash_erase_page((bsp_flash_addr_t &)mirr_addr_);
        
        updating(true);
        
        _param_crc_write((bsp_flash_addr_t &)mirr_addr_);
        
        _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
    }
    
    // Восстанавливаем настройки, выдернутые из архаичной структуры
    convert_old_struct(true); // см. описание функции
};


/**
 *  @brief      Сохраняет параметр или возвращает false, если параметр отсутствует
 *  @details    
 *              
 *  @param[in]  id    ID параметра для сохранения
 *  @param[in]  value значение параметра
 *              
 *  @return     true - параметр сохранен, false - сохранение невозможно (параметр отсутствует)
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
                case PARAM_DATA_BLOB  : break; // Не пишем данные, если недопустимый тип
                case PARAM_DATA_1_BYTE: val_not_eq = ((param_uint8_t  *)param_header)->data != (uint8_t )value; break;
                case PARAM_DATA_2_BYTE: val_not_eq = ((param_uint16_t *)param_header)->data != (uint16_t)value; break;
                case PARAM_DATA_4_BYTE: val_not_eq = ((param_uint32_t *)param_header)->data != (uint32_t)value; break;
                case PARAM_DATA_8_BYTE: val_not_eq = ((param_uint64_t *)param_header)->data != (uint64_t)value; break;
                default               : val_not_eq = param_header->state != _value_to_state(value); break;
            }
            
            if (val_not_eq)
            {
                // Стираем резервную страницу
                _flash_erase_page((bsp_flash_addr_t &)mirr_addr_);
        
                param_header_t header = *param_header;
                
                // Пишем все данные до места, которое надо поменять
                uint32_t len = (uint32_t)param_header - (uint32_t)main_addr_;
                
                _flash_memcpy((void *)main_addr_, (void *)mirr_addr_, len);
                
                // Пишем новые данные
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
                
                // Дописываем все, что осталось
                _flash_memcpy(
                    (uint8_t *)main_addr_ + len,
                    (uint8_t *)mirr_addr_ + len,
                    _flash_get_page_size((bsp_flash_addr_t &)main_addr_) - len - sizeof(uint32_t));
                // Здесь следить за выравниванием не нужно, потому что пишем до конца страницы
                
                _param_crc_write((bsp_flash_addr_t)mirr_addr_);
                        
                _flash_page_copy((bsp_flash_addr_t)mirr_addr_, (bsp_flash_addr_t)main_addr_);
            }
        }
    }

    flash_unlock();
    
    return result;
};


/**
 *  @brief      Сохранение значения типа BLOB
 *  @details    Записыват в настройки данные произвольной длины
 *              
 *  @param[in]  id  ID параметра
 *  @param[in]  src указатель на данные
 *  @param[in]  len длина массива данных. Если len приравнивается к нулю, то данные удаляются.
 *              В этом случае src может иметь значение NULL
 *              
 *  @return     true  - запись прошла успешно.
 *              false - нет параметра с таким ID;
 *                    - данные не влезают в страницу настроек;
 *                    - страница повреждена.
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
                        // Если мы оказались здесь, значит, мы пишем что-то зело невпихуемое в страницу памяти
                        header = NULL;
                        break;
                    }
                    else if (curr_blob.len > 0)
                    {
                        // Не записываем сами данные, т.к. запись в память должна идти последовательно
                        // иначе можно похерить невыровненные по словам данные
                        procrastinated_blob_t tmp =
                        {
                            .source_addr = data_ptr,
                            .target_addr = (uint8_t *)mirr_addr_ + (uint32_t)blob_ptr_ - (uint32_t)main_addr_ + curr_blob.shift,
                            .len         = curr_blob.len,
                        };
                        FIFO_ADD(*blob_buf, tmp); // Не проверяем на переполнение, достаточно проверки в param_updating
                        // flash_memcpy((uint32_t)data_ptr, (uint32_t)&params_reserve + sizeof(params_t) + curr_blob.shift, curr_blob.len);
                    }
                }
            }
            
            _procrastinate_write(blob_buf);
            
            // Дописываем контрольную сумму и копируем резервную страницу в основную
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
 *  @brief      Сохранение значения типа BLOB
 *  @details    Записыват в настройки  строку данных
 *              
 *  @param[in]  id  ID параметра
 *  @param[in]  src указатель на строку
 *              
 *  @return     true - запись прошла успешно, false - неверный тип параметра с данным ID, нет параметра с таким ID, данные не влезают в страницу настроек
 */
bool cpp_rw_params::set(PARAM_ID_TYPE_t id, char * src)
{
    return set(id, src, strlen(src) + 1);
}

/**
 *  @brief      Сдвиг параметров в очереди FIFO
 *  @details    Объединяет соседние параметры в fifo очередь и подготавливает очередь к приему новых
 *              данных, сдвигая старые по кольцу и затирая самое старое значение. При этом значения
 *              параметров сдвигаются по ID вниз очереди. Параметры должны быть одного типа.
 *              Используется совместно с fifo_set
 *
 *  @param[in]  id    ID первого параметра в очереди
 *  @param[in]  count длина очереди, т.е. количество параметров в структуре, следующих по порядку
 *              вниз, начиная с ID первого параметра, которые формируют эту очередь
 *
 *  @return     Сдвиг успешно завершен
 *  @todo       Функция написана неоптимально и на длинных FIFO (count > 2) дает большое количество
 *              циклов стирания/записи. Есть смысл прерписать её на более низком уровне, но в
 *              настоящий момент некритично, т.к. не предполагается её активное использование 
 *              (только с очередью count = 2) при прописывании МЗК.
 */
bool cpp_rw_params::fifo_shift(PARAM_ID_TYPE_t id, uint8_t count)
{
    param_header_t *param_hdr_start = NULL;
    param_header_t *param_hdr_end = NULL;
    
    if (count == 0)
    {
        return false; // Очередь нулевой длины
    }
    
    param_hdr_end = param_hdr_start = get_param_ptr(id);
    
    if (param_hdr_start == NULL)
    {
        return false; // Неверный ID
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
                return false; // Типы в заявленном диапазоне ячеек не совпадают
            }
    }
    
    // Сдвигаем все параметры вниз
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
 *  @brief      блокировка доступа к флеш-памяти
 *  @details    ожидаем освобождения памяти, захватываем мьютекс
 */
void cpp_rw_params::flash_lock(void)
{
    while(flash_mutex.acquire(cpp_os::wait_forever) != cpp_os::ok);
};

/**
 *  @brief      разблокировка доступа к памяти
 *  @details    освобождаем мьютекс
 */
void cpp_rw_params::flash_unlock(void)
{
    flash_mutex.release();
};

/**
 *  @brief      Добавление параметра в fifo очередь
 *  @details    Объединяет соседние параметры в fifo очередь и добавляет новые значения по кольцу,
 *              затирая самые старые. При этом значения параметров сдвигаются по ID вниз очереди.
 *              Параметры должны быть одного типа.
 *              
 *  @param[in]  id    ID первого параметра в очереди
 *  @param[in]  value значение параметра
 *  @param[in]  count длина очереди, т.е. количество параметров в структуре, следующих по порядку
 *              вниз, начиная с ID первого параметра, которые формируют эту очередь
 *              
 *  @return     true - элемент успешно записан
 *
 *  @warning    Пока используется только для HL_ID, но никто не мешает использовать для любого
 *              параметра типа не param_triiger_t и не param_blob_t
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
 *  @brief      Сохранение значения типа BLOB в FIFO
 *  @details    Объединяет соседние параметры в fifo очередь и добавляет новые значения по кольцу,
 *              затирая самые старые. При этом значения параметров сдвигаются по ID вниз очереди.
 *              Параметры должны быть одного типа.
 *              
 *  @param[in]  id    ID первого параметра в очереди
 *  @param[in]  src   указатель на данные
 *  @param[in]  len   длина массива данных. Если len приравнивается к нулю, то данные удаляются.
 *  @param[in]  count длина очереди, т.е. количество параметров в структуре, следующих по порядку
 *              вниз, начиная с ID первого параметра, которые формируют эту очередь
 *              
 *  @return     true - элемент успешно записан
 *
 *  @warning    Пока используется только для HL_ID, но никто не мешает использовать для любого
 *              параметра типа не param_triiger_t и не param_blob_t
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
 *  @brief      Сохранение значения типа BLOB в FIFO
 *  @details    Объединяет соседние параметры в fifo очередь и добавляет новые значения по кольцу,
 *              затирая самые старые. При этом значения параметров сдвигаются по ID вниз очереди.
 *              Параметры должны быть одного типа.
 *              
 *  @param[in]  id    ID первого параметра в очереди
 *  @param[in]  src   указатель на строку
 *  @param[in]  count длина очереди, т.е. количество параметров в структуре, следующих по порядку
 *              вниз, начиная с ID первого параметра, которые формируют эту очередь
 *              
 *  @return     true - элемент успешно записан
 *
 *  @warning    Пока используется только для HL_ID, но никто не мешает использовать для любого
 *              параметра типа не param_triiger_t и не param_blob_t
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
 *  @brief      Записывает в параметр дефолтное значение
 *              
 *  @param[in]  id ID параметра
 *              
 *  @return     false параметр не был сброшен в дефолтное значение
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
 *  @brief      Проверяет параметр на соответствие дефолтному значению
 *  @details    Иными словами - проверяет, что параметр не был изменен
 *              
 *  @param[in]  id ID параметра
 *              
 *  @return     true параметр не был изменен в процессе работы устройства (соответствует дефолтному)
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
 *  @brief      Сохранить параметр
 *  @details    Сохраняет параметр
 *              
 *  @param[in]  id    ID параметра
 *  @param[out] value значение параметра
 *              
 *  @return     Структура, содержащая данные или указатель на данные и их длину,
 *              в случае с blob-данными.
 *              если ID недоступен для чтения, то в поле id возвращаемой структуры будет PARAM_ID_NA
 *              Структура содержит данные, фактически расположенные в памяти flash и считанные 
 *              методом external_get
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
 *  @brief      Записать BLOB данные
 *  @details    Записывает BLOB данные
 *              
 *  @param[in]  id  ID параметра
 *  @param[in]  src указатель на начало данных для записи
 *  @param[in]  len длина данных для записи
 *              
 *  @return     Структура, содержащая данные или указатель на данные и их длину,
 *              в случае с blob-данными.
 *              если ID недоступен для чтения, то в поле id возвращаемой структуры будет PARAM_ID_NA
 *              Структура содержит данные, фактически расположенные в памяти flash и считанные 
 *              методом external_get
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
 *  @brief      Записать BLOB-строку
 *  @details    Записывает BLOB данные и в случае успешной записи возвращает записанное значение
 *              
 *  @param[in]  id  ID параметра
 *  @param[in]  src указатель на строку
 *              
 *  @return     Структура, содержащая данные или указатель на данные и их длину,
 *              в случае с blob-данными.
 *              если ID недоступен для чтения, то в поле id возвращаемой структуры будет PARAM_ID_NA
 *              Структура содержит данные, фактически расположенные в памяти flash и считанные 
 *              методом external_get
 */
flash_param_external_t cpp_rw_params::external_set(PARAM_ID_TYPE_t id, char * src)
{
    return external_set(id, src, strlen(src));
}

/***************************************************************************************************
 *                                        END OF FILE
 **************************************************************************************************/
