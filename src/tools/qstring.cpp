/****************************************************************************
**
** Implementation of QString class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Don't define it while compiling this module, or USERS of Qt will
// not be able to link.
#ifdef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_FROM_ASCII
#endif
#ifdef QT_NO_CAST_TO_ASCII
#undef QT_NO_CAST_TO_ASCII
#endif

#include "qstring.h"
#include "qregexp.h"
#include "qunicodetables_p.h"
#ifndef QT_NO_TEXTCODEC
#include <qtextcodec.h>
#endif
#include <qdatastream.h>
#include <qlist.h>

#include "qlocale.h"
#include "qlocale_p.h"

#include "qtools_p.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef LLONG_MAX
#define LLONG_MAX Q_INT64_C(9223372036854775807)
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - Q_INT64_C(1))
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX Q_UINT64_C(18446744073709551615)
#endif

static int ucstrcmp(const QString &as, const QString &bs)
{
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    if (a == b)
	return 0;
    if (a == 0)
	return 1;
    if (b == 0)
	return -1;
    int l=qMin(as.length(),bs.length());
    while (l-- && *a == *b)
	a++,b++;
    if (l==-1)
	return (as.length()-bs.length());
    return a->unicode() - b->unicode();
}

static int ucstrncmp(const QChar *a, const QChar *b, int l)
{
    while (l-- && *a == *b)
	a++,b++;
    if (l==-1)
	return 0;
    return a->unicode() - b->unicode();
}

static int ucstrnicmp(const QChar *a, const QChar *b, int l)
{
    while (l-- && ::lower(*a) == ::lower(*b))
	a++,b++;
    if (l==-1)
	return 0;
    return ::lower(*a).unicode() - ::lower(*b).unicode();
}


static bool qIsUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static bool qIsDigit(char c)
{
    return c >= '0' && c <= '9';
}

static char qToLower(char c)
{
    if (c >= 'A' && c <= 'Z')
    	return c - 'A' + 'a';
    else
    	return c;
}


const QString::Null QString::null=QString::Null();


/*!
    \class QCharRef qstring.h
    \reentrant
    \brief The QCharRef class is a helper class for QString.

    \ingroup text

    When you get an object of type QCharRef, if you can assign to it,
    the assignment will apply to the character in the string from
    which you got the reference. That is its whole purpose in life.
    The QCharRef becomes invalid once modifications are made to the
    string: if you want to keep the character, copy it into a QChar.

    Most of the QChar member functions also exist in QCharRef.
    However, they are not explicitly documented here.

    \sa QString::operator[]() QString::at() QChar
*/


/*!
    \class QString qstring.h
    \reentrant

    \brief The QString class provides an abstraction of Unicode text
    and the classic C '\0'-terminated char array.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass

    QString uses \link shclass.html implicit sharing\endlink, which
    makes it very efficient and easy to use.

    In all of the QString methods that take \c {const char *}
    parameters, the \c {const char *} is interpreted as a classic
    C-style '\0'-terminated ASCII string. It is legal for the \c
    {const char *} parameter to be 0. If the \c {const char *} is not
    '\0'-terminated, the results are undefined. If you need to pass a
    QString to a function that requires a C '\0'-terminated string use
    ascii().

    Lists of strings are handled by the QStringList class. You can
    split a string into a list of strings using QStringList::split(),
    and join a list of strings into a single string with an optional
    separator using QStringList::join(). You can obtain a list of
    strings from a string list that contain a particular substring or
    that match a particular \link qregexp.html regex\endlink using
    QStringList::grep().

    <b>Note for C programmers</b>

    Due to C++'s type system and the fact that QString is implicitly
    shared, QStrings may be treated like ints or other simple base
    types. For example:

    \code
    QString boolToString(bool b)
    {
	QString result;
	if (b)
	    result = "True";
	else
	    result = "False";
	return result;
    }
    \endcode

    The variable, result, is an auto variable allocated on the stack.
    When return is called, because we're returning by value, The copy
    constructor is called and a copy of the string is returned. (No
    actual copying takes place thanks to the implicit sharing, see
    below.)

    Throughout Qt's source code you will encounter QString usages like
    this:
    \code
    QString func(const QString& input)
    {
	QString output = input;
	// process output
	return output;
    }
    \endcode

    The 'copying' of input to output is almost as fast as copying a
    pointer because behind the scenes copying is achieved by
    incrementing a reference count. QString (like all Qt's implicitly
    shared classes) operates on a copy-on-write basis, only copying if
    an instance is actually changed.

    If you wish to create a deep copy of a QString without losing any
    Unicode information then you should use QDeepCopy.

    \sa QChar QByteArray
*/

/*! \enum Qt::ComparisonFlags
\internal
*/
/*!
    \enum Qt::StringComparisonMode

    This enum type is used to set the string comparison mode when
    searching for an item. It is used by QListBox, QListView and
    QIconView, for example. We'll refer to the string being searched
    as the 'target' string.

    \value CaseSensitive The strings must match case sensitively.
    \value ExactMatch The target and search strings must match exactly.
    \value BeginsWith The target string begins with the search string.
    \value EndsWith The target string ends with the search string.
    \value Contains The target string contains the search string.

    If you OR these flags together (excluding \c CaseSensitive), the
    search criteria be applied in the following order: \c ExactMatch,
    \c BeginsWith, \c EndsWith, \c Contains.

    Matching is case-insensitive unless \c CaseSensitive is set. \c
    CaseSensitive may be OR-ed with any combination of the other
    flags.

*/
QString::Data QString::shared_null = { Q_ATOMIC_INIT(1), 0, 0, 0, shared_null.array, 0, 0, 0, 0, 0, 0, {0} };
QString::Data QString::shared_empty = { Q_ATOMIC_INIT(1), 0, 0, 0, shared_empty.array, 0, 0, 0, 0, 0, 0, {0} };

inline int QString::grow (int size)
{
    return qAllocMore(size * sizeof(QChar), sizeof(Data)) / sizeof(QChar);
}

/*!
    \fn QString::QString()

    Constructs an empty string
    \sa isEmpty()
*/

/*!
    Constructs a string that is a deep copy of \a s, interpreted as
    a classic C string.

    If \a s is 0, then an empty string is created.

    This is a cast constructor, but it is perfectly safe: converting a
    Latin1 \c{const char *} to QString preserves all the
    information. You can disable this constructor by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You can
    also make QString objects by using fromAscii(), fromLatin1(),
    fromLocal8Bit(), and fromUtf8(). Or whatever encoding is
    appropriate for the 8-bit data you have.
*/
QString::QString(const char *s)
{
    if (!s) {
	d = &shared_null;
	++d->ref;
    } else if (!*s) {
	d = &shared_empty;
	++d->ref;
#ifndef QT_NO_TEXTCODEC
    } else if (QTextCodec::codecForCStrings()) {
	d = &shared_null;
	++d->ref;
	*this = fromAscii(s);
#endif
    } else {
	int len = strlen(s);
	d = (Data*) qMalloc(sizeof(Data)+len*sizeof(QChar));
	d->ref = 1;
	d->alloc = d->size = len;
	d->c = 0;
	d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
	d->data = d->array;
	ushort *i = d->array;
	while ((*i++ = (uchar) *s++))
	    ;
    }
}

/*! \fn QString::QString(const QString &s) :

Constructs an implicitly shared copy of \a s. This is very fast
    since it only involves incrementing a reference count.
*/

/*!
    Constructs a string that is a deep copy of the first \a size
    characters in the QChar array \a unicode.

    If \a unicode is 0, then an empty string is created.
*/
QString::QString(const QChar *unicode, int size)
{
   if (!unicode) {
	d = &shared_null;
	++d->ref;
    } else if (size <= 0) {
	d = &shared_empty;
	++d->ref;
    } else {
	d = (Data*) qMalloc(sizeof(Data)+size*sizeof(QChar));
	d->ref = 1;
	d->alloc = d->size = size;
	d->c = 0;
	d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
	d->data = d->array;
	memcpy(d->array, unicode, size*sizeof(QChar));
	d->array[size] = '\0';
    }
}


/*!
    Creates a string with \a size characters of value \a c
*/
QString::QString(int size, QChar c)
{
   if (size <= 0) {
	d = &shared_empty;
	++d->ref;
    } else {
	d = (Data*) qMalloc(sizeof(Data)+size*sizeof(QChar));
	d->ref = 1;
	d->alloc = d->size = size;
	d->c = 0;
	d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
	d->data = d->array;
	d->array[size] = '\0';
	QChar *i = (QChar*)d->array + size;
	QChar *b = (QChar*)d->array;
	while (i != b)
	   * --i = c;
    }
}

/*!
    Constructs a string of length one, containing the character \a c.
*/
QString::QString(QChar c)
{
    d = (Data*) qMalloc(sizeof(Data)+sizeof(QChar));
    d->ref = 1;
    d->alloc = d->size = 1;
    d->c = 0;
    d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
    d->data = d->array;
    d->array[0] = c.unicode();
    d->array[1] = '\0';
}

void QString::free(Data *d)
{
    if (d->c)
	((QByteArray*)&d->c)->~QByteArray();
    qFree(d);
}

/*!
    Ensures that at least \a size characters are allocated to the
    string, and sets the length of the string to \a size. Any new
    space allocated contains arbitrary data.

    \sa reserve(), truncate()
*/

void QString::resize(int size)
{
    d->cache = 0;
    if (size <= 0) {
	Data *x = &shared_empty;
	++x->ref;
	x = qAtomicSetPtr( &d, x );
	if (!--x->ref)
	    free(x);
    } else {
	if (d->ref != 1 || size > d->alloc || (size < d->size && size < d->alloc >> 1))
	    realloc(grow(size));
	d->array[size] = '\0';
	d->size = size;
    }
}


/*!
    Ensures that at least \a size characters are allocated to the
    string.

    This function is useful for code that needs to build up a long
    string and wants to avoid repeated reallocation. In this example,
    we want to add to the string until some condition is true, and
    we're fairly sure that size is big enough:
    \code
	QString result;
	int len = 0;
	result.reserve(maxSize);
	while (...) {
	    result[len++] = ...         // fill part of the space
	}
	result.squeeze();
    \endcode

    If \e maxSize is an underestimate, the worst that will happen is that
    the loop will slow down.

    If it is not possible to allocate enough memory, the string
    remains unchanged.

    \sa capacity(), squeeze(), resize()
*/

void QString::reserve(int size)
{
    if (d->ref != 1 || size > d->alloc)
	realloc(size);
}

/*!
    Squeezes the string's capacity to the current content.

    \sa capacity(), reserve()
*/
void QString::squeeze()
{
    realloc(d->size);
}

void QString::realloc(int alloc)
{
    if (d->ref == 1 ) {
	d = (Data*) qRealloc(d, sizeof(Data)+alloc*sizeof(QChar));
	d->alloc = alloc;
	d->data = d->array;
    } else {
	Data *x = (Data*) qMalloc(sizeof(Data)+alloc*sizeof(QChar));
	*x = *d;
	x->data = x->array;
	::memcpy(x->data, d->data, qMin(alloc, d->alloc)*sizeof(QChar));
	x->c = 0;
	x->cache = 0;
	x->ref = 1;
	x->alloc = alloc;
	x = qAtomicSetPtr( &d, x );
	if (!--x->ref)
	    free(x);
    }
}

void QString::realloc()
{
    realloc(d->size);
}

void QString::expand(int i)
{
    int sz = d->size;
    resize(qMax(i+1, d->size));
    if (d->size - 1 > sz) {
	ushort *n = d->data + d->size - 1;
	ushort *e = d->data + sz;
	while (n != e)
	   * --n = ' ';
    }
}

/*!
    Sets the string to contain just the single character \a c.
*/
QString &QString::operator=(QChar c)
{
    return operator=(QString(c));
}

/*!
    \fn QString& QString::operator=(const std::string &s)

    \overload

    Makes a deep copy of \a s and returns a reference to the deep
    copy.
*/

/*!
    \fn QString &QString::operator=(const QString &s)
    \overload

    Assigns a shallow copy of \a s to this string and returns a
    reference to this string. This is very fast because the string
    isn't actually copied.
*/

/*!
    \overload

    Makes a deep copy of \a s and returns a reference to the resultant
    QString.
*/
QString &QString::operator=(const char *s)
{
    Data *x;
    if (!s) {
	x = &shared_null;
    } else if (!*s) {
	x = &shared_empty;
#ifndef QT_NO_TEXTCODEC
    } else if (QTextCodec::codecForCStrings()) {
	   return (*this = fromAscii(s));
#endif
    } else {
	int len = strlen(s);
	if (d->ref != 1 || len > d->alloc || (len < d->size && len < d->alloc >> 1))
	    realloc(len);
	x = d;
	x->alloc = len;
	x->size = len;
	x->c = 0;
	x->clean = x->encoding = x->cache = x->simpletext = x->righttoleft = 0;
	ushort *i = x->data;
	while ((*i++ = (uchar)*s++))
	    ;
    }
    ++x->ref;
    x = qAtomicSetPtr( &d, x );
    if (!--x->ref)
	free(x);
    return *this;
}

/*!
    \fn bool QString::operator!() const

    Returns true if the string is empty; otherwise returns false.

    \sa isEmpty()
*/

/*!
    Inserts \a c into the string at position \a i and returns a
    reference to the string.

    If \a i is beyond the end of the string, the string is extended
    with spaces (ASCII 32) to length \a i and \a c is then
    appended.
*/

QString& QString::insert(int i, QChar c)
{
    d->cache = 0;
    if (i < 0)
	i += d->size;
    if (i < 0)
	return *this;
   expand(qMax(i, d->size));
   ::memmove(d->data + i + 1, d->data + i, (d->size - i)*sizeof(QChar));
   d->data[i] = c.unicode();
   return *this;
}

/*! \fn QString& QString::insert(int i, const QString& s)

    \overload

    Inserts \a s into the string at position \a i and returns a
    reference to the string.

    If \a i is beyond the end of the string, the string is extended
    with spaces (ASCII 32) to length \a i and \a s is then
    appended.

    \code
	QString s("I like fish");
	s.insert(2, "don't ");
	// s == "I don't like fish"
    \endcode

*/

QString& QString::insert(int i, const QChar *str, int len)
{
    if (i < 0 || len <= 0)
	return *this;
    d->cache = 0;

    if (i > d->size)
	i = d->size;

    unsigned short *s = (unsigned short *)str;

    if ( s >= d->data && (s - d->data) < d->alloc ) {
	// Part of me - take a copy.
	unsigned short *tmp = (unsigned short *)malloc(len*sizeof(QChar));
	memcpy(tmp, s, len*sizeof(QChar));
	insert(i, (const QChar *)tmp, len);
	::free(tmp);
	return *this;
    }

    expand(qMax(d->size, i) + len - 1);
    ::memmove(d->data + i + len, d->data + i, (d->size - i - len)*sizeof(QChar));
    memcpy(d->data + i, s, len*sizeof(QChar));
    return *this;
}

