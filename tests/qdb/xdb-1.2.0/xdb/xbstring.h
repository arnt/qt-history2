#ifndef __XBSTRING_H__
#define __XBSTRING_H__

#include <xdb/xbconfig.h>
#include <stdlib.h>

class XBDLLEXPORT xbString {
public:
  xbString();
  xbString(size_t size);
  xbString(char c);
  xbString(const char *s);
  xbString(const char *s, size_t maxlen);
  xbString(const xbString &s);
  ~xbString();
  void ctor(const char *s);
  xbString &operator=(const xbString &s);
  xbString &operator=(const char *s);
  xbString &operator=(char c);
  
  bool isNull() const;
  bool isEmpty() const;
  size_t len() const;

  void resize(size_t size);  
  xbString copy() const;
  
  xbString &sprintf(const char *format, ...);
  void setNum(long num);
  
  xbString& assign(const xbString& str, size_t pos = 0, int n = -1);
  char operator[](int n) { return data[n]; }
  char get_character( int n ) { return data[n]; }
  operator const char *() const;
  xbString &operator+=(const char *s);
  xbString &operator+=(char c);
  void put_at(size_t pos, char c);

  const char *getData() const;
  const char *c_str() const;
  void toLowerCase();
  int pos(char c);
  void trim();
  bool compare(char s);
  bool compare(const char *s);
protected:
  char *data;
  size_t size;
};

bool operator==(const xbString &s1, const char *s2);
bool operator!=(const xbString &s1, const char *s2);
bool operator==(const xbString &s1, char s2);
bool operator!=(const xbString &s1, char s2);

xbString operator+(const xbString &s1, const xbString &s2);
xbString operator+(const xbString &s1, const char *s2);
xbString operator+(const char *s1, const xbString &s2);
xbString operator+(const xbString &s1, char c2);
xbString operator+(char c1, const xbString &s2);

#endif
