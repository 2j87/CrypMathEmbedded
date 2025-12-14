#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using vec2 = std::vector<std::vector<int>>;

int charToInt(char ch)
{
    int arr[] = {1, 2, 3, 4, 5, 6, 7,8, 9,10,11, 20, 22, 24, 26, 28, 30, 32, 34, 35, 59, 62, 65, 68, 71, 74, 77,80,81,127,131,
135,139,143,147,151,155,156, 231, 236, 241, 246, 251, 256, 261, 266, 267, 378, 384, 390, 396, 402,
408, 414, 420, 421, 575, 582, 589, 596, 603, 610, 617, 624, 625,829,837,845,853,861,869,877,885,
886,1165,1174,1183,1192,1201,1210,1211,1536,1546,1556,1566,1576,1586,1606,1607, 2014};

    int retrn = -1;
    switch(ch)
    {
        case 'A': retrn = arr[0]; break;
        case 'B': retrn = arr[1]; break;
        case 'C': retrn = arr[2]; break;
        case 'D': retrn = arr[3]; break;
        case 'E': retrn = arr[4]; break;
        case 'F': retrn = arr[5]; break;
        case 'G': retrn = arr[6]; break;
        case 'H': retrn = arr[7]; break;
        case 'I': retrn = arr[8]; break;
        case 'J': retrn = arr[9]; break;
        case 'K': retrn = arr[10]; break;
        case 'L': retrn = arr[11]; break;
        case 'M': retrn = arr[12]; break;
        case 'N': retrn = arr[13]; break;
        case 'O': retrn = arr[14]; break;
        case 'P': retrn = arr[15]; break;
        case 'Q': retrn = arr[16]; break;
        case 'R': retrn = arr[17]; break;
        case 'S': retrn = arr[18]; break;
        case 'T': retrn = arr[19]; break;
        case 'U': retrn = arr[20]; break;
        case 'V': retrn = arr[21]; break;
        case 'W': retrn = arr[22]; break;
        case 'X': retrn = arr[23]; break;
        case 'Y': retrn = arr[24]; break;
        case 'Z': retrn = arr[25]; break;
        case 'a': retrn = arr[26]; break;
        case 'b': retrn = arr[27]; break;
        case 'c': retrn = arr[28]; break;
        case 'd': retrn = arr[29]; break;
        case 'e': retrn = arr[30]; break;
        case 'f': retrn = arr[31]; break;
        case 'g': retrn = arr[32]; break;
        case 'h': retrn = arr[33]; break;
        case 'i': retrn = arr[34]; break;
        case 'j': retrn = arr[35]; break;
        case 'k': retrn = arr[36]; break;
        case 'l': retrn = arr[37]; break;
        case 'm': retrn = arr[38]; break;
        case 'n': retrn = arr[39]; break;
        case 'o': retrn = arr[40]; break;
        case 'p': retrn = arr[41]; break;
        case 'q': retrn = arr[42]; break;
        case 'r': retrn = arr[43]; break;
        case 's': retrn = arr[44]; break;
        case 't': retrn = arr[45]; break;
        case 'u': retrn = arr[46]; break;
        case 'v': retrn = arr[47]; break;
        case 'w': retrn = arr[48]; break;
        case 'x': retrn = arr[49]; break;
        case 'y': retrn = arr[50]; break;
        case 'z': retrn = arr[51]; break;
        case '.': retrn = arr[52]; break;
        case '(': retrn = arr[53]; break;
        case ')': retrn = arr[54]; break;
        case ',': retrn = arr[55]; break;
        case ';': retrn = arr[56]; break;
        case ':': retrn = arr[57]; break;
        case '\'':retrn = arr[58]; break;
        case '@': retrn = arr[59]; break;
        case '"': retrn = arr[60]; break;
        case '?': retrn = arr[61]; break;
        case '!': retrn = arr[62]; break;
        case '/': retrn = arr[63]; break;
        case '-': retrn = arr[64]; break;
        case '+': retrn = arr[65]; break;
        case '=': retrn = arr[66]; break;
        case ' ': retrn = arr[67]; break;
        case '0': retrn = arr[68]; break;
        case '1': retrn = arr[69]; break;
        case '2': retrn = arr[70]; break;
        case '3': retrn = arr[71]; break;
        case '4': retrn = arr[72]; break;
        case '5': retrn = arr[73]; break;
        case '6': retrn = arr[74]; break;
        case '7': retrn = arr[75]; break;
        case '8': retrn = arr[76]; break;
        case '9': retrn = arr[77]; break;
    }

    return retrn;
}

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
    for(size_t i = 0; i < matris.size() ; ++i) matris[i].resize(4);

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

//Kronecker
//added rectangle multiples
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

