# Events Reader

This is a device made to read student ID's and check them against events
registrations.

## Getting started

To build the firmware on NixOS (or with the Nix package manager with flakes
enabled)
 - `nix develop`
 - Hold the `BOOTSEL` button, and plug the device into a USB port on your
 computer
 - `cd firmware && cargo run --release`
 - It will then build and flash the firmware to the device.

