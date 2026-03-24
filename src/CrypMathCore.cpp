#include "CrypMathCore.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double degToRad(double degrees) {
    return degrees * (M_PI / 180.0);
}

CordinateEcef convertGpsToEcef(const CordinateGps& gps) {
    const double a  = 6378137.0;
    const double f  = 1.0 / 298.257223563;
    const double e2 = 2.0 * f - f * f;
    const double h  = 0.0;
    double latRad = degToRad(gps.lat);
    double lonRad = degToRad(gps.lon);
    double N = a / std::sqrt(1.0 - e2 * std::sin(latRad) * std::sin(latRad));
    CordinateEcef result;
    result.x = (N + h) * std::cos(latRad) * std::cos(lonRad);
    result.y = (N + h) * std::cos(latRad) * std::sin(lonRad);
    result.z = (N * (1.0 - e2) + h) * std::sin(latRad);
    return result;
}

struct CharMap { char ch; int32_t val; };

static const CharMap hexArr[] = { 
    {'A', 1},   {'B', 2},   {'C', 3},   {'D', 5},   {'E', 6},   {'F', 7},
    {'G', 8},   {'H', 10},  {'I', 11},  {'J', 22},  {'K', 24},  {'L', 26},
    {'M', 28},  {'N', 30},  {'O', 32},  {'P', 35},  {'Q', 59},  {'R', 62},
    {'S', 65},  {'T', 71},  {'U', 74},  {'V', 80},  {'W', 81},  {'X', 127},
    {'Y', 131}, {'Z', 135},
    {'a', 139}, {'b', 143}, {'c', 147}, {'d', 155}, {'e', 156}, {'f', 231},
    {'g', 236}, {'h', 246}, {'i', 251}, {'j', 256}, {'k', 261}, {'l', 266},
    {'m', 267}, {'n', 378}, {'o', 384}, {'p', 390}, {'q', 396}, {'r', 402},
    {'s', 408}, {'t', 414}, {'u', 420}, {'v', 421}, {'w', 575}, {'x', 582},
    {'y', 589}, {'z', 596},
    {'.', 625}, {'(', 829}, {')', 837}, {',', 845}, {';', 853}, {':', 861},
    {'@', 877}, {'?', 886}, {'!', 165}, {'/', 174}, {'-', 183}, {'+', 192},
    {'=', 201}, {' ', 210},
    {'0', 211}, {'1', 536}, {'2', 546}, {'3', 556}, {'4', 566}, {'5', 576},
    {'6', 586}, {'7', 606}, {'8', 607}, {'9', 14}
};

// Fonksiyon gövdesi güncellendi
int32_t charToInt(char character) {
    for (const auto& e : hexArr) if (e.ch == character) return e.val;
    return -1;
}

char intToChar(int32_t val) {
    for (const auto& e : hexArr) if (e.val == val) return e.ch;
    return '\0';
}

int64_t DisDD(const CordinateEcef& s, const CordinateEcef& r) {
    double absX = std::abs(s.x-r.x), absY = std::abs(s.y-r.y), absZ = std::abs(s.z-r.z);
    double el1 = std::max({absX, absY, absZ});
    double el2 = (std::sqrt(2.0)-1.0) * std::min({absX+absY, absY+absZ, absZ+absX});
    double el3 = (std::sqrt(2.0)-2.0*std::sqrt(2.0)+1.0) * std::min({absX, absY, absZ});
    return std::abs(std::lround(el1+el2+el3));
}

int64_t TetHD(const CordinateEcef& s, const CordinateEcef& r) {
    double absX = std::abs(s.x-r.x), absY = std::abs(s.y-r.y), absZ = std::abs(s.z-r.z);
    double el1 = (std::sqrt(3.0)-1.0) * std::max({absX+absY, absX+absZ, absY+absZ});
    double el2 = (std::sqrt(3.0)-1.0) * std::max({absX, absY, absZ});
    return std::abs(std::lround(el1+el2));
}

int64_t TruOD(const CordinateEcef& s, const CordinateEcef& r) {
    double absX = std::abs(s.x-r.x), absY = std::abs(s.y-r.y), absZ = std::abs(s.z-r.z);
    return std::abs(std::lround(std::max(2.0/3.0*(absX+absY+absZ), std::max({absX,absY,absZ}))));
}