/*!
    Appends \a c to the string and returns a reference to the string.
*/
QString& QString::append(QChar c)
{
    d->cache = 0;
    if (d->ref != 1 || d->size + 1 > d->alloc)
	realloc(grow(d->size + 1));
    d->data[d->size++] = c.unicode();
    d->data[d->size] = '\0';
    return *this;
}

/*! \overload
    Appends \a s to the string and returns a reference to the string.
*/
QString& QString::append(const char *s)
{
    if (s) {
#ifndef QT_NO_TEXTCODEC
	if (QTextCodec::codecForCStrings())
	    return append(QString(s));
#endif
	d->cache = 0;
	int len = strlen(s);
	if (d->ref != 1 || d->size + len > d->alloc)
	    realloc(grow(d->size + len));
	ushort *i = d->data + d->size;
	while ((*i++ = (uchar)*s++))
	    ;
	d->size += len;
    }
    return *this;
}

/*! \overload
    Appends \a s to the string and returns a reference to the string.
*/
QString& QString::append(const QString& s)
{
    if (s.d != &shared_null) {
	d->cache = 0;
	if (d->ref != 1 || d->size + s.d->size > d->alloc)
	    realloc(grow(d->size + s.d->size));
	memcpy(d->data + d->size, s.d->data, (s.d->size+1)*sizeof(QChar)); // include null terminator
	d->size += s.d->size;
    }
    return *this;
}

/*!
    Prepends \a c to the string and returns a reference to the string.
*/
QString& QString::prepend(QChar c) { return insert(0, c); }

/*! \overload
    Prependsd \a s to the string and returns a reference to the string.
*/
QString& QString::prepend(const QString& s) { return insert(0, s); }

/*! \overload
    Prepends \a s to the string and returns a reference to the string.
*/
QString& QString::prepend(const char *s) { return insert(0, s); }


/*!
    Removes \a len characters from the string starting at position \a
    i, and returns a reference to the string.

    If \a i is beyond the length of the string, nothing happens.  If
    \a i is within the string, but \a i + \a len is beyond the end of
    the string, the string is truncated at position \a i.

    \code
	QString string("Montreal");
	string.remove(1, 4);      // string == "Meal"
    \endcode

    \sa insert(), replace()
*/

QString &QString::remove(int i, int len)
{
    d->cache = 0;
    if (i < 0)
	i += d->size;
    if (i < 0 || i >= d->size) {
	// range problems
    } else if (i + len >= d->size) {  // index ok
	resize(i);
    } else if (len != 0) {
	detach();
	memmove(d->data+i, d->data+i+len, (d->size-i-len)*sizeof(unsigned short));  // include null terminator
	d->size -= len;
    }
    return *this;
}

/*! \overload

    Removes every occurrence of the character \a c in the string.
    Returns a reference to the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    This is the same as replace(\a c, ''", \a cs).
*/
QString &QString::remove(QChar c, QString::CaseSensitivity cs)
{
    d->cache = 0;
    int i = 0;
    if (cs == CaseSensitive) {
	while (i < d->size)
	    if (*((const QChar*)d->data + i) == c)
		remove(i, 1);
	    else
		i++;
    } else {
	c = ::lower(c);
	while (i < d->size)
	    if (::lower(*((const QChar*)d->data + i)) == c)
		remove(i, 1);
	    else
		i++;
    }
    return *this;
}

/*! \overload

    Removes every occurrence of \a s in the string. Returns a
    reference to the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    This is the same as replace(\a s, "", \a cs).
*/
QString& QString::remove(const QString & s, QString::CaseSensitivity cs)
{
    d->cache = 0;
    if (s.d->size) {
	int i = 0;
	while ((i = indexOf(s, i, cs)) != -1)
	    remove(i, s.d->size);
    }
    return *this;
}

/*!
    \overload

    This is the same as replace(\a i, \a len, QString(\a after)).
*/
QString& QString::replace(int i, int len, QChar after)
{
    remove(i, len);
    return insert(i, after);
}

/*!
    Replaces \a len characters from the string with \a after, starting at
    position \a i, and returns a reference to the string.

    If \a i or \a i + the length of \a after is beyond the length of
    the string, the string is expanded accordingly.

    \code
	QString string("Say yes!");
	string = string.replace(4, 3, "NO");
	// string == "Say NO!"
    \endcode

    \sa insert(), remove()
*/
QString& QString::replace(int i, int len, const QChar *s, int slen)
{
    remove(i, len);
    return insert(i, s, slen);
}

/*! \overload
    Replaces every occurrence of the character \a c in the string
    with \a after. Returns a reference to the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.
*/

QString& QString::replace(QChar c, const QString & after, QString::CaseSensitivity cs)
{
    return replace(QString(c), after, cs);
}

/*! \overload

    Replaces every occurrence of \a before with the character \a
    after. Returns a reference to the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.
*/
QString& QString::replace(QChar before, QChar after, QString::CaseSensitivity cs)
{
    d->cache = 0;
    if (d->size) {
	QChar *i = data();
	QChar *e = i + d->size;
	if (cs == CaseSensitive) {
	    for (; i != e; ++i)
		if (*i == before)
		   * i = after;
	} else {
	    before = ::lower(before);
	    for (; i != e; ++i)
		if (::lower(*i) == before)
		   * i = after;
	}
    }
    return *this;
}

bool operator==(const QString &s1, const QString &s2)
{
    return (s1.size() == s2.size()) &&
	(memcmp((char*)s1.unicode(),(char*)s2.unicode(),
		s1.size()*sizeof(QChar))==0);
}

/*!
    Finds the first occurrence of the character \a c, starting at
    position \a i. If \a i is -1, the search starts at the last
    character; if -2, at the next to last character and so on.  (See
    findRev() for searching backwards.)

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    Returns the position of \a c or -1 if \a c could not be found.
*/
int QString::indexOf(QChar c, int from, QString::CaseSensitivity cs) const
{
    if (from < 0)
	from = qMax(from + d->size, 0);
    if (from  < d->size) {
	const QChar *n = (const QChar*)d->data + from - 1;
	const QChar *e = (const QChar*)d->data + d->size;
	if (cs == CaseSensitive) {
	    while (++n != e)
		if (*n == c)
		    return  n - (const QChar*)d->data;
	} else {
	    c = ::lower(c);
	    while (++n != e)
		if (::lower(*n) == c)
		    return  n - (const QChar*)d->data;
	}
    }
    return -1;
}

/*!

    Finds the first occurrence of the character \a c, starting at
    position \a i and searching backwards. If the i is -1, the
    search starts at the last character, if it is -2, at the next to
    last character and so on.

    Returns the position of \a c or -1 if \a c could not be found.

    If \a cs is CaseSensitivite (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \code
	QString string("bananas");
	int i = string.findRev('a');      // i == 5
    \endcode
*/
int QString::lastIndexOf(QChar c, int from, QString::CaseSensitivity cs) const
{
    if (from < 0)
	from += d->size;
    else if (from > d->size)
	from = d->size-1;
    if (from >= 0) {
	const QChar *n =  (const QChar*)d->data + from;
	const QChar *b = (const QChar*)d->data;
	if (cs == CaseSensitive) {
	    for (; n >= b; --n)
		if (*n == c)
		    return  n - b;
	} else {
	    c = ::lower(c);
	    for (; n >= b; --n)
		if (::lower(*n) == c)
		    return  n - b;
	}
    }
    return -1;
}

/* an implementation of the Boyer-Moore search algorithm
*/

/* initializes the skiptable to know how far ahead we can skip on a wrong match
*/
static void bm_init_skiptable(const QChar *uc, int l, uint *skiptable, QString::CaseSensitivity cs)
{
    int i = 0;
    register uint *st = skiptable;
    while (i++ < 0x100/8) {
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
	*(st++) = l;
    }
    if (cs == QString::CaseSensitive) {
	while (l--) {
	    skiptable[uc->cell()] = l;
	    uc++;
	}
    } else {
	while (l--) {
	    skiptable[::lower(*uc).cell()] = l;
	    uc++;
	}
    }
}

static int bm_find(const QChar *uc, uint l, int index, const QChar *puc, uint pl, uint *skiptable, QString::CaseSensitivity cs)
{
    if (pl == 0)
	return index > (int)l ? -1 : index;
    const uint pl_minus_one = pl - 1;

    register const QChar *current = uc + index + pl_minus_one;
    const QChar *end = uc + l;
    if (cs == QString::CaseSensitive) {
	while (current < end) {
	    uint skip = skiptable[current->cell()];
	    if (!skip) {
		// possible match
		while (skip < pl) {
		    if (*(current - skip) != puc[pl_minus_one-skip])
			break;
		    skip++;
		}
		if (skip > pl_minus_one) { // we have a match
		    return (current - uc) - skip + 1;
		}
		// in case we don't have a match we are a bit inefficient as we only skip by one
		// when we have the non matching char in the string.
		if (skiptable[(current-skip)->cell()] == pl)
		    skip = pl - skip;
		else
		    skip = 1;
	    }
	    current += skip;
	}
    } else {
	while (current < end) {
	    uint skip = skiptable[::lower(*current).cell()];
	    if (!skip) {
		// possible match
		while (skip < pl) {
		    if (::lower(*(current - skip)) != ::lower(puc[pl_minus_one-skip]))
			break;
		    skip++;
		}
		if (skip > pl_minus_one) // we have a match
		    return (current - uc) - skip + 1;
		// in case we don't have a match we are a bit inefficient as we only skip by one
		// when we have the non matching char in the string.
		if (skiptable[::lower(*(current - skip)).cell()] == pl)
		    skip = pl - skip;
		else
		    skip = 1;
	    }
	    current += skip;
	}
    }
    // not found
    return -1;
}


#define REHASH(a) \
    if (sl_minus_1 < sizeof(uint) * CHAR_BIT) \
	hashHaystack -= (a) << sl_minus_1; \
    hashHaystack <<= 1

/*!
    \overload

    Finds the first occurrence of the string \a s, starting at
    position \a i. If \a i is -1, the search starts at the last
    character, if it is -2, at the next to last character and so
    on. (See findRev() for searching backwards.)

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    Returns the position of \a str or -1 if \a s could not be found.
*/

int QString::indexOf(const QString& s, int from, QString::CaseSensitivity cs) const
{
    const uint l = d->size;
    const uint sl = s.d->size;
    if (from < 0)
	from += l;
    if (sl + from > l)
	return -1;
    if (!sl)
	return from;
    if (!l)
	return -1;

    if (sl == 1)
	return indexOf(*(const QChar*) s.d->data, from, cs);

    // we use the Boyer-Moore algorithm in cases where the overhead
    // for the hash table should pay off, otherwise we use a simple
    // hash function
    if (l > 500 && sl > 5) {
	uint skiptable[0x100];
	bm_init_skiptable((const QChar*)s.d->data , sl, skiptable, cs);
	return bm_find((const QChar*) d->data, l, from, (const QChar*)s.d->data , sl ,skiptable, cs);
    }

    /*
      We use some hashing for efficiency's sake. Instead of
      comparing strings, we compare the hash value of s with that of
      a part of this QString. Only if that matches, we call ucstrncmp
      or ucstrnicmp.
   */
    const QChar *needle = (const QChar*) s.d->data;
    const QChar *haystack = (const QChar*) d->data + from;
    const QChar *end = (const QChar*) d->data + (l-sl);
    const uint sl_minus_1 = sl-1;
    uint hashNeedle = 0, hashHaystack = 0, idx;

    if (cs == CaseSensitive) {
	for (idx = 0; idx < sl; ++idx) {
	    hashNeedle = ((hashNeedle<<1) + needle[idx].unicode());
	    hashHaystack = ((hashHaystack<<1) + haystack[idx].unicode());
	}
	hashHaystack -= (haystack+sl_minus_1)->unicode();

	while (haystack <= end) {
	    hashHaystack += (haystack+sl_minus_1)->unicode();
 	    if (hashHaystack == hashNeedle
		 && ucstrncmp(needle, haystack, sl) == 0)
		return haystack-unicode();

	    REHASH(haystack->unicode());
	    ++haystack;
	}
    } else {
	for (idx = 0; idx < sl; ++idx) {
	    hashNeedle = ((hashNeedle<<1) +
			  ::lower(needle[idx].unicode()).unicode());
	    hashHaystack = ((hashHaystack<<1) +
			    ::lower(haystack[idx].unicode()).unicode());
	}

	hashHaystack -= ::lower(*(haystack+sl_minus_1)).unicode();
	while (haystack <= end) {
	    hashHaystack += ::lower(*(haystack+sl_minus_1)).unicode();
	    if (hashHaystack == hashNeedle
		 && ucstrnicmp(needle, haystack, sl) == 0)
		return haystack-unicode();

	    REHASH(::lower(*haystack).unicode());
	    ++haystack;
	}
    }
    return -1;
}

/*!
    \overload

    Finds the first occurrence of the string \a s, starting at
    position \a i and searching backwards. If the index is -1, the
    search starts at the last character, if it is -2, at the next to
    last character and so on.

    Returns the position of \a s or -1 if \a s could not be found.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \code
    QString string("bananas");
    int i = string.findRev("ana");      // i == 3
    \endcode
*/
int QString::lastIndexOf(const QString& s, int from, QString::CaseSensitivity cs) const
{
    /*
      See QString::indexOf() for explanations.
   */
    const uint l = d->size;
    if (from < 0)
	from += l;
    const uint sl = s.d->size;
    int delta = l-sl;
    if (from < 0 || from > (int)l || delta < 0)
	return -1;
    if (from > delta)
	from = delta;

    if (sl == 1)
	return lastIndexOf(*(const QChar*) s.d->data, from, cs);

    const QChar *needle = (const QChar*) s.d->data;
    const QChar *haystack = (const QChar*) d->data + from;
    const QChar *end = (const QChar*) d->data;
    const uint sl_minus_1 = sl-1;
    const QChar *n = needle+sl_minus_1;
    const QChar *h = haystack+sl_minus_1;
    uint hashNeedle = 0, hashHaystack = 0, idx;

    if (cs == CaseSensitive) {
	for (idx = 0; idx < sl; ++idx) {
	    hashNeedle = ((hashNeedle<<1) + (n-idx)->unicode());
	    hashHaystack = ((hashHaystack<<1) + (h-idx)->unicode());
	}
	hashHaystack -= haystack->unicode();

	while (haystack >= end) {
	    hashHaystack += haystack->unicode();
 	    if (hashHaystack == hashNeedle
		 && ucstrncmp(needle, haystack, sl) == 0)
		return haystack-unicode();
	    --haystack;
	    REHASH((haystack+sl)->unicode());
	}
    } else {
	for (idx = 0; idx < sl; ++idx) {
	    hashNeedle = ((hashNeedle<<1)
			  + ::lower((n-idx)->unicode()).unicode());
	    hashHaystack = ((hashHaystack<<1)
			    + ::lower((h-idx)->unicode()).unicode());
	}
	hashHaystack -= ::lower(*haystack).unicode();

	while (haystack >= end) {
	    hashHaystack += ::lower(*haystack).unicode();
	    if (hashHaystack == hashNeedle
		 && ucstrnicmp(needle, haystack, sl) == 0)
		return haystack-unicode();
	    --haystack;
	    REHASH(::lower(*(haystack+sl)).unicode());
	}
    }
    return -1;
}

