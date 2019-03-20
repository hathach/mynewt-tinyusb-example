/**************************************************************************/
/*!
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/
#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#include "nrfx_power.h"
#include "tusb.h"

static volatile int g_task1_loops;

/* tinyusb function that handles power event (detected, ready, removed)
 * We must call it within SD's SOC event handler, or set it as power event handler if SD is not enabled.
 */
extern void tusb_hal_nrf_power_event(uint32_t event);

/* For LED toggling */
int g_led_pin;

void POWER_CLOCK_IRQHandler(void);
void USBD_IRQHandler(void);

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    NVIC_SetVector(POWER_CLOCK_IRQn, (uint32_t) POWER_CLOCK_IRQHandler);
    NVIC_SetVector(USBD_IRQn, (uint32_t) USBD_IRQHandler);

    // Power module init
    nrf_power_dcdcen_set(0);
    nrf_power_int_enable( NRF_POWER_INT_USBDETECTED_MASK | NRF_POWER_INT_USBREMOVED_MASK  | NRF_POWER_INT_USBPWRRDY_MASK);


    NRFX_IRQ_PRIORITY_SET(POWER_CLOCK_IRQn, 3);
    NRFX_IRQ_ENABLE(POWER_CLOCK_IRQn);

    uint32_t usb_reg = NRF_POWER->USBREGSTATUS;

    if ( usb_reg & POWER_USBREGSTATUS_VBUSDETECT_Msk ) {
      tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
    }

    if ( usb_reg & POWER_USBREGSTATUS_OUTPUTRDY_Msk ) {
      tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
    }

    tusb_init();

    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);

    while (1) {
        ++g_task1_loops;

        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC);

        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);
    }
    assert(0);

    return rc;
}


void POWER_CLOCK_IRQHandler(void)
{
  uint32_t enabled = nrf_power_int_enable_get();

    if ((0 != (enabled & NRF_POWER_INT_POFWARN_MASK)) &&
        nrf_power_event_get_and_clear(NRF_POWER_EVENT_POFWARN))
    {
        /* Cannot be null if event is enabled */
//        NRFX_ASSERT(m_pofwarn_handler != NULL);
//        m_pofwarn_handler();
    }

    if ((0 != (enabled & NRF_POWER_INT_SLEEPENTER_MASK)) &&
        nrf_power_event_get_and_clear(NRF_POWER_EVENT_SLEEPENTER))
    {
        /* Cannot be null if event is enabled */
//        NRFX_ASSERT(m_sleepevt_handler != NULL);
//        m_sleepevt_handler(NRFX_POWER_SLEEP_EVT_ENTER);
    }
    if ((0 != (enabled & NRF_POWER_INT_SLEEPEXIT_MASK)) &&
        nrf_power_event_get_and_clear(NRF_POWER_EVENT_SLEEPEXIT))
    {
        /* Cannot be null if event is enabled */
//        NRFX_ASSERT(m_sleepevt_handler != NULL);
//        m_sleepevt_handler(NRFX_POWER_SLEEP_EVT_EXIT);
    }

    if ((0 != (enabled & NRF_POWER_INT_USBDETECTED_MASK)) &&(NRF_POWER_EVENT_USBDETECTED))
    {
      tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
    }
    if ((0 != (enabled & NRF_POWER_INT_USBREMOVED_MASK)) && nrf_power_event_get_and_clear(NRF_POWER_EVENT_USBREMOVED))
    {
      tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_REMOVED);
    }
    if ((0 != (enabled & NRF_POWER_INT_USBPWRRDY_MASK)) && nrf_power_event_get_and_clear(NRF_POWER_EVENT_USBPWRRDY))
    {
      tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
    }
}
