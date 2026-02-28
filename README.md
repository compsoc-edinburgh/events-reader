# Events Reader

This is a device made to read student ID's and check them against events
registrations.

## Hardware platform

The hardware platform is a Raspberry Pi Pico 2W (RP2350 MCU with wireless
module) with a ST25DV16 for reading NFC tags and a SPI module for MicroSD cards.

## Getting started with the firmware

General architecture:
 - The idea is a series of subsystems working together to make the device run.
 Subsystems can either by synchronous or asynchronous. Sync subsystems run on
 Core 0, whereas async subsystems run on Core 1.
 - The following subsystems will exist:
    - `networking/` - async subsystem handling network requests
    - `storage/` - async subsystem handling storage operations
    - `domain/` - both sync and async subsystem handling domain logic
    - `nfc/` - sync subsystem handling reading nfc tags

To build the firmware on NixOS (or with the Nix package manager with flakes
enabled)
 - `nix develop`
 - Hold the `BOOTSEL` button, and plug the device into a USB port on your
 computer.
 - `cd firmware && make build`
 - It will then build the firmware to a `.uf2` file in `firmware/build/`
 - From there you should mount and copy the `.uf2` file into the drive
   - `sudo mount /dev/sda1 /mnt && sudo cp build/*.uf2 /mnt && sudo umount /mnt`
 - You've now flashed the firmware to the board!

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
 - Write the storage subsystem.
 - Write the domain logic subsystem.