/*! \overload

    Replaces every occurrence of the string \a before in the string
    with the string \a after. Returns a reference to the string.

    If \a cs is CaseSensitive (the default), the search is case sensitive;
    otherwise the search is case insensitive.

    Example:
    \code
    QString s = "Greek is Greek";
    s.replace("Greek", "English");
    // s == "English is English"
    \endcode
*/
QString& QString::replace(const QString & before, const QString & after, QString::CaseSensitivity cs)
{
    if (d->size == 0) {
	if (before.d->size)
	    return *this;
    } else {
	if (cs == CaseSensitive && before == after)
	    return *this;
    }
    d->cache = 0;
    if (d->ref != 1)
	realloc(d->size);

    int index = 0;
    uint skiptable[256];
    const int bl = before.d->size;
    const int al = after.d->size;
    bm_init_skiptable((const QChar*)before.d->data, bl, skiptable, cs);

    if (bl == al) {
	if (bl) {
	    const QChar *auc = (const QChar*) after.d->data;
	    while ((index = bm_find((const QChar*)d->data, d->size, index, (const QChar*)before.d->data, bl, skiptable, cs)) != -1) {
		memcpy(d->data + index, auc, al * sizeof(QChar));
		index += bl;
	    }
	}
    } else if (al < bl) {
	const QChar *auc = after.unicode();
	uint to = 0;
	uint movestart = 0;
	uint num = 0;
	while ((index = bm_find((const QChar*)d->data, d->size, index, (const QChar*)before.d->data, bl, skiptable, cs)) != -1) {
	    if (num) {
		int msize = index - movestart;
		if (msize > 0) {
		    memmove(d->data + to, d->data + movestart, msize*sizeof(QChar));
		    to += msize;
		}
	    } else {
		to = index;
	    }
	    if (al) {
		memcpy(d->data+to, auc, al*sizeof(QChar));
		to += al;
	    }
	    index += bl;
	    movestart = index;
	    num++;
	}
	if (num) {
	    int msize = d->size - movestart;
	    if (msize > 0)
		memmove(d->data + to, d->data + movestart, msize*sizeof(QChar));
	    resize(d->size - num*(bl-al));
	}
    } else {
	// the most complex case. We don't want to loose performance by doing repeated
	// copies and reallocs of the string.
	while (index != -1) {
	    uint indices[4096];
	    uint pos = 0;
	    while (pos < 4095) {
		index = bm_find((const QChar*) d->data, d->size, index, (const QChar*)before.d->data, bl, skiptable, cs);
		if (index == -1)
		    break;
		indices[pos++] = index;
		index += bl;
		// avoid infinite loop
		if (!bl)
		    index++;
	    }
	    if (!pos)
		break;

	    // we have a table of replacement positions, use them for fast replacing
	    int adjust = pos*(al-bl);
	    // index has to be adjusted in case we get back into the loop above.
	    if (index != -1)
		index += adjust;
	    int newLen = d->size + adjust;
	    int moveend = d->size;
	    if (newLen > d->size)
		resize(newLen);

	    while (pos) {
		pos--;
		int movestart = indices[pos] + bl;
		int insertstart = indices[pos] + pos*(al-bl);
		int moveto = insertstart + al;
		memmove(d->data + moveto, d->data + movestart, (moveend - movestart)*sizeof(QChar));
		memcpy(d->data + insertstart, after.unicode(), al*sizeof(QChar));
		moveend = movestart-bl;
	    }
	}
    }
    return *this;
}


#ifndef QT_NO_REGEXP_CAPTURE
/*! \overload

  Replaces every occurrence of the regexp \a rx in the string with
  \a after. Returns a reference to the string. For example:
  \code
    QString s = "banana";
    s.replace(QRegExp("an"), "");
    // s == "ba"
  \endcode

  For regexps containing \link qregexp.html#capturing-text capturing
  parentheses \endlink, occurrences of <b>\\1</b>, <b>\\2</b>, ...,
  in \a after are replaced with \a{rx}.cap(1), cap(2), ...

  \code
    QString t = "A <i>bon mot</i>.";
    t.replace(QRegExp("<i>([^<]*)</i>"), "\\emph{\\1}");
    // t == "A \\emph{bon mot}."
  \endcode

  \sa find(), findRev(), QRegExp::cap()
*/
QString& QString::replace(const QRegExp& rx, const QString& after)
{
    QRegExp rx2 = rx;

    if (isEmpty() && rx2.search(*this) == -1)
	return *this;

    d->cache = 0;
    realloc();

    int index = 0;
    int numCaptures = rx2.numCaptures();
    int al = after.length();
    QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;

    if (numCaptures > 0) {
	if (numCaptures > 9)
	    numCaptures = 9;

	const QChar *uc = after.unicode();
	int numBackRefs = 0;

	for (int i = 0; i < al - 1; i++) {
	    if (uc[i] == '\\') {
		int no = uc[i + 1].digitValue();
		if (no > 0 && no <= numCaptures)
		    numBackRefs++;
	    }
	}

	/*
	  This is the harder case where we have back-references.
	  We don't try to optimize it.
	*/
	if (numBackRefs > 0) {
	    int *capturePositions = new int[numBackRefs];
	    int *captureNumbers = new int[numBackRefs];
	    int j = 0;

	    for (int i = 0; i < al - 1; i++) {
		if (uc[i] == '\\') {
		    int no = uc[i + 1].digitValue();
		    if (no > 0 && no <= numCaptures) {
			capturePositions[j] = i;
			captureNumbers[j] = no;
			j++;
		    }
		}
	    }

	    while (index <= length()) {
		index = rx2.search(*this, index, caretMode);
		if (index == -1)
		    break;

		QString after2 = after;
		for (j = numBackRefs - 1; j >= 0; j--)
		    after2.replace(capturePositions[j], 2,
				    rx2.cap(captureNumbers[j]));

		replace(index, rx2.matchedLength(), after2);
		index += after2.length();

		if (rx2.matchedLength() == 0) {
		    // avoid infinite loop on 0-length matches (e.g., [a-z]*)
		    index++;
		}
		caretMode = QRegExp::CaretWontMatch;
	    }
	    delete[] capturePositions;
	    delete[] captureNumbers;
	    return *this;
	}
    }

    /*
      This is the simple and optimized case where we don't have
      back-references.
   */
    while (index != -1) {
	struct {
	    int pos;
	    int length;
	} replacements[2048];

	uint pos = 0;
	int adjust = 0;
	while (pos < 2047) {
	    index = rx2.search(*this, index, caretMode);
	    if (index == -1)
		break;
	    int ml = rx2.matchedLength();
	    replacements[pos].pos = index;
	    replacements[pos++].length = ml;
	    index += ml;
	    adjust += al - ml;
	    // avoid infinite loop
	    if (!ml)
		index++;
	}
	if (!pos)
	    break;
	replacements[pos].pos = d->size;
	uint newlen = d->size + adjust;

	// to continue searching at the right position after we did
	// the first round of replacements
	if (index != -1)
	    index += adjust;
	QString newstring;
	newstring.reserve(newlen + 1);
	QChar *newuc = newstring.data();
	QChar *uc = newuc;
	int copystart = 0;
	uint i = 0;
	while (i < pos) {
	    int copyend = replacements[i].pos;
	    int size = copyend - copystart;
	    memcpy(uc, d->data + copystart, size * sizeof(QChar));
	    uc += size;
	    memcpy(uc, after.d->data, al * sizeof(QChar));
	    uc += al;
	    copystart = copyend + replacements[i].length;
	    i++;
	}
	memcpy(uc, d->data + copystart, (d->size - copystart) * sizeof(QChar));
	newstring.resize(newlen);
	*this = newstring;
	caretMode = QRegExp::CaretWontMatch;
    }
    return *this;
}
#endif

/*!
    Returns the number of times the character \a c occurs in the
    string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \code
    QString string("Trolltech and Qt");
    int n = string.contains('t', false);
    // n == 3
    \endcode

    \sa contains()
*/

int QString::count(QChar c, QString::CaseSensitivity cs) const
{
    int num = 0;
    const QChar *i = (const QChar*) d->data + d->size;
    const QChar *b = (const QChar*) d->data;
    if (cs == CaseSensitive) {
	while (i != b)
	    if (*--i == c)
		++num;
    } else {
	c = ::lower(c);
	while (i != b)
	    if (::lower(*--i) == c)
		++num;
    }
    return num;
}

/*!
    \overload

    Returns the number of times the string \a s occurs in the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa contains()

*/
int QString::count(const QString& s, QString::CaseSensitivity cs) const
{
    int num = 0;
    int i = -1;
    while ((i = indexOf(s, i+1, cs)) != -1)
	++num;
    return num;
}


/*! \fn bool QString::contains(QChar c, QString::CaseSensitivity cs = QString::CaseSensitive)
    Returns whether the character \a c occurs in the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa count()
*/

/*! \fn bool QString::contains(const QString& s, QString::CaseSensitivity cs = QString::CaseSensitive)
    \overload
    Returns whether the substring \a s occurs in the string.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa count()
*/

/*! \fn bool QString::contains(const QRegExp& rx)
    \overload
    Returns whether the regexp \a rx matches the string.

    \sa count()
*/


#ifndef QT_NO_REGEXP
/*!
    Finds the first match of the regular expression \a rx, starting
    from position \a from. If \a from is -1, the search starts at
    the last character; if -2, at the next to last character and so
    on. (See findRev() for searching backwards.)

    Returns the position of the first match of \a rx or -1 if no match
    was found.

    \code
	QString string("bananas");
	int i = string.find(QRegExp("an"), 0);    // i == 1
    \endcode

    \sa findRev() replace() contains()
*/
int QString::indexOf(const QRegExp& rx, int from) const { return rx.search(*this, from); }

/*!
    Finds the first match of the regexp \a rx, starting at position \a
    from and searching backwards. If \a from is -1, the search
    starts at the last character, if it is -2, at the next to last
    character and so on. (See findRev() for searching backwards.)

    Returns the position of the match or -1 if no match was found.

    \code
	QString string("bananas");
	int i = string.findRev(QRegExp("an"));      // i == 3
    \endcode

    \sa find()
*/

int QString::lastIndexOf(const QRegExp& rx, int from) const { return rx.searchRev(*this, from); }

/*!
    Returns the number of times the regexp, \a rx, matches in the
    string.

    This function counts overlapping matches, so in the example below,
    there are four instances of "ana" or "ama".

    \code
	QString str = "banana and panama";
	QRegExp rxp = QRegExp("a[nm]a", true, false);
	int i = str.contains(rxp);    // i == 4
    \endcode

    \sa contains(), find() findRev()
*/

int QString::count(const QRegExp& rx) const
{
    int count = 0;
    int index = -1;
    int len = length();
    while (index < len - 1) {                 // count overlapping matches
	index = rx.search(*this, index + 1);
	if (index == -1)
	    break;
	count++;
    }
    return count;
}
#endif // QT_NO_REGEXP



/*!
    \enum QString::SectionFlags

    \value SectionDefault Empty fields are counted, leading and
    trailing separators are not included, and the separator is
    compared case sensitively.

    \value SectionSkipEmpty Treat empty fields as if they don't exist,
    i.e. they are not considered as far as \e start and \e end are
    concerned.

    \value SectionIncludeLeadingSep Include the leading separator (if
    any) in the result string.

    \value SectionIncludeTrailingSep Include the trailing separator
    (if any) in the result string.

    \value SectionCaseInsensitiveSeps Compare the separator
    case-insensitively.

    Any of the last four values can be OR-ed together to form a flag.

    \sa section()
*/

/*!
    \fn QString QString::section( QChar sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    character, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString csv( "forename,middlename,surname,phone" );
    QString s = csv.section( ',', 2, 2 );   // s == "surname"

    QString path( "/usr/local/bin/myapp" ); // First field is empty
    QString s = path.section( '/', 3, 4 );  // s == "bin/myapp"
    QString s = path.section( '/', 3, 3, SectionSkipEmpty ); // s == "myapp"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString csv( "forename,middlename,surname,phone" );
    QString s = csv.section( ',', -3, -2 );  // s == "middlename,surname"

    QString path( "/usr/local/bin/myapp" ); // First field is empty
    QString s = path.section( '/', -1 ); // s == "myapp"
    \endcode

    \sa QStringList::split()
*/

/*!
    \overload

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    string, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString data( "forename**middlename**surname**phone" );
    QString s = data.section( "**", 2, 2 ); // s == "surname"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString data( "forename**middlename**surname**phone" );
    QString s = data.section( "**", -3, -2 ); // s == "middlename**surname"
    \endcode

    \sa QStringList::split()
*/

QString QString::section( const QString &sep, int start, int end, int flags ) const
{
    QStringList sections = QStringList::split(sep, *this, true);
    if(sections.isEmpty())
	return QString();
    if(!(flags & SectionSkipEmpty)) {
	if(start < 0)
	    start += sections.count();
	if(end < 0)
	    end += sections.count();
    } else {
	int skip = 0;
	for(QStringList::Iterator it = sections.begin(); it != sections.end(); ++it) {
	    if((*it).isEmpty())
		skip++;
	}
	if(start < 0)
	    start += sections.count() - skip;
	if(end < 0)
	    end += sections.count() - skip;
    }
    int x = 0, run = 0;
    QString ret;
    for(QStringList::Iterator it = sections.begin(); x <= end && it != sections.end(); ++it) {
	if(x >= start) {
	    if((*it).isEmpty()) {
		run++;
	    } else {
		if(!ret.isEmpty() || !(flags & SectionSkipEmpty)) {
		    int i_end = run;
		    if(!ret.isEmpty() && !(flags & SectionIncludeTrailingSep))
			i_end++;
		    if((flags & SectionIncludeLeadingSep) && it != sections.begin() && x == start)
			i_end++;
		    for(int i = 0; i < i_end; i++)
			ret += sep;
		} else if((flags & SectionIncludeLeadingSep) && it != sections.begin()) {
		    ret += sep;
		}
		run = 0;
		ret += (*it);
		if((flags & SectionIncludeTrailingSep) && it != sections.end())
		    ret += sep;
	    }
	}
	if(!(*it).isEmpty() || !(flags & SectionSkipEmpty))
	    x++;
    }
    return ret;
}

