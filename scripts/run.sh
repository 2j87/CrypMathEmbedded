#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/build" "$ROOT/io"

echo "-> Compiling C++ (g++)..."
# Eğer sisteminde libcurl yoksa: sudo apt install libcurl4-openssl-dev  (Debian/Ubuntu)
g++ -std=c++17 "$ROOT/src/main.cpp" -o "$ROOT/build/crypmath" -lcurl || {
  echo "Compilation failed. Eğer 'undefined reference' vs. hatası alırsan eski libstdc++ sürümünde olabilirsin. Denemek için: g++ -std=c++17 \"$ROOT/src/main.cpp\" -o \"$ROOT/build/crypmath\" -lcurl -lstdc++fs"
  exit 1
}

echo "-> Binary hazır: $ROOT/build/crypmath"
echo "-> Başlatılıyor GUI..."
python3 "$ROOT/src/guiApp.py"
