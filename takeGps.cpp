#include <curl/curl.h>
#include <iostream>
#include <string>

// YARDIMCI: cURL verisini string'e yazma fonksiyonu
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

void getIpLocation() {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    // Ücretsiz, keysiz IP API servisi
    std::string url = "http://ip-api.com/json/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Hızlı sonuç için timeout koyalım (5 saniye)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    std::cout << "IP servisine baglaniliyor..." << std::endl;
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "Baglanti Hatasi: %s\n", curl_easy_strerror(res));
    } else {
      std::cout << "\n--- KONUM BULUNDU ---" << std::endl;
      // JSON parse işlemi (Basit string arama)

      // Şehir
      size_t cityPos = readBuffer.find("\"city\":\"");
      if (cityPos != std::string::npos) {
        size_t end = readBuffer.find("\"", cityPos + 8);
        std::cout << "Sehir : "
                  << readBuffer.substr(cityPos + 8, end - (cityPos + 8))
                  << std::endl;
      }

      // Enlem (Lat)
      size_t latPos = readBuffer.find("\"lat\":");
      if (latPos != std::string::npos) {
        size_t end = readBuffer.find(",", latPos);
        std::cout << "Enlem : "
                  << readBuffer.substr(latPos + 6, end - (latPos + 6))
                  << std::endl;
      }

      // Boylam (Lon)
      size_t lonPos = readBuffer.find("\"lon\":");
      if (lonPos != std::string::npos) {
        size_t end = readBuffer.find(",", lonPos);
        std::cout << "Boylam: "
                  << readBuffer.substr(lonPos + 6, end - (lonPos + 6))
                  << std::endl;
      }

      std::cout << "---------------------" << std::endl;
      std::cout
          << "Not: Bu konum internet servis saglayicinizin santral konumudur."
          << std::endl;
    }

    curl_easy_cleanup(curl);
  }
}

int main() {
  getIpLocation();
  return 0;
}