#ifndef QT_NO_REGEXP
class section_chunk {
public:
    section_chunk(int l, QString s) { length = l; string = s; }
    int length;
    QString string;
};

/*!
    \overload

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    regular expression, \a reg. The returned string consists of the
    fields from position \a start to position \a end inclusive. If \a
    end is not specified, all fields from position \a start to the end
    of the string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behaviour, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \code
    QString line( "forename\tmiddlename  surname \t \t phone" );
    QRegExp sep( "\s+" );
    QString s = line.section( sep, 2, 2 ); // s == "surname"
    \endcode

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \code
    QString line( "forename\tmiddlename  surname \t \t phone" );
    QRegExp sep( "\\s+" );
    QString s = line.section( sep, -3, -2 ); // s == "middlename  surname"
    \endcode

    \warning Using this QRegExp version is much more expensive than
    the overloaded string and character versions.

    \sa QStringList::split() simplifyWhiteSpace()
*/
QString QString::section( const QRegExp &reg, int start, int end, int flags ) const
{
    const QChar *uc = unicode();
    if(!uc)
	return QString();

    QRegExp sep(reg);
    sep.setCaseSensitive(!(flags & SectionCaseInsensitiveSeps));

    QList<section_chunk> l;
    int n = length(), m = 0, last_m = 0, last = 0, last_len = 0;

    while ( ( m = sep.search( *this, m ) ) != -1 ) {
	l.append(section_chunk(last_len, QString(uc + last_m, m - last_m)));
	last_m = m;
	last_len = sep.matchedLength();
	if((m += sep.matchedLength()) >= n) {
	    last = 1;
	    break;
	}
    }
    if(!last)
	l.append(section_chunk(last_len, QString(uc + last_m, n - last_m)));

    if(start < 0)
	start = l.count() + start;
    if(end == -1)
	end = l.count();
    else if(end < 0)
	end = l.count() + end;

    int i = 0;
    QString ret;
    for (int idx = 0; idx < l.size(); ++idx) {
	const section_chunk &chk = l.at(idx);
	if((flags & SectionSkipEmpty) && chk.length == (int)chk.string.length()) {
	    if(i <= start)
		start++;
	    end++;
	}
	if(i == start) {
	    ret = (flags & SectionIncludeLeadingSep) ? chk.string : chk.string.mid(chk.length);
	} else if(i > start) {
	    ret += chk.string;
	}
	if(i == end) {
	    if(idx < l.size()-1 && flags & SectionIncludeTrailingSep)
		ret += chk.string.left(chk.length);
	    break;
	}
    }
    return ret;
}
#endif

/*!
    \fn QString QString::section( char sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    \overload
*/

/*!
    \fn QString QString::section( const char *sep, int start, int end = 0xffffffff, int flags = SectionDefault ) const

    \overload
*/


/*!
    Returns a substring that contains the \a len leftmost characters
    of the string.

    The whole string is returned if \a len exceeds the length of the
    string.

    \code
	QString s = "Pineapple";
	QString t = s.left(4);    // t == "Pine"
    \endcode

    \sa right(), mid(), isEmpty()
*/
QString QString::left(int len)  const
{
    if (d == &shared_null)
	return QString();
    if (len > d->size)
	return *this;
    if (len < 0)
	len = 0;
    return QString((const QChar*) d->data, len);
}

/*!
    Returns a string that contains the \a len rightmost characters of
    the string.

    If \a len is greater than the length of the string then the whole
    string is returned.

    \code
	QString string("Pineapple");
	QString t = string.right(5);   // t == "apple"
    \endcode

    \sa left(), mid(), isEmpty()
*/

QString QString::right(int len) const
{
    if (d == &shared_null)
	return QString();
    if (len > d->size)
	return *this;
    if (len < 0)
	len = 0;
    return QString((const QChar*) d->data + d->size - len, len);
}

/*!
    Returns a string that contains the \a len characters of this
    string, starting at position \a i.

    Returns an empty string if the string is empty or index \a i
    exceeds the length of the string. If there are less than \a len
    characters available in the string starting at position \a i, the
    function returns all characters that are available.

    \code
	QString s("Five pineapples");
	QString t = s.mid(5, 4);                  // t == "pine"
    \endcode

    \sa left(), right()
*/

QString QString::mid(int i, int len) const
{
    if (d == &shared_null || i >= d->size)
	return QString();
    if (len < 0)
	len = d->size - i;
    if (i < 0) {
	len += i;
	i = 0;
    }
    if (len + i > d->size)
	len = d->size - i;
    if (i == 0 && len == d->size)
	return *this;
    return QString((const QChar*) d->data + i, len);
}

/*!
    Returns true if the string starts with \a s; otherwise returns
    false.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \code
	QString str("Bananas");
	str.startsWith("Ban");     // returns true
	str.startsWith("Car");     // returns false
    \endcode

    \sa endsWith()
*/
bool QString::startsWith(const QString& s, QString::CaseSensitivity cs) const
{
    if (d == &shared_null)
	return (s.d == &shared_null);
    if (d->size == 0)
	return s.d->size == 0;
    if (s.d->size > d->size)
	return false;
    if (cs == CaseSensitive) {
	for (int i = 0; i < s.d->size; ++i)
	    if (d->data[i] != s.d->data[i])
		return false;
    } else {
	for (int i = 0; i < s.d->size; ++i)
	    if (::lower(d->data[i]) != ::lower(s.d->data[i]))
		return false;
    }
    return true;
}

/*!
    Returns true if the string ends with \a s; otherwise returns
    false.

    If \a cs is CaseSensitive (the default), the search is case
    sensitive; otherwise the search is case insensitive.

    \code
	QString str("Bananas");
	str.endsWith("anas");         // returns true
	str.endsWith("pple");         // returns false
    \endcode

    \sa startsWith()
*/
bool QString::endsWith(const QString& s, QString::CaseSensitivity cs) const
{
    if (d == &shared_null)
	return (s.d == &shared_null);
    if (d->size == 0)
	return s.d->size == 0;
    int pos = d->size - s.d->size;
    if (pos < 0)
	return false;
    if (cs == CaseSensitive) {
	for (int i = 0; i < s.d->size; i++)
	    if (d->data[pos+i] != s.d->data[i])
		return false;
    } else {
	for (int i = 0; i < (int) s.length(); i++)
	    if (::lower(d->data[pos+i]) != ::lower(s.d->data[i]))
		return false;
    }
    return true;
}

bool operator!=(const QString &s1, const QString &s2)
{ return !(s1==s2); }

bool operator<(const QString &s1, const QString &s2)
{ return ucstrcmp(s1,s2) < 0; }

bool operator<=(const QString &s1, const QString &s2)
{ return ucstrcmp(s1,s2) <= 0; }

bool operator>(const QString &s1, const QString &s2)
{ return ucstrcmp(s1,s2) > 0; }

bool operator>=(const QString &s1, const QString &s2)
{ return ucstrcmp(s1,s2) >= 0; }

bool operator==(const QString &s1, const char *s2)
{
    if (!s2)
	return (s1 == QString());

#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() )
	return (s1 == QString::fromAscii(s2));
#endif

    int len = s1.length();
    const QChar *uc = s1.unicode();
    while (len) {
	if (!(*s2) || uc->unicode() != (uchar) *s2)
	    return false;
	++uc;
	++s2;
	--len;
    }
    return !*s2;
}


bool operator<(const QString &s1, const char *s2)
{ return ucstrcmp(s1,s2) < 0; }

bool operator<(const char *s1, const QString &s2)
{ return ucstrcmp(s1,s2) < 0; }

bool operator<=(const QString &s1, const char *s2)
{ return ucstrcmp(s1,s2) <= 0; }

bool operator<=(const char *s1, const QString &s2)
{ return ucstrcmp(s1,s2) <= 0; }

bool operator>(const QString &s1, const char *s2)
{ return ucstrcmp(s1,s2) > 0; }

bool operator>(const char *s1, const QString &s2)
{ return ucstrcmp(s1,s2) > 0; }

bool operator>=(const QString &s1, const char *s2)
{ return ucstrcmp(s1,s2) >= 0; }

bool operator>=(const char *s1, const QString &s2)
{ return ucstrcmp(s1,s2) >= 0; }


/*! \fn const char *QString::latin1() const

    Returns a Latin11 representation of the string. The
    returned value is undefined if the string contains non-Latin1
    characters. If you want to convert strings into formats other than
    Unicode, see the QTextCodec classes.

    This function is mainly useful for boot-strapping legacy code to
    use Unicode.

    The result remains valid so long as one unmodified copy of the
    source string exists and no other 8 bit representations of the
    same string are requested.

    \sa fromLatin1(), ascii(), utf8(), local8Bit()
*/
/*! \fn const char *QString::utf8() const
    Returns the string encoded in UTF-8 format.

    See QTextCodec for more diverse coding/decoding of Unicode strings.

    \sa fromUtf8(), toUtf8()
*/

/*!
    Returns a Latin1 representation of the string in a QByteArray.
    The returned value is undefined if the string contains non-Latin1
    characters. If you want to convert strings into formats other than
    Unicode, see the QTextCodec classes.

    \sa fromLatin1(), latin1(), toAscii(), toUtf8(), toLocal8Bit()
*/
QByteArray QString::toLatin1() const
{
    QByteArray *ba = (QByteArray*) &d->c;
    if (!d->cache || d->encoding != Data::Latin1) {
	if (!d->c)
	    new (&d->c) QByteArray;
	else
	    ba->clear();
	d->cache = true;
	d->encoding = Data::Latin1;
	if (d->size) {
	    ba->resize(d->size);
	    const ushort *i = d->data;
	    const ushort *e = d->data + d->size;
	    uchar *s = (uchar*) ba->data();
	    while (i != e) {
		*s++ = (*i>0xff) ? '?' : (uchar) *i;
		++i;
	    }
	}
    }
    return *ba;
}

/*!
    Returns an ASCII representation of the string in a QByteArray. The
    returned value is undefined if the string contains non-ASCII
    characters. If you want to convert strings into formats other than
    Unicode, see the QTextCodec classes.

    \sa fromLatin1(), ascii(), toLatin1(), toUtf8(), toLocal8Bit()
*/
QByteArray QString::toAscii() const
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	QByteArray *ba = (QByteArray*) &d->c;
	if (!d->cache || d->encoding != Data::Latin1) {
	    if (!d->c)
		new (&d->c) QByteArray;
	    d->cache = true;
	    d->encoding = Data::Ascii;
	    if (d == &shared_null)
		ba->clear();
	    else
		*ba = QTextCodec::codecForCStrings()->fromUnicode( *this );
	}
	return *ba;
    }
#endif // QT_NO_TEXTCODEC
    return toLatin1();
}

/*!
    Returns a local 8-bit representation of the string in a
    QByteArray. The returned value is undefined if the string contains
    characters that are not in the local 8-bit character set. If you
    want to convert strings into formats other than Unicode, see the
    QTextCodec classes.

    \sa fromLatin1(), local8Bit(), toAscii(), toLatin1(), toUtf8()
*/
QByteArray QString::toLocal8Bit() const
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForLocale() ) {
	QByteArray *ba = (QByteArray*) &d->c;
	if (!d->cache || d->encoding != Data::Local8Bit) {
	    if (!d->c)
		new (&d->c) QByteArray;
	    d->cache = true;
	    d->encoding = Data::Local8Bit;
	    if (d == &shared_null)
		ba->clear();
	    else
		*ba = QTextCodec::codecForLocale()->fromUnicode( *this );
	}
	return *ba;
    }
#endif // QT_NO_TEXTCODEC
    return toLatin1();
}

/*!
    Returns the string encoded in UTF-8 format.

    See QTextCodec for more diverse coding/decoding of Unicode strings.

    \sa fromUtf8(), utf8()
*/

