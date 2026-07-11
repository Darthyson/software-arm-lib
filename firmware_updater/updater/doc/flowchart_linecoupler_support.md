# Line Coupler Support

This document shows the flow of `DeviceManagement::startIntoBootLoader`, with
emphasis on which paths work transparently through KNX line couplers and which may fail.

## Background

**Old bootloader v1.20 (and older)** hardcodes its KNX physical address to `15.15.192` when entering bootloader mode.
Line couplers typically do **not** route to/from a device that lives on another line.
So the Updater may not reach the device at `15.15.192` and an update will fail.

**New bootloader** uses the same KNX physical address as the application address (`--device`).
This allows the Updater to reach the device through line couplers, even if the device is on a different line than the
Updater.

## Relevant CLI options

| Option                | Description                                                                 |
|-----------------------|-----------------------------------------------------------------------------|
| `--device <addr>`     | KNX address of the device while running the application.                    |
| `--progDevice <addr>` | KNX address the device uses in bootloader/prog mode (default: `15.15.192`). |

## Flowchart for `DeviceManagement::startIntoBootLoader`

```mermaid
flowchart TD
    START(["startIntoBootLoader(device, progDevice)"]) --> CHECK_DEVICE_SET{"--device set?"}

    %% Only --progDevice supplied
    CHECK_DEVICE_SET -- "No\n(legacy path)" --> NO_DEVICE_SET["checkDevicesInProgrammingMode(progDevice)"]
    NO_DEVICE_SET --> DEVICES_IN_PROG_MODE_RESULT{"Only 1 device and\nequals progDevice?"}
    DEVICES_IN_PROG_MODE_RESULT -- "No (0 or >1)" --> THROW_MANUAL(["throw:\nNo device or wrong device\nin programming mode"])
    DEVICES_IN_PROG_MODE_RESULT -- "Yes" --> RET_PROG_A(["return progDevice"])

    %% --device supplied: initial scan
    CHECK_DEVICE_SET -- "Yes" --> LIST_PROG_DEVICES["listDevicesInProgrammingMode()"]
    LIST_PROG_DEVICES --> COUNT0{"# devices\nin prog mode?"}

    %% 0 devices: restart the device then re-scan
    COUNT0 -- "0\n(device still running app)" --> RESTART["restartDeviceToBootloader(device)\nre-scan after restart\nlistDevicesInProgrammingMode()"]
    RESTART --> COUNT1{"# after\nrestart?"}

    COUNT1 -- "1 = device\n(new BL: kept own addr)" --> DEV_MATCH
    COUNT1 -- "1 = progDevice\n(old BL: moved to 15.15.192)" --> PROG_MATCH
    COUNT1 -- "0 / 1=other / >1" --> THROW_NO_RESTART(["throw:\nDevice did not appear\nor unexpected device"])

    %% Already 1 = device
    COUNT0 -- "1 = device\n(new BL already running)" --> DEV_MATCH["Device is at device\nWorks through line coupler"]
    DEV_MATCH --> RET_DEV_A(["return device"])

    %% Already 1 = progDevice  OR  restart -> old BL
    COUNT0 -- "1 = progDevice\n(old/new BL at 15.15.192\nbefore restart)" --> PROG_MATCH["Device is at progDevice\n15.15.192"]
    PROG_MATCH --> CHECK_OCC{"isAddressOccupied\n(device)?"}
    CHECK_OCC -- "Yes\n(address collision)" --> THROW_OCC(["throw:\n--device addr already\nin use by another device"])
    CHECK_OCC -- "No\n(address free)" --> PROG_PA["programNewPhysicalAddress(device)\nSend KNX write-address to the single\ndevice currently in prog mode"]
    PROG_PA --> PA_RES{"Address\nwritten OK?"}
    PA_RES -- "true\n(BL accepted new addr)" --> RET_DEV_B(["return device"])
    PA_RES -- "false\n(legacy BL disconnected /\nignored write)" --> PA_FAIL["Warn: falling back to progDevice\nKNX disconnect from legacy BL\nDevice stays at 15.15.192"]
    PA_FAIL --> FALLBACK_CHECK["checkDevicesInProgrammingMode\n(progDevice)"]
    FALLBACK_CHECK --> RET_PROG_B(["return progDevice\nlegacy path;\nline coupler may block"])

    %% Already 1 = unrelated  OR  >1
    COUNT0 -- "1 = other addr\n(unrelated device)" --> THROW_UNRELATED(["throw:\nunexpected device\nin prog mode"])
    COUNT0 -- ">1\n(ambiguous)" --> THROW_MULTI(["throw:\nmultiple devices\nin prog mode"])

    %% Styling
    classDef ok    fill:#2d6a2d,color:#fff,stroke:#1a4a1a
    classDef warn  fill:#8a6a00,color:#fff,stroke:#5a4a00
    classDef err   fill:#8a2020,color:#fff,stroke:#5a1010
    class RET_DEV_A,RET_DEV_B,RET_PROG_A ok
    class RET_PROG_B,PA_FAIL warn
    class THROW_MANUAL,THROW_NO_RESTART,THROW_OCC,THROW_UNRELATED,THROW_MULTI err
```

## Key structural detail: after restart

After `restartDeviceToBootloader()` the code calls `listDevicesInProgrammingMode()` again and
**falls through the same if-conditions** as the "already in prog mode" paths.  This means:

- **Restart → new BL** (device at `device`) → fast return, no address programming needed.
- **Restart → old BL** (device at `progDevice`) → goes through the **full** address-conflict check
  and `programNewPhysicalAddress` attempt, just like when the device was already in prog mode before
  the updater started. This ensures the address is always corrected if possible.

## Scenario summary

| # | `--device` | `--progDevice` | # Devices initially in prog mode | Running          | Result                                                                  | Line coupler safe? |
|---|------------|----------------|----------------------------------|------------------|-------------------------------------------------------------------------|--------------------|
| 1 | not set    | set            | 1 = progDevice                   | old or new BL    | return progDevice                                                       | no                 |
| 2 | set        | default        | 0                                | app + **new BL** | restart → keeps device address → return device                          | yes                |
| 3 | set        | default        | 0                                | app + **new BL** | restart → falls back to progDevice address → PA program → return device | yes                |
| 4 | set        | default        | 0                                | app + **old BL** | restart → gets progDevice address → PA rejected → fallback progDevice   | no                 |
| 5 | set        | default        | 1 = device                       | **new BL**       | return device                                                           | yes                |
| 6 | set        | 15.15.192      | 1 = progDevice                   | **new BL**       | PA program → return device                                              | yes                |
| 7 | set        | 15.15.192      | 1 = progDevice                   | **old BL**       | PA rejected → fallback progDevice                                       | no                 |
| 8 | set        | any            | 1 = other                        | -                | throw: unexpected device in prog mode                                   | -                  |
| 9 | set        | any            | >1                               | -                | throw: multiple devices in prog mode                                    | -                  |

## Line coupler compatibility behavior

- **New bootloader with application**   
  Device has the same address after a restart into bootloader → Should work through line couplers.
- **New bootloader without application**  
  Updater attempts `programNewPhysicalAddress` with value of `--device`.
  If successfully, device should have a line coupler compatible address.
- **Old bootloader**  
  Updater will fail `programNewPhysicalAddress` with KNX disconnect, because the old bootloader doesn't support it.
  Updater falls back to `--progDevice` (default 15.15.192). → 
  **No line coupler support** unless the line couplers filters are disabled.

## Disclaimer
Parts of this document and flowchart contain information generated by AI.
