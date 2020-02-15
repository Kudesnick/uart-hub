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
 *   File:          date_macro.h
 *   Description:   
 *
 ***************************************************************************************************
 *   History:       07.06.2019 - file created
 *
 **************************************************************************************************/

#pragma once

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
    using namespace std;
#endif

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

#define D __DATE__

// ��������� ��� �� ascii � �������-���������� ���
#define DATE_YEAR ((D[7] & 0x0F) << 12 | (D[8] & 0x0F) << 8 | (D[9] & 0x0F) << 4 | (D[10] & 0x0F))

// ���� byte > 0, �� ��������� - 1, ����� - 0
#define DIG_TO_ONE(byte) (((byte) >> 7 | (byte) >> 6 | (byte) >> 5 | (byte) >> 4 | \
                           (byte) >> 3 | (byte) >> 2 | (byte) >> 1 | (byte)) & 1)

// ��������� ����� byte, ���� ch1 == ch2, ����� - 0
#define CHR_TO_DIG(byte, ch1, ch2) ((byte) << (DIG_TO_ONE((ch1) ^ (ch2)) << 3) & 0xFF)

// ���������� �������-����������� ������ ������ �� ������������� ���������
#define DATE_MONTH ((CHR_TO_DIG(0x01, D[0], 'J') & CHR_TO_DIG(0x01, D[1], 'a')) + /* Jan */\
                    (CHR_TO_DIG(0x02, D[0], 'F'))                               + /* Feb */\
                    (CHR_TO_DIG(0x03, D[0], 'M') & CHR_TO_DIG(0x03, D[2], 'r')) + /* Mar */\
                    (CHR_TO_DIG(0x04, D[0], 'A') & CHR_TO_DIG(0x04, D[1], 'p')) + /* Apr */\
                    (CHR_TO_DIG(0x05, D[2], 'y'))                               + /* May */\
                    (CHR_TO_DIG(0x06, D[1], 'u') & CHR_TO_DIG(0x06, D[2], 'n')) + /* Jun */\
                    (CHR_TO_DIG(0x07, D[2], 'l'))                               + /* Jul */\
                    (CHR_TO_DIG(0x08, D[0], 'A') & CHR_TO_DIG(0x08, D[1], 'u')) + /* Aug */\
                    (CHR_TO_DIG(0x09, D[0], 'S'))                               + /* Sep */\
                    (CHR_TO_DIG(0x10, D[0], 'O'))                               + /* Oct */\
                    (CHR_TO_DIG(0x11, D[0], 'N'))                               + /* Nov */\
                    (CHR_TO_DIG(0x12, D[0], 'D'))                               ) /* Dec */

// ��������� ���� ������ �� ascii � �������-���������� ���
#define DATE_DAY  ((D[4] & 0x0F) << 4 | (D[5] & 0x0F))

#define DATE ((uint32_t)(DATE_YEAR << 16 | DATE_MONTH << 8 | DATE_DAY))

// ������ ��� ���������� �������������
#define DATE_CHAR_ARR ((DATE >> 20 & 0xF) + '0'), \
                      ((DATE >> 16 & 0xF) + '0'), \
                      ((DATE >> 12 & 0xF) + '0'), \
                      ((DATE >>  8 & 0xF) + '0'), \
                      ((DATE >>  4 & 0xF) + '0'), \
                      ((DATE >>  0 & 0xF) + '0')

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
