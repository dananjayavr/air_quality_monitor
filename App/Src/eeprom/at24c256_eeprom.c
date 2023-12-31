//
// Created by Dananjaya RAMANAYAKE on 01/12/2023.
//
#include <math.h>
#include <string.h>
#include "at24c256_eeprom.h"

// Define the I2C
extern I2C_HandleTypeDef hi2c2;
#define EEPROM_I2C &hi2c2

// EEPROM ADDRESS (8bits)
#define EEPROM_ADDR 0xA0

// Define the Page Size and number of pages
#define PAGE_SIZE 64     // in Bytes
#define PAGE_NUM  512    // number of pages

uint8_t bytes_temp[4];

// function to determine the remaining bytes
uint16_t bytestowrite (uint16_t size, uint16_t offset)
{
    if ((size+offset)<PAGE_SIZE) return size;
    else return PAGE_SIZE-offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void EEPROM_Write (uint16_t page, uint16_t offset, uint8_t *data, uint16_t size)
{

    // Find out the number of bit, where the page addressing starts
    int paddrposition = log(PAGE_SIZE)/log(2);

    // calculate the start page and the end page
    uint16_t startPage = page;
    uint16_t endPage = page + ((size+offset)/PAGE_SIZE);

    // number of pages to be written
    uint16_t numofpages = (endPage-startPage) + 1;
    uint16_t pos=0;

    // write the data
    for (int i=0; i<numofpages; i++)
    {
        /* calculate the address of the memory location
         * Here we add the page address with the byte address
         */
        uint16_t MemAddress = startPage<<paddrposition | offset;
        uint16_t bytesremaining = bytestowrite(size, offset);  // calculate the remaining bytes to be written

        HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADDR, MemAddress, 2, &data[pos], bytesremaining, 1000);  // write the data to the EEPROM

        startPage += 1;  // increment the page, so that a new page address can be selected for further write
        offset=0;   // since we will be writing to a new page, so offset will be 0
        size = size-bytesremaining;  // reduce the size of the bytes
        pos += bytesremaining;  // update the position for the data buffer

        HAL_Delay (5);  // Write cycle delay (5ms)
    }
}

void EEPROM_Read (uint16_t page, uint16_t offset, uint8_t *data, uint16_t size)
{
    int paddrposition = log(PAGE_SIZE)/log(2);

    uint16_t startPage = page;
    uint16_t endPage = page + ((size+offset)/PAGE_SIZE);

    uint16_t numofpages = (endPage-startPage) + 1;
    uint16_t pos=0;

    for (int i=0; i<numofpages; i++)
    {
        uint16_t MemAddress = startPage<<paddrposition | offset;
        uint16_t bytesremaining = bytestowrite(size, offset);
        HAL_I2C_Mem_Read(EEPROM_I2C, EEPROM_ADDR, MemAddress, 2, &data[pos], bytesremaining, 1000);
        startPage += 1;
        offset=0;
        size = size-bytesremaining;
        pos += bytesremaining;
    }
}

void EEPROM_PageErase (uint16_t page)
{
    // calculate the memory address based on the page number
    int paddrposition = log(PAGE_SIZE)/log(2);
    uint16_t MemAddress = page<<paddrposition;

    // create a buffer to store the reset values
    uint8_t data[PAGE_SIZE];
    memset(data,0xff,PAGE_SIZE);

    // write the data to the EEPROM
    HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADDR, MemAddress, 2, data, PAGE_SIZE, 1000);

    HAL_Delay (5);  // write cycle delay
}

void EEPROM_AllPageErase (void) {
    for (uint16_t i=0; i<PAGE_NUM; i++)
    {
        EEPROM_PageErase(i);
    }
}
