import sys
import os
import webbrowser
import re
from PyQt5.QtWidgets import (
    QApplication, QWidget, QTabWidget, QVBoxLayout, 
    QHBoxLayout, QLabel, QLineEdit, QTextEdit, 
    QPushButton, QGridLayout, QFileDialog, QMessageBox,
    QFrame
)
from PyQt5.QtCore import QProcess, Qt

# path to executable
# path to executable
if sys.platform == 'win32':
    EXEC_NAME = 'crypmath.exe'
else:
    EXEC_NAME = 'crypmath'

ROOT_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

CRYPTO_EXEC = os.path.normpath(os.path.join(ROOT_DIR, 'build', EXEC_NAME))

IO_DIR = os.path.normpath(os.path.join(ROOT_DIR, 'io'))
DEF_IN_FILE = os.path.join(IO_DIR, 'input_plaintext.txt')
DEF_OUT_FILE = os.path.join(IO_DIR, 'output_ciphertext.txt')
DEF_DEC_FILE = os.path.join(IO_DIR, 'output_decrypted.txt')

class crypto_app(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("CrypMath")
        self.showMaximized() # Full screen usage
        
        # Modern Dark Theme Styling
        self.setStyleSheet("""
            QWidget {
                background-color: #121212;
                color: #e0e0e0;
                font-family: 'Helvetica', sans-serif;
                font-size: 14px;
            }
            QTabWidget::pane {
                border: 1px solid #333;
                background: #1e1e1e;
                border-radius: 10px;
            }
            QTabBar::tab {
                background: #2c2c2c;
                color: #aaa;
                padding: 10px 20px;
                border-top-left-radius: 10px;
                border-top-right-radius: 10px;
                margin-right: 2px;
            }
            QTabBar::tab:selected {
                background: #3d3d3d;
                color: white;
                font-weight: bold;
            }
            QLineEdit, QTextEdit {
                background-color: #2d2d2d;
                border: 1px solid #3d3d3d;
                border-radius: 10px;
                padding: 8px;
                color: white;
            }
            QPushButton {
                background-color: #007bff;
                color: white;
                border-radius: 10px;
                padding: 12px;
                font-weight: bold;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #0056b3;
            }
            QLabel {
                color: #b0b0b0;
                font-weight: 500;
            }
        """)

        self.proc = QProcess(self) 
        self.proc.readyReadStandardOutput.connect(self.handle_out)
        self.proc.readyReadStandardError.connect(self.handle_err)
        self.proc.finished.connect(self.handle_end)

        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(20, 20, 20, 20)
        self.layout.setSpacing(15)

        self.tabs = QTabWidget()
        self.tab_enc = QWidget()
        self.tab_dec = QWidget()
        self.tab_gps = QWidget()
        
        self.tabs.addTab(self.tab_enc, "Şifreleme")
        self.tabs.addTab(self.tab_dec, "Deşifreleme")
        self.tabs.addTab(self.tab_gps, "GPS Araçları")
        
        self.layout.addWidget(self.tabs)
        
        self.init_enc_tab()
        self.init_dec_tab()
        self.init_gps_tab()
        
        self.curr_op = ""

    # setup encrypt tab
    def init_enc_tab(self):
        layout = QVBoxLayout(self.tab_enc)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        
        grid = QGridLayout()
        grid.setVerticalSpacing(15)

        # receiver gps input
        self.recv_gps = QLineEdit("40 30")
        
        grid.addWidget(QLabel("Alıcı GPS (Lat Lon):"), 0, 0)
        grid.addWidget(self.recv_gps, 0, 1)

        layout.addLayout(grid)
        layout.addWidget(QLabel("Şifrelenecek Açık Metin:"))
        
        self.plain_in = QTextEdit()
        self.plain_in.setPlaceholderText("Metni buraya girin...")
        layout.addWidget(self.plain_in)

        # output file selection
        out_box = QHBoxLayout()
        self.enc_out_path = QLineEdit(DEF_OUT_FILE)
        out_box.addWidget(QLabel("Çıktı Dosyası:"))
        out_box.addWidget(self.enc_out_path)
        out_box.addWidget(self.file_btn(self.enc_out_path))
        layout.addLayout(out_box)

        self.enc_btn = QPushButton("Şifrele")
        self.enc_btn.clicked.connect(lambda: self.run_proc("encrypt"))
        layout.addWidget(self.enc_btn)

        layout.addWidget(QLabel("Konsol Çıktıları:"))
        self.enc_console = QTextEdit()
        self.enc_console.setReadOnly(True)
        self.enc_console.setStyleSheet("font-family: 'Courier New'; font-size: 12px;")
        layout.addWidget(self.enc_console)

    # setup decrypt tab
    def init_dec_tab(self):
        layout = QVBoxLayout(self.tab_dec)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        
        # input file selection
        in_box = QHBoxLayout()
        self.dec_in_path = QLineEdit(DEF_OUT_FILE)
        in_box.addWidget(QLabel("Şifreli Dosya:"))
        in_box.addWidget(self.dec_in_path)
        in_box.addWidget(self.file_btn(self.dec_in_path, "open"))
        layout.addLayout(in_box)
        
        # output file selection
        out_box = QHBoxLayout()
        self.dec_out_path = QLineEdit(DEF_DEC_FILE)
        out_box.addWidget(QLabel("Çözülmüş Dosya:"))
        out_box.addWidget(self.dec_out_path)
        out_box.addWidget(self.file_btn(self.dec_out_path))
        layout.addLayout(out_box)

        self.dec_btn = QPushButton("Deşifrele")
        self.dec_btn.clicked.connect(lambda: self.run_proc("decrypt"))
        layout.addWidget(self.dec_btn)

        layout.addWidget(QLabel("Çözülen Metin:"))
        self.plain_out = QTextEdit()
        self.plain_out.setReadOnly(True)
        layout.addWidget(self.plain_out)

        layout.addWidget(QLabel("Konsol Çıktıları:"))
        self.dec_console = QTextEdit()
        self.dec_console.setReadOnly(True)
        self.dec_console.setStyleSheet("font-family: 'Courier New'; font-size: 12px;")
        layout.addWidget(self.dec_console)

    # setup gps tools tab
    def init_gps_tab(self):
        # Lighter gray background for this tab
        self.tab_gps.setStyleSheet("background-color: #252525; border-radius: 10px;")
        
        layout = QVBoxLayout(self.tab_gps)
        layout.setContentsMargins(40, 40, 40, 40)
        layout.setSpacing(25)
        layout.setAlignment(Qt.AlignCenter)
        
        # Instruction Box
        instr_box = QFrame()
        instr_box.setStyleSheet("""
            QFrame {
                background-color: #333333;
                border-radius: 15px;
                border: 1px solid #444444;
            }
            QLabel {
                color: #dddddd;
                font-size: 15px;
                border: none;
            }
        """)
        box_layout = QVBoxLayout(instr_box)
        box_layout.setContentsMargins(20, 20, 20, 20)
        
        info = QLabel("1. 'Haritayı Aç' butonuna basın.\n2. Konumu seçin.\n3. URL'yi kopyalayıp aşağıya yapıştırın.")
        info.setAlignment(Qt.AlignCenter)
        box_layout.addWidget(info)
        
        layout.addWidget(instr_box)
        
        # Widgets Style
        widget_style = """
            QLineEdit {
                background-color: #2d2d2d;
                border: 1px solid #3d3d3d;
                border-radius: 10px;
                padding: 12px;
                color: white;
                font-size: 14px;
            }
            QPushButton {
                background-color: #007bff;
                color: white;
                border-radius: 10px;
                padding: 12px;
                font-weight: bold;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #0056b3;
            }
        """
        
        btn_open = QPushButton("Google Maps'i Aç")
        btn_open.setFixedWidth(300)
        btn_open.setStyleSheet(widget_style)
        btn_open.clicked.connect(lambda: webbrowser.open("https://www.google.com/maps"))
        layout.addWidget(btn_open, alignment=Qt.AlignCenter)
        
        self.url_in = QLineEdit()
        self.url_in.setPlaceholderText("Google Maps URL'sini buraya yapıştırın...")
        self.url_in.setFixedWidth(500)
        self.url_in.setStyleSheet(widget_style)
        self.url_in.textChanged.connect(self.parse_url)
        layout.addWidget(self.url_in, alignment=Qt.AlignCenter)
        
        res_lbl = QLabel("Koordinatlar:")
        res_lbl.setAlignment(Qt.AlignCenter)
        res_lbl.setStyleSheet("color: #b0b0b0; font-weight: 500; font-size: 14px;")
        layout.addWidget(res_lbl)
        
        self.res_disp = QLineEdit()
        self.res_disp.setReadOnly(True)
        self.res_disp.setFixedWidth(300)
        self.res_disp.setAlignment(Qt.AlignCenter)
        # Custom style for result to make it pop but keep theme
        self.res_disp.setStyleSheet("""
            background-color: #1e1e1e;
            border: 2px solid #4CAF50;
            border-radius: 10px;
            padding: 10px;
            color: #4CAF50;
            font-size: 18px;
            font-weight: bold;
        """)
        layout.addWidget(self.res_disp, alignment=Qt.AlignCenter)
        
        btn_copy = QPushButton("Kopyala")
        btn_copy.setFixedWidth(200)
        btn_copy.setStyleSheet(widget_style)
        btn_copy.clicked.connect(self.copy_coords)
        layout.addWidget(btn_copy, alignment=Qt.AlignCenter)

    # parse google maps url
    def parse_url(self):
        url = self.url_in.text()
        match = re.search(r'@(-?\d+\.\d+),(-?\d+\.\d+)', url)
        if match:
            lat, lon = match.groups()
            self.res_disp.setText(f"{lat} {lon}")
        else:
            self.res_disp.clear()

    # copy coordinates
    def copy_coords(self):
        coords = self.res_disp.text()
        if coords:
            QApplication.clipboard().setText(coords)
            QMessageBox.information(self, "Başarılı", "Koordinatlar kopyalandı!")
        else:
            QMessageBox.warning(self, "Hata", "Geçerli koordinat bulunamadı.")

    # create file button
    def file_btn(self, target, mode="save"):
        btn = QPushButton("Gözat...")
        if mode == "save":
            btn.clicked.connect(lambda: self.save_diag(target))
        else:
            btn.clicked.connect(lambda: self.open_diag(target))
        return btn

    # show save dialog
    def save_diag(self, target):
        fname, _ = QFileDialog.getSaveFileName(self, 'Kaydet', target.text(), "Tüm (*)")
        if fname:
            target.setText(fname)

    # show open dialog
    def open_diag(self, target):
        fname, _ = QFileDialog.getOpenFileName(self, 'Aç', target.text(), "Tüm (*)")
        if fname:
            target.setText(fname)
            
    # start crypto process
    def run_proc(self, op):
        self.curr_op = op
        args = []
        console = None

        if op == "encrypt":
            # write temp input
            text = self.plain_in.toPlainText()
            if not text:
                QMessageBox.warning(self, "Uyarı", "Metin girin.")
                return
            
            with open(DEF_IN_FILE, 'w', encoding='utf-8') as f:
                f.write(text)
            
            out_path = self.enc_out_path.text()
            out_path = os.path.abspath(self.enc_out_path.text())
            args = ['--encrypt', '-r',  os.path.abspath(DEF_IN_FILE), '-o', out_path]
            console = self.enc_console
            
        elif op == "decrypt":
            in_path = self.dec_in_path.text()
            if not os.path.exists(in_path):
                QMessageBox.warning(self, "Uyarı", "Dosya yok.")
                return
            
            out_path = self.dec_out_path.text()
            args = ['--decrypt', '-r', in_path, '-o', out_path]
            console = self.dec_console

        # clear console
        console.clear()
        console.append(f"Komut: {os.path.basename(CRYPTO_EXEC)} {' '.join(args)}")
        
        # start process
        try:
            self.proc.start(CRYPTO_EXEC, args)
            self.tabs.setEnabled(False)
        except Exception as e:
            console.append(f"Hata: {e}")

    # handle stdout
    def handle_out(self):
        data = self.proc.readAllStandardOutput().data().decode('utf-8', errors='replace')
        
        # send input if asked
        if "[Input]: Please input" in data:
             # get receiver gps
            gps = self.recv_gps.text()
            self.proc.write(f"{gps}\n".encode('utf-8'))

        if self.curr_op == "encrypt":
            self.enc_console.append(data.strip())
        else:
            self.dec_console.append(data.strip())

    # handle stderr
    def handle_err(self):
        data = self.proc.readAllStandardError().data().decode('utf-8', errors='replace')
        if self.curr_op == "encrypt":
            self.enc_console.append(f"ERR: {data.strip()}")
        else:
            self.dec_console.append(f"ERR: {data.strip()}")

    # handle process finished
    def handle_end(self, code, status):
        self.tabs.setEnabled(True)
        
        if status == 0 and code == 0:
            QMessageBox.information(self, "Tamam", "İşlem bitti.")
            
            if self.curr_op == "decrypt":
                try:
                    with open(self.dec_out_path.text(), 'r', encoding='utf-8') as f:
                        self.plain_out.setText(f.read())
                except:
                    QMessageBox.critical(self, "Hata", "Dosya okunamadı.")
        else:
            QMessageBox.critical(self, "Hata", "İşlem başarısız.")

if __name__ == '__main__':
    # check executable exists
    if not os.path.exists(CRYPTO_EXEC):
        QMessageBox.critical(None, "Hata", "Motor bulunamadı.")
        sys.exit(1)
        
    app = QApplication(sys.argv)
    ex = crypto_app()
    ex.show()
    sys.exit(app.exec_())