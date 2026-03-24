#include <Arduino.h>
#include <STM32SD.h>
#include "CrypMathCore.h"

/**
 * @project: CrypMathEmbedded
 * @platform: STM32H750
 * @description: Secure matrix-based streaming encryption system via SDMMC.
 */

// ------------------------------------------------------------
// --- SYSTEM CONSTANTS ---
// ------------------------------------------------------------
static const uint8_t  BLOCK_CHARS      = 16;
static const uint8_t  MAX_CONFIG_SLOTS  = 10;

enum SystemState : uint8_t {
    STATE_CONFIG  = 0,
    STATE_READY   = 1,
    STATE_ENCRYPT = 2,
    STATE_DECRYPT = 3,
    STATE_ERROR   = 4
};

CordinateGps sGps, rGps;
int          currentHour = 0;
SystemState  systemState = STATE_CONFIG;

// ------------------------------------------------------------
// --- HELPER: FIXED-WIDTH INTEGER IO (SD STORAGE) ---
// ------------------------------------------------------------

/**
 * @brief Writes a 5-character fixed-width integer to the file.
 */
inline void writeInt(File& f, int32_t v) {
    char buf[5];
    bool neg = (v < 0);
    uint32_t uv = neg ? (uint32_t)(-v) : (uint32_t)v;
    buf[4] = '0' + (uv % 10); uv /= 10;
    buf[3] = '0' + (uv % 10); uv /= 10;
    buf[2] = '0' + (uv % 10); uv /= 10;
    buf[1] = '0' + (uv % 10); uv /= 10;
    buf[0] = neg ? '-' : ('0' + (uv % 10));
    f.write((uint8_t*)buf, 5);
}

/**
 * @brief Reads a 5-character fixed-width integer from the file.
 */
inline int32_t readInt(File& f) {
    char buf[5];
    if (f.read(buf, 5) != 5) return 0;
    bool neg = (buf[0] == '-');
    int32_t val = 0;
    for (int i = neg ? 1 : 0; i < 5; ++i) val = val * 10 + (buf[i] - '0');
    return neg ? -val : val;
}

// ------------------------------------------------------------
// --- ENCRYPTION PIPELINE (STREAMING MODE) ---
// ------------------------------------------------------------

void runEncryptPipeline(File& inFile, const KeyContext& ctx) {
    SD.remove("output.enc");
    File outFile = SD.open("output.enc", FILE_WRITE);
    if (!outFile) { 
        Serial.println("[ERROR] IO_FAILURE: Could not create output.enc"); 
        return; 
    }

    // Header Construction
    outFile.print("HDR:Z");
    outFile.print(ctx.zone);
    outFile.print(":D");
    char distBuf[21]; int pos = 20; distBuf[pos] = '\0';
    int64_t d = ctx.distance;
    if (d == 0) { distBuf[--pos] = '0'; }
    else { while (d > 0) { distBuf[--pos] = '0' + (d % 10); d /= 10; } }
    outFile.print(distBuf + pos);
    outFile.print("\n");

    size_t blockIndex = 0;
    while (inFile.available()) {
        char raw[BLOCK_CHARS];
        int  bytesRead = 0;
        while (bytesRead < BLOCK_CHARS && inFile.available())
            raw[bytesRead++] = (char)inFile.read();
        for (int i = bytesRead; i < BLOCK_CHARS; ++i) raw[i] = ' ';

        Matrix4 plainMat{};
        for (int i = 0; i < BLOCK_CHARS; ++i) {
            int32_t val = charToInt(raw[i]);
            if (val == -1) {
                Serial.print("[WARN] CHARSET_MISMATCH: Block ");
                Serial.print(blockIndex);
                Serial.println(" - Char replaced with space");
                val = charToInt(' ');
            }
            plainMat[i / 4][i % 4] = val;
        }

        Matrix4 magic = magicMatris(ctx.keyDigits[blockIndex % ctx.keyLen]);

        // Zone-based Matrix Transformation
        switch (ctx.zone) {
            case 1: { Matrix4  res = hadamardMul(magic, plainMat);   for (auto& row : res) for (int32_t v : row) writeInt(outFile, v); break; }
            case 2: { Matrix8  res = khatriRaoMul(magic, plainMat);  for (auto& row : res) for (int32_t v : row) writeInt(outFile, v); break; }
            case 3: { Matrix16 res = kroneckerMul(magic, plainMat);  for (auto& row : res) for (int32_t v : row) writeInt(outFile, v); break; }
            case 4: { Matrix16 res = tracySinghMul(magic, plainMat); for (auto& row : res) for (int32_t v : row) writeInt(outFile, v); break; }
        }
        ++blockIndex;
    }
    outFile.close();
    Serial.print("[SUCCESS] Encryption complete. Blocks: "); Serial.println(blockIndex);
    Serial.println("[INFO] Ciphertext saved to SD/output.enc");
}

