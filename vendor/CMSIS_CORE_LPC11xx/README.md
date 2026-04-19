# CMSIS_CORE_LPC11xx for Cmake

## Overview

This project provides a CMake-based build system for the
ARM Cortex-M0 CMSIS core files targeting the NXP LPC1115 microcontroller.
It uses ARM CMSIS 3.20 core files together with NXP's LPC11xx device-specific headers.

## Supported Architectures

The build supports multiple target architectures via the `CMSIS_ARCHITECTURE` cache variable:

| Architecture | Target Library           | Description                                                   |
|--------------|--------------------------|---------------------------------------------------------------|
| `arm`        | `CMSIS_CORE_LPC11xx_arm` | Cross-compilation for ARM Cortex-M0 (requires toolchain file) |
| `x86`        | `CMSIS_CORE_LPC11xx_x86` | Native 32-bit build for testing/emulation                     |
| `x64`        | `CMSIS_CORE_LPC11xx_x64` | Native 64-bit build for testing/emulation                     |

The x86/x64 builds define `IAP_EMULATION` for use in host-based unit testing.

## Requirements

- [CMake](https://cmake.org/download/) >= 3.28
- [Ninja](https://ninja-build.org/) (Multi-Config generator)
- For ARM builds: `arm-none-eabi` toolchain of [MCUXpresso](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE)

## Building with Presets

```bash
# ARM target (Debug + Release)
cmake --workflow --preset build-arm-all

# x64 target (Debug + Release)
cmake --workflow --preset build-x64-all

# x86 target (Debug + Release)
cmake --workflow --preset build-x86-all
```

Or configure and build individually:

```bash
cmake --preset arm
cmake --build --preset debug-arm
```

## License

See [CMSIS END USER LICENSE AGREEMENT](docs/CMSIS%20END%20USER%20LICENCE%20AGREEMENT.pdf) for the ARM CMSIS license terms.

## Inspired and adapted from
mariosk8s´s https://github.com/mariosk8s/CMSIS_CORE_LPC11xx
