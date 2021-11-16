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


class MemFS;

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
    if (base == nullptr) return 0;
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

  #if defined(__IMXRT1062__)
  if ( base == nullptr) return;
  if ( mode == FILE_READ) return;
  if ( ((intptr_t) base < 0x20000000) || ((intptr_t) base + sz > 0x20000000 + 512 * 1024))
    return;
  //like arm_dcache_flush(), but improved
  uintptr_t location = (31 + (uintptr_t) base) & 0xFFFFFFE0;
  uintptr_t end_addr = ((uintptr_t) base + sz) & 0xFFFFFFE0;
	asm volatile("": : :"memory");
	asm("dsb");
	while (location < end_addr) {
		SCB_CACHE_DCCMVAC = location;
		location += 32;
	};
	asm("dsb");
	asm("isb");
  #endif

  }
  virtual size_t read(void *buf, size_t nbyte) {
    if (base == nullptr) return 0;
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
    return base != nullptr ? _name : nullptr;
  }
  virtual boolean isDirectory(void) {
    return false;
  }
  virtual File openNextFile(uint8_t mode) {
    return File();
  }
  virtual void rewindDirectory(void) {
  }
 	bool getCreateTime(DateTimeFields &tm) {
		return false;
	}
	bool getModifyTime(DateTimeFields &tm) {
		return false;
	}
	bool setCreateTime(const DateTimeFields &tm) {
		return false;
	}
	bool setModifyTime(const DateTimeFields &tm) {
		return false;
	}
private:
  friend class MemFS;
  MemFile(const char *name, char *p, size_t size, uint8_t _mode) {
    base = p;
    ofs = 0;
    sz = size;
    mode = _mode;
    strncpy(_name, name, sizeof(_name));
    _name[sizeof(_name)-1] = 0; // make sure null terminated. 
  }
  char *base = nullptr;
  int ofs;
  int sz;
  uint8_t mode;
  char _name[32]; // not full length but good enough...
};


class MemRootDir : public FileImpl
{
public:
  virtual size_t write(const void *buf, size_t nbyte) {
    return 0;
  }
  virtual int peek() { //""Returns the next character"
   return -1;
  }
  virtual int available() {
    return 0;
  }
  virtual void flush() {
  }

  virtual size_t read(void *buf, size_t nbyte) {
    return 0;
  }
  virtual bool truncate(uint64_t size) {
    return false;
  }
  virtual bool seek(uint64_t pos, int mode = SeekSet) {
    return false;
  }
  virtual uint64_t position() {
    return 0;
  }
  virtual uint64_t size() {
    return 0;
  }
  virtual void close() {
  }
  virtual bool isOpen() {
    return _pmemfs != nullptr;
  }
  virtual operator bool() {
    return _pmemfs != nullptr;
  }
  virtual const char * name() {
    return _pmemfs != nullptr ? "/" : nullptr;
  }
  virtual boolean isDirectory(void) {
    return true;
  }
  virtual File openNextFile(uint8_t mode);
  virtual void rewindDirectory(void) {
    _rewound = true;
  }
  bool getCreateTime(DateTimeFields &tm) {
    return false;
  }
  bool getModifyTime(DateTimeFields &tm) {
    return false;
  }
  bool setCreateTime(const DateTimeFields &tm) {
    return false;
  }
  bool setModifyTime(const DateTimeFields &tm) {
    return false;
  }
private:
  friend class MemFS;
  MemRootDir(MemFS *pmemfs) : _pmemfs(pmemfs) {
  }

  MemFS *_pmemfs;
  bool _rewound = true;
};



class MemFS : public FS
{
public:
  MemFS(char *buffer=nullptr, size_t size=0) : _buffer(buffer), _size(size) {
  }

  bool begin(char *buffer, size_t size) {
    _buffer = buffer;
    _size = size;
    _memory_allocated = false;
    return true;
  }
  bool begin(size_t size) {
    #if defined(__IMXRT1062__)
    char *buffer = (char*)extmem_malloc(size);
    #else
    char *buffer = (char*)malloc(size);
    #endif 
    if (buffer) return begin(buffer, size);
    return false;
  }

  File open(const char *name, uint8_t mode) {
    if (_buffer && _size) {
      if (strcmp(name, "/") == 0) return File(new MemRootDir(this));
      _file = File(new MemFile(name, _buffer, _size, mode));
      _memory_allocated = true;
      return _file;
    }
    return File(); // invalid
  }
  File open(char *ptr, size_t size, uint8_t mode = FILE_READ) {
    _size = size;
    if (size > 0) return File(new MemFile((const char *)F("mem"),ptr, size, mode));
    return File();
  }
  File open(void *ptr, size_t size, uint8_t mode = FILE_READ) {
    return open( (char *)ptr, size, mode);
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
    return _memory_allocated? _size : 0;
  }
  uint64_t totalSize() {
    return _size;
  }

protected:
  friend class MemRootDir;
  char *_buffer = nullptr;
  bool _memory_allocated = true;
  size_t _size;
  File _file;
};

// some method implementations have to come after all of the defines
File MemRootDir::openNextFile(uint8_t mode) {
  if (_rewound && _pmemfs) {
    _rewound = false; // only return once...
    return _pmemfs->_file;
  }
  return File();
}


#pragma GCC diagnostic pop

#endif
