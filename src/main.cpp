#include <Arduino.h>
#include <STM32SD.h>
#include "CrypMathCore.h"

static const uint8_t BLOCK_CHARS = 16;

enum SystemState : uint8_t {
    STATE_CONFIG  = 0,
    STATE_READY   = 1,
    STATE_ENCRYPT = 2,
    STATE_DECRYPT = 3,
    STATE_ERROR   = 4
};

CordinateGps sGps, rGps;
int          currentHour = 12;
SystemState  systemState = STATE_CONFIG;

inline void writeInt(File& f, int32_t v) {
    char buf[5];
    bool neg = (v < 0);
    uint32_t uv = neg ? (uint32_t)(-v) : (uint32_t)v;
    buf[4]='0'+(uv%10); uv/=10;
    buf[3]='0'+(uv%10); uv/=10;
    buf[2]='0'+(uv%10); uv/=10;
    buf[1]='0'+(uv%10); uv/=10;
    buf[0]=neg?'-':('0'+(uv%10));
    f.write((uint8_t*)buf, 5);
}

inline int32_t readInt(File& f) {
    char buf[5];
    if (f.read(buf, 5) != 5) return 0;
    bool neg = (buf[0] == '-');
    int32_t val = 0;
    for (int i = neg?1:0; i < 5; ++i) val = val*10 + (buf[i]-'0');
    return neg ? -val : val;
}

void runEncryptPipeline(File& inFile, const KeyContext& ctx) {
    File outFile = SD.open("output.enc", FILE_WRITE);
    if (!outFile) { Serial.println("[Error] output.enc acilamadi!"); return; }

    outFile.print("HDR:Z");
    outFile.print(ctx.zone);
    outFile.print(":D");
    char distBuf[21]; int pos=20; distBuf[pos]='\0';
    int64_t d=ctx.distance;
    if (d==0) { distBuf[--pos]='0'; }
    else { while(d>0){distBuf[--pos]='0'+(d%10);d/=10;} }
    outFile.print(distBuf+pos);
    outFile.print("\n");

    size_t blockIndex = 0;
    while (inFile.available()) {
        char raw[BLOCK_CHARS];
        int  bytesRead = 0;
        while (bytesRead < BLOCK_CHARS && inFile.available())
            raw[bytesRead++] = (char)inFile.read();
        for (int i=bytesRead; i<BLOCK_CHARS; ++i) raw[i]=' ';

        Matrix4 plainMat{};
        for (int i=0; i<BLOCK_CHARS; ++i) plainMat[i/4][i%4] = charToInt(raw[i]);

        Matrix4 magic = magicMatris(ctx.keyDigits[blockIndex % ctx.keyLen]);

        switch (ctx.zone) {
            case 1: { Matrix4  res=hadamardMul(magic,plainMat);  for(auto&row:res) for(int32_t v:row) writeInt(outFile,v); break; }
            case 2: { Matrix8  res=khatriRaoMul(magic,plainMat); for(auto&row:res) for(int32_t v:row) writeInt(outFile,v); break; }
            case 3: { Matrix16 res=kroneckerMul(magic,plainMat); for(auto&row:res) for(int32_t v:row) writeInt(outFile,v); break; }
            case 4: { Matrix16 res=tracySinghMul(magic,plainMat);for(auto&row:res) for(int32_t v:row) writeInt(outFile,v); break; }
        }
        ++blockIndex;
    }
    outFile.close();
    Serial.print("[Encrypt] Tamamlandi. Blok: "); Serial.println(blockIndex);
}

inline uint16_t intsPerBlock(uint8_t zone) {
    switch(zone){ case 1:return 16; case 2:return 64; case 3:case 4:return 256; default:return 16; }
}

