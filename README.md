# mynewt-tinyusb-example
Examples of TinyUSB running on mynewt

## How to run

Clone and install dependency repos

```
$ git clone git@github.com:hathach/mynewt-tinyusb-example.git
$ cd mynewt-tinyusb-example
$ newt install
```

### Nordic nRF52840DK PCA10056

Build and load bootloader

```
$ newt build pca10056_boot
$ newt load pca10056_boot
```

Now we can build tinyusb example, create image and load it

```
$ newt build pca10056_tinyusb
$ newt create-image pca10056_tinyusb 0.0.0
$ newt load pca10056_tinyusb
```

### STMicroeclectronic STM32F4 Discovery

Work in progresss, not able to blink LED just yet.
