# ICAUR
Extremely simple AUR helper written in C, that only gets PKGBUILD files for you.
## Dependencies
```
curl
cjson
```
## Usage with gcc
Compile the main.c file
```
gcc -o icaur main.c -lcurl -lcjson
```
Run the compiled file
```
./icaur
```