void runDecryptPipeline(File& encFile, File& decFile) {
    char hdr[64]; int hLen=0;
    while (encFile.available() && hLen<63) {
        char ch=(char)encFile.read();
        if (ch=='\n') break;
        hdr[hLen++]=ch;
    }
    hdr[hLen]='\0';

    uint8_t zone=0; int64_t distance=0;
    char* zPtr=strchr(hdr,'Z'); char* dPtr=strchr(hdr,'D');
    if (!zPtr||!dPtr) { Serial.println("[Error] Gecersiz header!"); return; }
    zone=(uint8_t)(*(zPtr+1)-'0');
    char* dVal=dPtr+1;
    while(*dVal) { distance=distance*10+(*dVal++-'0'); }

    Serial.print("[Decrypt] Zone="); Serial.print(zone);
    Serial.print(" Distance="); Serial.println((long)distance);

    uint8_t keyDigits[10]; uint8_t keyLen=0;
    int64_t tmp=distance;
    while(tmp>0&&keyLen<10){keyDigits[keyLen++]=(uint8_t)(tmp%10);tmp/=10;}
    if(keyLen==0){keyDigits[0]=0;keyLen=1;}

    uint16_t blockIntCount=intsPerBlock(zone);
    size_t   blockIndex=0;

    while (encFile.available()) {
        int32_t buf[256];
        for (uint16_t i=0; i<blockIntCount; ++i) buf[i]=readInt(encFile);

        Matrix4 magic=magicMatris(keyDigits[blockIndex%keyLen]);
        Matrix4 plainMat{};

        switch(zone){
            case 1:{Matrix4 C{}; for(int i=0;i<4;++i)for(int j=0;j<4;++j)C[i][j]=buf[i*4+j]; plainMat=inverseHadamardMul(magic,C); break;}
            case 2:{Matrix8 C{}; for(int i=0;i<8;++i)for(int j=0;j<8;++j)C[i][j]=buf[i*8+j]; plainMat=inverseKhatriRaoMul(magic,C); break;}
            case 3:{Matrix16 C{}; for(int i=0;i<16;++i)for(int j=0;j<16;++j)C[i][j]=buf[i*16+j]; plainMat=inverseKroneckerMul(magic,C); break;}
            case 4:{Matrix16 C{}; for(int i=0;i<16;++i)for(int j=0;j<16;++j)C[i][j]=buf[i*16+j]; plainMat=inverseTracySinghMul(magic,C); break;}
        }

        for (int i=0; i<BLOCK_CHARS; ++i) {
            char ch=intToChar(plainMat[i/4][i%4]);
            if(ch!='\0') decFile.write((uint8_t)ch);
        }
        ++blockIndex;
    }
    Serial.print("[Decrypt] Tamamlandi. Blok: "); Serial.println(blockIndex);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("\n[SYSTEM] CrypMath STM32H750 Basliyor...");
    if (SD.begin()) {
        Serial.println("[SD] MicroSD Baglandi (SDMMC 4-bit)");
    } else {
        Serial.println("[SD] HATA: Kart bulunamadi!");
        systemState = STATE_ERROR;
        return;
    }
    Serial.println("[Komutlar] config / encrypt / decrypt");
}

void loop() {
    if (!Serial.available()) return;
    char cmdBuf[32]; int cmdLen=0;
    while (Serial.available() && cmdLen<31) {
        char ch=(char)Serial.read();
        if(ch=='\n'||ch=='\r') break;
        cmdBuf[cmdLen++]=ch;
    }
    cmdBuf[cmdLen]='\0';

    if (strcmp(cmdBuf,"config")==0) {
        sGps={41,28}; rGps={39,32}; currentHour=12;
        systemState=STATE_READY;
        Serial.println("[Config] IST(41,28)->ANK(39,32), Saat:12");
        Serial.println("[State] Hazir.");
    }
    else if (strcmp(cmdBuf,"encrypt")==0) {
        if(systemState!=STATE_READY){Serial.println("[Error] Once 'config' calistirin.");return;}
        File inFile=SD.open("input.txt",FILE_READ);
        if(!inFile){Serial.println("[Error] input.txt bulunamadi!");return;}
        systemState=STATE_ENCRYPT;
        KeyContext ctx=deriveKey(sGps,rGps,currentHour);
        Serial.print("[Encrypt] Zone="); Serial.print(ctx.zone);
        Serial.print(" Distance="); Serial.println((long)ctx.distance);
        runEncryptPipeline(inFile,ctx);
        inFile.close();
        systemState=STATE_READY;
    }
    else if (strcmp(cmdBuf,"decrypt")==0) {
        if(systemState!=STATE_READY){Serial.println("[Error] Once 'config' calistirin.");return;}
        File encFile=SD.open("output.enc",FILE_READ);
        if(!encFile){Serial.println("[Error] output.enc acilamadi!");return;}
        File decFile=SD.open("output.dec",FILE_WRITE);
        if(!decFile){Serial.println("[Error] output.dec acilamadi!");encFile.close();return;}
        systemState=STATE_DECRYPT;
        runDecryptPipeline(encFile,decFile);
        encFile.close(); decFile.close();
        systemState=STATE_READY;
    }
    else if(cmdLen > 0){
        Serial.print("[Error] Bilinmeyen komut: "); Serial.println(cmdBuf);
    }
}