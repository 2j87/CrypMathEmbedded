#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <string>
#include <sstream>
#include <locale>
#include <curl/curl.h>

using vec2 = std::vector<std::vector<int>>;

struct CordinateEcef
{
    //meter type
    double x;
    double y;
    double z;
};

struct CordinateGps
{
    int GpsLat;//Enlem x1 000 000 degree
    int GpsLon;//Boylam x1 000 000 degree
};


// helper funct: cURL verisini string'e yazma fonksiyonu
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Get Location:
// [Location]: 
CordinateGps getIpLocation()
{
    CordinateGps returnCordinate;
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string Latilute, Longitude;

    curl = curl_easy_init();
    if(curl)
    {
        // Ücretsiz, keysiz IP API servisi
        std::string url = "http://ip-api.com/json/";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Hızlı sonuç için timeout koyalım (5 saniye)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        std::cout << "IP servisine baglaniliyor..." << "\n";
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "Baglanti Hatasi: %s\n", curl_easy_strerror(res));
        }
        else
        {
            std::cout << "[Location]: Found succesfuly\n" << "\n";
            // JSON parse işlemi (Basit string arama)

            // Şehir
            size_t cityPos = readBuffer.find("\"city\":\"");
            if (cityPos != std::string::npos)
            {
                size_t end = readBuffer.find("\"", cityPos + 8);
                std::cout << "[Location]: City : "
                        << readBuffer.substr(cityPos + 8, end - (cityPos + 8))
                        << "\n";
            }

            // Enlem (Lat)
            size_t latPos = readBuffer.find("\"lat\":");
            if (latPos != std::string::npos)
            {
                size_t end = readBuffer.find(",", latPos);

                Latilute = readBuffer.substr(latPos + 6, end - (latPos + 6));

                std::cout << "[Location]: Latitude: "
                        << Latilute
                        << "\n";
            }

            // Boylam (Lon)
            size_t lonPos = readBuffer.find("\"lon\":");
            if (lonPos != std::string::npos)
            {
                size_t end = readBuffer.find(",", lonPos);

                Longitude = readBuffer.substr(latPos + 6, end - (latPos + 6));

                std::cout << "[Location]: Longitude: "
                        << Longitude
                        << "\n";
            }

            std::cout << "[Warning]: This location is your internet service provider's central office location.\n";
        }

        curl_easy_cleanup(curl);
    }

    return returnCordinate;
}

// hexArr türü:
struct CharMap
{
    wchar_t ch;
    int val;
};

// hexagonal sayılar dizisi
const std::vector<CharMap> hexArr = {

    // büyük harfler 
    {L'A', 1}, {L'B', 2}, {L'C', 3}, {L'Ç', 4}, {L'D', 5}, {L'E', 6}, {L'F', 7}, 
    {L'G', 8}, {L'Ğ', 9}, {L'H', 10}, {L'I', 11}, {L'İ', 20}, {L'J', 22}, {L'K', 24}, 
    {L'L', 26}, {L'M', 28}, {L'N', 30}, {L'O', 32}, {L'Ö', 34}, {L'P', 35}, {L'Q', 59}, 
    {L'R', 62}, {L'S', 65}, {L'Ş', 68}, {L'T', 71}, {L'U', 74}, {L'Ü', 77}, {L'V', 80}, 
    {L'W', 81}, {L'X', 127}, {L'Y', 131}, {L'Z', 135},

    // küçük harfler
    {L'a', 139}, {L'b', 143}, {L'c', 147}, {L'ç', 151}, {L'd', 155}, {L'e', 156}, 
    {L'f', 231}, {L'g', 236}, {L'ğ', 241}, {L'h', 246}, {L'ı', 251}, {L'i', 256}, 
    {L'j', 261}, {L'k', 266}, {L'l', 267}, {L'm', 378}, {L'n', 384}, {L'o', 390}, 
    {L'ö', 396}, {L'p', 402}, {L'q', 408}, {L'r', 414}, {L's', 420}, {L'ş', 421}, 
    {L't', 575}, {L'u', 582}, {L'ü', 589}, {L'v', 596}, {L'w', 603}, {L'x', 610}, 
    {L'y', 617}, {L'z', 624},

    // özel karakterler
    {L'.', 625}, {L'(', 829}, {L')', 837}, {L',', 845}, {L';', 853}, {L':', 861}, 
    {L'‘', 869}, {L'@', 877}, {L'“', 885}, {L'?', 886}, {L'!', 165}, {L'/', 174}, 
    {L'-', 183}, {L'+', 192}, {L'=', 201}, {L' ', 210},

    // sayılar
    {L'0', 211}, {L'1', 536}, {L'2', 546}, {L'3', 556}, {L'4', 566}, {L'5', 576}, 
    {L'6', 586}, {L'7', 606}, {L'8', 607}, {L'9', 14}
};

