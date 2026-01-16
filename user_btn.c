/******************************************************************************
 * File Name: user_btn.c
 *
 * Description: This file demonstrates the on-board User Button Configuration.
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
 *        Header Files
 *******************************************************************************/
#include <stdint.h>

#include "user_btn.h"

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "cybsp_types.h"
#include "cycfg_pins.h"
#include "cyhal_gpio.h"

#include "wiced_bt_types.h"
#include "wiced_timer.h"

/*******************************************************************************
 * Macros
 ********************************************************************************/

/* Stack size for BTN task */
#define BTN_TASK_STACK_SIZE (512u)

#define GPIO_INTERRUPT_PRIORITY (4u)

#define USER_BUTTON_TIMER_INTERVAL_MS 200

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

static void gpio_interrupt_handler(void *callback_arg, cyhal_gpio_event_t event);

/*******************************************************************************
 * Variable Definitions
 *******************************************************************************/

/* Handle of the btn task */
static TaskHandle_t btn_handle;
static uint32_t ms_timer_count = 0;
static gpio_interrupt_handler_t g_gpio_interrupt_handler_ptr = NULL;
static cyhal_gpio_callback_data_t cb_data = {.callback = gpio_interrupt_handler, .callback_arg = NULL};

/*******************************************************************************
 * Function Name: gpio_interrupt_handler
 ********************************************************************************
 * Summary:
 *   GPIO interrupt handler.
 *
 * Parameters:
 *  void *handler_arg (unused)
 *  cyhal_gpio_irq_event_t (unused)
 *
 * Return
 *  None
 *
 *******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint32_t previous_btn_pressed_timer = 0;
    static uint32_t current_btn_pressed_timer = 0;

    if (event != CYHAL_GPIO_IRQ_FALL) return;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
    vTaskNotifyGiveFromISR() unblocks the handling task, and the priority of
    the handling task is higher than the priority of the currently running task,
    then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
    xHigherPriorityTaskWoken = pdFALSE;

    current_btn_pressed_timer = ms_timer_count;

    /* Debounce logic to prevent spurious callbacks. Ensures atleast 600ms between button presses*/
    if ((current_btn_pressed_timer - previous_btn_pressed_timer) > (600 / USER_BUTTON_TIMER_INTERVAL_MS)) {
        previous_btn_pressed_timer = current_btn_pressed_timer;

        /* Unblock the handling task so the task can perform any processing necessitated
            by the interrupt.  xHandlingTask is the task's handle, which was obtained
            when the task was created. */
        vTaskNotifyGiveIndexedFromISR(btn_handle, 0, &xHigherPriorityTaskWoken);

        /* Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.
            The macro used to do this is dependent on the port and may be called
            portEND_SWITCHING_ISR. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*******************************************************************************
 * Function Name: user_btn_thread
 ********************************************************************************
 * Summary:
 *  This is a FreeRTOS task that handles the button press events.
 *
 * Parameters:
 *  None
 *
 * Return
 *  None
 *
 *******************************************************************************/
void user_btn_thread(void *arg)
{
    for (;;) {

        /* Block indefinitely (without a timeout, so no need to check the function's
        return value) to wait for a notification.  Here the RTOS task notification
        is being used as a binary semaphore, so the notification value is cleared
        to zero on exit.  NOTE!  Real applications should not block indefinitely,
        but instead time out occasionally in order to handle error conditions
        that may prevent the interrupt from sending any more notifications. */
        ulTaskNotifyTakeIndexed(0,              /* Use the 0th notification */
                                pdTRUE,         /* Clear the notification value before exiting. */
                                portMAX_DELAY); /* Block indefinitely. */

        if (g_gpio_interrupt_handler_ptr) g_gpio_interrupt_handler_ptr();
    }
}

void btn_timeout_handler(TimerHandle_t timer_handle)
{
    ms_timer_count++;
}

/**************************************************************************************************
 * Function Name: configure_user_btn
 ***************************************************************************************************
 * Summary:
 *   This function initializes a pin as input that triggers interrupt on falling edges.
 *
 * Parameters:
 *   None
 *
 * Return:
 *  None
 *
 *************************************************************************************************/
void configure_user_btn(gpio_interrupt_handler_t gpio_interrupt_handler)
{
    BaseType_t xReturned;
    TimerHandle_t hello_sensor_user_btn_timer;

    xReturned = xTaskCreate(user_btn_thread, "User btn task", BTN_TASK_STACK_SIZE, NULL, 4, &btn_handle);
    if (xReturned != pdPASS)
        return;

    /* Initialize the user btn */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN, &cb_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    hello_sensor_user_btn_timer = xTimerCreate("User Button Timer", pdMS_TO_TICKS(USER_BUTTON_TIMER_INTERVAL_MS), pdTRUE,
                                               NULL, btn_timeout_handler);
    xTimerStart(hello_sensor_user_btn_timer, USER_BUTTON_TIMER_INTERVAL_MS);

    g_gpio_interrupt_handler_ptr = gpio_interrupt_handler;
}