int64_t TriOD(const CordinateEcef& s, const CordinateEcef& r) {
    double absX = std::abs(s.x-r.x), absY = std::abs(s.y-r.y), absZ = std::abs(s.z-r.z);
    return std::abs(std::lround(absX+absY+absZ + (std::sqrt(2.0)-2.0)*std::min({absX,absY,absZ})));
}

KeyContext deriveKey(const CordinateGps& sg, const CordinateGps& rg, int hour) {
    KeyContext ctx{};
    CordinateEcef sEcef = convertGpsToEcef(sg);
    CordinateEcef rEcef = convertGpsToEcef(rg);
    if      (hour < 6)  { ctx.distance = DisDD(sEcef,rEcef); ctx.zone = 1; }
    else if (hour < 12) { ctx.distance = TetHD(sEcef,rEcef); ctx.zone = 2; }
    else if (hour < 18) { ctx.distance = TruOD(sEcef,rEcef); ctx.zone = 3; }
    else                { ctx.distance = TriOD(sEcef,rEcef); ctx.zone = 4; }
    int64_t tmp = ctx.distance;
    while (tmp > 0 && ctx.keyLen < 10) { ctx.keyDigits[ctx.keyLen++] = (uint8_t)(tmp%10); tmp /= 10; }
    if (ctx.keyLen == 0) { ctx.keyDigits[0] = 0; ctx.keyLen = 1; }
    return ctx;
}

Matrix4 magicMatris(size_t i) {
    Matrix4 mat{};
    switch (i) {
        case 0: mat[0]={16,2,3,13};  mat[1]={5,11,10,8};   mat[2]={9,7,6,12};   mat[3]={4,14,15,1};  break;
        case 1: mat[0]={16,9,5,4};   mat[1]={2,7,11,14};   mat[2]={3,6,10,15};  mat[3]={13,12,8,1};  break;
        case 2: mat[0]={13,8,12,1};  mat[1]={2,11,7,14};   mat[2]={3,10,6,15};  mat[3]={16,5,9,4};   break;
        case 3: mat[0]={4,5,9,16};   mat[1]={14,11,7,2};   mat[2]={15,10,6,3};  mat[3]={1,8,12,13};  break;
        case 4: mat[0]={13,2,3,16};  mat[1]={12,7,6,9};    mat[2]={8,11,10,5};  mat[3]={4,5,9,16};   break;
        case 5: mat[0]={13,2,3,16};  mat[1]={8,11,10,5};   mat[2]={12,7,6,9};   mat[3]={1,14,15,4};  break;
        case 6: mat[0]={16,5,9,4};   mat[1]={2,11,7,14};   mat[2]={3,10,6,15};  mat[3]={13,8,12,1};  break;
        case 7: mat[0]={4,14,15,1};  mat[1]={5,11,10,8};   mat[2]={9,7,6,12};   mat[3]={16,2,3,13};  break;
        case 8: mat[0]={1,8,12,13};  mat[1]={14,11,7,2};   mat[2]={15,10,6,3};  mat[3]={4,5,9,16};   break;
        case 9: mat[0]={1,14,15,4};  mat[1]={8,11,10,5};   mat[2]={12,7,6,9};   mat[3]={13,2,3,16};  break;
        default: break;
    }
    return mat;
}

Matrix2 subMatrix(const Matrix4& M, int r, int c) {
    Matrix2 S{};
    S[0][0]=M[r][c]; S[0][1]=M[r][c+1];
    S[1][0]=M[r+1][c]; S[1][1]=M[r+1][c+1];
    return S;
}

Matrix4 hadamardMul(const Matrix4& A, const Matrix4& B) {
    Matrix4 res{};
    for (int i=0;i<4;++i) for (int j=0;j<4;++j) res[i][j]=A[i][j]*B[i][j];
    return res;
}

Matrix16 kroneckerMul(const Matrix4& A, const Matrix4& B) {
    Matrix16 res{};
    for (int a=0;a<4;++a) for (int b=0;b<4;++b)
        for (int i=0;i<4;++i) for (int j=0;j<4;++j)
            res[a*4+i][b*4+j]=A[a][b]*B[i][j];
    return res;
}