// wchar_t ve int dönüştürme fonksiyonları:
//charToInt
//intToChar
int charToInt(wchar_t character)
{
    for (const CharMap& entry : hexArr)
        if (entry.ch == character)
            return entry.val;

    return -1;
}

wchar_t intToChar(int val)
{
    for (const CharMap& entry : hexArr)
        if (entry.val == val)
            return entry.ch;

    return L'\0'; 
}

//Disdyakis Dodecahedron distance
int64_t DisDD(CordinateEcef s, CordinateEcef r)
{
    double absX = std::abs(s.x - r.x);
    double absY = std::abs(s.y - r.y);
    double absZ = std::abs(s.z - r.z);

    double el1 = std::max({absX, absY, absZ});
    double el2 = (std::sqrt(2.0) - 1.0) * std::min({absX + absY , absY + absZ, absZ + absX});
    double el3 = (std::sqrt(2.0) - 2.0 * std::sqrt(2.0) + 1.0 ) * std::min({absX, absY, absZ});

    return std::abs(std::lround(el1 + el2 + el3));
}

//Tetrakis Hexahedron Uzaklık
int64_t TetHD(CordinateEcef s, CordinateEcef r)
{
    double absX = std::abs(s.x - r.x);
    double absY = std::abs(s.y - r.y);
    double absZ = std::abs(s.z - r.z);

    double el1 = (std::sqrt(3.0) - 1.0) * std::max({absX + absY, absX + absZ, absY + absZ});
    double el2 = (std::sqrt(3.0) - 1.0) * std::max({absX, absY, absZ});

    return std::abs(std::lround(el1 + el2));
}

//Truncated Octahedron Uzaklık
int64_t TruOD(CordinateEcef s, CordinateEcef r)
{
    double absX = std::abs(s.x - r.x);
    double absY = std::abs(s.y - r.y);
    double absZ = std::abs(s.z - r.z);

    return std::abs(std::lround(std::max((2.0/3.0*(absX + absY + absZ)), std::max({absX, absY, absZ}))));
}

//Triakis Octahedron Uzaklık
int64_t TriOD(CordinateEcef s, CordinateEcef r)
{
    double absX = std::abs(s.x - r.x);
    double absY = std::abs(s.y - r.y);
    double absZ = std::abs(s.z - r.z);

    double el1 = (std::sqrt(2.0) - 2.0) * std::min({absX, absY, absZ});

    return std::abs(std::lround(absX + absY + absZ + el1));
}

