#include <xdb/xbase.h>
#include <xdb/xbconfig.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <xdb/xbstring.h>

xbString::xbString() {
  ctor(NULL);
}

xbString::xbString(size_t size) {
  data = (char *)malloc(size);
  this->size = size;
}

xbString::xbString(char c) {
  ctor(NULL);
  *this = c;
}

xbString::xbString(const char *s) {
  ctor(s);
}

xbString::xbString(const xbString &s) {
  ctor((const char *)s);
}

xbString::xbString(const char *s, size_t maxlen) {
  size_t len = strlen(s);
  
  if (maxlen<len)
    maxlen = len;

  size = maxlen+1;
  data = (char *)malloc(size);
  strncpy(data, s, maxlen);
	data[maxlen] = 0;
}

xbString::~xbString() {
  if (data != NULL)
    free(data);
}

void xbString::ctor(const char *s) {
  if (s == NULL) {
    data = NULL;
    size =0;
    return;
  }

  size = strlen(s)+1;

  data = (char *)malloc(size);
  strcpy(data, s);
}

xbString &xbString::operator=(char c) {
  if (data != NULL)
    free(data);

  data = (char *)malloc(2);
  data[0] = c;
  data[1] = 0;

  size = 2;

  return (*this);
}

xbString &xbString::operator=(const xbString &s) {
  if (data != NULL)
    free(data);

  const char *sd = s;
  if (sd == NULL) {
    data = NULL;
    size = 0;
    return (*this);
  }

  data = (char *)malloc(strlen(s)+1);
  strcpy(data, s);

  size = strlen(data)+1;
  
  return (*this);
}

xbString &xbString::operator=(const char *s) {
  if (data != NULL)
    free(data);

	if (s == NULL) {
		data = NULL;
		size = 0;
		return (*this);
	}

  data = (char *)malloc(strlen(s)+1);
  strcpy(data, s);

  size = strlen(data)+1;
  
  return (*this);
}

void xbString::resize(size_t size) {
  data = (char *)realloc(data, size);
  if (size>0)
    data[size-1] = 0;
  this->size = size;
}
     
bool xbString::isNull() const {
  return (data == NULL);
}

bool xbString::isEmpty() const {
  if (data == NULL)
    return true;
  if (data[0] == 0)
    return true;
  return false;
}

size_t xbString::len() const {
  return ((data == NULL)?0:strlen(data));
}

xbString xbString::copy() const {
  return (*this);
}

xbString &xbString::sprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  if (size < 256)
    resize(256);              // make string big enough

#ifdef HAVE_VSPRINTF
  vsprintf(data, format, ap);
#elif HAVE_VSNPRINTF
  if (vsnprintf(data, size, format, ap) == -1)
    data[size-1] = 0;
#else
#  error "You have neither vsprintf nor vsnprintf!!!"
#endif

  resize(strlen(data)+1);               // truncate
  va_end(ap);
  return (*this);
}

xbString::operator const char *() const {
  return data;
}

xbString &xbString::operator+=(const char *s) {
  if (s == NULL)
    return (*this);
  int len = strlen(s);
  int oldlen = this->len();

  data = (char *)realloc(data, oldlen+len+1);
	if (oldlen == 0)
		data[0] = 0;
  strcat(data, s);

  size += len;
  return (*this);
}

xbString &xbString::operator+=(char c) {
  int len = 1;
  int oldlen = this->len();

  data = (char *)realloc(data, oldlen+len+1);
  data[oldlen] = c;
  data[oldlen+1] = 0;

  size++;

  return (*this);
}

const char *xbString::getData() const {
  return data;
}

void xbString::toLowerCase() {
  int len = this->len();
  for (int i=0;i<len;i++)
    data[i] = (char)tolower(data[i]);
}

const char *xbString::c_str() const {
  return data;
}

int xbString::pos(char c) {
  if (data == NULL)
    return (-1);

  const char *p = strchr(data, c);

  if (p == NULL)
    return (-1);

  return p-data;
}

void xbString::setNum(long num) {
  sprintf("%ld", num);
}

bool operator==(const xbString &s1, const char *s2) {
  if (s2 == NULL) {
    if (s1.getData() == NULL)
      return true;
    return false;
  }

	if ((s2[0] == 0) && s1.getData() == NULL)
		return true;

  if (s1.getData() == NULL)
    return false;

  return (strcmp(s1, s2) == 0);
}

bool operator!=(const xbString &s1, const char *s2) {
  if (s2 == NULL) {
    if (s1.getData() == NULL)
      return false;
    return true;
  }

	if ((s2[0] == 0) && s1.getData() == NULL)
		return false;

  if (s1.getData() == NULL)
    return true;

  return (strcmp(s1, s2) != 0);
}

xbString operator+(const xbString &s1, const xbString &s2) {
	xbString tmp(s1.getData());
	tmp += s2;
	return tmp;
}

xbString operator+(const xbString &s1, const char *s2) {
	xbString tmp(s1.getData());
	tmp += s2;
	return tmp;
}

xbString operator+(const char *s1, const xbString &s2) {
	xbString tmp(s1);
	tmp += s2;
	return tmp;
}

xbString operator+(const xbString &s1, char c2) {
	xbString tmp(s1.getData());
	tmp += c2;
	return tmp;
}

xbString operator+(char c1, const xbString &s2) {
	xbString tmp(c1);
	tmp += s2;
	return tmp;
}

void xbString::put_at(size_t pos, char c) {
	if (pos>len())
		return;

	data[pos] = c;
}

xbString& xbString::assign(const xbString& str, size_t pos, int n) {
	if (data != NULL)
		free(data);

	if (str.len() <= pos) {
		size = 0;
		return (*this);
	}

	if (str.len() < pos+n) {
		n = str.len()-pos;
	}

	const char *d = str;
		
	if (n == -1) {
		data = (char *)malloc(str.len()-pos+1);
		strcpy(data, d+pos);
		size = str.len()-pos+1;
	} else {
		data = (char *)malloc(n);
		strncpy(data, d+pos, n);
		size = n+1;
	}

	return (*this);
}

void xbString::trim() {
  int l = len()-1;

	for (;;) {
		if (data[l] != ' ')
			break;
		data[l] = 0;
		if (l == 0)
			break;
		l--;
	}
}
