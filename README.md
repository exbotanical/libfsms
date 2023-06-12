# fsms

## Dynamic Linking

Linking to `fsms`:

```bash
# 1) include and use fsms in your project
# 2) generate object file for your project
gcc -I ../path/to/fsms -c main.c -o main.o
# 3) generate shared object file
make
# 4) link your project to fsms
gcc -o main main.o -L../path/to/fsms -lfsms
# you may need to add the lib location to your PATH
```