//tracySinghMul için yardımcı fonksiyon
//alt matrisin (2*2) sol üst köşesini alıp alt matrisi döndürür
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

//Tracy Singh matris çarpımı
//matrislerin ikisi de 4*4 diye varsayılıyor

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

//Khatri-Rao matris çarpımı (kroneckerMul kullanılarak)
//A ve B 4*4 varsayıldı
vec2 khatriRaoMul(vec2 A, vec2 B)
{
    vec2 returnMatris; returnMatris.resize(8);
    for(size_t i = 0; i < returnMatris.size(); ++i) returnMatris[i].resize(8);
    //son matris 4*4

    std::vector<std::vector<std::pair<vec2, vec2>>> matrisPairs;
    matrisPairs.resize(2);
    for (size_t i = 0; i < 2; ++i)
    {
        matrisPairs[i].resize(2);
        for (size_t j = 0; j < 2; ++j)
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
    matrisPairs[1][0] = {A12, B11};
    matrisPairs[1][1] = {A12, B12};

    for (size_t i = 0; i < 2; ++i)
    {
        for (size_t j = 0; j < 2; ++j)
        {
            // (i,j) -> kronecker çarpımı
            vec2 tmp = kroneckerMul(matrisPairs[i][j].first, matrisPairs[i][j].second);
            size_t blockSize = tmp.size();// save type translation

            for (size_t k = 0; k < blockSize; ++k)
                for (size_t l = 0; l < blockSize; ++l)
                    returnMatris[i * blockSize + k][j * blockSize + l] = tmp[k][l];
        }
    }

    
    return returnMatris;
}

signed main(int argc, char* argv[])
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* time = std::localtime(&t);
    //time->tm_hour // saat
    //time->tm_min // dakika
    //std::cout << time->tm_hour << "\n";
    
    //GPS to ECEF
    //örnek koordinatlar
    CordinateEcef SenderCord = {4113913, 3440529, 3440829};
    CordinateEcef ReceiverCord = {1118567, 902131, -6193309};
    
    /*
        plaintext: şifrelenecek metin
        ciphertext: şifrelenmiş, deşifrelenecek metin
        operation: yapılacak işlem, şifreleme/deşifreleme
    */
    std::string plaintext = "";
    std::vector<int> ciphertext;
    std::string operation;// ecrypt/decrypt/empty

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
    }

    if(operation == "encrypt")
    {
        std::cout << "[Input]: Enter input: ";
        std::cin >> plaintext;
    }
    else if(operation == "decrypt")
    {
        std::cout << "[Input]: Enter input size: ";
        int n;
        std::cin >> n; 
        std::cout << "[Input]: Enter input: ";
        for(int i = 0; i < n; ++i)
        {
            int j;
            std::cin >> j;
            ciphertext.push_back(j);
        }
    }

    std::cout << "[Operation]: ";
    if(operation == "encrypt")
    {
        std::cout << "Encryption\n";
        if(!plaintext.empty()) std::cout << "[Input]: Succesful\n";
        std::cout << "[Input]: Input size: " << plaintext.size() << "\n";
    }
    else if(operation == "decrypt")
    {
        std::cout << "Decryption\n";
        if(!ciphertext.empty()) std::cout << "[Input]: Succesful\n";
        std::cout << "[Input]: Input size: " << ciphertext.size() << "\n";
    }

    std::string timeZone;
    int timeInt = time->tm_hour;
    if(timeInt < 6) timeZone = "first";
    else if(timeInt < 12) timeZone = "second";
    else if(timeInt < 18) timeZone = "third";
    else if(timeInt < 24) timeZone = "fourth";

    if(operation == "encrypt")
    {
        std::vector<int> intPlaintext;
        std::vector<vec2> plainMatris;
        std::vector<int> digits;
        std::vector<vec2> digitsMatris;
        int64_t distance;

        for(int i = 0; i < plaintext.size(); ++i)
            intPlaintext.push_back(charToInt(plaintext[i]));
        
            //kalan yerleri 0 ile doldurma
        while (intPlaintext.size() % 16 != 0)
            intPlaintext.push_back(0);

        for (size_t i = 0; i < intPlaintext.size(); i += 16)
        {
            if(intPlaintext[i] == -1) throw std::invalid_argument("Input have non-english character\n");

            //ile önce block'a yazıp sonra bloks a eklio 
            vec2 block(4, std::vector<int>(4));

            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col)
                    block[row][col] = intPlaintext[i + (row * 4 + col)];
                    
            plainMatris.push_back(block);
        }

        //control yazdırımı:
        for(auto& row : plainMatris[0])
        {
            for(int val : row) std::cout << val << "\t";
            std::cout << std::endl;
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
        
    }
    else if(operation == "decrypt")
    {

    }

    return 0;
}