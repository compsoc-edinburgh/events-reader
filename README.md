# Events Reader

This is a device made to read student ID's and check them against events
registrations.

## Hardware platform

The hardware platform is a Raspberry Pi Pico 2W (RP2350 MCU with wireless
module) with a ST25DV16 for reading NFC tags.

## Getting started with the firmware

To build the firmware on NixOS (or with the Nix package manager with flakes
enabled)
 - `nix develop`
 - Hold the `BOOTSEL` button, and plug the device into a USB port on your
 computer.
 - `cd firmware && cargo run --release`
 - It will then build and flash the firmware to the device.

## TODOs:

### Hardware TODOs:
 - Build a debugger with another Pico to allow me to access SWD logs.
 - Build the rest of the hardware platform prototype.
 - Make proper schematics for the device (include in `hardware/schematics/`).
 - Design a 3D printed case for the device (include in `hardware/case/`).
 - Add a battery and allow the device to be used headlessly.

### Firmware TODOs:
 - Write the networking subsystem.
 - Write the NFC subsystem.
 - Write the rest of the owl
