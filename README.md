# mynewt-tinyusb-example
Examples of TinyUSB running on mynewt

## How to run

Install dependency repos

```
$ newt install
```

### Nordic nRF52840DK PCA10056

Build and load bootloader

```
$ newt build pca10056_boot
$ newt load pca10056_boot
```

**NOTE** Tinyusb uses `nrfx_power` module to detect UBS VBUS state change. However nrfx_power is not ignored by
mynewt, you will need to modify 2 places as follow:

Firstly, remove nrfx_power.c from ignore list here https://github.com/apache/mynewt-core/blob/master/hw/mcu/nordic/pkg.yml#L40

```
pkg.ign_files.BSP_NRF52840:
    #- "nrfx_power.c"
    - "nrfx_power_clock.c"
```

Secondly, wrap #ifndef around `NRFX_POWER_ENABLED` so that we could defined in using compiler cfags https://github.com/apache/mynewt-core/blob/master/hw/mcu/nordic/nrf52xxx/include/nrfx52840_config.h#L31

```
#ifndef NRFX_POWER_ENABLED
#define NRFX_POWER_ENABLED 0
#endif
```

These are temporarily modification to get example running as proof of concept. I will try to come up with nicer solution later on.

Now we can build tinyusb example, create image and load it

```
$ newt build pca10056_tinyusb
$ newt create-image pca10056_tinyusb 0.0.0
$ newt load pca10056_tinyusb
```

### STMicroeclectronic STM32F4 Discovery

Work in progresss, not able to blink LED just yet.
