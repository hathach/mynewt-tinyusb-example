# mynewt-tinyusb-example

![tinyUSB_240x100](https://user-images.githubusercontent.com/249515/62646655-f9393200-b978-11e9-9c53-484862f15503.png)

[![Build Status](https://github.com/hathach/mynewt-tinyusb-example/workflows/Build/badge.svg)](https://github.com/hathach/mynewt-tinyusb-example/actions) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

Due to the newt package build system, Mynewt examples are better to be a spin-off from the [main stack repo](https://github.com/hathach/tinyusb). To run the examples

- Firstly check out the [official Mynewt documentation](https://mynewt.apache.org/documentation/) to set up and install newt tool
- Clone and install dependency repos

```
$ git clone https://github.com/hathach/mynewt-tinyusb-example.git
$ cd mynewt-tinyusb-example
$ newt install
```

## Supported Boards

Examples should be able to run with boards that are supported by Mynewt, provided that the mcu is already supported by TinyUSB stack. Following is board that is mainly used and tested

- [Nordic nRF52840 Development Kit (aka pca10056)](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)

### Nordic nRF52840DK PCA10056

Build and load bootloader

```
$ newt build pca10056-boot
$ newt load pca10056-boot
```

Now we can build tinyusb example, create image and load it

#### CDC MSC HID composite example

This example enumerated as composite device with CDC, MSC and HID

```
$ newt build pca10056-cdc_msc
$ newt create-image pca10056-cdc_msc 1.0
$ newt load pca10056-cdc_msc
```

#### MSC dual Logical Unit

This example enumerated as mass storage with dual ram disk

```
$ newt build pca10056-msc_dual_lun
$ newt create-image pca10056-msc_dual_lun 1.0
$ newt load pca10056-msc_dual_lun
```