//magix Matrises
vec2 magicMatris(size_t i)
{
    vec2 matris;
    matris.resize(4);
    for(size_t j = 0; j < matris.size() ; ++j) matris[j].resize(4);

    switch(i)
    {
        case 0:
            matris[0] = {16,2,3,13};
            matris[1] = {5,11,10,8};
            matris[2] = {9,7,6,12};
            matris[3] = {4,14,15,1};
            break;
        case 1:
            matris[0] = {16,9,5,4};
            matris[1] = {2,7,11,14};
            matris[2] = {3,6,10,15};
            matris[3] = {13,12,8,1};
            break;
        case 2:
            matris[0] = {13,8,12,1};
            matris[1] = {2,11,7,14};
            matris[2] = {3,10,6,15};
            matris[3] = {16,5,9,4};
            break;
        case 3:
            matris[0] = {4,5,9,16};
            matris[1] = {14,11,7,2};
            matris[2] = {15,10,6,3};
            matris[3] = {1,8,12,13};
            break;
        case 4:
            matris[0] = {13,2,3,16};
            matris[1] = {12,7,6,9};
            matris[2] = {8,11,10,5};
            matris[3] = {4,5,9,16};
            break;
        case 5:
            matris[0] = {13,2,3,16};
            matris[1] = {8,11,10,5};
            matris[2] = {12,7,6,9};
            matris[3] = {1,14,15,4};
            break;
        case 6:
            matris[0] = {16,5,9,4};
            matris[1] = {2,11,7,14};
            matris[2] = {3,10,6,15};
            matris[3] = {13,8,12,1};
            break;
        case 7:
            matris[0] = {4,14,15,1};
            matris[1] = {5,11,10,8};
            matris[2] = {9,7,6,12};
            matris[3] = {16,2,3,13};
            break;
        case 8:
            matris[0] = {1,8,12,13};
            matris[1] = {14,11,7,2};
            matris[2] = {15,10,7,2};
            matris[3] = {15,10,6,3};
            break;
        case 9:
            matris[0] = {1,14,15,4};
            matris[1] = {8,1,10,5};
            matris[2] = {12,7,6,9};
            matris[3] = {13,2,3,16};
            break;
    }

    return matris;    
} 

//sadece 4x4 lerde işe yarıyo
vec2 hadamardMul(vec2 A, vec2 B)
{
    vec2 matris;
    matris.resize(4); for(size_t i = 0 ; i < matris.size(); ++i) matris[i].resize(4);

    for(size_t i = 0 ; i < matris.size(); ++i)
        for(size_t j = 0 ; j < matris.size(); ++j)
            matris[i][j] = A[i][j] * B[i][j];

    return matris;
}

// inverse Hadamard (Decrypt)
// C = A * B => B = C / A
vec2 inverseHadamardMul(vec2 A, vec2 C)
{
    vec2 matris;
    matris.resize(4); for(size_t i = 0 ; i < matris.size(); ++i) matris[i].resize(4);

    for(size_t i = 0 ; i < matris.size(); ++i)
        for(size_t j = 0 ; j < matris.size(); ++j)
        {
            if(A[i][j] != 0)
                matris[i][j] = C[i][j] / A[i][j];
            else 
                matris[i][j] = 0; // Should not happen with given magic matrices
        }

    return matris;
}

// Kronecker
// added rectangle multiples
vec2 kroneckerMul(const vec2& A, const vec2& B)
{
    int rowsA = A.size();
    int colsA = rowsA == 0 ? 0 : A[0].size();

    int rowsB = B.size();
    int colsB = rowsB == 0 ? 0 : B[0].size();

    int rowsR = rowsA * rowsB;
    int colsR = colsA * colsB;
    vec2 returnMatris(rowsR);
    for (int i = 0; i < rowsR; ++i) returnMatris[i].resize(colsR);

    for (int a = 0; a < rowsA; ++a)
        for (int b = 0; b < colsA; ++b)
            for (int i = 0; i < rowsB; ++i)
                for (int j = 0; j < colsB; ++j)
                    returnMatris[a * rowsB + i][b * colsB + j] = A[a][b] * B[i][j];

                    
    return returnMatris;
}

// inverse Kronecker (Decrypt)
vec2 inverseKroneckerMul(const vec2& A, const vec2& C)
{
    vec2 returnMatris(4, std::vector<int>(4));
    
    // A[0][0] 0 değil sayıyoz (sihirli matrislerde 0 yok)
    int scaler = A[0][0];
    
    // Block size of B is 4x4
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j) 
        {
            if(scaler != 0)
                returnMatris[i][j] = C[i][j] / scaler;
            else
                returnMatris[i][j] = 0;
        }
    }
    return returnMatris;
}

// tracySinghMul için yardımcı fonksiyon
// alt matrisin (2*2) sol üst köşesini alıp alt matrisi döndürür
vec2 subMatrix(const vec2& M, int r, int c)
{
    //bir tık güvenilirlik
    if (r + 1 >= static_cast<int>(M.size()) || c + 1 >= static_cast<int>(M[0].size()))
        throw std::out_of_range("subMatrix: indices out of range");

    vec2 S(2, std::vector<int>(2));
    S[0][0] = M[r][c];
    S[0][1] = M[r][c + 1];
    S[1][0] = M[r + 1][c];
    S[1][1] = M[r + 1][c + 1];
    return S;
}

