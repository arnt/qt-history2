#ifndef __XBEXCEPT_H__
#define __XBEXCEPT_H__

#include <xdb/xbconfig.h>
#include <xdb/xtypes.h>

const char *xbStrError(xbShort err);

#ifndef HAVE_EXCEPTIONS
#define xb_error(code) { return code;}
#define xb_io_error(code,name) { return code;}
#define xb_open_error(name) { return XB_OPEN_ERROR;}
#define xb_memory_error { return XB_NO_MEMORY;}
#define xb_eof_error { return XB_EOF;}
#else

#ifdef HAVE_EXCEPTION

#include <exception>
#elif HAVE_G___EXCEPTION_H
#include <g++/exception.h>
#elif
#error "Exceptions are unsupported on your system."
#endif

#ifdef __BORLANDC__
#define XB_THROW throw ()
using std::exception;
#else
#define XB_THROW
#endif

/* FIXME:	class exception is member of <stdexcept.h> -- willy */
class XBDLLEXPORT xbException : public exception {
public:
  xbException (int err);
  virtual ~xbException () XB_THROW;
  virtual const char* what() const XB_THROW;
  virtual const char *error();
  int getErr();
private:
  int err;
};

#define xb_error(code) {throw xbException(code);return (code);}

class XBDLLEXPORT xbIOException : public xbException {
public:
  xbIOException (int err);
  xbIOException (int err, const char *n);
  virtual ~xbIOException () XB_THROW;
  virtual const char* what() const XB_THROW;
  const char *_errno() const;
  const char *name;
protected:
  int m_errno;
};

#define xb_io_error(code, name) {throw xbIOException(code,name);return (code);}

class XBDLLEXPORT xbOpenException : public xbIOException {
public:
  xbOpenException ();
  xbOpenException (const char *n);
  virtual ~xbOpenException () XB_THROW;
  virtual const char* what() const XB_THROW;
};

#define xb_open_error(name) { throw xbOpenException(name); return 0;}

class XBDLLEXPORT xbOutOfMemoryException : public xbException {
public:
  xbOutOfMemoryException ();
  virtual ~xbOutOfMemoryException () XB_THROW;
  virtual const char* what() const XB_THROW;
};

#define xb_memory_error {throw xbOutOfMemoryException();return 0;}

class XBDLLEXPORT xbEoFException : public xbIOException {
public:
  xbEoFException ();
  virtual ~xbEoFException () XB_THROW;
  virtual const char* what() const XB_THROW;
};

#define xb_eof_error {throw xbEoFException();return 0;}

#endif

#endif // __XBEXCEPT_H__
