\# EmbedCrypMath — STM32H750 Donanımsal Şifreleme Modülü



\## Donanım Gereksinimleri



\### Ana Kart

\- \*\*WeAct Studio MiniSTM32H750VBTX\*\*

&#x20; - MCU: STM32H750VBT6 (Cortex-M7, 480MHz)

&#x20; - RAM: 128KB DTCM

&#x20; - Flash: 128KB (dahili) + 8MB QSPI (harici)

&#x20; - USB: Full Speed (USB-CDC Serial)



\### SD Kart Modülü

\- Kart üzerindeki dahili SDMMC modülü (4-bit mod)

\- FAT32 formatlı MicroSD kart (max 32GB önerilir)



\### Bağlantı

\- USB-C → Bilgisayar veya telefon (OTG) üzerinden Serial haberleşme

\- ST-Link \*\*gerekmez\*\* — DFU modu ile firmware yükleme



\---



\## Kurulum



\### 1. Gereksinimler

\- VS Code + PlatformIO eklentisi

\- STM32CubeProgrammer (firmware yüklemek için)



\### 2. Derleme

```

PlatformIO → Build

```



\### 3. Firmware Yükleme (DFU)

1\. BOOT0 butonunu basılı tut

2\. RESET butonuna bas ve bırak

3\. BOOT0 butonunu bırak

4\. STM32CubeProgrammer → USB → Connect

5\. firmware.bin dosyasını seç → Start Address: 0x08000000

6\. Start Programming



\### 4. SD Kart Hazırlığı

\- FAT32 formatla

\- `input.txt` dosyası oluştur, şifrelenecek metni yaz



\---



\## Kullanım



Serial Monitor'ü aç (115200 baud):



| Komut | Açıklama |

|-------|----------|

| `config` | GPS ve saat ayarla (varsayılan: İST→ANK, 12:00) |

| `encrypt` | SD/input.txt → SD/output.enc |

| `decrypt` | SD/output.enc → SD/output.dec |



\---



\## Proje Mimarisi

```

USB Serial (config/encrypt/decrypt)

&#x20;       ↓

&#x20;  State Machine

&#x20;       ↓

&#x20; Key Derivation (GPS + Saat → Mesafe → Anahtar)

&#x20;       ↓

&#x20; CrypMathCore (Hadamard / KhatriRao / Kronecker / TracySingh)

&#x20;       ↓

&#x20; SDMMC HAL (input.txt → output.enc)

```



\---



\## Güvenlik Özellikleri

\- \*\*Air-gapped\*\*: İnternet veya OS bağlantısı yok

\- \*\*Dinamik anahtar\*\*: GPS koordinatları + saat dilimi tabanlı

\- \*\*Donanımsal I/O\*\*: Tüm veri SD kart üzerinden, RAM'e tam yükleme yok

\- \*\*Zero heap\*\*: `std::array` tabanlı sabit bellek yapısı

