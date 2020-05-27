# mynewt-tinyusb-example

![tinyUSB_240x100](https://user-images.githubusercontent.com/249515/62646655-f9393200-b978-11e9-9c53-484862f15503.png)

[![Build Status](https://github.com/hathach/mynewt-tinyusb-example/workflows/Build/badge.svg)](https://github.com/hathach/mynewt-tinyusb-example/actions) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

Due to the newt package build system, Mynewt examples are better to be a spin-off from the [main stack repo](https://github.com/hathach/tinyusb). To run the examples

- Firstly check out the [official Mynewt documentation](https://mynewt.apache.org/documentation/) to set up and install newt tool
- Clone and install dependency repos

```
$ git clone https://github.com/hathach/mynewt-tinyusb-example.git
$ cd mynewt-tinyusb-example
$ newt upgrade
```

## Supported Boards

Examples should be able to run with boards that are supported by Mynewt, provided that the mcu is already supported by TinyUSB stack. Following is board that is mainly used and tested

- [Nordic nRF52840 Development Kit (aka pca10056)](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)


## Examples

There are several ready-to-build target examples, following is build instructions

### Bootloader

Each board needs to have an bootloader if not previously loaded. This only needs to do once.

```
$ newt build pca10056-boot
$ newt load pca10056-boot
```

#### CDC MSC Composite

This example enumerated as composite device with CDC, MSC

```
$ newt build pca10056-cdc_msc
$ newt create-image pca10056-cdc_msc 1.0
$ newt load pca10056-cdc_msc
```

#### MSC Dual Logical Unit

This example enumerated as mass storage with dual ram disk

```
$ newt build pca10056-msc_dual_lun
$ newt create-image pca10056-msc_dual_lun 1.0
$ newt load pca10056-msc_dual_lun
```

### Bluetooth HCI Driver

This example enumerated as Bluetooth controller and allow Host OS to use it to perform scanning, connecting etc ...

```
$ newt build pca10056-blehci
$ newt create-image pca10056-blehci 1.0
$ newt load pca10056-blehci
```

Here are some Linux command to use:
Show available controllers (NimBLE should show up along with preexisting ones)

```shell script
hciconfig
```

To scan device around command line tool can be used

```
bluetoothctl

[bluetooth]# list
[bluetooth]# select CC:BB:10:10:20:20
[bluetooth]# scan on
[bluetooth]# scan off
[bluetooth]# devices
[bluetooth]# connect <some address from devices>

```

To see packets (that should match what USB transfers)

```shell script
btmon
```
