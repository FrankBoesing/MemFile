# MemFile
A minimal file system in the "Teensy" address space. It allows to access arbitrary memory areas as a file.

This can be an array in RAM, a memory area in PSRAM, or even a "PROGMEM const array" in the program flash.


```
#include "MemFIle.h" 

MemFS myfs;
char testData[512] = {}; 

void setup() { 
File dataFile = myfs.open(&testData, sizeof(testData), FILE_WRITE);

... 
```

It has no filenames, nor directories. 
The default open (filename, filemode) does not work. It needs the address of your adata, and its size:

`open(char * ptr, size, mode)`

For writes, think of it as a preallocated file.
Any write will *not* make the file grow. You can it use to overwrite contents only.

The normal api calls work (read(), write(), seek() etc)

