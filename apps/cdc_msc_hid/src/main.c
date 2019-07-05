/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, hathach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "tusb.h"

#ifdef NRF52840_XXAA
#include "nrf_power.h"
#include "nrfx_power.h" // for NRFX_POWER_USB_EVT_DETECTED/USBREADY/USBREMOVED enum
#endif

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

#define USBD_STACK_SIZE   150
static struct os_task usbd_tsk;
static os_stack_t usbd_stack[OS_STACK_ALIGN(USBD_STACK_SIZE)];

#define CDC_STACK_SIZE    100
static struct os_task cdc_tsk;
static os_stack_t cdc_stack[OS_STACK_ALIGN(CDC_STACK_SIZE)];

#define HID_STACK_SIZE    100
static struct os_task hid_tsk;
static os_stack_t hid_stack[OS_STACK_ALIGN(HID_STACK_SIZE)];


void usb_hardware_init(void);
void usb_device_task(void* param);

//------------- main -------------//
int main (int argc, char **argv)
{
  int rc;

  sysinit();
  usb_hardware_init();

  tusb_init();

  // Create a task for tinyusb device stack
  os_task_init(&usbd_tsk, "usbd", usb_device_task, NULL, OS_TASK_PRI_HIGHEST+2, OS_WAIT_FOREVER, usbd_stack, USBD_STACK_SIZE);

#if CFG_TUD_CDC
  // Create a task for cdc
  extern void cdc_task(void* param);
  os_task_init(&cdc_tsk, "cdc", cdc_task, NULL, OS_TASK_PRI_HIGHEST+3, OS_WAIT_FOREVER, cdc_stack, CDC_STACK_SIZE);
#endif

#if CFG_TUD_HID
  // Create a task for hid
  extern void hid_task(void* params);
  os_task_init(&hid_tsk, "hid", hid_task, NULL, OS_TASK_PRI_HIGHEST+4, OS_WAIT_FOREVER, hid_stack, HID_STACK_SIZE);
#endif

  hal_gpio_init_out(LED_BLINK_PIN, 1);
  hal_gpio_init_in(BUTTON_1, HAL_GPIO_PULL_UP);

  while ( 1 )
  {
    // Use main task for blinking
    os_time_delay( os_time_ms_to_ticks32(blink_interval_ms) );
    hal_gpio_toggle(LED_BLINK_PIN);
  }

  assert(0);
  return rc;
}

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void usb_device_task(void* param)
{
  (void) param;

  // RTOS forever loop
  while (1)
  {
    // tinyusb device task
    tud_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
#if CFG_TUD_CDC
void cdc_task(void* params)
{
  (void) params;

  // RTOS forever loop
  while ( 1 )
  {
    if ( tud_cdc_connected() )
    {
      // connected and there are data available
      if ( tud_cdc_available() )
      {
        uint8_t buf[64];

        // read and echo back
        uint32_t count = tud_cdc_read(buf, sizeof(buf));

        for(uint32_t i=0; i<count; i++)
        {
          tud_cdc_write_char(buf[i]);

          if ( buf[i] == '\r' ) tud_cdc_write_char('\n');
        }

        tud_cdc_write_flush();
      }
    }

    // delay to yield
    os_time_delay(1);
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;

  // connected
  if ( dtr && rts )
  {
    // print initial message when connected
    tud_cdc_write_str("\r\nTinyUSB CDC MSC HID device with MyNewt example\r\n");
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}

#endif

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+
#if CFG_TUD_HID

// Must match with ID declared by HID Report Descriptor, better to be in header file
enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE
};

void hid_task(void* params)
{
  (void) params;

  while (1)
  {
    // Poll every 10ms
    os_time_delay( os_time_ms_to_ticks32(10) );

    // button is active low
    int const btn = 1 - hal_gpio_read(BUTTON_1);

    // Remote wakeup
    if ( tud_suspended() && btn )
    {
      // Wake up host if we are in suspend mode
      // and REMOTE_WAKEUP feature is enabled by host
      tud_remote_wakeup();
    }

    /*------------- Mouse -------------*/
    if ( tud_hid_ready() )
    {
      if ( btn )
      {
        int8_t const delta = 5;

        // no button, right + down, no scroll pan
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);

        // delay a bit before attempt to send keyboard report
        os_time_delay( os_time_ms_to_ticks32(2) );
      }
    }

    /*------------- Keyboard -------------*/
    if ( tud_hid_ready() )
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

        has_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_key = false;
      }
    }
  }
}

uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // TODO not Implemented
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

#endif


//--------------------------------------------------------------------+
// NRF52840 power management
//--------------------------------------------------------------------+
#ifdef NRF52840_XXAA

// tinyusb function that handles power event (detected, ready, removed)
// We must call it within SD's SOC event handler, or set it as power event handler if SD is not enabled.
extern void tusb_hal_nrf_power_event(uint32_t event);

extern void USBD_IRQHandler(void);
extern void POWER_CLOCK_IRQHandler(void);

void usb_hardware_init(void)
{
  // Setup USB IRQ
  NVIC_SetVector(USBD_IRQn, (uint32_t) USBD_IRQHandler);
  NVIC_SetPriority(USBD_IRQn, 2);

  // Setup Power IRQ to detect USB VBUS state ( detected, ready, removed)
  NVIC_SetVector(POWER_CLOCK_IRQn, (uint32_t) POWER_CLOCK_IRQHandler);
  NVIC_SetPriority(POWER_CLOCK_IRQn, 7);
  nrf_power_int_enable(
        NRF_POWER_INT_USBDETECTED_MASK |
        NRF_POWER_INT_USBREMOVED_MASK  |
        NRF_POWER_INT_USBPWRRDY_MASK);

  NVIC_EnableIRQ(POWER_CLOCK_IRQn);

  // USB power may already be ready at this time -> no event generated
  // We need to invoke the handler based on the status initially
  uint32_t usb_reg = NRF_POWER->USBREGSTATUS;

  if ( usb_reg & POWER_USBREGSTATUS_VBUSDETECT_Msk )
  {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
  }

  if ( usb_reg & POWER_USBREGSTATUS_OUTPUTRDY_Msk )
  {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
  }
}

// Power ISR to detect USB VBUS state
void POWER_CLOCK_IRQHandler(void)
{
  uint32_t enabled = nrf_power_int_enable_get();

  if ((0 != (enabled & NRF_POWER_INT_USBDETECTED_MASK)) &&
      nrf_power_event_get_and_clear(NRF_POWER_EVENT_USBDETECTED))
  {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
  }

  if ((0 != (enabled & NRF_POWER_INT_USBREMOVED_MASK)) &&
      nrf_power_event_get_and_clear(NRF_POWER_EVENT_USBREMOVED))
  {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_REMOVED);
  }

  if ((0 != (enabled & NRF_POWER_INT_USBPWRRDY_MASK)) &&
      nrf_power_event_get_and_clear(NRF_POWER_EVENT_USBPWRRDY))
  {
    tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
  }
}

#endif