// ------------------------------------------------------------
// --- DECRYPTION PIPELINE (STREAMING MODE) ---
// ------------------------------------------------------------

inline uint16_t intsPerBlock(uint8_t zone) {
    switch (zone) { case 1: return 16; case 2: return 64; case 3: case 4: return 256; default: return 16; }
}

void runDecryptPipeline(File& encFile, File& decFile) {
    char hdr[64]; int hLen = 0;
    while (encFile.available() && hLen < 63) {
        char ch = (char)encFile.read();
        if (ch == '\n') break;
        hdr[hLen++] = ch;
    }
    hdr[hLen] = '\0';

    uint8_t zone = 0; int64_t distance = 0;
    char* zPtr = strchr(hdr, 'Z');
    char* dPtr = strchr(hdr, 'D');
    if (!zPtr || !dPtr) { 
        Serial.println("[ERROR] HEADER_PARSE_FAILED: Invalid format"); 
        return; 
    }
    
    zone = (uint8_t)(*(zPtr + 1) - '0');
    char* dVal = dPtr + 1;
    while (*dVal) { distance = distance * 10 + (*dVal++ - '0'); }

    Serial.print("[INFO] Decoding Stream: Zone="); Serial.print(zone);
    Serial.print(" | Distance="); Serial.println((long)distance);

    uint8_t keyDigits[10]; uint8_t keyLen = 0;
    int64_t tmp = distance;
    while (tmp > 0 && keyLen < 10) { keyDigits[keyLen++] = (uint8_t)(tmp % 10); tmp /= 10; }
    if (keyLen == 0) { keyDigits[0] = 0; keyLen = 1; }

    uint16_t blockIntCount = intsPerBlock(zone);
    size_t   blockIndex    = 0;

    while (encFile.available()) {
        int32_t buf[256];
        for (uint16_t i = 0; i < blockIntCount; ++i) buf[i] = readInt(encFile);

        Matrix4 magic = magicMatris(keyDigits[blockIndex % keyLen]);
        Matrix4 plainMat{};

        switch (zone) {
            case 1: { Matrix4  C{}; for (int i=0;i<4;++i)  for (int j=0;j<4;++j)  C[i][j]=buf[i*4+j];  plainMat=inverseHadamardMul(magic,C);  break; }
            case 2: { Matrix8  C{}; for (int i=0;i<8;++i)  for (int j=0;j<8;++j)  C[i][j]=buf[i*8+j];  plainMat=inverseKhatriRaoMul(magic,C); break; }
            case 3: { Matrix16 C{}; for (int i=0;i<16;++i) for (int j=0;j<16;++j) C[i][j]=buf[i*16+j]; plainMat=inverseKroneckerMul(magic,C); break; }
            case 4: { Matrix16 C{}; for (int i=0;i<16;++i) for (int j=0;j<16;++j) C[i][j]=buf[i*16+j]; plainMat=inverseTracySinghMul(magic,C);break; }
        }

        for (int i = 0; i < BLOCK_CHARS; ++i) {
            char ch = intToChar(plainMat[i / 4][i % 4]);
            if (ch != '\0') decFile.write((uint8_t)ch);
        }
        ++blockIndex;
    }
    Serial.print("[SUCCESS] Decryption complete. Blocks: "); Serial.println(blockIndex);
    Serial.println("[INFO] Plaintext saved to SD/output.dec");
}

// ------------------------------------------------------------
// --- CONFIGURATION MANAGEMENT ---
// ------------------------------------------------------------

void configSave(uint8_t slot, int s_lat, int s_lon, int r_lat, int r_lon, int hour) {
    if (slot >= MAX_CONFIG_SLOTS) { 
        Serial.println("[ERROR] INVALID_SLOT: Index out of bounds (0-9)"); 
        return; 
    }
    if (!SD.exists("configs")) SD.mkdir("configs");
    char path[24];
    snprintf(path, sizeof(path), "configs/cfg_%d.txt", slot);
    SD.remove(path);
    File f = SD.open(path, FILE_WRITE);
    if (!f) { 
        Serial.println("[ERROR] FS_FAILURE: Could not write config file"); 
        return; 
    }
    f.printf("%d %d %d %d %d\n", s_lat, s_lon, r_lat, r_lon, hour);
    f.close();
    Serial.print("[SUCCESS] Configuration saved to Slot "); Serial.println(slot);
}

