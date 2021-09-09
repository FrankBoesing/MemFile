# MemFile
A minimal file in the "Teensy" address space. It allows to access arbitrary memory areas as a file.

This can be an array in RAM, a memory area in PSRAM, or even a "PROGMEM const array" in the program flash.

You could malloc() an area and use it as file.

It can access the whole Teensy adress space (where allowed).
Internally it uses just a pointer.There are no address checks. Forbidden addresses cause a crash - the normal behavior as with other pointers. It also does not write to the Flash.

```
#include "MemFile.h" 

MemFS myfs;
File dataFile;
char testData[512] = {}; 

void setup() { 
  dataFile = myfs.open(&testData, sizeof(testData), FILE_WRITE);

... 
```

It has no filenames, nor directories. 
The default open (filename, filemode) does not work. It needs the address of your data, and its size:

`open(char * ptr, size, mode)`

For writes, think of it as a preallocated file.
Any write will *not* make the file grow. You can it use to overwrite contents only.

The normal api calls work (read(), write(), seek() etc)
"flush()" exists for comptibility, but does nothing.

