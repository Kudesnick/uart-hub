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
 *   File:          cpp_ms.h
 *   Description:  
 *
 ***************************************************************************************************
 *   History:       09.12.2019 - file created
 *
 **************************************************************************************************/
 
#pragma once
 
/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdlib.h>
#include <stdint.h>

#include "cpp_os.h"

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/
 
/***************************************************************************************************
 *                                      PUBLIC TYPES
 **************************************************************************************************/

/// ������ ������ �������
typedef enum : uint16_t
{
    MS_REGIME_WAIT        = (1U <<  0), // ����� ��������
    MS_REGIME_INIT        = (1U <<  1), // ��������� ���������
    BIZ_REGIME_IMO_ARM    = (1U <<  2), // ������������ � ���� � ������
    MS_REGIME_IMO         = (1U <<  3), // ���� � ������
    MS_REGIME_AUTH        = (1U <<  4), // ������ �����������
    MS_REGIME_SERVICE     = (1U <<  5), // ��������� �����
    MS_REGIME_UPD         = (1U <<  6), // ���������� ��������
    MS_REGIME_ASTRT_LEARN = (1U <<  7), // �������� ������� ���������

    // �������������� �����
    MS_REGIME_NORST       = (1U << 14), // �� ������������ ������� ��� ����� ������
    MS_REGIME_NOKILL      = (1U << 15), // �� ������� ������� ��� ����� ������
    
    MS_REGIME_ALL         = ((1U << 14) - 1), // ��������� ������� �� ���� �������
} MS_REGIME_t;

/// �����-������ �������������
class cpp_ms_list: public cpp_list<cpp_ms_list>
{
public:
    virtual void change(MS_REGIME_t _regime) = 0;
};

/// �����������
template<uint32_t T_stack_size = OS_STACK_SIZE> class cpp_microservice:
    public cpp_os_thread<T_stack_size>, public cpp_ms_list
{
private:
    const uint32_t flags_;

public:
    cpp_microservice(const uint32_t _flags, const char * _name):
        cpp_os_thread<T_stack_size>(false, _name),
        flags_(_flags)
    {
    };
    
    void change(MS_REGIME_t _regime)
    {
        if (   true
            && !(flags_ & MS_REGIME_NOKILL)
            && (   false
                || !(flags_ & MS_REGIME_NORST)
                || !(flags_ & _regime)
                )
            )
        {
            this->terminate();
        }

        if (flags_ & _regime)
        {
            this->run();
        }
    };
};

/***************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/

/***************************************************************************************************
 *                                PUBLIC FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