// Tracy Singh matris çarpımı
// matrislerin ikisi de 4*4 diye varsayılıyor
vec2 tracySinghMul(vec2 A, vec2 B)
{
    vec2 returnMatris; returnMatris.resize(16);
    for(size_t i = 0; i < returnMatris.size(); ++i) returnMatris[i].resize(16);
    //final matris size is 16*16

    std::vector<std::vector<std::pair<vec2, vec2>>> matrisPairs;
    matrisPairs.resize(4);
    for (size_t i = 0; i < 4; ++i)
    {
        matrisPairs[i].resize(4);
        for (size_t j = 0; j < 4; ++j)
        {
            matrisPairs[i][j].first.resize(2);
            matrisPairs[i][j].second.resize(2);

            for (size_t r = 0; r < 2; ++r)
            {
                matrisPairs[i][j].first[r].resize(2);
                matrisPairs[i][j].second[r].resize(2);
            }
        }
    }

    vec2 A11 = subMatrix(A, 0, 0);
    vec2 A12 = subMatrix(A, 0, 2);
    vec2 A21 = subMatrix(A, 2, 0);
    vec2 A22 = subMatrix(A, 2, 2);

    vec2 B11 = subMatrix(B, 0, 0);
    vec2 B12 = subMatrix(B, 0, 2);
    vec2 B21 = subMatrix(B, 2, 0);
    vec2 B22 = subMatrix(B, 2, 2);

    matrisPairs[0][0] = {A11, B11};
    matrisPairs[0][1] = {A11, B12};
    matrisPairs[0][2] = {A12, B11};
    matrisPairs[0][3] = {A12, B12};

    matrisPairs[1][0] = {A11, B21};
    matrisPairs[1][1] = {A11, B22};
    matrisPairs[1][2] = {A12, B21};
    matrisPairs[1][3] = {A12, B22};

    matrisPairs[2][0] = {A21, B11};
    matrisPairs[2][1] = {A21, B12};
    matrisPairs[2][2] = {A22, B11};
    matrisPairs[2][3] = {A22, B12};

    matrisPairs[3][0] = {A21, B21};
    matrisPairs[3][1] = {A21, B22};
    matrisPairs[3][2] = {A22, B21};
    matrisPairs[3][3] = {A22, B22};


    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            // (i,j) -> kronecker çarpımı
            vec2 tmp = kroneckerMul(matrisPairs[i][j].first, matrisPairs[i][j].second);
            size_t blockSize = static_cast<size_t>(tmp.size());// save type translation

            for (size_t k = 0; k < blockSize; ++k)
                for (size_t l = 0; l < blockSize; ++l)
                    returnMatris[i * blockSize + k][j * blockSize + l] = tmp[k][l];
        }
    }

    return returnMatris;

}

// inverse Tracy-Singh
vec2 inverseTracySinghMul(vec2 A, vec2 C)
{
    vec2 returnMatris(4, std::vector<int>(4));

    vec2 A11 = subMatrix(A, 0, 0);

    int scaler = A11[0][0];
    if(scaler == 0) scaler = 1; // Safety!

    // Recover B11
    for(int r=0; r<2; ++r) for(int c=0; c<2; ++c)
        returnMatris[r][c] = C[r][c] / scaler;

    // Recover B12
    for(int r=0; r<2; ++r) for(int c=0; c<2; ++c)
        returnMatris[r][c+2] = C[r][c+4] / scaler;

    // Recover B21
    for(int r=0; r<2; ++r) for(int c=0; c<2; ++c)
        returnMatris[r+2][c] = C[r+4][c] / scaler;

    // Recover B22
    for(int r=0; r<2; ++r) for(int c=0; c<2; ++c)
        returnMatris[r+2][c+2] = C[r+4][c+4] / scaler;

    return returnMatris;
}

