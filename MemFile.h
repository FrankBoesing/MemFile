/*

Copyright (c) 2021 Frank Bösing

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.


   Diese Bibliothek ist freie Software: Sie können es unter den Bedingungen
   der GNU General Public License, wie von der Free Software Foundation,
   Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
   veröffentlichten Version, weiterverbreiten und/oder modifizieren.

   Diese Bibliothek wird in der Hoffnung, dass es nützlich sein wird, aber
   OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
   Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
   Siehe die GNU General Public License für weitere Details.

   Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
   Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.

*/
#pragma once
#if defined(__cplusplus)

#include <FS.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"


class MemFile : public FileImpl
{
public:
  //Buffer is not checked for null pointers etc. We have no assert(), and just returning 0 would be no good.
  //So..just let the mcu crash if something is wrong.
  virtual size_t write(const void *buf, size_t nbyte) {
    if (base == nullptr) return 0;
    if (mode == FILE_READ) return 0;
    if (ofs + nbyte > (unsigned)sz) nbyte = sz - ofs;
    if (nbyte > 0) memcpy(base + ofs, buf, nbyte);
    ofs += nbyte;    
    return nbyte;
  }
  virtual int peek() { //""Returns the next character"
    int p = ofs + 1;
    if (p > sz) return -1;
    return *(base + p);
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
  virtual bool truncate(uint64_t size) {
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
    if (base == nullptr) return 0;
    return sz;
  }
  virtual void close() {
    base = nullptr;
    sz = 0;
    ofs = 0;
    mode = 0;
  }
  virtual bool isOpen() {
    return base != nullptr;
  }  
  virtual operator bool() {
    return base != nullptr;
  }
  virtual const char * name() {
    return base != nullptr ? "" : nullptr;
  }
  virtual boolean isDirectory(void) {
    return false;
  }
  virtual File openNextFile(uint8_t mode) {
    return File();
  }
  virtual void rewindDirectory(void) {
  }
private:
  friend class MemFS;
  MemFile(char *p, size_t size, uint8_t _mode) {
    base = p;
    ofs = 0;
    sz = size;    
    mode = _mode;
  }  
  char *base = nullptr;
  int ofs;
  int sz;
  uint8_t mode;
};




class MemFS : public FS
{
public:
  MemFS() {
  }  
  File open(const char *ptr, uint8_t mode) {    
    return File();
  }
  File open(char *ptr, size_t size, uint8_t mode = FILE_READ) {
    this->size = size;
    if (size > 0) return File(new MemFile(ptr, size, mode));
    return File();
  }  
  bool exists(const char *filepath) {
    return true;
  }
  bool mkdir(const char *filepath) {
    return false;
  }
  bool rename(const char *oldfilepath, const char *newfilepath) {
    return false;
  }
  bool remove(const char *filepath) {
    return false;
  }
  bool rmdir(const char *filepath) {
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

#pragma GCC diagnostic pop

#endif