QByteArray QString::toUtf8() const
{
    QByteArray *ba = (QByteArray*) &d->c;
    if (!d->cache || d->encoding != Data::Utf8) {
	if (!d->c)
	    new (&d->c) QByteArray;
	else
	    ba->clear();
	d->cache = true;
	d->encoding = Data::Utf8;
	if (d->size) {
	    int l = d->size;
	    int rlen = l*3+1;
	    ba->resize(rlen);
	    uchar *cursor = (uchar*)ba->data();
	    const ushort *ch =d->data;
	    for (int i=0; i < l; i++) {
		uint u = *ch;
		if ( u < 0x80 ) {
		    *cursor++ = (uchar)u;
		} else {
		    if ( u < 0x0800 ) {
			*cursor++ = 0xc0 | ((uchar) (u >> 6));
		    } else {
			if (u >= 0xd800 && u < 0xdc00 && i < l-1) {
			    unsigned short low = ch[1];
			    if (low >= 0xdc00 && low < 0xe000) {
				++ch;
				++i;
				u = (u - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
			    }
			}
			if (u > 0xffff) {
			    // if people are working in utf8, but strings are encoded in eg. latin1, the resulting
			    // name might be invalid utf8. This and the corresponding code in fromUtf8 takes care
			    // we can handle this without loosing information. This can happen with latin filenames
			    // and a utf8 locale under Unix.
			    if (u > 0x10fe00 && u < 0x10ff00) {
				*cursor++ = (u - 0x10fe00);
				++ch;
				continue;
			    } else {
				*cursor++ = 0xf0 | ((uchar) (u >> 18));
				*cursor++ = 0x80 | ( ((uchar) (u >> 12)) & 0x3f);
			    }
			} else {
			    *cursor++ = 0xe0 | ((uchar) (u >> 12));
			}
			*cursor++ = 0x80 | ( ((uchar) (u >> 6)) & 0x3f);
		    }
		    *cursor++ = 0x80 | ((uchar) (u&0x3f));
		}
		++ch;
	    }
	    ba->resize(cursor - (uchar*)ba->constData());
	}
    }
    return *ba;
}

/*!
    Returns the Unicode string decoded from the first \a size
    characters of \a s, ignoring the rest of \a s. If \a size is -1 or
    bigger than the length of \a s, then the length of \a s is used.

    \sa fromAscii()
*/
QString QString::fromLatin1(const char *s, int size)
{
    Data *d;
    if (!s) {
	d = &shared_null;
	++d->ref;
    } else if (!*s) {
	d = &shared_empty;
	++d->ref;
    } else {
	int len = size < 0 ? strlen(s) : size;
	d = (Data*) qMalloc(sizeof(Data)+len*sizeof(QChar));
	d->ref = 1;
	d->alloc = d->size = len;
	d->c = 0;
	d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
	d->data = d->array;
	ushort *i = d->data;
	while(len--)
	   * i++ = (uchar)*s++;
    }
    return QString(d);
}

#ifdef Q_OS_WIN32
#include "qt_windows.h"

QByteArray qt_winQString2MB( const QString& s, int uclen )
{
    if ( uclen < 0 )
	uclen = s.length();
    if ( s.isNull() )
	return QByteArray();
    if ( uclen == 0 )
	return QByteArray("");
    BOOL used_def;
    QByteArray mb(4096);
    int len;
    while ( !(len=WideCharToMultiByte(CP_ACP, 0, (const WCHAR*)s.unicode(), uclen,
		mb.data(), mb.size()-1, 0, &used_def)) )
    {
	int r = GetLastError();
	if ( r == ERROR_INSUFFICIENT_BUFFER ) {
	    mb.resize(1+WideCharToMultiByte( CP_ACP, 0,
				(const WCHAR*)s.unicode(), uclen,
				0, 0, 0, &used_def));
		// and try again...
	} else {
#ifndef QT_NO_DEBUG
	    // Fail.
	    qWarning("WideCharToMultiByte cannot convert multibyte text (error %d): %s (UTF-8)",
		r, s.utf8());
#endif
	    break;
	}
    }
    mb.resize(len);
    return mb;
}

QString qt_winMB2QString( const char* mb, int mblen )
{
    if ( !mb || !mblen )
	return QString::null;
    const int wclen_auto = 4096;
    WCHAR wc_auto[wclen_auto];
    int wclen = wclen_auto;
    WCHAR *wc = wc_auto;
    int len;
    while ( !(len=MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
		mb, mblen, wc, wclen )) )
    {
	int r = GetLastError();
	if ( r == ERROR_INSUFFICIENT_BUFFER ) {
	    if ( wc != wc_auto ) {
		qWarning("Size changed in MultiByteToWideChar");
		break;
	    } else {
		wclen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
				    mb, mblen, 0, 0 );
		wc = new WCHAR[wclen];
		// and try again...
	    }
	} else {
	    // Fail.
	    qWarning("MultiByteToWideChar cannot convert multibyte text");
	    break;
	}
    }
    if ( len <= 0 )
	return QString::null;
    QString s( (QChar*)wc, len - 1 ); // len - 1: we don't want terminator
    if ( wc != wc_auto )
	delete [] wc;
    return s;
}
#endif // Q_OS_WIN32

/*!
    Returns the Unicode string decoded from the first \a len
    bytes of \a local8Bit, ignoring the rest of \a local8Bit. If
    \a len is -1 then the length of \a local8Bit is used. If \a len is
    bigger than the length of \a local8Bit then it will use the length
    of \a local8Bit.

    \code
	QString str = QString::fromLocal8Bit( "123456789", 5 );
	// str == "12345"
    \endcode

    \a local8Bit is assumed to be encoded in a locale-specific format.

    See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QString QString::fromLocal8Bit( const char* local8Bit, int len )
{
    if ( !local8Bit )
	return QString::null;
    if (len < 0)
	len = strlen(local8Bit);
#if defined(Q_OS_DARWIN)
    return fromUtf8(local8Bit,len);
#elif defined(Q_OS_WIN32)
    if ( len >= 0 ) {
	QByteArray s(local8Bit,len+1);
	return qt_winMB2QString(s);
    }
    return qt_winMB2QString( local8Bit );
#elif defined(Q_OS_UNIX)
#  if !defined(QT_NO_TEXTCODEC)
    QTextCodec *codec = QTextCodec::codecForLocale();
    if (codec)
	return codec->toUnicode( local8Bit, len );
#  endif // !QT_NO_TEXTCODEC
#endif
    return fromLatin1( local8Bit, len );
}

/*!
    Returns the Unicode string decoded from the first \a len
    characters of \a ascii, ignoring the rest of \a ascii. If \a len
    is -1 then the length of \a ascii is used. If \a len is bigger
    than the length of \a ascii then it will use the length of \a
    ascii.

    If a codec has been set using QTextCodec::codecForCStrings(),
    it is used to convert Unicode to 8-bit char. Otherwise, this function
    does the same as fromLatin1().

    This is the same as the QString(const char*) constructor, but you
    can make that constructor invisible if you compile with the define
    \c QT_NO_CAST_ASCII, in which case you can explicitly create a
    QString from 8-bit ASCII text using this function.

    \code
	QString str = QString::fromAscii( "123456789", 5 );
	// str == "12345"
    \endcode
 */
QString QString::fromAscii( const char* ascii, int len )
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	if ( !ascii )
	    return QString::null;
	if ( len < 0 )
	    len = strlen( ascii );
	if ( len == 0 || *ascii == '\0' )
	    return QString::fromLatin1( "" );
	return QTextCodec::codecForCStrings()->toUnicode( ascii, len );
    }
#endif
    return fromLatin1( ascii, len );
}


/*!
    Returns the Unicode string decoded from the first \a len
    bytes of \a utf8, ignoring the rest of \a utf8. If \a len is
    -1 then the length of \a utf8 is used. If \a len is bigger than
    the length of \a utf8 then it will use the length of \a utf8.

    \code
	QString str = QString::fromUtf8("123456789", 5);
	// str == "12345"
    \endcode

    See QTextCodec for more diverse coding/decoding of Unicode strings.
*/
QString QString::fromUtf8( const char* utf8, int len )
{
    if ( !utf8 )
	return QString::null;

    if ( len < 0 )
	len = strlen( utf8 );
    QString result;
    result.resize( len*2 ); // worst case
    unsigned short *qch = result.d->data;
    uint uc = 0;
    int need = 0;
    int error = -1;
    uchar ch;
    for (int i=0; i<len; i++) {
	ch = utf8[i];
	if (need) {
	    if ( (ch&0xc0) == 0x80 ) {
		uc = (uc << 6) | (ch & 0x3f);
		need--;
		if ( !need ) {
		    if (uc > 0xffff) {
			// surrogate pair
			uc -= 0x10000;
			unsigned short high = uc/0x400 + 0xd800;
			unsigned short low = uc%0x400 + 0xdc00;
			*qch++ = high;
			*qch++ = low;
		    } else {
			*qch++ = uc;
		    }
		}
	    } else {
		// See QString::utf8() for explanation.
		//
		// The surrogate below corresponds to a Unicode value of (0x10fe00+ch) which
		// is in one of the private use areas of Unicode.
		i = error;
		*qch++ = 0xdbff;
		*qch++ = 0xde00+((uchar)utf8[i]);
		need = 0;
	    }
	} else {
	    if ( ch < 128 ) {
		*qch++ = ch;
	    } else if ((ch & 0xe0) == 0xc0) {
		uc = ch & 0x1f;
		need = 1;
		error = i;
	    } else if ((ch & 0xf0) == 0xe0) {
		uc = ch & 0x0f;
		need = 2;
		error = i;
	    } else if ((ch&0xf8) == 0xf0) {
		uc = ch & 0x07;
		need = 3;
		error = i;
	    } else {
	        // Error
	        *qch++ = 0xdbff;
		*qch++ = 0xde00+((uchar)utf8[i]);
 	    }
	}
    }
    if (need) {
	// we have some invalid characters remaining we need to add to the string
	for (int i = error; i < len; ++i) {
	    *qch++ = 0xdbff;
	    *qch++ = 0xde00+((uchar)utf8[i]);
	}
    }

    result.truncate( qch - result.d->data );
    return result;
}

/*!
  Constructs a string that is a deep copy of \a str, interpreted as an
  ISO-10646-UCS-2 encoded, zero terminated, Unicode string.

  If \a str is 0, then a null string is created.

  \sa isNull()
*/
QString QString::fromUcs2( const unsigned short *str )
{
    if ( !str )
	return QString();
    int length = 0;
    while ( str[length] != 0 )
	length++;
    return QString((const QChar *)str, length);
}

/*!
    Resizes the string to \a size characters and copies \a unicode
    into the string. If \a unicode is 0, nothing is copied, but the
    string is still resized to \a size.

    \sa unicode()
*/
QString& QString::setUnicode(const QChar *unicode, int size)
{
     resize(size);
     if (unicode && size)
	 memcpy(d->data, unicode, size*sizeof(QChar));
     return *this;
}

/*!
    Resizes the string to \a size characters and copies \a
    unicode_as_ushorts into the string.

    If \a unicode_as_ushorts is 0, nothing is copied, but the string
    is still resized to \a size.

    \sa setUnicode(), ucs2()
*/
QString& QString::setUnicodeCodes(const ushort *unicode_as_ushorts, int size)
{
     return setUnicode((const QChar*)unicode_as_ushorts, size);
}


/*!
    Returns a string that has whitespace removed from the start and
    the end, and which has each sequence of internal whitespace
    replaced with a single space.

    Whitespace means any character for which QChar::isSpace() returns
    true. This includes Unicode characters with decimal values 9
    (TAB), 10 (LF), 11 (VT), 12 (FF), 13 (CR), and 32 (Space).

    \code
	QString string = "  lots\t of\nwhite    space ";
	QString t = string.simplified();
	// t == "lots of white space"
    \endcode

    \sa trimmed()
*/
QString QString::simplified() const
{
    if (d->size == 0)
	return *this;
    QString result;
    result.resize(d->size);
    const QChar *from = (const QChar*) d->data;
    const QChar *fromend = (const QChar*) from+d->size;
    int outc=0;
    QChar *to   = (QChar*) result.d->data;
    for (;;) {
	while (from!=fromend && from->isSpace())
	    from++;
	while (from!=fromend && !from->isSpace())
	    to[outc++] = *from++;
	if (from!=fromend)
	    to[outc++] = ' ';
	else
	    break;
    }
    if (outc > 0 && to[outc-1] == ' ')
	outc--;
    result.truncate(outc);
    return result;
}

/*!
    Returns a string that has whitespace removed from the start and
    the end.

    Whitespace means any character for which QChar::isSpace() returns
    true. This includes Unicode characters with decimal values 9
    (TAB), 10 (LF), 11 (VT), 12 (FF), 13 (CR) and 32 (Space), and may
    also include other Unicode characters.

    \code
	QString string = "   white space   ";
	QString s = string.trimmed();       // s == "white space"
    \endcode

    \sa simplified()
*/
QString QString::trimmed() const
{
    if (d->size == 0)
	return *this;
    const QChar *s = (const QChar*)d->data;
    if (!s->isSpace() && !s[d->size-1].isSpace())
	return *this;
    int start = 0;
    int end = d->size - 1;
    while (start<=end && s[start].isSpace())  // skip white space from start
	start++;
    if (start <= end) {                          // only white space
	while (end && s[end].isSpace())           // skip white space from end
	    end--;
    }
    int l = end - start + 1;
    if (l <= 0)
    	return QString("");
    return QString(s+start, l);
}

/*!
    \fn const QChar QString::operator[](int i) const

    Returns the character at index \a i, or QChar::null if \a i is
    beyond the length of the string.

    If the QString is not const (i.e., const QString) or const\&
    (i.e., const QString\&), then the non-const overload of operator[]
    will be used instead.
*/

/*!
    \fn QCharRef QString::operator[](int i)

    \overload

    The function returns a reference to the character at index \a i.
    The resulting reference can then be assigned to, or used
    immediately, but it will become invalid once further modifications
    are made to the original string.

    If \a i is beyond the length of the string, the string is expanded
    when you assign to the reference.

    The QCharRef internal class can be used much like a constant
    QChar, but if you assign to it, you change the original string
    (which will detach itself because of QString's copy-on-write
    semantics). You will get compilation errors if you try to use the
    result as anything but a QChar.
*/

/*! \fn  void QString::truncate(int maxSize)

    If \a maxSize is less than the length of the string, then the
    string is truncated at position \a maxSize. Otherwise nothing
    happens.

    \code
	QString s = "truncate me";
	s.truncate(5);            // s == "trunc"
    \endcode

    \sa resize();
*/

/*!
    Fills the string with \a size characters of character \a c, and
    returns a reference to the string.

    If \a size is negative (the default), the current string length is
    used.

    \code
	QString str;
	str.fill('g', 5);      // string == "ggggg"
    \endcode
*/

QString& QString::fill(QChar c, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size) {
	QChar *i = (QChar*)d->data + d->size;
	QChar *b = (QChar*)d->data;
	while (i != b)
	   * --i = c;
    }
    return *this;
}

/*!
    \fn int QString::length() const

    Returns the length of the string. The function is equivalent to size()

    \sa isEmpty(), resize()
*/

/*!
    \fn int QString::size() const

    Returns the length of the string. The function is equivalent to length()

    \sa isEmpty(), resize()
*/


/*!
    \fn bool QString::isNull() const

    Returns TRUE if the string is null; otherwise returns FALSE. A
    null string is always empty.

    \code
	QString a;          // a.unicode() == 0, a.length() == 0
	a.isNull();         // TRUE, because a.unicode() == 0
	a.isEmpty();        // TRUE, because a.length() == 0
    \endcode

    \sa isEmpty(), length()
*/

/*!
    \fn bool QString::isEmpty() const

    Returns true if the string is empty, i.e. if length() == 0;
    otherwise returns false.

    \sa length()
*/

/*
  \fn QString& operator+=(QChar c)

    Appends \a c to the string and returns a reference to the string.

    Equivalent to append().
 */

/* \overload
  \fn QString& operator+=(const QString& s)

    Appends \a s to the string and returns a reference to the string.

    Equivalent to append().
 */

/* \overload
  \fn QString& operator+=(const std::string& s)

    Appends \a s to the string and returns a reference to the string.

    Equivalent to append().
 */

/* \overload
  \fn QString& operator+=(char c)

    Appends \a c to the string and returns a reference to the string.

    Equivalent to append().
 */

/* \overload
  \fn QString& operator+=(const char *s)

    Appends \a s to the string and returns a reference to the string.

    Equivalent to append().
 */

/*!
    \fn bool operator==(const QString &s1, const QString &s2)

    \relates QString

    Returns true if \a s1 is equal to \a s2; otherwise returns false.

    Equivalent to compare(\a s1, \a s2) != 0.
*/

