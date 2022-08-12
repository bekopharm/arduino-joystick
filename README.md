# Arduino joystick

This repo allows you to emulate a joystick via an Arduino Mega connected via a serial port (on Linux only).

Gwilym Kuiper put this originally together for his Raspberry Pi based games console, in order to avoid having to attach a usb joystick. Read more about this in the blog post [here](https://gwilym.dev/2021/02/virtual-joystick-on-linux/).

My usecase is very similar. I was originally simply going to use my Arduino Mega as a DIY joystick for my simpit (simulated cockpit) to play various space games, like Elite Dangerous and Star Citizen) and perhaps also some flight simulator. For this I need _a lot_ more inputs but wasn't really aware that the Mega does not work as HID device. Read my blog post [here](https://beko.famkos.net/2022/03/27/using-an-arduino-mega-as-joystick-on-linux-pc/) to understand the problem.

I'm aware that this behavior can be changed by flashing the USB controller on the Mega. I'm fine with this approach though since I'm on Linux PC anyway and Gwilym Kuiper kindly provided most of the work on this already so I only had to extend the button range somewhat.

# Dependencies

Requires [PinChangeInterrupt](https://github.com/NicoHood/PinChangeInterrupt) v1.2.9

# Building

You will need the Arduino IDE to compile and upload the firmware and the Rust toolchain in order to build the driver.

The `joystick-firmware` directory contains the required code which you need to flash to an Arduino of your choice.
You can change the supported buttons, but there should be no more than 48 of them.

You will also need to update the corresponding buttons in buttons.rs if you change them in the firmware.

The driver can be compiled by doing `cargo build --release` in the `joystick-daemon` directory.
The resulting binary in `target/release` is what you need to run in order to connect this joystick.

# Building for a Raspberry Pi

Since this was intended for a Raspberry Pi build, you could either build this directly on a Pi, or do a cross compile.

## Cross compiling

Go to the `joystick-daemon/build-images` directory and build the crossbuild image.

```sh
$ docker build . -t crossbuild:local
```

The image _must_ be called `crossbuild:local`.

Once you've done that, go back to the `joystick-daemon` directory and using the cargo cross tool, run

```sh
$ cross build --target armv7-unknown-linux-gnueabihf --release
```

You can find the `cross` command here: https://github.com/rust-embedded/cross

The resulting executable can be found in `target/armv7-unknown-linux-gnueabihf/release/joystick-daemon`