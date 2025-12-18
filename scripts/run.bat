@echo off
SETLOCAL
SET SCRIPT_DIR=%~dp0
PUSHD %SCRIPT_DIR%..
SET ROOT=%CD%

IF NOT EXIST build (
  mkdir build
)
IF NOT EXIST io (
  mkdir io
)

echo -> Compiling (g++)...
REM MinGW g++ kullanımına göre. Eğer MSVC kullanıyorsan aşağıya bak.
g++ -std=c++17 "%ROOT%\src\main.cpp" -o "%ROOT%\build\crypmath.exe" -lcurl
IF ERRORLEVEL 1 (
  echo Compilation failed. If using MSVC, use the MSVC command below instead.
  POPD
  ENDLOCAL
  exit /b 1
)

echo -> Binary hazır: %ROOT%\build\crypmath.exe
echo -> Başlatılıyor GUI...
python "%ROOT%\src\guiApp.py"

POPD
ENDLOCAL
