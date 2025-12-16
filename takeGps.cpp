#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <sstream>
#include <curl/curl.h>

// SENİN VERDİĞİN TOKEN BURAYA EKLENDİ
const std::string API_TOKEN = "pk.2199f693381d18835d41ed1e7a442800";

struct AccessPoint {
    std::string mac;
    int signal;
};

// YARDIMCI: Komut satırı çıktısını okumak için
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() başarısız oldu!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// 1. ADIM: Wi-Fi Ağlarını Tara (nmcli kullanarak)
std::vector<AccessPoint> scanWifiNetworks() {
    std::vector<AccessPoint> aps;
    
    // Linux terminal komutu: nmcli
    // Sadece BSSID (MAC) ve SIGNAL gücünü alıyoruz.
    std::string output = exec("nmcli -t -f BSSID,SIGNAL dev wifi list");
    
    std::stringstream ss(output);
    std::string line;

    while (std::getline(ss, line)) {
        size_t lastColon = line.find_last_of(':');
        if (lastColon != std::string::npos) {
            std::string mac = line.substr(0, lastColon);
            
            // MAC adresindeki ters slashları temizle (nmcli bazen kaçış karakteri ekler)
            std::string cleanMac;
            for(char c : mac) {
                if(c != '\\') cleanMac += c;
            }

            std::string signalStr = line.substr(lastColon + 1);
            
            try {
                int signal = std::stoi(signalStr);
                aps.push_back({cleanMac, signal});
            } catch (...) {
                continue;
            }
        }
    }
    return aps;
}

// YARDIMCI: cURL için yazma fonksiyonu
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 2. ADIM: Unwired Labs API'sine İstek Gönder
void getCoordinates(const std::vector<AccessPoint>& aps) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // --- ÖNEMLİ DEĞİŞİKLİK BURADA ---
    // Unwired Labs için JSON formatı oluşturuluyor.
    // Format: { "token": "...", "wifi": [ {"bssid":"...", "signal":...}, ... ] }
    
    std::string jsonPayload = "{ \"token\": \"" + API_TOKEN + "\", \"wifi\": [";
    
    // En fazla 5-6 tane Wi-Fi göndermek genelde yeterlidir ve hızlı çalışır
    int limit = (aps.size() > 7) ? 7 : aps.size();
    
    for (size_t i = 0; i < limit; ++i) {
        jsonPayload += "{ \"bssid\": \"" + aps[i].mac + "\",";
        jsonPayload += " \"signal\": " + std::to_string(aps[i].signal) + "}";
        
        if (i < limit - 1) jsonPayload += ",";
    }
    jsonPayload += "]}";

    curl = curl_easy_init();
    if(curl) {
        // Unwired Labs API Adresi
        std::string url = "https://us1.unwiredlabs.com/v2/process.php";
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // SSL sertifika hatalarını yok say (Test amaçlı)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        std::cout << "Sunucuya istek gönderiliyor..." << std::endl;
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "cURL Bağlantı Hatası: %s\n", curl_easy_strerror(res));
        } else {
            std::cout << "\n--- API YANITI ---\n" << readBuffer << std::endl;
            
            // Basitçe lat ve lon değerlerini bulup gösterelim
            size_t latPos = readBuffer.find("\"lat\":");
            size_t lonPos = readBuffer.find("\"lon\":");
            
            if (latPos != std::string::npos && lonPos != std::string::npos) {
                std::cout << "\n-----------------------------------" << std::endl;
                std::cout << " KONUM BAŞARIYLA ÇÖZÜLDÜ!" << std::endl;
                // Parse işlemi (string manipülasyonu ile)
                // Gerçek projede nlohmann/json kullanman daha sağlıklı olur.
                std::cout << " Detaylar yukarıdaki JSON çıktısındadır." << std::endl;
                std::cout << "-----------------------------------" << std::endl;
            } else {
                std::cout << "\n[HATA] Konum çözülemedi. Çevrede yeterli kayıtlı Wi-Fi olmayabilir." << std::endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int main() {
    std::cout << "Etraftaki Wi-Fi aglari taraniyor (nmcli)..." << std::endl;
    
    // Wi-Fi verilerini çek
    std::vector<AccessPoint> aps = scanWifiNetworks();

    if (aps.empty()) {
        std::cout << "HATA: Hiçbir Wi-Fi ağı bulunamadı!" << std::endl;
        std::cout << "1. Wi-Fi adaptörünün açık olduğundan emin ol." << std::endl;
        std::cout << "2. 'nmcli dev wifi list' komutunun terminalde çalıştığını kontrol et." << std::endl;
        return 1;
    }

    std::cout << aps.size() << " adet Wi-Fi ağı bulundu. Konum API'sine soruluyor..." << std::endl;
    
    // API'ye gönder
    getCoordinates(aps);

    return 0;
}