Matrix16 tracySinghMul(const Matrix4& A, const Matrix4& B) {
    Matrix16 res{};
    Matrix2 Ab[2][2]={{subMatrix(A,0,0),subMatrix(A,0,2)},{subMatrix(A,2,0),subMatrix(A,2,2)}};
    Matrix2 Bb[2][2]={{subMatrix(B,0,0),subMatrix(B,0,2)},{subMatrix(B,2,0),subMatrix(B,2,2)}};
    for (int i=0;i<2;++i) for (int j=0;j<2;++j)
    for (int k=0;k<2;++k) for (int l=0;l<2;++l)
    for (int r=0;r<2;++r) for (int c=0;c<2;++c)
    for (int br=0;br<2;++br) for (int bc=0;bc<2;++bc) {
        int row = (i*2 + k)*4 + (r*2 + br);
        int col = (j*2 + l)*4 + (c*2 + bc);
        res[row][col] = Ab[i][j][r][c]*Bb[k][l][br][bc];
    }
    return res;
}

Matrix8 khatriRaoMul(const Matrix4& A, const Matrix4& B) {
    Matrix8 res{};
    for (int i=0;i<2;++i) for (int j=0;j<2;++j) {
        Matrix2 Ai=subMatrix(A,i*2,j*2), Bi=subMatrix(B,i*2,j*2);
        for (int r=0;r<2;++r) for (int c=0;c<2;++c)
        for (int br=0;br<2;++br) for (int bc=0;bc<2;++bc)
            res[i*4+r*2+br][j*4+c*2+bc]=Ai[r][c]*Bi[br][bc];
    }
    return res;
}

Matrix4 inverseHadamardMul(const Matrix4& A, const Matrix4& C) {
    Matrix4 res{};
    for (int i=0;i<4;++i) for (int j=0;j<4;++j)
        res[i][j]=(A[i][j]!=0)?(C[i][j]/A[i][j]):0;
    return res;
}

Matrix4 inverseKroneckerMul(const Matrix4& A, const Matrix16& C) {
    Matrix4 res{};
    int32_t s=(A[0][0]!=0)?A[0][0]:1;
    for (int i=0;i<4;++i) for (int j=0;j<4;++j) res[i][j]=C[i][j]/s;
    return res;
}

Matrix4 inverseTracySinghMul(const Matrix4& A, const Matrix16& C) {
    Matrix4 res{};
    int32_t s=(A[0][0]!=0)?A[0][0]:1;
    for (int r=0; r<2; ++r) {
        for (int c=0; c<2; ++c) {
            res[r]  [c]   = C[r]  [c]   / s;   // Bb[0][0] -> Offset (0,0)
            res[r]  [c+2] = C[r]  [c+4] / s;   // Bb[0][1] -> Offset (0,4)
            res[r+2][c]   = C[r+4][c]   / s;   // Bb[1][0] -> Offset (4,0)
            res[r+2][c+2] = C[r+4][c+4] / s;   // Bb[1][1] -> Offset (4,4)
        }
    }
    return res;
}

Matrix4 inverseKhatriRaoMul(const Matrix4& A, const Matrix8& C) {
    Matrix4 B{};
    int32_t s11=(A[0][0]!=0)?A[0][0]:1, s12=(A[0][2]!=0)?A[0][2]:1;
    int32_t s21=(A[2][0]!=0)?A[2][0]:1, s22=(A[2][2]!=0)?A[2][2]:1;
    for (int i=0;i<2;++i) for (int j=0;j<2;++j) B[i]  [j]   = C[i]  [j]   /s11;
    for (int i=0;i<2;++i) for (int j=0;j<2;++j) B[i]  [j+2] = C[i]  [j+4] /s12;
    for (int i=0;i<2;++i) for (int j=0;j<2;++j) B[i+2][j]   = C[i+4][j]   /s21;
    for (int i=0;i<2;++i) for (int j=0;j<2;++j) B[i+2][j+2] = C[i+4][j+4] /s22;
    return B;
}