/*!
    \fn bool operator==(const QString &s1, const char *s2)

    \overload
    \relates QString

    Returns true if \a s1 is equal to \a s2; otherwise returns false.
    Note that no string is equal to \a s2 being 0.

    Equivalent to s2!=0 && compare(\a s1, \a s2) == 0.
*/

/*!
    \fn bool operator==(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is equal to \a s2; otherwise returns false.
    Note that no string is equal to \a s1 being 0.

    Equivalent to s1!=0 && compare(\a s1, \a s2) == 0.
*/

/*!
    \fn bool operator!=(const QString &s1, const QString &s2)

    \relates QString

    Returns true if \a s1 is not equal to \a s2; otherwise returns false.

    Equivalent to compare(\a s1, \a s2) != 0.
*/

/*!
    \fn bool operator!=(const QString &s1, const char *s2)

    \overload
    \relates QString

    Returns true if \a s1 is not equal to \a s2; otherwise returns false.
    Note that no string is equal to \a s2 being 0.

    For \a s2 != 0, this is equivalent to compare(\a s1, \a s2) != 0.
*/

/*!
    \fn bool operator!=(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is not equal to \a s2; otherwise returns false.
    Note that no string is equal to \a s1 being 0.

    For \a s1 != 0, this is equivalent to compare(\a s1, \a s2) != 0.
*/

/*!
    \fn bool operator<(const QString &s1, const char *s2)

    \relates QString

    Returns true if \a s1 is lexically less than \a s2; otherwise returns false.
    The comparison is case sensitive.

    For \a s2 != 0, this is equivalent to compare(\a s1, \a s2) \< 0.
*/

/*!
    \fn bool operator<(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is lexically less than \a s2; otherwise returns false.
    The comparison is case sensitive.

    For \a s1 != 0, equivalent to compare(\a s1, \a s2) \< 0.
*/

/*!
    \fn bool operator<=(const QString &s1, const char *s2)

    \relates QString

    Returns true if \a s1 is lexically less than or equal to \a s2;
    otherwise returns false.
    The comparison is case sensitive.

    For \a s2 != 0, this is equivalent to compare(\a s1,\a s2) \<= 0.
*/

/*!
    \fn bool operator<=(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is lexically less than or equal to \a s2;
    otherwise returns false.
    The comparison is case sensitive.

    For \a s1 != 0, this is equivalent to compare(\a s1, \a s2) \<= 0.
*/

/*!
    \fn bool operator>(const QString &s1, const char *s2)

    \relates QString

    Returns true if \a s1 is lexically greater than \a s2; otherwise
    returns false.
    The comparison is case sensitive.

    For \a s2 != 0, this is equivalent to compare(\a s1, \a s2) \> 0.
*/

/*!
    \fn bool operator>(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is lexically greater than \a s2; otherwise
    returns false.
    The comparison is case sensitive.

    Equivalent to compare(\a s1, \a s2) \> 0.
*/

/*!
    \fn bool operator>=(const QString &s1, const char *s2)

    \relates QString

    Returns true if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns false.
    The comparison is case sensitive.

    For \a s2 != 0, this is equivalent to compare(\a s1, \a s2) \>= 0.
*/

/*!
    \fn bool operator>=(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns true if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns false.
    The comparison is case sensitive.

    For \a s1 != 0, this is equivalent to compare(\a s1, \a s2) \>= 0.
*/

/*!
    \fn const QString operator+(const QString &s1, const QString &s2)

    \relates QString

    Returns a string which is the result of concatenating the string
    \a s1 and the string \a s2.

    Equivalent to \a {s1}.append(\a s2).
*/

/*!
    \fn const QString operator+(const QString &s1, const char *s2)

    \overload
    \relates QString

    Returns a string which is the result of concatenating the string
    \a s1 and character \a s2.

    Equivalent to \a {s1}.append(\a s2).
*/

/*!
    \fn const QString operator+(const char *s1, const QString &s2)

    \overload
    \relates QString

    Returns a string which is the result of concatenating the
    character \a s1 and string \a s2.
*/

/*!
    \fn const QString operator+(const QString &s, char c)

    \overload
    \relates QString

    Returns a string which is the result of concatenating the string
    \a s and character \a c.

    Equivalent to \a {s}.append(\a c).
*/

/*!
    \fn const QString operator+(char c, const QString &s)

    \overload
    \relates QString

    Returns a string which is the result of concatenating the
    character \a c and string \a s.

    Equivalent to \a {s}.prepend(\a c).
*/

/*!
    \fn int QString::compare(const QString & s1, const QString & s2)

    Lexically compares \a s1 with \a s2 and returns an integer less
    than, equal to, or greater than zero if \a s1 is less than, equal
    to, or greater than \a s2.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings with
    QString::localeAwareCompare().

    \code
	int a = QString::compare("def", "abc");   // a > 0
	int b = QString::compare("abc", "def");   // b < 0
	int c = QString::compare("abc", "abc");   // c == 0
    \endcode
*/

/*!
    \overload

   Lexically compares this string with \a s and returns an integer
   less than, equal to, or greater than zero if it is less than, equal
   to, or greater than \a s.
*/
int QString::compare(const QString& s) const
{
    return ucstrcmp(*this, s);
}


/*!
    \fn int QString::localeAwareCompare( const QString & s1, const QString & s2 )

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa QString::compare() QTextCodec::locale()
*/

/*!
    \overload

    Compares this string with \a s.
*/

#if !defined(CSTR_LESS_THAN)
#define CSTR_LESS_THAN    1
#define CSTR_EQUAL        2
#define CSTR_GREATER_THAN 3
#endif

int QString::localeAwareCompare( const QString& s ) const
{
    // do the right thing for null and empty
    if ( isEmpty() || s.isEmpty() )
	return compare( s );

#if defined(Q_OS_WIN32)
    int res;
    QT_WA( {
	const TCHAR* s1 = (TCHAR*)ucs2();
	const TCHAR* s2 = (TCHAR*)s.ucs2();
	res = CompareStringW( LOCALE_USER_DEFAULT, 0, s1, length(), s2, s.length() );
    } , {
	QByteArray s1 = toLocal8Bit();
	QByteArray s2 = s.toLocal8Bit();
	res = CompareStringA( LOCALE_USER_DEFAULT, 0, s1.data(), s1.length(), s2.data(), s2.length() );
    } );

    switch ( res ) {
    case CSTR_LESS_THAN:
	return -1;
    case CSTR_GREATER_THAN:
	return 1;
    default:
	return 0;
    }
#elif defined(Q_OS_UNIX)
    // declared in <string.h>
    int delta = strcoll( local8Bit(), s.local8Bit() );
    if ( delta == 0 )
	delta = ucstrcmp( *this, s );
    return delta;
#else
    return ucstrcmp( *this, s );
#endif
}


/*!
    \fn const QChar *QString::unicode() const

    Returns the Unicode representation of the string. The result
    remains valid until the string is modified.

    \sa ucs2()
*/

/*!
  \fn const ushort *QString::ucs2() const

    Returns the QString as a zero terminated array of unsigned shorts.
    The result remains valid until the string is modified.

    \sa unicode()
*/


/*!
    Returns a string of length \a width that contains this string
    padded by the \a fill character.

    If \a truncate is false and the length of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is true and the length of the string is more than
    \a width, then any characters in a copy of the string after length
    \a width are removed, and the copy is returned.

    \code
	QString s("apple");
	QString t = s.leftJustified(8, '.');        // t == "apple..."
    \endcode

    \sa rightJustified()
*/

QString QString::leftJustified(int width, QChar fill, bool truncate) const
{
    QString result;
    int len = length();
    int padlen = width - len;
    if (padlen > 0) {
	result.resize(len+padlen);
	if (len)
	    memcpy(result.d->data, d->data, sizeof(QChar)*len);
	QChar *uc = (QChar*)result.d->data + len;
	while (padlen--)
	   * uc++ = fill;
    } else {
	if (truncate)
	    result = left(width);
	else
	    result = *this;
    }
    return result;
}

/*!
    Returns a string of length \a width that contains the \a fill
    character followed by the string.

    If \a truncate is false and the length of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is true and the length of the string is more than
    \a width, then the resulting string is truncated at position \a
    width.

    \code
	QString string("apple");
	QString t = string.rightJustified(8, '.');  // t == "...apple"
    \endcode

    \sa leftJustified()
*/

QString QString::rightJustified(int width, QChar fill, bool truncate) const
{
    QString result;
    int len = length();
    int padlen = width - len;
    if (padlen > 0) {
	result.resize(len+padlen);
	QChar *uc = (QChar*)result.d->data;
	while (padlen--)
	   * uc++ = fill;
	if (len)
	    memcpy(uc, d->data, sizeof(QChar)*len);
    } else {
	if (truncate)
	    result = left(width);
	else
	    result = *this;
    }
    return result;
}

/*!
    Returns a lowercase copy of the string.

    \code
	QString string("TROlltECH");
	str = string.toLower();   // str == "trolltech"
    \endcode

    \sa upper()
*/

QString QString::toLower() const
{
    int l = d->size;
    if (l) {
	QChar *p = (QChar*)d->data;
	if (p) {
	    while (l) {
		if (*p != ::lower(*p)) {
		    QString s(*this);
		    p = (QChar*)s.data() + (p - (QChar*)d->data);
		    while (l) {
			*p = ::lower(*p);
			l--;
			p++;
		    }
		    return s;
		}
		l--;
		p++;
	    }
	}
    }
    return *this;
}

/*!
    Returns an uppercase copy of the string.

    \code
	QString string("TeXt");
	str = string.toUpper();     // t == "TEXT"
    \endcode

    \sa lower()
*/

QString QString::toUpper() const
{
    int l = d->size;
    if (l) {
	QChar *p = (QChar*)d->data;
	if (p) {
	    while (l) {
		if (*p != ::upper(*p)) {
		    QString s(*this);
		    p = s.data() + (p - (QChar*)d->data);
		    while (l) {
			*p = ::upper(*p);
			l--;
			p++;
		    }
		    return s;
		}
		l--;
		p++;
	    }
	}
    }
    return *this;
}


#ifndef QT_NO_SPRINTF
/*!
    Safely builds a formatted string from the format string \a cformat
    and an arbitrary list of arguments.

    The format string supports most of the conversion specifiers
    provided by printf() in the standard C library. It doesn't honor
    the length modifiers (e.g. \c h for \c short, \c ll for \c{long
    long}). If you need those, use the standard sprintf() function
    instead:

    \code
	char buf[BufSize];
	::sprintf(buf, "%lld", 123456789LL);
	QString str = QString::fromAscii(buf);
    \endcode

    \warning We do not recommend using QString::sprintf() in new Qt
    code. Instead, consider using QTextOStream or arg(), both of
    which support Unicode strings seamlessly and are type-safe.
    Here's an example that uses QTextOStream:

    \code
	QString str;
	QString s = ...;
	int x = ...;
	QTextOStream(&str) << s << " : " << x;
    \endcode

    For \link QObject::tr() translations,\endlink especially if the
    strings contains more than one escape sequence, you should
    consider using the arg() function instead. This allows the order
    of the replacements to be controlled by the translator.

    The %lc escape sequence expects a unicode character of type ushort
    (as returned by QChar::unicode()). The %ls escape sequence expects
    a pointer to a zero-terminated array of unicode characters of type
    ushort (as returned by QString::ucs2()).

    \sa arg()
*/

