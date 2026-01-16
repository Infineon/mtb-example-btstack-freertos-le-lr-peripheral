/******************************************************************************
* File Name: app_serial_flash.c
*
* Description: This file contains block device function implementations
*              required by kv-store library
*
* Related Document: See README.md
*
*
*******************************************************************************
 * (c) 2021-2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
 *                              INCLUDES
 ******************************************************************************/

#include "cycfg_qspi_memslot.h"
#include "cy_serial_flash_qspi.h"
#include "mtb_kvstore.h"
#include "app_serial_flash.h"

/*******************************************************************************
 *                              FUNCTION DECLARATIONS
 ******************************************************************************/
static uint32_t bd_read_size(void* context, uint32_t addr);
static uint32_t bd_program_size(void* context, uint32_t addr);
static uint32_t bd_erase_size(void* context, uint32_t addr);
static cy_rslt_t bd_read(void* context, uint32_t addr, uint32_t length, uint8_t* buf);
static cy_rslt_t bd_program(void* context, uint32_t addr, uint32_t length, const uint8_t* buf);
static cy_rslt_t bd_erase(void* context, uint32_t addr, uint32_t length);

/*******************************************************************************
 *                              FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * Function Name:
 * bd_read_size
 *
 * Function Description:
 * @brief Function to get the read size of the block device for a specific address.
 *
 * @param  void* context : Context object that is passed into mtb_kvstore_init
           uint32_t addr : Address for which the read size is queried. This address is passed in as start_addr + offset.
 *
 *
 * @return uint32_t: Read size of the memory device.
 *
 */
static uint32_t bd_read_size(void* context, uint32_t addr)
{
    (void)context;
    (void)addr;
    return 1;
}

/**
 * Function Name:
 * bd_program_size
 *
 * Function Description:
 * @brief Function to get the program size of the block device for a specific address.
 *
 * @param  void* context: Context object that is passed into mtb_kvstore_init
           uint32_t addr: Address for which the program size is queried. This address is passed in as start_addr + offset.
 *
 *
 * @return uint32_t: Program size of the memory device.
 */
static uint32_t bd_program_size(void* context, uint32_t addr)
{
    (void)context;
    return cy_serial_flash_qspi_get_prog_size(addr);
}

/**
 * Function Name:
 * bd_erase_size
 *
 * Function Description:
 * @brief Function prototype to get the erase size of the block device for a specific address.
 *
 * @param  void* context: Context object that is passed into mtb_kvstore_init
           uint32_t addr: Address for which the program size is queried. This address is passed in as start_addr + offset.
 *
 *
 * @return uint32_t Erase size of the memory device.
 * */
static uint32_t bd_erase_size(void* context, uint32_t addr)
{
    (void)context;
    return cy_serial_flash_qspi_get_erase_size(addr);
}

/**
 * Function Name:
 * bd_read
 *
 * Function Description:
 * @brief Function for reading data from the block device.
 *
 * @param  void* context : Context object that is passed into mtb_kvstore_init
           uint32_t addr: Address to read the data from the block device. This address is passed in as start_addr + offset
           uint32_t length : Length of the data to be read into the buffer
           uint8_t* buf : Buffer to read the data.
 *
 *
 * @return cy_rslt_t: Result of the read operation.
 * */
static cy_rslt_t bd_read(void* context, uint32_t addr, uint32_t length, uint8_t* buf)
{
    (void)context;
    return cy_serial_flash_qspi_read(addr, length, buf);
}

/**
 * Function Name:
 * bd_program
 *
 * Function Description:
 * @brief
 *
 * @param  void* context : Context object that is passed into mtb_kvstore_init
           uint32_t addr: Address to program the data into the block device. This address is passed in as start_addr + offset
           uint32_t length : Length of the data to be written
           uint8_t* buf : Data that needs to be written
 * @return cy_rslt_t: Result of the program operation.
 **/
static cy_rslt_t bd_program(void* context, uint32_t addr, uint32_t length, const uint8_t* buf)
{
    (void)context;
    return cy_serial_flash_qspi_write(addr, length, buf);
}

/**
 * Function Name:
 * bd_erase
 *
 * Function Description:
 * @brief
 *
 * @param  context: Context object that is passed into mtb_kvstore_init
           uint32_t addr: Address to erase the data from the device. This address is passed in as start_addr + offset
           uint32_t length: Length of the data that needs to be erased
 *
 *
 * @return cy_rslt_t : Result of the erase operation.
 **/
static cy_rslt_t bd_erase(void* context, uint32_t addr, uint32_t length)
{
    (void)context;
    return cy_serial_flash_qspi_erase(addr, length);
}
/**
 * Function Name:
 * app_kvstore_bd_init
 *
 * Function Description:
 * @brief  This function provides the pointer to the implementated prototype function for the block device.
 *
 * @param  mtb_kvstore_bd_t : Block device interface
 *
 * @return void
 **/
void app_kvstore_bd_init(mtb_kvstore_bd_t* device)
{
    device->read         = bd_read;
    device->program      = bd_program;
    device->erase        = bd_erase;
    device->read_size    = bd_read_size;
    device->program_size = bd_program_size;
    device->erase_size   = bd_erase_size;
    device->context      = NULL;
}

/* END OF FILE [] */
