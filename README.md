# CrypMath-Embedded Version

> A custom cryptographic engine running on STM32H750, completely isolated from the internet and operating systems. All I/O is handled through a hardware SD card module (SDMMC), preventing any sensitive data from leaking to a computer or network.

---

##  Hardware

<img width="970" height="728" alt="image" src="https://github.com/user-attachments/assets/25d5636b-0b08-4a83-900f-954568d4f31a" />


*WeAct Studio MiniSTM32H750VBTX — the target board for this project*

---

##  Hardware Requirements

### Main Board — WeAct Studio MiniSTM32H750VBTX

| Specification | Detail |
|---|---|
| MCU | STM32H750VBT6 |
| Architecture | ARM Cortex-M7 |
| Clock Speed | 480 MHz |
| DTCM RAM | 128 KB |
| Internal Flash | 128 KB |
| External Flash | 8 MB QSPI |
| FPU | Double precision (FPv5-D16) |
| USB | Full Speed USB-CDC (Virtual COM Port) |
| SD Interface | SDMMC1 (4-bit mode, hardware) |
| Dimensions | 40mm x 60mm |

### MicroSD Card
| Specification | Detail |
|---|---|
| Format | FAT32 (mandatory) |
| Max Size | 32GB recommended |
| Interface | SDMMC 4-bit hardware (not SPI) |
| Speed | Class 10 or above recommended |

### Connection
- **USB-C** → PC or smartphone (via OTG adapter) for Serial communication
- **No ST-Link required** — firmware upload via DFU bootloader

---

##  PlatformIO Configuration

```ini
[env:weact_mini_h750vbtx]
platform = ststm32
board = weact_mini_h750vbtx
framework = arduino
upload_protocol = dfu

build_flags =
    -D PIO_FRAMEWORK_ARDUINO_ENABLE_CDC
    -D USBCON
    -D USBD_VID=0x0483
    -D USBD_PID=0x5740
    -D HAL_PCD_MODULE_ENABLED
    -mfpu=fpv5-d16        ; Enable double-precision FPU
    -mfloat-abi=hard      ; Hard float ABI
    -D ARM_MATH_CM7       ; CMSIS-DSP
    -Os                   ; Size optimization
    -std=gnu++17
    -Wno-psabi

lib_deps =
    stm32duino/FatFs
```

> **Note:** The `lib/STM32duino STM32SD` folder is included directly in this repo with a patched `SdFatFs.cpp` (f_unmount → f_mount fix for FatFs 2.0.3 compatibility).

---

##  Build & Flash Instructions

### Requirements
- [VS Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)

### 1. Clone & Build
```bash
git clone https://github.com/yourusername/EmbedCrypMath.git
cd EmbedCrypMath
# Open in VS Code, then PlatformIO → Build
```

### 2. Enter DFU Mode
1. Hold **BOOT0** button
2. Press and release **RESET** button
3. Release **BOOT0** button
4. Board is now in DFU bootloader mode

### 3. Flash Firmware (STM32CubeProgrammer)
1. Open STM32CubeProgrammer
2. Select **USB** interface → **Connect**
3. Go to **Erasing & Programming** tab
4. File path: `.pio/build/weact_mini_h750vbtx/firmware.bin`
5. Start address: `0x08000000`
6. Click **Start Programming**
7. Press **RESET** to run the firmware

### 4. Prepare SD Card
- Format as **FAT32**
- Create `input.txt` with the plaintext to encrypt
- Insert into the board

---

##  Usage

Open Serial Monitor at **115200 baud** (PlatformIO, PuTTY, or Serial USB Terminal on Android):

| Command | Description |
|---|---|
| `config` | Load default GPS config (Istanbul → Ankara, Hour: 12) |
| `encrypt` | Read `SD/input.txt` → encrypt → write `SD/output.enc` |
| `decrypt` | Read `SD/output.enc` → decrypt → write `SD/output.dec` |

### Example Session
```
[SYSTEM] CrypMath STM32H750 Booting...
[SD] MicroSD Mounted (SDMMC 4-bit)
[Komutlar] config / encrypt / decrypt

> config
[Config] IST(41,28) -> ANK(39,32), Hour: 12
[State] Ready.

> encrypt
[Encrypt] Zone=3 Distance=1234567
[Encrypt] Done. Blocks: 4
[Encrypt] Output: SD/output.enc

> decrypt
[Decrypt] Zone=3 Distance=1234567
[Decrypt] Done. Blocks: 4
[Decrypt] Output: SD/output.dec
```

---

##  System Architecture

```
┌─────────────────────────────────────────┐
│         USB Serial Interface            │
│     config / encrypt / decrypt          │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│            USB State Machine            │
│   STATE_CONFIG → READY → ENCRYPT/       │
│                          DECRYPT        │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│           Key Derivation Layer          │
│  GPS Coordinates + Hour → ECEF →        │
│  Distance Metric → keyDigits[]          │
│                                         │
│  Hour 00-05 → DisDD  (Chebyshev)        │
│  Hour 06-11 → TetHD  (Tetrahedral)      │
│  Hour 12-17 → TruOD  (Truncated Oct.)   │
│  Hour 18-23 → TriOD  (Triangular)       │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│            CrypMathCore Engine          │
│                                         │
│  Zone 1 → Hadamard    (Matrix4)         │
│  Zone 2 → Khatri-Rao  (Matrix8)         │
│  Zone 3 → Kronecker   (Matrix16)        │
│  Zone 4 → Tracy-Singh (Matrix16)        │
│                                         │
│  Zero heap: std::array based            │
│  Stack allocated, deterministic         │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│         SDMMC HAL Driver                │
│  Streaming I/O — no full RAM load       │
│  input.txt → [encrypt] → output.enc     │
│  output.enc → [decrypt] → output.dec    │
└─────────────────────────────────────────┘
```

---

##  Security Features

| Feature | Detail |
|---|---|
| **Air-gapped** | No internet, no OS, fully isolated |
| **Dynamic keys** | GPS coordinates + time-based derivation |
| **Hardware I/O** | All data via SDMMC, never exposed to host |
| **Zero heap** | `std::array` only, no heap fragmentation |
| **Streaming** | Files processed block-by-block, no full RAM load |
| **Output header** | `HDR:Z<zone>:D<distance>` — decryption self-contained |

---

##  Output File Format

```
output.enc:
  HDR:Z3:D1234567        ← zone + distance (needed for decryption)
  0023400156...          ← encrypted blocks, each int is 5 chars wide
```

---

##  Project Structure

```
EmbedCrypMath/
├── src/
│   ├── main.cpp            ← State machine + pipelines
│   ├── CrypMathCore.cpp    ← Math engine implementation
│   └── CrypMathCore.h      ← Types, structs, declarations
├── lib/
│   └── STM32duino STM32SD/ ← Patched library (f_mount fix)
├── platformio.ini
├── .gitignore
└── README.md
```

---

##  License

You can find details from LICENSE.md file