bool configLoad(uint8_t slot) {
    if (slot >= MAX_CONFIG_SLOTS) return false;
    char path[24];
    snprintf(path, sizeof(path), "configs/cfg_%d.txt", slot);
    File f = SD.open(path, FILE_READ);
    if (!f) { 
        Serial.print("[ERROR] LOAD_FAILURE: Slot "); Serial.print(slot); Serial.println(" is empty"); 
        return false; 
    }
    char buf[32]; int len = 0;
    while (f.available() && len < 31) buf[len++] = (char)f.read();
    buf[len] = '\0'; f.close();
    int s_lat, s_lon, r_lat, r_lon, hour;
    if (sscanf(buf, "%d %d %d %d %d", &s_lat, &s_lon, &r_lat, &r_lon, &hour) != 5) {
        Serial.println("[ERROR] CORRUPT_CONFIG: Data integrity check failed"); 
        return false;
    }
    sGps = {s_lat, s_lon}; rGps = {r_lat, r_lon}; currentHour = hour;
    systemState = STATE_READY;
    Serial.print("[INFO] Configuration loaded from Slot "); Serial.println(slot);
    return true;
}

void configList() {
    Serial.println("[INFO] Scanning configuration slots...");
    bool found = false;
    char path[24];
    for (uint8_t i = 0; i < MAX_CONFIG_SLOTS; ++i) {
        snprintf(path, sizeof(path), "configs/cfg_%d.txt", i);
        if (SD.exists(path)) {
            File f = SD.open(path, FILE_READ);
            char buf[32]; int len = 0;
            while (f.available() && len < 31) buf[len++] = (char)f.read();
            buf[len] = '\0'; f.close();
            int s_lat, s_lon, r_lat, r_lon, hour;
            sscanf(buf, "%d %d %d %d %d", &s_lat, &s_lon, &r_lat, &r_lon, &hour);
            Serial.print("  Slot ["); Serial.print(i);
            Serial.print("]: SRC("); Serial.print(s_lat); Serial.print(","); Serial.print(s_lon);
            Serial.print(") DST("); Serial.print(r_lat); Serial.print(","); Serial.print(r_lon);
            Serial.print(") Time:"); Serial.print(hour); Serial.println(":00");
            found = true;
        }
    }
    if (!found) Serial.println("[INFO] No valid configurations found on SD card.");
}

// ------------------------------------------------------------
// --- UI / COMMAND INTERFACE ---
// ------------------------------------------------------------

void printHelp() {
    Serial.println("\r\n----------------------------------------");
    Serial.println(" CrypMathEmbedded Terminal Interface ");
    Serial.println("----------------------------------------");
    Serial.println(" CONFIGURATION COMMANDS:");
    Serial.println("  config <slat> <slon> <rlat> <rlon> <hour>");
    Serial.println("  config save <slot> <slat> <slon> <rlat> <rlon> <hour>");
    Serial.println("  config load <slot>");
    Serial.println("  config list");
    Serial.println("  config delete <slot>");
    Serial.println(" OPERATIONS:");
    Serial.println("  encrypt   Process SD/input.txt -> SD/output.enc");
    Serial.println("  decrypt   Process SD/output.enc -> SD/output.dec");
    Serial.println(" SYSTEM:");
    Serial.println("  status    Display hardware and key state");
    Serial.println("  help      Display this reference");
    Serial.println("----------------------------------------\r\n");
}

void printStatus() {
    Serial.println("--- SYSTEM STATUS REPORT ---");
    Serial.print("System Mode: ");
    switch (systemState) {
        case STATE_CONFIG:  Serial.println("IDLE / AWAITING_CONFIG"); break;
        case STATE_READY:   Serial.println("READY"); break;
        case STATE_ENCRYPT: Serial.println("BUSY / ENCRYPTING"); break;
        case STATE_DECRYPT: Serial.println("BUSY / DECRYPTING"); break;
        case STATE_ERROR:   Serial.println("CRITICAL_ERROR"); break;
    }
    Serial.print("Sender GPS:   "); Serial.print(sGps.lat); Serial.print(", "); Serial.println(sGps.lon);
    Serial.print("Receiver GPS: "); Serial.print(rGps.lat); Serial.print(", "); Serial.println(rGps.lon);
    Serial.print("Active Hour:  "); Serial.println(currentHour);
    Serial.print("Cipher Zone:  ");
    if      (currentHour < 6)  Serial.println("1 (Hadamard)");
    else if (currentHour < 12) Serial.println("2 (Khatri-Rao)");
    else if (currentHour < 18) Serial.println("3 (Kronecker)");
    else                       Serial.println("4 (Tracy-Singh)");
    Serial.println("----------------------------");
}

