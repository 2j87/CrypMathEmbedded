import sys
import subprocess
import os
import datetime
from PyQt5.QtWidgets import (
    QApplication, QWidget, QTabWidget, QVBoxLayout, 
    QHBoxLayout, QLabel, QLineEdit, QTextEdit, 
    QPushButton, QGridLayout, QTimeEdit, QFileDialog, 
    QMessageBox
)
from PyQt5.QtCore import QTime, QProcess

# ----------------------------------------------------
# KRİPTO MOTORU AYARLARI
# ----------------------------------------------------

# crypmath yürütülebilir dosyasının yolu. 
# Linux'ta ./build/crypmath olarak varsayılmıştır.
# Lütfen doğru yolu kontrol edin.
CRYPTO_EXECUTABLE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'build', 'crypmath')

# Varsayılan G/Ç dosyaları (GUI metin alanları için ara bellek görevi görür)
DEFAULT_INPUT_TEXT_FILE = 'input_plaintext.txt'
DEFAULT_OUTPUT_TEXT_FILE = 'output_ciphertext.txt'
DEFAULT_DECRYPT_OUTPUT_FILE = 'output_decrypted.txt'

class CryptoApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("CrypMath")
        self.setGeometry(100, 100, 900, 700)
        self.process = QProcess(self) # C++ motorunu çalıştırmak için QProcess kullanıyoruz
        self.process.readyReadStandardOutput.connect(self.handle_stdout)
        self.process.readyReadStandardError.connect(self.handle_stderr)
        self.process.finished.connect(self.handle_finished)

        self.layout = QVBoxLayout(self)
        self.tabs = QTabWidget()
        self.tab_encrypt = QWidget()
        self.tab_decrypt = QWidget()
        
        self.tabs.addTab(self.tab_encrypt, "Şifreleme")
        self.tabs.addTab(self.tab_decrypt, "Deşifreleme")
        
        self.layout.addWidget(self.tabs)
        
        self.setup_encrypt_tab()
        self.setup_decrypt_tab()
        
        self.current_operation = ""

    # ----------------------------------------------------
    # ARAYÜZ KURULUMU
    # ----------------------------------------------------
    def setup_encrypt_tab(self):
        layout = QVBoxLayout(self.tab_encrypt)
        grid = QGridLayout()

        # GPS ve Zaman Girişleri (Proje Mantığı İçin Gerekli)
        self.sender_gps = QLineEdit("39.92556K, 32.8375D")
        self.receiver_gps = QLineEdit("29.97611K, 31.13278D")
        
        # C++ motoru zamanı sistemden alıyor, bu alan sadece bilgi amaçlı/simülasyon için
        self.send_time = QTimeEdit(QTime.currentTime())
        self.send_time.setDisplayFormat("HH:mm") 
        self.send_time.setEnabled(False) # C++ kodunuz zamanı sistemden aldığı için devre dışı

        grid.addWidget(QLabel("Gönderici GPS:"), 0, 0)
        grid.addWidget(self.sender_gps, 0, 1)
        grid.addWidget(QLabel("Alıcı GPS:"), 1, 0)
        grid.addWidget(self.receiver_gps, 1, 1)
        grid.addWidget(QLabel("Sistem Saati (Anahtar Tipi):"), 2, 0)
        grid.addWidget(self.send_time, 2, 1)

        layout.addLayout(grid)
        layout.addWidget(QLabel("Şifrelenecek Açık Metin:"))
        
        self.plain_text_input = QTextEdit()
        self.plain_text_input.setPlaceholderText("Açık metni buraya girin...")
        layout.addWidget(self.plain_text_input)

        # Çıktı Dosya Yolu Seçimi
        output_hbox = QHBoxLayout()
        self.encrypt_output_path = QLineEdit(DEFAULT_OUTPUT_TEXT_FILE)
        output_hbox.addWidget(QLabel("Çıktı Dosyası (-o):"))
        output_hbox.addWidget(self.encrypt_output_path)
        output_hbox.addWidget(self.create_file_dialog_button(self.encrypt_output_path))
        layout.addLayout(output_hbox)

        self.encrypt_button = QPushButton("Şifrele ve Dosyaya Kaydet")
        self.encrypt_button.clicked.connect(lambda: self.start_crypto_process("encrypt"))
        layout.addWidget(self.encrypt_button)

        layout.addWidget(QLabel("C++ Konsol Çıktıları:"))
        self.encrypt_console_output = QTextEdit()
        self.encrypt_console_output.setReadOnly(True)
        layout.addWidget(self.encrypt_console_output)


    def setup_decrypt_tab(self):
        layout = QVBoxLayout(self.tab_decrypt)
        
        # Dosya Girişi Seçimi (-r)
        input_hbox = QHBoxLayout()
        self.decrypt_input_path = QLineEdit(DEFAULT_OUTPUT_TEXT_FILE)
        input_hbox.addWidget(QLabel("Şifreli Metin Dosyası (-r):"))
        input_hbox.addWidget(self.decrypt_input_path)
        input_hbox.addWidget(self.create_file_dialog_button(self.decrypt_input_path, mode="open"))
        layout.addLayout(input_hbox)
        
        # Çıktı Dosya Yolu Seçimi (-o)
        output_hbox = QHBoxLayout()
        self.decrypt_output_path = QLineEdit(DEFAULT_DECRYPT_OUTPUT_FILE)
        output_hbox.addWidget(QLabel("Çözülmüş Metin Dosyası (-o):"))
        output_hbox.addWidget(self.decrypt_output_path)
        output_hbox.addWidget(self.create_file_dialog_button(self.decrypt_output_path))
        layout.addLayout(output_hbox)

        self.decrypt_button = QPushButton("Deşifrele ve Dosyaya Kaydet")
        self.decrypt_button.clicked.connect(lambda: self.start_crypto_process("decrypt"))
        layout.addWidget(self.decrypt_button)

        layout.addWidget(QLabel("Deşifrelenen Açık Metin:"))
        self.plain_text_output = QTextEdit()
        self.plain_text_output.setReadOnly(True)
        layout.addWidget(self.plain_text_output)

        layout.addWidget(QLabel("C++ Konsol Çıktıları:"))
        self.decrypt_console_output = QTextEdit()
        self.decrypt_console_output.setReadOnly(True)
        layout.addWidget(self.decrypt_console_output)
        

    def create_file_dialog_button(self, target_line_edit, mode="save"):
        btn = QPushButton("Gözat...")
        if mode == "save":
            btn.clicked.connect(lambda: self.show_save_file_dialog(target_line_edit))
        else:
            btn.clicked.connect(lambda: self.show_open_file_dialog(target_line_edit))
        return btn

    def show_save_file_dialog(self, target_line_edit):
        fname, _ = QFileDialog.getSaveFileName(self, 'Dosya Kaydet', target_line_edit.text(), "Tüm Dosyalar (*)")
        if fname:
            target_line_edit.setText(fname)

    def show_open_file_dialog(self, target_line_edit):
        fname, _ = QFileDialog.getOpenFileName(self, 'Dosya Aç', target_line_edit.text(), "Tüm Dosyalar (*)")
        if fname:
            target_line_edit.setText(fname)
            
    # ----------------------------------------------------
    # İŞLEM YÖNETİMİ (QProcess)
    # ----------------------------------------------------
    def start_crypto_process(self, operation):
        self.current_operation = operation
        
        input_path = ""
        output_path = ""
        console_widget = None

        if operation == "encrypt":
            # 1. Açık metni geçici dosyaya yaz
            plain_text = self.plain_text_input.toPlainText()
            if not plain_text:
                QMessageBox.warning(self, "Uyarı", "Lütfen şifrelenecek metni girin.")
                return
            
            input_path = DEFAULT_INPUT_TEXT_FILE
            with open(input_path, 'w') as f:
                f.write(plain_text)
            
            # 2. Çıktı yolunu al
            output_path = self.encrypt_output_path.text()
            if not output_path:
                QMessageBox.warning(self, "Uyarı", "Lütfen çıktı dosya yolunu belirtin.")
                return

            # C++'ın beklediği argümanlar
            arguments = ['--encrypt', '-r', input_path, '-o', output_path]
            console_widget = self.encrypt_console_output
            
        elif operation == "decrypt":
            # 1. Girdi dosyasını al (-r zorunlu)
            input_path = self.decrypt_input_path.text()
            if not input_path or not os.path.exists(input_path):
                QMessageBox.warning(self, "Uyarı", "Geçerli bir şifreli metin dosyası (-r) belirtin.")
                return
            
            # 2. Çıktı yolunu al
            output_path = self.decrypt_output_path.text()
            if not output_path:
                QMessageBox.warning(self, "Uyarı", "Lütfen çıktı dosya yolunu belirtin.")
                return

            # C++'ın beklediği argümanlar
            arguments = ['--decrypt', '-r', input_path, '-o', output_path]
            console_widget = self.decrypt_console_output

        # Konsolu temizle
        console_widget.clear()
        console_widget.append(f"Çalıştırılan komut: {os.path.basename(CRYPTO_EXECUTABLE)} {' '.join(arguments)}")
        
        # QProcess'i başlat
        try:
            self.process.start(CRYPTO_EXECUTABLE, arguments)
            self.tabs.setEnabled(False) # İşlem bitene kadar arayüzü kilitle
        except Exception as e:
            console_widget.append(f"HATA: Kripto motoru başlatılamadı: {e}")
            QMessageBox.critical(self, "Hata", f"Kripto motoru başlatılamadı. Yol: {CRYPTO_EXECUTABLE}")

    def handle_stdout(self):
        data = self.process.readAllStandardOutput().data().decode()
        if self.current_operation == "encrypt":
            self.encrypt_console_output.append(data.strip())
        elif self.current_operation == "decrypt":
            self.decrypt_console_output.append(data.strip())

    def handle_stderr(self):
        data = self.process.readAllStandardError().data().decode()
        if self.current_operation == "encrypt":
            self.encrypt_console_output.append(f"HATA (STDERR): {data.strip()}")
        elif self.current_operation == "decrypt":
            self.decrypt_console_output.append(f"HATA (STDERR): {data.strip()}")

    def handle_finished(self, exitCode, exitStatus):
        self.tabs.setEnabled(True)
        
        # Sonuç mesajı
        if exitStatus == QProcess.NormalExit and exitCode == 0:
            QMessageBox.information(self, "İşlem Tamamlandı", f"{self.current_operation.capitalize()} işlemi başarıyla tamamlandı.")
            
            # Başarılı çıktı dosyasını oku ve metin kutusuna yazdır (sadece deşifrelemede anlamlı)
            if self.current_operation == "decrypt":
                try:
                    with open(self.decrypt_output_path.text(), 'r') as f:
                        decrypted_text = f.read()
                    self.plain_text_output.setText(decrypted_text)
                except Exception as e:
                    QMessageBox.critical(self, "Okuma Hatası", f"Çözülmüş çıktı dosyası okunamadı: {e}")
                    
            elif self.current_operation == "encrypt":
                # Şifrelemede oluşan sayılar çok uzun olduğu için konsolda okumak yerine, 
                # sadece çıktının oluştuğunu belirtmek yeterlidir.
                self.encrypt_console_output.append(f"Şifreli metin başarıyla '{self.encrypt_output_path.text()}' dosyasına yazıldı.")

        else:
            # Hata durumunda (Non-zero exit code)
            QMessageBox.critical(self, "İşlem Başarısız", f"{self.current_operation.capitalize()} işlemi başarısız oldu. Konsol çıktılarını kontrol edin.")


if __name__ == '__main__':
    # QProcess'in yolu bulabilmesi için, eğer dosya aynı dizinde değilse PATH'e eklemek gerekebilir.
    # Bu basit kodda, dosyanın ./build/crypmath yolunda olduğunu varsayıyoruz.
    if not os.path.exists(CRYPTO_EXECUTABLE):
        QMessageBox.critical(None, "Kritik Hata", f"Kripto Motoru Bulunamadı: {CRYPTO_EXECUTABLE}\nLütfen yolun doğru olduğundan emin olun ve C++ dosyasını derleyin.")
        sys.exit(1)
        
    app = QApplication(sys.argv)
    ex = CryptoApp()
    ex.show()
    sys.exit(app.exec_())