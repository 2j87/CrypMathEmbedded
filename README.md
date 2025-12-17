### Tubitak Project about cryptology

# Usage

> [!Warning]
> You must encrypt or decrypt

> [!Warning]
> while compile dont forget use `-lcurl` flag for library curl

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
| `--encrypt` | starts by encrypt mode | 
| `--decrypt` | starts by decrypt mode | 
| `-o` | assigns output file | 
| `-r` | assigns input file | 


compile example : 
```bash
g++ main.cpp -o ./build/crypmath -Wall -Wextra -O2 -lcurl
```