QString &QString::sprintf( const char* cformat, ... )
{
    QLocale locale(QLocale::C);

    va_list ap;
    va_start( ap, cformat );

    if ( !cformat || !*cformat ) {
	// Qt 1.x compat
	*this = fromLatin1( "" );
	return *this;
    }

    // Parse cformat

    QString result;
    const char *c = cformat;
    for (;;) {
    	// Copy non-escape chars to result
    	while (*c != '\0' && *c != '%')
	    result.append(*c++);

	if (*c == '\0')
	    break;

	// Found '%'
	const char *escape_start = c;
	++c;

	if (*c == '\0') {
	    result.append('%'); // a % at the end of the string - treat as non-escape text
	    break;
	}
    	if (*c == '%') {
	    result.append('%'); // %%
	    ++c;
	    continue;
	}

	// Parse flag characters
	unsigned flags = 0;
	bool no_more_flags = false;
	do {
	    switch (*c) {
		case '#': flags |= QLocalePrivate::Alternate; break;
		case '0': flags |= QLocalePrivate::ZeroPadded; break;
		case '-': flags |= QLocalePrivate::LeftAdjusted; break;
		case ' ': flags |= QLocalePrivate::BlankBeforePositive; break;
		case '+': flags |= QLocalePrivate::AlwaysShowSign; break;
		case '\'': flags |= QLocalePrivate::ThousandsGroup; break;
		default: no_more_flags = true; break;
    	    }

	    if (!no_more_flags)
	    	++c;
	} while (!no_more_flags);

	if (*c == '\0') {
	    result.append(escape_start); // incomplete escape, treat as non-escape text
	    break;
	}

	// Parse field width
	int width = -1; // -1 means unspecified
	if (qIsDigit(*c)) {
	    QString width_str;
	    while (*c != '\0' && qIsDigit(*c))
	    	width_str.append(*c++);

	    // can't be negative - started with a digit
    	    // contains at least one digit
	    width = width_str.toInt();
	}
	else if (*c == '*') {
	    width = va_arg(ap, int);
	    if (width < 0)
	    	width = -1; // treat all negative numbers as unspecified
	    ++c;
	}

	if (*c == '\0') {
	    result.append(escape_start); // incomplete escape, treat as non-escape text
	    break;
	}

	// Parse precision
	int precision = -1; // -1 means unspecified
	if (*c == '.') {
	    ++c;
	    if (qIsDigit(*c)) {
		QString precision_str;
		while (*c != '\0' && qIsDigit(*c))
	    	    precision_str.append(*c++);

		// can't be negative - started with a digit
    	    	// contains at least one digit
		precision = precision_str.toInt();
	    }
	}
	else if (*c == '*') {
	    precision = va_arg(ap, int);
	    if (precision < 0)
	    	precision = -1; // treat all negative numbers as unspecified
	    ++c;
	}

	if (*c == '\0') {
	    result.append(escape_start); // incomplete escape, treat as non-escape text
	    break;
	}

    	// Parse the length modifier
    	enum LengthMod { lm_none, lm_hh, lm_h, lm_l, lm_ll, lm_L, lm_j, lm_z, lm_t };
	LengthMod length_mod = lm_none;
	switch (*c) {
	    case 'h':
	    	++c;
		if (*c == 'h') {
		    length_mod = lm_hh;
		    ++c;
		}
		else
		    length_mod = lm_h;
		break;

	    case 'l':
	    	++c;
		if (*c == 'l') {
		    length_mod = lm_ll;
		    ++c;
		}
		else
		    length_mod = lm_l;
		break;

	    case 'L':
	    	++c;
		length_mod = lm_L;
		break;

	    case 'j':
	    	++c;
		length_mod = lm_j;
		break;

	    case 'z':
	    case 'Z':
	    	++c;
		length_mod = lm_z;
		break;

	    case 't':
	    	++c;
		length_mod = lm_t;
		break;

	    default: break;
	}

	if (*c == '\0') {
	    result.append(escape_start); // incomplete escape, treat as non-escape text
	    break;
	}

	// Parse the conversion specifier and do the conversion
	QString subst;
	switch (*c) {
	    case 'd':
	    case 'i': {
	    	Q_LLONG i;
		switch (length_mod) {
		    case lm_none: i = va_arg(ap, int); break;
		    case lm_hh: i = va_arg(ap, int); break;
		    case lm_h: i = va_arg(ap, int); break;
		    case lm_l: i = va_arg(ap, long int); break;
		    case lm_ll: i = va_arg(ap, Q_LLONG); break;
		    case lm_j: i = va_arg(ap, long int); break;
		    case lm_z: i = va_arg(ap, size_t); break;
		    case lm_t: i = va_arg(ap, int); break;
		    default: i = 0; break;
		}
		subst = locale.d->longLongToString(i, precision, 10, width, flags);
		++c;
		break;
    	    }
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X': {
	    	Q_ULLONG u;
		switch (length_mod) {
		    case lm_none: u = va_arg(ap, unsigned int); break;
		    case lm_hh: u = va_arg(ap, unsigned int); break;
		    case lm_h: u = va_arg(ap, unsigned int); break;
		    case lm_l: u = va_arg(ap, unsigned long int); break;
		    case lm_ll: u = va_arg(ap, Q_ULLONG); break;
		    default: u = 0; break;
		}

		if (qIsUpper(*c))
		    flags |= QLocalePrivate::CapitalEorX;

    	    	int base = 10;
		switch (qToLower(*c)) {
		    case 'o':
	    		base = 8; break;
		    case 'u':
			base = 10; break;
		    case 'x':
			base = 16; break;
		    default: break;
		}
		subst = locale.d->unsLongLongToString(u, precision, base, width, flags);
		++c;
		break;
    	    }
	    case 'E':
	    case 'e':
    	    case 'F':
	    case 'f':
	    case 'G':
	    case 'g':
	    case 'A':
	    case 'a': {
	    	double d;
		if (length_mod == lm_L)
		    d = va_arg(ap, long double); // not supported - converted to a double
		else
		    d = va_arg(ap, double);

		if (qIsUpper(*c))
		    flags |= QLocalePrivate::CapitalEorX;

    	    	QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
		switch (qToLower(*c)) {
		    case 'e': form = QLocalePrivate::DFExponent; break;
		    case 'a': 	    	    	// not supported - decimal form used instead
		    case 'f': form = QLocalePrivate::DFDecimal; break;
		    case 'g': form = QLocalePrivate::DFSignificantDigits; break;
		    default: break;
		}
		subst = locale.d->doubleToString(d, precision, form, width, flags);
		++c;
		break;
	    }
	    case 'c': {
	    	if (length_mod == lm_l)
		    subst = QChar((ushort) va_arg(ap, int));
    	    	else
		    subst = (uchar) va_arg(ap, int);
		++c;
		break;
	    }
	    case 's': {
	    	if (length_mod == lm_l) {
		    const ushort *buff = va_arg(ap, const ushort*);
		    const ushort *ch = buff;
		    while (*ch != 0)
		    	++ch;
		    subst.setUnicodeCodes(buff, ch - buff);
		} else
	    	    subst = va_arg(ap, const char*);
		if (precision != -1)
		    subst.truncate(precision);
		++c;
		break;
	    }
	    case 'p': {
		Q_ULLONG i;
#ifdef Q_OS_WIN64
                i = (Q_ULLONG) va_arg(ap, void*);
#else
                i = (Q_ULONG) va_arg(ap, void*);
#endif
		flags |= QLocalePrivate::Alternate;
		subst = locale.d->unsLongLongToString(i, precision, 16, width, flags);
		++c;
		break;
	    }
	    case 'n':
	    	switch (length_mod) {
		    case lm_hh: {
		    	signed char *n = va_arg(ap, signed char*);
			*n = result.length();
			break;
		    }
		    case lm_h: {
		    	short int *n = va_arg(ap, short int*);
			*n = result.length();
    	    	    	break;
		    }
		    case lm_l: {
		    	long int *n = va_arg(ap, long int*);
			*n = result.length();
			break;
		    }
		    case lm_ll: {
		    	Q_LLONG *n = va_arg(ap, Q_LLONG*);
			*n = result.length();
			break;
		    }
		    default: {
		    	int *n = va_arg(ap, int*);
			*n = result.length();
			break;
		    }
		}
		++c;
		break;

	    default: // bad escape, treat as non-escape text
	    	for (const char *cc = escape_start; cc != c; ++cc)
		    result.append(*cc);
		continue;
	}

	if (flags & QLocalePrivate::LeftAdjusted)
	    result.append(subst.leftJustified(width));
	else
	    result.append(subst.rightJustified(width));
    }

    va_end(ap);
    *this = result;

    return *this;
}
#endif

/*!
    Returns the string converted to a \c {long long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

Q_LLONG QString::toLongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if ( base != 0 && (base < 2 || base > 36) ) {
	qWarning( "QString::toLongLong: Invalid base (%d)", base );
	base = 10;
    }
#endif

    QLocale locale(QLocale::C);
    return locale.d->stringToLongLong(*this, base, ok);
}

/*!
    Returns the string converted to an \c {unsigned long long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

Q_ULLONG QString::toULongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if ( base != 0 && (base < 2 || base > 36) ) {
	qWarning( "QString::toULongLong: Invalid base %d", base );
	base = 10;
    }
#endif

    QLocale locale(QLocale::C);
    return locale.d->stringToUnsLongLong(*this, base, ok);
}

/*!
    Returns the string converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

long QString::toLong(bool *ok, int base) const
{
    Q_LLONG v = toLongLong(ok, base);
    if (v < LONG_MIN || v > LONG_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return long(v);
}

/*!
    Returns the string converted to an \c {unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

ulong QString::toULong(bool *ok, int base) const
{
    Q_ULLONG v = toULongLong(ok, base);
    if (v > ULONG_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return ulong(v);
}

/*!
    Returns the string converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \code
	QString str("FF");
	bool ok;
	int hex = str.toInt(&ok, 16);     // hex == 255, ok == true
	int dec = str.toInt(&ok, 10);     // dec == 0, ok == false
    \endcode

    \sa number()
*/

int QString::toInt(bool *ok, int base) const
{
    long v = toLongLong(ok, base);
    if (v < INT_MIN || v > INT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (int)v;
}

/*!
    Returns the string converted to an \c {unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

uint QString::toUInt(bool *ok, int base) const
{
    ulong v = toULongLong(ok, base);
    if (v > UINT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (uint)v;
}

/*!
    Returns the string converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

short QString::toShort(bool *ok, int base) const
{
    long v = toLongLong(ok, base);
    if (v < SHRT_MIN || v > SHRT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (short)v;
}

/*!
    Returns the string converted to an \c {unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36 or 0. If
    \a base is 0, the base is determined automatically using the
    following rules: If the string begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    FALSE; otherwise \a *ok is set to TRUE.

    \sa number()
*/

ushort QString::toUShort(bool *ok, int base) const
{
    ulong v = toULongLong(ok, base);
    if (v > USHRT_MAX) {
	if (ok)
	    *ok = false;
	v = 0;
    }
    return (ushort)v;
}


/*!
    Returns the string converted to a \c double value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \code
	QString string("1234.56");
	double a = string.toDouble();   // a == 1234.56
    \endcode

    \sa number()
*/

double QString::toDouble(bool *ok) const
{
    char *end;

    const char *a = latin1();
    double val = strtod(a ? a : "", &end);
    if (ok)
	*ok = (a && *a && (end == 0 || (end - a) == (int)length()));
    return val;
}

/*!
    Returns the string converted to a \c float value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \sa number()
*/

float QString::toFloat(bool *ok) const
{
    return (float)toDouble(ok);
}


/*!
    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.

    \code
	QString string;
	string = string.setNum(1234);     // string == "1234"
    \endcode
*/

QString &QString::setNum(Q_LLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->longLongToString(n, -1, base);
    return *this;
}

/*!
    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

QString &QString::setNum(Q_ULLONG n, int base)
{
#if defined(QT_CHECK_RANGE)
    if ( base < 2 || base > 36 ) {
	qWarning( "QString::setNum: Invalid base %d", base );
	base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    *this = locale.d->unsLongLongToString(n, -1, base);
    return *this;
}

/*!
    \fn QString &QString::setNum(long n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum(ulong n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum(int n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum(uint n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum(short n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \fn QString &QString::setNum(ushort n, int base)

    \overload

    Sets the string to the printed value of \a n in base \a base and
    returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.
*/

/*!
    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/

QString &QString::setNum(double n, char f, int prec)
{
    QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
    uint flags = 0;

    if (qIsUpper(f))
    	flags = QLocalePrivate::CapitalEorX;
    f = qToLower(f);

    switch (f) {
	case 'f':
	    form = QLocalePrivate::DFDecimal;
	    break;
	case 'e':
	    form = QLocalePrivate::DFExponent;
	    break;
	case 'g':
	    form = QLocalePrivate::DFSignificantDigits;
	    break;
	default:
#if defined(QT_CHECK_RANGE)
	    qWarning( "QString::setNum: Invalid format char '%c'", f );
#endif
	    break;
    }

    QLocale locale(QLocale::C);
    *this = locale.d->doubleToString(n, prec, form, -1, flags);
    return *this;
}

/*!
    \fn QString &QString::setNum(float n, char f, int prec)

    \overload

    Sets the string to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    string.

    The format \a f can be 'f', 'F', 'e', 'E', 'g' or 'G'. See \link
    #arg-formats arg \endlink() for an explanation of the formats.
*/


/*!
    A convenience function that returns a string equivalent of the
    number \a n to base \a base, which is 10 by default and must be
    between 2 and 36.

    \code
	long a = 63;
	QString str = QString::number(a, 16);             // str == "3f"
	QString str = QString::number(a, 16).upper();     // str == "3F"
    \endcode

    \sa setNum()
*/
QString QString::number(long n, int base)
{
    QString s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number(ulong n, int base)
{
    QString s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number(int n, int base)
{
    QString s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    A convenience factory function that returns a string
    representation of the number \a n to the base \a base, which is 10
    by default and must be between 2 and 36.

    \sa setNum()
*/
QString QString::number(uint n, int base)
{
    QString s;
    s.setNum(n, base);
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number( Q_LLONG n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}

/*!
    \overload

    \sa setNum()
*/
QString QString::number( Q_ULLONG n, int base )
{
    QString s;
    s.setNum( n, base );
    return s;
}


/*!
    \overload

    Argument \a n is formatted according to the \a f format specified,
    which is \c g by default, and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \code
    double d = 12.34;
    QString ds = QString("'E' format, precision 3, gives %1")
		    .arg(d, 0, 'E', 3);
    // ds == "1.234E+001"
    \endcode

    \sa setNum()
   */
QString QString::number(double n, char f, int prec)
{
    QString s;
    s.setNum(n, f, prec);
    return s;
}

struct ArgEscapeData
{
    uint min_escape;	    // lowest escape sequence number
    uint occurrences;	    // number of occurences of the lowest escape
    	    	    	    // sequence number
    uint locale_occurrences; // number of occurences of the lowest escape
    	    	    	    // sequence number which contain 'L'
    uint escape_len;	    // total length of escape sequences which will
    	    	    	    // be replaced
};

static ArgEscapeData findArgEscapes(const QString &s)
{
    const QChar *uc_begin = s.unicode();
    const QChar *uc_end = uc_begin + s.length();

    ArgEscapeData d;

    d.min_escape = 10;
    d.occurrences = 0;
    d.escape_len = 0;
    d.locale_occurrences = 0;

    const QChar *c = uc_begin;
    while (c != uc_end) {
    	while (c != uc_end && c->unicode() != '%')
	    ++c;

	if (c == uc_end || ++c == uc_end)
	    break;

	bool locale_arg = false;
    	if (c->unicode() == 'L') {
	    locale_arg = true;
	    if (++c == uc_end)
		break;
	}

    	if (c->unicode() < '0' || c->unicode() > '9')
	    continue;

	uint escape = c->unicode() - '0';
    	++c;

    	if (escape > d.min_escape)
	    continue;

    	if (escape < d.min_escape) {
	    d.min_escape = escape;
	    d.occurrences = 0;
	    d.escape_len = 0;
	    d.locale_occurrences = 0;
	}

#if QT_VERSION < 0x040000
    	// ### remove preprocessor in Qt 4.0
	/* Since in Qt < 4.0 only the first instance is replaced,
	   escape_len should hold the length of only the first escape
	   sequence */
    	if (d.occurrences == 0)
#endif
    	{
	    ++d.occurrences;
    	    if (locale_arg) {
    	    	++d.locale_occurrences;
		d.escape_len += 3;
	    }
	    else
		d.escape_len += 2;
	}
    }

    return d;
}

static QString replaceArgEscapes(const QString &s, const ArgEscapeData &d, int field_width,
    	    	    	    	    const QString &arg, const QString &larg)
{
    const QChar *uc_begin = s.unicode();
    const QChar *uc_end = uc_begin + s.length();

    int abs_field_width = QABS(field_width);
    uint result_len = s.length()
    	    	    	- d.escape_len
			+ (d.occurrences - d.locale_occurrences)
			    *qMax(abs_field_width, arg.length())
			+ d.locale_occurrences
			    *qMax(abs_field_width, larg.length());

    QString result;
    result.resize(result_len);
    QChar *result_buff = (QChar*) result.unicode();

    QChar *rc = result_buff;
    const QChar *c = uc_begin;
    uint repl_cnt = 0;
    while (c != uc_end) {
    	/* We don't have to check if we run off the end of the string with c,
	   because as long as d.occurrences > 0 we KNOW there are valid escape
	   sequences. */

    	const QChar *text_start = c;

    	while (c->unicode() != '%')
	    ++c;

	const QChar *escape_start = c++;

	bool locale_arg = false;
    	if (c->unicode() == 'L') {
	    locale_arg = true;
	    ++c;
	}

    	if (c->unicode() != '0' + d.min_escape) {
	    memcpy(rc, text_start, (c - text_start)*sizeof(QChar));
	    rc += c - text_start;
	}
	else {
    	    ++c;

	    memcpy(rc, text_start, (escape_start - text_start)*sizeof(QChar));
	    rc += escape_start - text_start;

    	    uint pad_chars;
    	    if (locale_arg)
	    	pad_chars = qMax(abs_field_width, larg.length()) - larg.length();
	    else
	    	pad_chars = qMax(abs_field_width, arg.length()) - arg.length();

    	    if (field_width > 0) { // left padded
		for (uint i = 0; i < pad_chars; ++i)
	    	    (rc++)->unicode() = ' ';
	    }

    	    if (locale_arg) {
	    	memcpy(rc, larg.unicode(), larg.length()*sizeof(QChar));
		rc += larg.length();
	    }
	    else {
	    	memcpy(rc, arg.unicode(), arg.length()*sizeof(QChar));
	    	rc += arg.length();
	    }

    	    if (field_width < 0) { // right padded
		for (uint i = 0; i < pad_chars; ++i)
	    	    (rc++)->unicode() = ' ';
	    }

	    if (++repl_cnt == d.occurrences) {
		memcpy(rc, c, (uc_end - c)*sizeof(QChar));
		rc += uc_end - c;
		Q_ASSERT(rc - result_buff == (int)result_len);
		c = uc_end;
	    }
	}
    }

    return result;
}

/*!
    This function will return a string that replaces the lowest
    numbered occurrence of \c %1, \c %2, ..., \c %9 with \a a.

    The \a fieldWidth value specifies the minimum amount of space that
    \a a is padded to. A positive value will produce right-aligned
    text, whereas a negative value will produce left-aligned text.

    The following example shows how we could create a 'status' string
    when processing a list of files:
    \code
    QString status = QString( "Processing file %1 of %2: %3" )
			.arg( i )         // current file's number
			.arg( total )     // number of files to process
			.arg( fileName ); // current file's name
    \endcode

    It is generally fine to use filenames and numbers as we have done
    in the example above. But note that using arg() to construct
    natural language sentences does not usually translate well into
    other languages because sentence structure and word order often
    differ between languages.

    If there is no place marker (\c %1, \c %2, etc.), a warning
    message (qWarning()) is output and the result is undefined.
*/
QString QString::arg( const QString& a, int fieldWidth ) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning( "QString::arg(): Argument missing: %s, %s", latin1(),
                  a.latin1() );
        return *this;
    }

    return replaceArgEscapes(*this, d, fieldWidth, a, a);
}


/*!
    \fn QString QString::arg(const QString& a1, const QString& a2) const
    \overload

    This is the same as str.arg(\a a1).arg(\a a2), except that
    the strings are replaced in one pass. This can make a difference
    if \a a1 contains e.g. \c{%1}:

    \code
    QString str("%1 %2");
    str.arg("Hello", "world");        // returns "Hello world"
    str.arg("Hello").arg("world");  // returns "Hello world"

    str.arg("(%1)", "Hello");           // returns "(%1) Hello"
    str.arg("(%1)").arg("Hello");     // returns "(Hello) %2"
    \endcode
*/

/*!
    \fn QString QString::arg(const QString& a1, const QString& a2,
			      const QString& a3) const
    \overload

    This is the same as calling str.arg(\a a1).arg(\a a2).arg(\a a3),
    except that the strings are replaced in one pass.
*/

/*!
    \fn QString QString::arg(const QString& a1, const QString& a2,
			      const QString& a3, const QString& a4) const
    \overload

    This is the same as calling
    str.arg(\a a1).arg(\a a2).arg(\a a3).arg(\a a4),
    except that the strings are replaced in one pass.
*/

/*!
    \overload

    The \a fieldWidth value specifies the minimum amount of space that
    \a a is padded to. A positive value will produce a right-aligned
    number, whereas a negative value will produce a left-aligned
    number.

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.

    The '%' can be followed by an 'L', in which case the sequence is
    replaced with a localized representation of \a a. The conversion uses
    the default locale, set by QLocale::setDefaultLocale(). If no default
    locale was specified, the "C" locale is used. The 'L' flag is ignored
    if \a base is not 10.

    \code
	QString str;
	str = QString("Decimal 63 is %1 in hexadecimal")
		.arg(63, 0, 16);
	// str == "Decimal 63 is 3f in hexadecimal"

	QLocale::setDefaultLocale(QLocale::English, QLocale::UnitedStates);
	str = QString( "%1 %l2 %l3" )
		.arg( 12345 )
		.arg( 12345 )
		.arg( 12345, 0, 16 );
	// str == "12345 12,345 3039"
    \endcode
*/
QString QString::arg( Q_LLONG a, int fieldWidth, int base ) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning( "QString::arg(): Argument missing: %s, %lld", latin1(),
                  a );
        return *this;
    }

    QString arg;
    if (d.occurrences > d.locale_occurrences)
    	arg = number(a, base);

    QString locale_arg;
    if (d.locale_occurrences > 0) {
	QLocale locale(QLocale::DefaultLanguage);
	locale_arg = locale.d->longLongToString(a, -1, base, -1, QLocalePrivate::ThousandsGroup);
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg);
}

/*!
    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36. If \a base is 10, the '%l' syntax can be used
    to produce localized strings.

*/
QString QString::arg( Q_ULLONG a, int fieldWidth, int base ) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning( "QString::arg(): Argument missing: %s, %llu", latin1(),
                  a );
        return *this;
    }

    QString arg;
    if (d.occurrences > d.locale_occurrences)
    	arg = number(a, base);

    QString locale_arg;
    if (d.locale_occurrences > 0) {
	QLocale locale(QLocale::DefaultLanguage);
	locale_arg = locale.d->unsLongLongToString(a, -1, base, -1, QLocalePrivate::ThousandsGroup);
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg);
}

