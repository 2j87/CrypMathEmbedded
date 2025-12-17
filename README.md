# CrypMath

CrypMath is a hybrid encryption tool that combines C++ performance with a modern Python GUI. It uses a custom mathematical encryption algorithm based on 3D coordinate systems and matrix operations.

## Features

### 1. Encryption & Decryption
-   **Custom Algorithm**: Uses ECEF coordinates, magic matrices, and complex geometric distances (Disdyakis Dodecahedron, etc.) for encryption.
-   **High Performance**: Core logic is written in C++ for speed.
-   **File-Based**: Processes text files for input and output.

### 2. Modern GUI
-   **Dark Theme**: Sleek, modern interface with a dark color scheme.
-   **Full Screen**: Designed for immersive usage.
-   **User Friendly**: Simple tabs for Encryption, Decryption, and Tools.

### 3. Integrated GPS Tools
-   **Built-in Maps**: Open Google Maps directly from the application.
-   **Coordinate Extraction**: Paste a Google Maps URL to automatically extract Latitude and Longitude.
-   **One-Click Copy**: Easily copy coordinates for use in encryption.

## Installation

### Prerequisites
-   **Linux** (Tested on Ubuntu/Debian)
-   **Python 3** with `PyQt5`
    ```bash
    sudo apt install python3-pyqt5
    ```
-   **G++ Compiler**
    ```bash
    sudo apt install g++
    ```
-   **libcurl** (for C++ IP location)
    ```bash
    sudo apt install libcurl4-openssl-dev
    ```

### Build
1.  Compile the C++ engine:
    ```bash
    mkdir -p build
    g++ main.cpp -o build/crypmath -lcurl
    ```

## Usage
1.  Run the Python GUI:
    ```bash
    python3 guiApp.py
    ```
2.  **Encryption**:
    -   Enter Receiver GPS coordinates (or use the GPS Tools tab).
    -   Type your message.
    -   Click "Şifrele".
3.  **Decryption**:
    -   Select the encrypted file.
    -   Click "Deşifrele".

## Project Structure
-   `main.cpp`: The core C++ encryption engine.
-   `guiApp.py`: The Python PyQt5 graphical interface.
-   `build/`: Directory containing the compiled executable.