//Khatri-Rao matris çarpımı (kroneckerMul kullanılarak)
//A ve B 4*4 varsayıldı
vec2 khatriRaoMul(vec2 A, vec2 B)
{
    // Sonuç matrisi 8x8 olacak (4 tane 4x4'lük bloktan oluşacak)
    vec2 returnMatris(8, std::vector<int>(8));

    // A ve B'yi 2x2'lik alt bloklara ayır
    vec2 A11 = subMatrix(A, 0, 0); vec2 A12 = subMatrix(A, 0, 2);
    vec2 A21 = subMatrix(A, 2, 0); vec2 A22 = subMatrix(A, 2, 2);

    vec2 B11 = subMatrix(B, 0, 0); vec2 B12 = subMatrix(B, 0, 2);
    vec2 B21 = subMatrix(B, 2, 0); vec2 B22 = subMatrix(B, 2, 2);

    // C_ij = A_ij x B_ij
    vec2 C11 = kroneckerMul(A11, B11); // Sol-Üst Blok İçin
    vec2 C12 = kroneckerMul(A12, B12); // Sağ-Üst Blok İçin
    vec2 C21 = kroneckerMul(A21, B21); // Sol-Alt Blok İçin
    vec2 C22 = kroneckerMul(A22, B22); // Sağ-Alt Blok İçin

    
    // Sol Üst C11
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            returnMatris[i][j] = C11[i][j];

    // Sağ Üst C12
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            returnMatris[i][j + 4] = C12[i][j];

    // Sol Alt C21
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            returnMatris[i + 4][j] = C21[i][j];

    // Sağ Alt C22
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            returnMatris[i + 4][j + 4] = C22[i][j];

    return returnMatris;
}

// Inverse Khatri-Rao
// (8x8 : C) & (4x4 : A) = (4x4 : B)
vec2 inverseKhatriRaoMul(vec2 A, vec2 C)
{
    vec2 B(4, std::vector<int>(4));

    // 0'a bölme hatası önlemi
    int s11 = A[0][0]; if(s11 == 0) s11 = 1;
    int s12 = A[0][2]; if(s12 == 0) s12 = 1;
    int s21 = A[2][0]; if(s21 == 0) s21 = 1;
    int s22 = A[2][2]; if(s22 == 0) s22 = 1;

    // B11: C sol üst
    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            B[i][j] = C[i][j] / s11;

    // B12: C sağ üst
    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            B[i][j + 2] = C[i][j + 4] / s12;

    // B21: C sol alt
    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            B[i + 2][j] = C[i + 4][j] / s21;

    // B22: C sağ alt
    for(int i = 0; i < 2; ++i)
        for(int j = 0; j < 2; ++j)
            B[i + 2][j + 2] = C[i + 4][j + 4] / s22;

    return B;
}