/*!
    \fn QString QString::arg(int a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36. If \a base is 10, the '%l' syntax can be used
    to produce localized strings.
*/

/*!
    \fn QString QString::arg(uint a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36. If \a base is 10, the '%l' syntax can be used
    to produce localized strings.
*/

/*!
    \fn QString QString::arg(short a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36. If \a base is 10, the '%l' syntax can be used
    to produce localized strings.
*/

/*!
    \fn QString QString::arg(ushort a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36. If \a base is 10, the '%l' syntax can be used
    to produce localized strings.
*/


/*!
    \overload

    \a a is assumed to be in the Latin1 character set.
*/
QString QString::arg(char a, int fieldWidth) const
{
    QString c;
    c += a;
    return arg(c, fieldWidth);
}

/*!
    \overload
*/
QString QString::arg(QChar a, int fieldWidth) const
{
    QString c;
    c += a;
    return arg(c, fieldWidth);
}

/*!
    \overload

    \target arg-formats

    Argument \a a is formatted according to the \a fmt format specified,
    which is 'g' by default and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \code
	double d = 12.34;
	QString ds = QString("'E' format, precision 3, gives %1")
			.arg(d, 0, 'E', 3);
	// ds == "1.234E+001"
    \endcode

    The '%l' syntax can be used to produce localized strings.
*/
QString QString::arg( double a, int fieldWidth, char fmt, int prec ) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning( "QString::arg(): Argument missing: %s, %g", latin1(),
                  a );
        return *this;
    }

    QString arg;
    if (d.occurrences > d.locale_occurrences)
    	arg = number(a, fmt, prec);

    QString locale_arg;
    if (d.locale_occurrences > 0) {
	QLocale locale(QLocale::DefaultLanguage);

	QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
	uint flags = 0;

	if (qIsUpper(fmt))
    	    flags = QLocalePrivate::CapitalEorX;
	fmt = qToLower(fmt);

	switch (fmt) {
	    case 'f':
		form = QLocalePrivate::DFDecimal;
		break;
	    case 'e':
		form = QLocalePrivate::DFExponent;
		break;
	    case 'g':
		form = QLocalePrivate::DFSignificantDigits;
		break;
	    default:
#if defined(QT_CHECK_RANGE)
		qWarning( "QString::setNum: Invalid format char '%c'", fmt );
#endif
		break;
	}

    	flags |= QLocalePrivate::ThousandsGroup;

	locale_arg = locale.d->doubleToString(a, prec, form, -1, flags);
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg);
}


QString QString::multiArg(int numArgs, const QString& a1, const QString& a2,
			   const QString& a3, const QString& a4) const
{
    QString result;
    union {
	int digitUsed[10];
	int argForDigit[10];
    };
    const QChar *uc = (const QChar*) d->data;
    const QString *args[4];
    const int len = d->size;
    const int end = len - 1;
    int lastDigit = -1;
    int i;

    memset(digitUsed, 0, sizeof(digitUsed));
    args[0] = &a1;
    args[1] = &a2;
    args[2] = &a3;
    args[3] = &a4;

    for (i = 0; i < end; i++) {
	if (uc[i] == '%') {
	    int digit = uc[i + 1].unicode() - '0';
	    if (digit >= 0 && digit <= 9)
		digitUsed[digit]++;
	}
    }

    for (i = 0; i < numArgs; i++) {
	do {
	    ++lastDigit;
	} while (lastDigit < 10 && digitUsed[lastDigit] == 0);

	if (lastDigit == 10) {
	    qWarning("QString::arg(): Argument missing: %s, %s",
		      latin1(), args[i]->latin1());
	    numArgs = i;
	    lastDigit = 9;
	    break;
	}
	argForDigit[lastDigit] = i;
    }

    i = 0;
    while (i < len) {
	if (uc[i] == '%' && i != end) {
	    int digit = uc[i + 1].unicode() - '0';
	    if (digit >= 0 && digit <= lastDigit) {
		result += *args[argForDigit[digit]];
		i += 2;
		continue;
	    }
	}
	result += uc[i++];
    }
    return result;
}



/*! \internal
 */
void QString::updateProperties() const
{
    unsigned short *p = d->data;
    unsigned short *end = p + d->size;
    d->simpletext = true;
    while ( p < end ) {
	ushort uc = *p;
	// sort out regions of complex text formatting
	if ( uc > 0x058f && ( uc < 0x1100 || uc > 0xfb0f ) ) {
	    d->simpletext = false;
	}
	p++;
    }

    p = d->data;
    d->righttoleft = false;
    while ( p < end ) {
	switch( ::direction( *p ) )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    goto end;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    d->righttoleft = true;
	    goto end;
	default:
	    break;
	}
	++p;
    }
 end:
    d->clean = true;
    return;
}

/*! \fn bool QString::simpleText() const
  \internal
*/

/*! \fn bool QString::isRightToLeft() const
  \internal
 */


/*!
  \class QConstString qstring.h
  \reentrant
  \ingroup text
  \brief The QConstString class provides string objects using constant Unicode data.

    In order to minimize copying, highly optimized applications can
    use QConstString to provide a QString-compatible object from
    existing Unicode data. It is then the programmer's responsibility
    to ensure that the Unicode data exists for the entire lifetime of
    the QConstString object.

    A QConstString is created with the QConstString constructor. The
    string held by the object can be obtained by calling string().
*/

/*!
    Constructs a QConstString that uses the first \a length Unicode
    characters in the array \a unicode. Any attempt to modify copies
    of the string will cause it to create a copy of the data, thus it
    remains forever unmodified.

    The data in \a unicode is not copied. The caller must be able to
    guarantee that \a unicode will not be deleted or modified.
*/

QConstString::QConstString(const QChar *unicode, int length)
    : QString((Data *)qMalloc(sizeof(Data)))
{
    d->ref = 1;
    d->alloc = d->size = length;
    d->c = 0;
    d->data = unicode ? (unsigned short *)unicode : d->array;
    *d->array = 0;
    d->clean = d->encoding = d->cache = d->simpletext = d->righttoleft = 0;
}

/*!
    \fn const QString& QConstString::string() const

    Returns a constant string referencing the data passed during
    construction.
*/



#ifndef QT_NO_DATASTREAM
/*!
    \relates QString

    Writes the string \a str to the stream \a s.

    See also \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QString &str )
{
    if ( s.version() == 1 ) {
	QByteArray l( str.latin1() );
	s << l;
    }
    else {
	int byteOrder = s.byteOrder();
	const QChar* ub = str.unicode();
	if ( !str.isNull() || s.version() < 3 ) {
	    static const uint auto_size = 1024;
	    char t[auto_size];
	    char *b;
	    if ( str.length()*sizeof(QChar) > auto_size ) {
		b = new char[str.length()*sizeof(QChar)];
	    } else {
		b = t;
	    }
	    int l = str.length();
	    char *c=b;
	    while ( l-- ) {
		if ( byteOrder == QDataStream::BigEndian ) {
		    *c++ = (char)ub->row();
		    *c++ = (char)ub->cell();
		} else {
		    *c++ = (char)ub->cell();
		    *c++ = (char)ub->row();
		}
		ub++;
	    }
	    s.writeBytes( b, sizeof(QChar)*str.length() );
	    if ( str.length()*sizeof(QChar) > auto_size )
		delete [] b;
	} else {
	    // write null marker
	    s << (Q_UINT32)0xffffffff;
	}
    }
    return s;
}

/*!
    \relates QString

    Reads a string from the stream \a s into string \a str.

    See also \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QString &str )
{
#ifdef QT_QSTRING_UCS_4
#if defined(Q_CC_GNU)
#warning "operator>> not working properly"
#endif
#endif
    if ( s.version() == 1 ) {
	QByteArray l;
	s >> l;
	str = QString( l );
    }
    else {
	Q_UINT32 bytes;
	s >> bytes;                                     // read size of string
	if ( bytes == 0xffffffff ) {                    // null string
	    str = QString::null;
	} else if ( bytes > 0 ) {                       // not empty
	    int byteOrder = s.byteOrder();
	    str.resize( bytes/2 );
	    QChar* ch = str.data();
	    static const uint auto_size = 1024;
	    char t[auto_size];
	    char *b;
	    if ( bytes > auto_size ) {
		b = new char[bytes];
	    } else {
		b = t;
	    }
	    s.readRawBytes( b, bytes );
	    int bt = bytes/2;
	    char *oldb = b;
	    while ( bt-- ) {
		if ( byteOrder == QDataStream::BigEndian )
		    *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
		else
		    *ch++ = (ushort) (((ushort)b[1])<<8) | (uchar)b[0];
		b += 2;
	    }
	    if ( bytes > auto_size )
		delete [] oldb;
	} else {
	    str = "";
	}
    }
    return s;
}
#endif // QT_NO_DATASTREAM
