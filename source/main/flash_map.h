/***************************************************************************************************
 *   Project:     
 *   Author:        Stulov Tikhon (kudesnick@inbox.ru)
 ***************************************************************************************************
 *   Distribution:  
 *
 ***************************************************************************************************
 *   MCU Family:    STM32F
 *   Compiler:      ARMCC
 ***************************************************************************************************
 *   File:          flash_map.h
 *   Description:   
 *
 ***************************************************************************************************
 *   History:       14.11.2019 - file created
 *
 **************************************************************************************************/

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

#pragma once

/***************************************************************************************************
 *                                      INCLUDED FILES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       DEFINITIONS
 **************************************************************************************************/

// Device consts
#if defined(STM32F10X_CL)

    #define SETT_PAGE_SIZE      0x00000800 // ������ ������� ���������������� ��������
    #define FLASH_SIZE          0x00020000 // ������ ����-������
    #define FLASH_START         0x08000000 // ��������� ����� ����-������
    #define PROTECTED_SIZE      0x00001000 // ������ ����������������� �������
    // �������� �� ������� �������� ���������� ��� ���������� ����������� ��������
    #define RO_SETT_OFFSET      0x00000800 

    #define APPL_START          0x08004000 // ��������� ����� �������� ���������

    #define RAM_START           0x20000000 // ��������� ����� ���
    #define RAM_SIZE            0x00010000 // ������ ���

#elif defined(STM32F413xx)

    #define SETT_PAGE_SIZE      0x00004000 // ������ ������� ���������������� ��������
    #define FLASH_SIZE          0x00100000 // ������ ����-������
    #define FLASH_START         0x08000000 // ��������� ����� ����-������
    #define PROTECTED_SIZE      0x00008000 // ������ ����������������� �������
    // �������� �� ������� �������� ���������� ��� ���������� ����������� ��������
    #define RO_SETT_OFFSET      0x00000800

    #define APPL_START          0x08020000 // ��������� ����� �������� ���������

    #define RAM_START           0x20000000 // ��������� ����� ���
    #define RAM_SIZE            0x00050000 // ������ ���

#else
    #error Device type not found!
#endif

// Memory map vars
// ��������� ����� ���������������
#define MICROBOOT_START     FLASH_START                            
// ����� ��������, ����������� �� ������������
#define HW_SETT_START       (FLASH_START + RO_SETT_OFFSET)         
// ����� �������� �������� ���������������� ��������
#define USR_SETT_MAIN_START (FLASH_START + PROTECTED_SIZE)         
// ����� ��������� �������� ���������������� ��������
#define USR_SETT_MIRR_START (USR_SETT_MAIN_START + SETT_PAGE_SIZE) 
// ����� ��������� ����������
#define BOOT_START          (USR_SETT_MIRR_START + SETT_PAGE_SIZE) 
// ������ ��������� ����������
#define BOOT_SIZE           (APPL_START - BOOT_START)     

/***************************************************************************************************
 *                                      PUBLIC TYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/

/***************************************************************************************************
 *                                PUBLIC FUNCTION PROTOTYPES
 **************************************************************************************************/

/***************************************************************************************************
 *                                       END OF FILE
 **************************************************************************************************/