void parseConfig(const char* cmd) {
    int s_lat, s_lon, r_lat, r_lon, hour;
    if (sscanf(cmd, "config %d %d %d %d %d", &s_lat, &s_lon, &r_lat, &r_lon, &hour) != 5) {
        Serial.println("[ERROR] ARGUMENT_ERROR: Expected 5 parameters");
        return;
    }
    if (hour < 0 || hour > 23) { 
        Serial.println("[ERROR] DATA_ERROR: Hour must be in range 0-23"); 
        return; 
    }
    sGps = {s_lat, s_lon}; rGps = {r_lat, r_lon}; currentHour = hour;
    systemState = STATE_READY;
    Serial.println("[SUCCESS] Runtime parameters updated.");
}

// ------------------------------------------------------------
// --- MAIN INITIALIZATION ---
// ------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("\r\n[SYSTEM] CrypMathEmbedded v1.0 Booting...");
    Serial.println("[SYSTEM] Target Hardware: STM32H750VGT6");

    if (SD.begin()) {
        Serial.println("[STORAGE] SD Card initialized (SDMMC 4-bit mode)");
    } else {
        Serial.println("[ERROR] STORAGE_FAILURE: SD Card not detected");
        systemState = STATE_ERROR;
        return;
    }
    printHelp();
}

// ------------------------------------------------------------
// --- MAIN LOOP / TASK SCHEDULER ---
// ------------------------------------------------------------

void loop() {
    if (!Serial.available()) return;

    char cmdBuf[64]; int cmdLen = 0;
    while (Serial.available() && cmdLen < 63) {
        char ch = (char)Serial.read();
        if (ch == '\n' || ch == '\r') break;
        cmdBuf[cmdLen++] = ch;
    }
    cmdBuf[cmdLen] = '\0';
    if (cmdLen == 0) return;

    if (strncmp(cmdBuf, "config save", 11) == 0) {
        int slot, s_lat, s_lon, r_lat, r_lon, hour;
        if (sscanf(cmdBuf, "config save %d %d %d %d %d %d", &slot, &s_lat, &s_lon, &r_lat, &r_lon, &hour) != 6)
            Serial.println("[ERROR] CMD_ERROR: Usage: config save <slot> <slat> <slon> <rlat> <rlon> <hour>");
        else configSave((uint8_t)slot, s_lat, s_lon, r_lat, r_lon, hour);
    }
    else if (strncmp(cmdBuf, "config load", 11) == 0) {
        int slot;
        if (sscanf(cmdBuf, "config load %d", &slot) != 1) 
            Serial.println("[ERROR] CMD_ERROR: Usage: config load <slot>");
        else configLoad((uint8_t)slot);
    }
    else if (strcmp(cmdBuf, "config list") == 0) { configList(); }
    else if (strncmp(cmdBuf, "config", 6) == 0)  { parseConfig(cmdBuf); }
    else if (strcmp(cmdBuf, "encrypt") == 0) {
        if (systemState != STATE_READY) { 
            Serial.println("[ERROR] STATE_MISMATCH: Configuration required before encryption"); 
            return; 
        }
        File inFile = SD.open("input.txt", FILE_READ);
        if (!inFile) { 
            Serial.println("[ERROR] IO_FAILURE: input.txt not found on SD"); 
            return; 
        }
        systemState = STATE_ENCRYPT;
        KeyContext ctx = deriveKey(sGps, rGps, currentHour);
        Serial.print("[INFO] Starting Encryption Sequence | Zone: "); Serial.println(ctx.zone);
        runEncryptPipeline(inFile, ctx);
        inFile.close();
        systemState = STATE_READY;
    }
    else if (strcmp(cmdBuf, "decrypt") == 0) {
        if (systemState != STATE_READY) { 
            Serial.println("[ERROR] STATE_MISMATCH: Configuration required before decryption"); 
            return; 
        }
        File encFile = SD.open("output.enc", FILE_READ);
        if (!encFile) { 
            Serial.println("[ERROR] IO_FAILURE: output.enc not found on SD"); 
            return; 
        }
        SD.remove("output.dec");
        File decFile = SD.open("output.dec", FILE_WRITE);
        if (!decFile) { 
            Serial.println("[ERROR] IO_FAILURE: Could not create output.dec"); 
            encFile.close(); 
            return; 
        }
        systemState = STATE_DECRYPT;
        runDecryptPipeline(encFile, decFile);
        encFile.close(); decFile.close();
        systemState = STATE_READY;
    }
    else if (strcmp(cmdBuf, "status") == 0) { printStatus(); }
    else if (strcmp(cmdBuf, "help") == 0)   { printHelp();   }
    else {
        Serial.print("[WARN] UNKNOWN_COMMAND: "); Serial.println(cmdBuf);
    }
}