# mynewt-tinyusb-example
Examples of TinyUSB running on mynewt

## How to run

Install dependency repos

```
$ newt install
```

Build bootloader, create image and load it

```
$ newt build nrf52_boot
$ newt create-image nrf52_boot 0.0.0
$ newt load nrf52_boot
```

Build tinyusb example, create image and load it

```
$ newt build nrf52_tinyusb
$ newt create-image nrf52_tinyusb 0.0.0
$ newt load nrf52_tinyusb
```
