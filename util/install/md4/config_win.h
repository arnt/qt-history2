/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** Licensees holding valid Qt Palmtop Developer license may use this 
** file in accordance with the Qt Palmtop Developer License Agreement 
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
** PURPOSE.
**
** email sales@trolltech.com for information about Qt Palmtop License 
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader 2.13.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
/* #define STDC_HEADERS 1 */

/* Define this if your sockaddr structure contains sin_len */
/* #undef HAVE_SOCK_SIN_LEN */

/* How many bits would you like to have in an off_t? */
#define _FILE_OFFSET_BITS 64

/* Define to include GNU C library extensions. */
#define _GNU_SOURCE 1

/* Define to a replacement type if intmax_t is not a builtin, or in
   sys/types.h or stdlib.h or stddef.h */
/* #undef intmax_t */

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a off_t.  */
#define SIZEOF_OFF_T 8

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a size_t.  */
#define SIZEOF_SIZE_T 4

/* The number of bytes in a unsigned char.  */
#define SIZEOF_UNSIGNED_CHAR 1

/* The number of bytes in a unsigned int.  */
#define SIZEOF_UNSIGNED_INT 4

/* The number of bytes in a unsigned long.  */
#define SIZEOF_UNSIGNED_LONG 4

/* The number of bytes in a unsigned short.  */
#define SIZEOF_UNSIGNED_SHORT 2

/* Define if you have the mtrace function.  */
#define HAVE_MTRACE 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the <alloca.h> header file.  */
#define HAVE_ALLOCA_H 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <libintl.h> header file.  */
#define HAVE_LIBINTL_H 1

/* Define if you have the <mcheck.h> header file.  */
#define HAVE_MCHECK_H 1

/* Define if you have the <stdint.h> header file.  */
/* #define HAVE_STDINT_H 1 */

/* Define if you have the popt library (-lpopt).  */
#define HAVE_LIBPOPT 1

/* Name of package */
#define PACKAGE "librsync"

/* Version number of package */
#define VERSION "0.9.3"

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to make ftello visible on some hosts (e.g. HP-UX 10.20). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to make ftello visible on some hosts (e.g. glibc 2.1.3). */
/* #undef _XOPEN_SOURCE */

#define OS_WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define write _write
#define STDERR_FILENO stderr