//main lo
// int62_t
// int
// signed = signed int
// long long = long long int
signed main(int argc, char* argv[])
{
    //time->tm_hour // saat
    //time->tm_min // dakika
    //std::cout << time->tm_hour << "\n";
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* time = std::localtime(&t);
    
    //GPS to ECEF
    //örnek koordinatlar
    CordinateEcef SenderCord = {4113913, 3440529, 3440829};
    
    //CordinateEcef SenderCord = getIpLocation();
    
    CordinateEcef ReceiverCord = {1118567, 902131, -6193309};
    
    /*
        plaintext: şifrelenecek metin (Geniş karakter stringi olmalı)
        ciphertext: şifrelenmiş, deşifrelenecek metin
        operation: yapılacak işlem, şifreleme/deşifreleme
    */
    std::wstring plaintext = L""; // DEĞİŞİKLİK: wstring kullanıldı
    std::vector<int> ciphertext;
    std::string operation;// ecrypt/decrypt/empty

    std::string outputFilePath;
    std::string inputFilePath;
    bool isInputFile = false;

    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if(arg == "--encrypt")
        {
            if(!operation.empty()) std::cerr << "[Error]: There's decript and encrypt!" << "\n";
            operation = "encrypt";
        }
        else if(arg == "--decrypt")
        {
            if(!operation.empty()) std::cerr << "[Error]: There's decript and encrypt!" << "\n";
            operation = "decrypt";
        }
        else if(arg == "-o" && i+1 < argc)
        {
            std::string tmp = argv[++i];
            outputFilePath = tmp;
        }
        else if(arg == "-r" && i+1 < argc)
        {
            std::string tmp = argv[++i];
            inputFilePath = tmp;
            isInputFile = true;
        }
    }

    // Operasyon ayarları yapma:
    // inputların alınması : 
    std::cout << "[Operation]: ";
    if(operation == "encrypt")
    {
        std::cout << "Encryption\n";
        
        if(isInputFile)
        {
            // buffer ile okuma
            std::wifstream inputFile(inputFilePath);
            inputFile.imbue(std::locale("")); // UTF-8 ve Türkçe karakter desteği

            try
            {
                if(inputFile.is_open())
                {
                    std::wstringstream wss;
                    wss << inputFile.rdbuf();
                    plaintext = wss.str();

                    inputFile.close();
                }
                else 
                {
                    throw std::string("fileCantOpen");
                }
            }
            catch(std::string e)
            {
                if(e == "fileCantOpen") 
                    std::cerr << "[Error]: Input file cant opened: " << inputFilePath << "\n";
                return 1; 
            }
        }
        else
        {
            std::cout << "[Input]: Enter input: ";
            std::getline(std::wcin, plaintext); 
        }

        if(!plaintext.empty()) std::cout << "[Input]: Succesful\n";
        std::cout << "[Input]: Input size: " << plaintext.size() << "\n";

        //input yoksa işlem yok:
        if(plaintext.empty())
        {
            std::cerr << "[Error]: input is empty\n";
            exit(1);
        }
    }
    else if(operation == "decrypt")
    {
        std::cout << "Decryption\n";

        if(isInputFile)
        {
            std::ifstream inputFile(inputFilePath);
            std::string content = "";
            char c;

            try
            {
                if(inputFile.is_open())
                {
                    while (inputFile >> c)
                        content.push_back(c);
                    
                    inputFile.close();
                    // content groups the 5 digit blocks
                    if(content.size() % 5 != 0)
                        std::cerr << "[Warning]: File content length not multiple of 5. Truncating.\n";

                    for(size_t i = 0; i < content.size(); i += 5)
                    {
                        if(i + 5 <= content.size())
                        {
                            std::string sub = content.substr(i, 5);
                            ciphertext.push_back(std::stoi(sub));
                        }
                    }
                }
                else throw std::string("fileCantOpen");
            }
            catch(std::string e)
            {
                if(e == "fileCantOpen") std::cerr << "[Error]: Input file cant opened: " << inputFilePath << "\n";
            }
        }
        else
        {
            // Manual input not implemented
        }

        if(!ciphertext.empty()) std::cout << "[Input]: Succesful\n";
        std::cout << "[Input]: Input size: " << ciphertext.size() << "\n";
    }

    std::string timeZone;
    int timeInt = time->tm_hour;
    if(timeInt < 6) timeZone = "first";
    else if(timeInt < 12) timeZone = "second";
    else if(timeInt < 18) timeZone = "third";
    else if(timeInt < 24) timeZone = "fourth";

    // şifreleme:
    if(operation == "encrypt")
    {
        std::vector<int> oneDigitFinalVec;
        std::vector<int> oneDimensionMultipled;
        std::vector<int> intPlaintext;
        std::vector<int> digits;
        std::vector<vec2> plainMatrisVec;
        std::vector<vec2> magicMatrisVec;
        std::vector<vec2> multipledMatrisVec;
        int64_t distance = 0;

        for(size_t i = 0; i < plaintext.size(); ++i)
            intPlaintext.push_back(charToInt(plaintext[i]));
        
        //kalan yerleri 0 ile doldurma
        while (intPlaintext.size() % 16 != 0)
            intPlaintext.push_back(0);

        for (size_t i = 0; i < intPlaintext.size(); i += 16)
        {
            if(intPlaintext[i] == -1) throw std::invalid_argument("Input have non-defined character\n");

            //ile önce block'a yazıp sonra bloks a eklio 
            vec2 block(4, std::vector<int>(4));

            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col)
                    block[row][col] = intPlaintext[i + (row * 4 + col)];
                    
            plainMatrisVec.push_back(block);
        }

        if(timeZone == "first") distance = DisDD(SenderCord, ReceiverCord);
        else if(timeZone == "second") distance = TetHD(SenderCord, ReceiverCord);
        else if(timeZone == "third") distance = TruOD(SenderCord, ReceiverCord);
        else if(timeZone == "fourth") distance = TriOD(SenderCord, ReceiverCord);

        int64_t temp = std::abs(distance);
        while (temp > 0)
        {
            int currentDigit = temp % 10;
            digits.push_back(currentDigit);
            temp /= 10;
        }
        std::reverse(digits.begin(), digits.end());

        magicMatrisVec.resize(digits.size()); // Boyutu ayarla
        for(size_t i = 0 ; i < digits.size(); ++i)
            magicMatrisVec[i] = magicMatris(digits[i]);

        
        //magicMatris works well!

        //Well
        //magicMatrisVel and 
        //plainMatrisVec
        //Will multiple here : 

        vec2 matrisPlane(4, std::vector<int>(4, 0));

        while(magicMatrisVec.size() > plainMatrisVec.size())
            plainMatrisVec.push_back(matrisPlane);

        while(magicMatrisVec.size() < plainMatrisVec.size())
            magicMatrisVec.push_back(matrisPlane);
    
        if(timeZone == "first")
        {
            for(size_t i = 0; i < magicMatrisVec.size() && i < plainMatrisVec.size(); ++i)
                multipledMatrisVec.push_back(hadamardMul(magicMatrisVec[i], plainMatrisVec[i]));
        }
        else if(timeZone == "second")
        {
            for(size_t i = 0; i < magicMatrisVec.size() && i < plainMatrisVec.size(); ++i)
                multipledMatrisVec.push_back(khatriRaoMul(magicMatrisVec[i], plainMatrisVec[i]));
        }
        else if(timeZone == "third")
        {
            for(size_t i = 0; i < magicMatrisVec.size() && i < plainMatrisVec.size(); ++i)
                multipledMatrisVec.push_back(kroneckerMul(magicMatrisVec[i], plainMatrisVec[i]));
        }
        else if(timeZone == "fourth")
        {
            for(size_t i = 0; i < magicMatrisVec.size() && i < plainMatrisVec.size(); ++i)
                multipledMatrisVec.push_back(tracySinghMul(magicMatrisVec[i], plainMatrisVec[i]));
        }
        
        for (const auto& matris : multipledMatrisVec)
            for (const auto& row : matris)
                for (int val : row)
                    oneDimensionMultipled.push_back(val);

        
        for (int val : oneDimensionMultipled) 
        {
            std::vector<int> tempDigits(5, 0);
            int tempVal = std::abs(val);
            
            // basamakları yerleştir:
            for (int i = 4; i >= 0 && tempVal > 0; --i)
            {
                tempDigits[i] = tempVal % 10;
                tempVal /= 10;
            }

            // add temp to vector
            for (int digit : tempDigits)
                oneDigitFinalVec.push_back(digit);
        }

        // Result:
        ciphertext = oneDigitFinalVec;
        // Output:
        try
        {
            if(outputFilePath.empty()) throw std::string("NofilePath");
        
            std::ofstream outputFile(outputFilePath);

            if(!outputFile.is_open()) throw "fileCantOpen";
            while(outputFile.is_open())
            {
                for (size_t i = 0; i < ciphertext.size(); ++i)
                    outputFile << ciphertext[i];
                
                outputFile.close();
                std::cout << "[Output]: Output size: " << ciphertext.size() << "\n";
                std::cout << "[Output]: Output file: " << outputFilePath << "\n";
                std::cout << "[Output]: Succesful\n";
            }
        }
        catch(const std::string e)
        {
            if(e == "NofilePath")
            {
                outputFilePath = "output.txt";
                std::ofstream outputFile(outputFilePath);
                ciphertext = oneDigitFinalVec;

                if(!outputFile.is_open()) throw "fileCantOpen";
                while(outputFile.is_open())
                {
                    for (size_t i = 0; i < ciphertext.size(); ++i)
                        outputFile << ciphertext[i];
                    
                    outputFile.close();
                    std::cout << "[Output]: Output size: " << ciphertext.size() << "\n";
                    std::cout << "[Output]: Output file: " << outputFilePath << "\n";
                    std::cout << "[Output]: Succesful\n";
                }
            }
            if(e == "fileCantOpen") std::cerr << "[Error]: Output file cant opened\n";
        }        
    }
    // deşifreleme:
    else if(operation == "decrypt")
    {
        std::vector<int> digits;
        std::vector<vec2> magicMatrisVec;
        std::vector<vec2> cipherMatrisVec;
        std::vector<vec2> decryptedMatrisVec;
        int64_t distance = 0;

        // Anahtar ı hesapla
        if(timeZone == "first") distance = DisDD(SenderCord, ReceiverCord);
        else if(timeZone == "second") distance = TetHD(SenderCord, ReceiverCord);
        else if(timeZone == "third") distance = TruOD(SenderCord, ReceiverCord);
        else if(timeZone == "fourth") distance = TriOD(SenderCord, ReceiverCord);

        int64_t temp = std::abs(distance);
        while (temp > 0)
        {
            int currentDigit = temp % 10;
            digits.push_back(currentDigit);
            temp /= 10;
        }
        std::reverse(digits.begin(), digits.end());

        magicMatrisVec.resize(digits.size()); 
        for(size_t i = 0 ; i < digits.size(); ++i)
            magicMatrisVec[i] = magicMatris(digits[i]);

        // saate göre matris boyutlarını ayarla:
        int matRows = 0;
        int matCols = 0;

        if(timeZone == "first")         { matRows =  4; matCols =  4; }
        else if(timeZone == "second")   { matRows =  8; matCols =  8; }
        else if(timeZone == "third")    { matRows = 16; matCols = 16; }
        else if(timeZone == "fourth")   { matRows = 16; matCols = 16; }

        int elementsPerMatrix = matRows * matCols;
        
        // Reconstruct matrices
        for(size_t i = 0; i < ciphertext.size(); i += elementsPerMatrix)
        {
            if (i + elementsPerMatrix > ciphertext.size()) break;

            vec2 mat(matRows, std::vector<int>(matCols));
            for(int r = 0; r < matRows; ++r)
                for(int c = 0; c < matCols; ++c)
                    mat[r][c] = ciphertext[i + (r * matCols + c)];
            
            cipherMatrisVec.push_back(mat);
        }

        vec2 matrisPlane(4, std::vector<int>(4, 0));
        
        while(magicMatrisVec.size() < cipherMatrisVec.size())
            magicMatrisVec.push_back(matrisPlane);

        // Decrypt Matrixes
        if(timeZone == "first")
            for(size_t i = 0; i < cipherMatrisVec.size(); ++i)
                decryptedMatrisVec.push_back(inverseHadamardMul(magicMatrisVec[i], cipherMatrisVec[i]));
        else if(timeZone == "second")
            for(size_t i = 0; i < cipherMatrisVec.size(); ++i)
                decryptedMatrisVec.push_back(inverseKhatriRaoMul(magicMatrisVec[i], cipherMatrisVec[i]));
        else if(timeZone == "third")
            for(size_t i = 0; i < cipherMatrisVec.size(); ++i)
                decryptedMatrisVec.push_back(inverseKroneckerMul(magicMatrisVec[i], cipherMatrisVec[i]));
        else if(timeZone == "fourth")
            for(size_t i = 0; i < cipherMatrisVec.size(); ++i)
                decryptedMatrisVec.push_back(inverseTracySinghMul(magicMatrisVec[i], cipherMatrisVec[i]));


        // Translate to String
        for(const auto& mat : decryptedMatrisVec)
            for(const auto& row : mat)
                for(int val : row)
                {
                    wchar_t ch = intToChar(val);
                    if(ch != 0) plaintext.push_back(ch);
                }

        // Output
        if(outputFilePath.empty()) outputFilePath = "output.txt";
        
        std::wofstream outputFile(outputFilePath);
        outputFile.imbue(std::locale("")); // Türkçe karakter desteği

        if(!outputFile.is_open()) 
            std::cerr << "[Error]: Output file cant opened\n";
        else
        {
            outputFile << plaintext;
            outputFile.close();
            
            std::wcout << "[Output]: Decrypted text: " << plaintext << "\n";
            std::cout << "[Output]: Output file: " << outputFilePath << "\n";
            std::cout << "[Output]: Succesful\n";
        }
    }

    // wowie
    return 0;
}
// version 2.0