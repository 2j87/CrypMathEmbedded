### Tubitak Project about cryptology

# Usage

> [!Warning]
> You must encrypt or decrypt

> [!Warning]
> while compile dont forget use `-lcurl` flag for library curl

for install curl library (int linux/ubuntu):
```bash
sudo apt-get install libcurl4-openssl-dev
```

encrypt:

`./build/crypmath --encrypt`

decrypt:

`./build/crypmath --decrypt`

assign output file:
default ouputfile is "output.txt"

`./build/crypmath --encrypt -o output.txt`

assign input file:
default input is terminal

`./build/crypmath --encrypt -r input.txt`

## Simple Chart

| **flag** | **usage** | 
|:---|:---|
| `--encrypt` | starts by encrypt mode | 
| `--decrypt` | starts by decrypt mode | 
| `-o` | assigns output file | 
| `-r` | assigns input file | 


compile example : 
```bash
g++ main.cpp -o ./build/crypmath -Wall -Wextra -O2 -lcurl
```

encrypt example : 
```bash
./build/crypmath -encrypt -r ./build/input.txt -o ./build/output.txt
```

decrypt example : 
```bash
./build/crypmath -decrypt -r ./build/output.txt -o ./build/outputDecrypt.txt
```
