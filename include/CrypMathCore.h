#pragma once

#include <array>
#include <cstdint>
#include <cmath>

// --- Sabit Boyutlu Matris Tanımlamaları (Zero Heap Allocation) ---
using Matrix2  = std::array<std::array<int32_t, 2>, 2>;
using Matrix4  = std::array<std::array<int32_t, 4>, 4>;
using Matrix8  = std::array<std::array<int32_t, 8>, 8>;
using Matrix16 = std::array<std::array<int32_t, 16>, 16>;

// --- Koordinat Yapıları ---
struct CordinateEcef {
    double x, y, z;
};

struct CordinateGps {
    int lat;
    int lon;
};

// --- Anahtar Türetme Bağlamı ---
struct KeyContext {
    uint8_t keyDigits[10];
    uint8_t keyLen;
    uint8_t zone;    // 1=Hadamard 2=KhatriRao 3=Kronecker 4=TracySingh
    int64_t distance;
};

// --- Yardımcı Fonksiyonlar ---
double        degToRad(double degrees);
CordinateEcef convertGpsToEcef(const CordinateGps& gps);

// --- Karakter / Sayı Dönüşümleri ---
int32_t charToInt(char character);
char    intToChar(int32_t val);

// --- Uzaklık Hesaplamaları ---
int64_t DisDD(const CordinateEcef& s, const CordinateEcef& r);
int64_t TetHD(const CordinateEcef& s, const CordinateEcef& r);
int64_t TruOD(const CordinateEcef& s, const CordinateEcef& r);
int64_t TriOD(const CordinateEcef& s, const CordinateEcef& r);

// --- Anahtar Türetme ---
KeyContext deriveKey(const CordinateGps& sg, const CordinateGps& rg, int hour);

// --- Sihirli Matris ---
Matrix4 magicMatris(size_t i);

// --- Alt Matris ---
Matrix2 subMatrix(const Matrix4& M, int r, int c);

// --- Şifreleme ---
Matrix4  hadamardMul   (const Matrix4& A, const Matrix4& B);
Matrix16 kroneckerMul  (const Matrix4& A, const Matrix4& B);
Matrix16 tracySinghMul (const Matrix4& A, const Matrix4& B);
Matrix8  khatriRaoMul  (const Matrix4& A, const Matrix4& B);

// --- Deşifreleme ---
Matrix4 inverseHadamardMul   (const Matrix4& A, const Matrix4&  C);
Matrix4 inverseKroneckerMul  (const Matrix4& A, const Matrix16& C);
Matrix4 inverseTracySinghMul (const Matrix4& A, const Matrix16& C);
Matrix4 inverseKhatriRaoMul  (const Matrix4& A, const Matrix8&  C);