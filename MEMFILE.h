/*
MIT License

Copyright (c) 2021 Frank Bösing

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once

#include <Arduino.h>
#include <FS.h>

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
static const char empty[] = "";

class MemFile : public File
{
public:
  //Buffer is not checked for null pointers etc. We have no assert(), and just returning 0 would be no good.
  //So..just let the mcu crash if something is wrong.
  virtual size_t write(const void *buf, size_t nbyte) {
    if (base == nullptr) return 0;
    if (readonly) return 0;
    if (ofs + nbyte > (unsigned)sz) nbyte = sz - ofs;
    if (nbyte > 0) memcpy(base + ofs, buf, nbyte);
    ofs += nbyte;    
    return nbyte;
  }
  virtual int peek() {
    return *(base + ofs);
  }
  virtual int available() {
    if (base == nullptr) return 0;
    int s = sz - ofs;
    if (s < 0) s = 0;
    return s;
  }
  virtual void flush() {
  }
  virtual size_t read(void *buf, size_t nbyte) {
    if (ofs + nbyte > (unsigned)sz) nbyte = sz - ofs;
    if (nbyte > 0) memcpy(buf, base + ofs, nbyte);
    ofs += nbyte;
    return nbyte;
  }
  virtual bool truncate(uint64_t UNUSED(size)) {
    return false;
  }
  virtual bool seek(uint64_t pos, int mode = SeekSet) {
    if (base == nullptr) return false;
    int p = pos;    
    if (mode == SeekCur) p = ofs + pos;
    else if (mode == SeekEnd) p = ofs + sz - pos;
    if (p < 0 || p > sz) return false;
    ofs = p;
    return true;
  }
  virtual uint64_t position() {
    if (base == nullptr) return 0;
    return ofs;
  }
  virtual uint64_t size() {
    return sz;
  }
  virtual void close() {
     base = nullptr;
     ofs = 0;
     sz = 0;
  }
  virtual operator bool() {
    return base != nullptr;
  }
  virtual const char * name() {
    return empty;
  }
  virtual boolean isDirectory(void) {
    return false;
  }
  virtual File openNextFile(uint8_t UNUSED(mode)) {
    return File();
  }
  virtual void rewindDirectory(void) {
  }
  using Print::write;
private:
  friend class MemFS;
  MemFile(char *p, size_t size, uint8_t mode) {
    base = p;
    ofs = 0;
    this->sz = size;    
    readonly = (mode == FILE_READ);
  }  
  char *base = nullptr;
  int ofs = 0;
  int sz = 0;
  bool readonly = false;
};




class MemFS : public FS
{
public:
  MemFS() {
  }  
  File open(const char *UNUSED(ptr), uint8_t UNUSED(mode)) {    
    return File();
  }
  File open(char *ptr, size_t size, uint8_t mode = FILE_READ) {
    this->size = size;
    if (size > 0) return File(new MemFile(ptr, size, mode));
    return File();
  }  
  bool exists(const char *UNUSED(filepath)) {
    return true;
  }
  bool mkdir(const char *UNUSED(filepath)) {
    return false;
  }
  bool rename(const char *UNUSED(oldfilepath), const char *UNUSED(newfilepath)) {
    return false;
  }
  bool remove(const char *UNUSED(filepath)) {
    return false;
  }
  bool rmdir(const char *UNUSED(filepath)) {
    return false;
  }
  uint64_t usedSize() {
    return size;
  }
  uint64_t totalSize() {
    return size;
  }
  
protected:
  size_t size;
};
