
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
// ######
#include <qptrlist.h>

#include "qtools_p.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <new>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#ifndef LLONG_MAX
#define LLONG_MAX Q_INT64_C(9223372036854775807)
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - Q_INT64_C(1))
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX Q_UINT64_C(18446744073709551615)
#endif

#ifdef CLEAR_ASCII_CACHE
#undef CLEAR_ASCII_CACHE
#endif
#define CLEAR_ASCII_CACHE(d) if (d->c){((QByteArray*)&d->c)->~QByteArray();d->c=0;d->dirty=true;}

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
    int l=QMIN(as.length(),bs.length());
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

QString::Null QString::null;


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
static unsigned short null_char = 0;
QString::Data QString::shared_null = { Q_ATOMIC_INIT(1), 0, 0, 0, &null_char, 0, 0, 0, 0, 0, {0} };
QString::Data QString::shared_empty = { Q_ATOMIC_INIT(1), 0, 0, 0, &null_char, 0, 0, 0, 0, 0, {0} };

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
    Latin-1 \c{const char *} to QString preserves all the
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
	d->encoding = d->simpletext = d->righttoleft = 0;
	d->dirty = 1;
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
	d->encoding = d->simpletext = d->righttoleft = 0;
	d->dirty = 1;
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
	d->encoding = d->simpletext = d->righttoleft = 0;
	d->dirty = 1;
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
    d->encoding = d->simpletext = d->righttoleft = 0;
    d->dirty = 1;
    d->data = d->array;
    d->array[0] = c.unicode();
    d->array[1] = '\0';
}

void QString::free(Data *d)
{
    CLEAR_ASCII_CACHE(d)
    qFree(d);
}

/*!
    Ensures that at least \a size characters are allocated to the
    string, and sets the length of the string to \a size. Any new
    space allocated contains arbitrary data.

    \sa reserve(), truncate()
*/

void QString::resize(int size)
{ CLEAR_ASCII_CACHE(d)
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


/*
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
	Data *x =  (Data*) qMalloc(sizeof(Data)+alloc*sizeof(QChar));
	*x = *d;
	x->data = x->array;
	::memcpy(x->data, d->data, QMIN(alloc, d->alloc)*sizeof(QChar));
	x->c = 0;
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
    resize(QMAX(i+1, d->size));
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
    \fn QString& QString::operator=(const std::string& s)

    \overload

    Makes a deep copy of \a s and returns a reference to the deep
    copy.
*/

/*!
  \fn QString &QString::operator=(const QString & a)
    \overload

    Assigns a shallow copy of \a s to this string and returns a
    reference to this string. This is very fast because the string
    isn't actually copied.
*/

QString &QString::operator=(const char *s)
{
    CLEAR_ASCII_CACHE(d)
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
	x->encoding = x->simpletext = x->righttoleft = 0;
	x->dirty = 1;
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
    Inserts \a c into the string at position \a i and returns a
    reference to the string.

    If \a i is beyond the end of the string, the string is extended
    with spaces (ASCII 32) to length \a i and \a c is then
    appended.
*/

QString& QString::insert(int i, QChar c)
{ CLEAR_ASCII_CACHE(d)
    if (i < 0)
	i += d->size;
    if (i < 0)
	return *this;
   expand(QMAX(i, d->size));
   ::memmove(d->data + i + 1, d->data + i, (d->size - i)*sizeof(QChar));
   d->data[i] = c.unicode();
   return *this;
}

/*!
    Inserts \a s into the string at position \a i and returns a
    reference to the string.

    If \a i is beyond the end of the string, the string is extended
    with spaces (ASCII 32) to length \a i and \a c is then
    appended.

    \code
	QString s("I like fish");
	s.insert(2, "don't ");
	// s == "I don't like fish"
    \endcode

*/
QString& QString::insert(int i, const QString& s)
{ CLEAR_ASCII_CACHE(d)
    if (i < 0 || s.d->size == 0)
	return *this;
    // protect against s == *this
    QString t = s;
    expand(QMAX(d->size, i) + t.d->size - 1);
    ::memmove(d->data + i + t.d->size, d->data + i, (d->size - i - t.d->size)*sizeof(QChar));
    memcpy(d->data + i, t.d->data, t.d->size*sizeof(QChar));
    return *this;
}

/*!
    Appends \a c to the string and returns a reference to the string.
*/
QString& QString::append(QChar c)
{ CLEAR_ASCII_CACHE(d)
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
{ CLEAR_ASCII_CACHE(d)
    if (s) {
#ifndef QT_NO_TEXTCODEC
	if (QTextCodec::codecForCStrings())
	    return append(QString(s));
#endif
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
{ CLEAR_ASCII_CACHE(d)
    if (s.d != &shared_null) {
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
    the string, the string is truncated at position \a iq.

    \code
	QString string("Montreal");
	string.remove(1, 4);      // string == "Meal"
    \endcode

    \sa insert(), replace()
*/

QString &QString::remove(int i, int len)
{ CLEAR_ASCII_CACHE(d)
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
{ CLEAR_ASCII_CACHE(d)
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
{ CLEAR_ASCII_CACHE(d)
    if (s.d->size) {
	int i = 0;
	while ((i = find(s, i, cs)) != -1)
	    remove(i, s.d->size);
    }
    return *this;
}

/*! \overload

    This is the same as replace(\a index, \a len, QString(\a after)).
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
QString& QString::replace(int i, int len, const QString& after)
{
    remove(i, len);
    return insert(i, after);
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
{ CLEAR_ASCII_CACHE(d)
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
	from = QMAX(from + d->size, 0);
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
		if (skiptable[::lower((current - skip)->cell())] == pl)
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

    if (sl == 1)
	return find(*(const QChar*) s.d->data, from, cs);

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

#ifndef MACOSX_101
    if (sl == 1)
	return findRev(*(const QChar*) s.d->data, from, cs);
#endif

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
    CLEAR_ASCII_CACHE(d)
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

    CLEAR_ASCII_CACHE(d)
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
    while ((i = find(s, i+1, cs)) != -1)
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
/*! \overload
    Finds the first match of the regular expression \a rx, starting
    from position \a from. If \a i is -1, the search starts at
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
    \overload

    Finds the first match of the regexp \a rx, starting at position \a
    from and searching backwards. If the index is -1, the search
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
    \overload

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
    QStringList sections = QStringList::split(sep, *this, TRUE);
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
		    if(!ret.isEmpty())
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
#if 0
QString QString::section( const QRegExp &reg, int start, int end, int flags ) const
{
    const QChar *uc = unicode();
    if(!uc)
	return QString();

    QRegExp sep(reg);
    sep.setCaseSensitive(!(flags & SectionCaseInsensitiveSeps));

    QList<section_chunk *> l;
    l.setAutoDelete(true);
    int n = length(), m = 0, last_m = 0, last = 0, last_len = 0;

    while ( ( m = sep.search( *this, m ) ) != -1 ) {
	l.append(new section_chunk(last_len, QString(uc + last_m, m - last_m)));
	last_m = m;
	last_len = sep.matchedLength();
	if((m += sep.matchedLength()) >= n) {
	    last = 1;
	    break;
	}
    }
    if(!last)
	l.append(new section_chunk(last_len, QString(uc + last_m, n - last_m)));

    if(start < 0)
	start = l.count() + start;
    if(end == -1)
	end = l.count();
    else if(end < 0)
	end = l.count() + end;

    int i = 0;
    QString ret;
    QList<section_chunk *>::iterator it = l.begin();
    for ( ; it != l.end(); ++it, i++ ) {
	if((flags & SectionSkipEmpty) && (*it)->length == (int)(*it)->string.length()) {
	    if(i <= start)
		start++;
	    end++;
	}
	if(i == start) {
	    ret = (flags & SectionIncludeLeadingSep) ? (*it)->string : (*it)->string.mid((*it)->length);
	} else if(i > start) {
	    ret += (*it)->string;
	}
	if(i == end) {
	    ++it;
	    if(it != l.end() && flags & SectionIncludeTrailingSep)
		ret += (*it)->string.left((*it)->length);
	    break;
	}
    }
    return ret;
}
#else

QString QString::section( const QRegExp &reg, int start, int end, int flags ) const
{
    const QChar *uc = unicode();
    if(!uc)
	return QString();

    QRegExp sep(reg);
    sep.setCaseSensitive(!(flags & SectionCaseInsensitiveSeps));

    QPtrList<section_chunk> l;
    l.setAutoDelete(true);
    int n = length(), m = 0, last_m = 0, last = 0, last_len = 0;

    while ( ( m = sep.search( *this, m ) ) != -1 ) {
	l.append(new section_chunk(last_len, QString(uc + last_m, m - last_m)));
	last_m = m;
	last_len = sep.matchedLength();
	if((m += sep.matchedLength()) >= n) {
	    last = 1;
	    break;
	}
    }
    if(!last)
	l.append(new section_chunk(last_len, QString(uc + last_m, n - last_m)));

    if(start < 0)
	start = l.count() + start;
    if(end == -1)
	end = l.count();
    else if(end < 0)
	end = l.count() + end;

    int i = 0;
    QString ret;
    for ( section_chunk *chk=l.first(); chk; chk=l.next(), i++ ) {
	if((flags & SectionSkipEmpty) && chk->length == (int)chk->string.length()) {
	    if(i <= start)
		start++;
	    end++;
	}
	if(i == start) {
	    ret = (flags & SectionIncludeLeadingSep) ? chk->string : chk->string.mid(chk->length);
	} else if(i > start) {
	    ret += chk->string;
	}
	if(i == end) {
	    if((chk=l.next()) && flags & SectionIncludeTrailingSep)
		ret += chk->string.left(chk->length);
	    break;
	}
    }
    return ret;
}
#endif
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

    Returns a Latin-1 representation of the string. The
    returned value is undefined if the string contains non-Latin-1
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
    Returns a Latin-1 representation of the string as QByteArray. The
    returned value is undefined if the string contains non-Latin-1
    characters. If you want to convert strings into formats other than
    Unicode, see the QTextCodec classes.

    \sa fromLatin1(), latin1(), toAscii(), toUtf8(), toLocal8Bit()
*/
QByteArray QString::toLatin1() const
{
    QByteArray *ba = (QByteArray*) &d->c;
    if (!d->c || d->encoding != Data::Latin1) {
	if (!d->c)
	    new (&d->c) QByteArray;
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

QByteArray QString::toAscii() const
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForCStrings() ) {
	QByteArray *ba = (QByteArray*) &d->c;
	if (!d->c || d->encoding != Data::Latin1) {
	    if (!d->c)
		new (&d->c) QByteArray;
	    d->encoding = Data::Ascii;
	    *ba = QTextCodec::codecForCStrings()->fromUnicode( *this );
	}
	return *ba;
    }
#endif // QT_NO_TEXTCODEC
    return toLatin1();
}

QByteArray QString::toLocal8Bit() const
{
#ifndef QT_NO_TEXTCODEC
    if ( QTextCodec::codecForLocale() ) {
	QByteArray *ba = (QByteArray*) &d->c;
	if (!d->c || d->encoding != Data::Local8Bit) {
	    if (!d->c)
		new (&d->c) QByteArray;
	    d->encoding = Data::Local8Bit;
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
    if (!d->c || d->encoding != Data::Utf8) {
	if (!d->c)
	    new (&d->c) QByteArray;
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
			    *cursor++ = 0xf0 | ((uchar) (u >> 18));
			    *cursor++ = 0x80 | ( ((uchar) (u >> 12)) & 0x3f);
			} else {
			    *cursor++ = 0xe0 | ((uchar) (u >> 12));
			}
			*cursor++ = 0x80 | ( ((uchar) (u >> 6)) & 0x3f);
		    }
		    *cursor++ = 0x80 | ((uchar) (u&0x3f));
		}
		ch++;
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
	d->encoding = d->simpletext = d->righttoleft = 0;
	d->dirty = 1;
	d->data = d->array;
	ushort *i = d->data;
	while(len--)
	   * i++ = (uchar)*s++;
    }
    return QString(d);
}

#ifdef Q_OS_WIN32

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
	    qWarning("WideCharToMultiByte cannot convert multibyte text (error %d): %s (UTF8)",
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
    characters of \a local8Bit, ignoring the rest of \a local8Bit. If
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
#ifdef QT_NO_TEXTCODEC
    return fromLatin1( local8Bit, len );
#else

    if ( !local8Bit )
	return QString::null;
#ifdef Q_WS_X11
    QTextCodec* codec = QTextCodec::codecForLocale();
    if ( len < 0 )
	len = strlen( local8Bit );
    return codec
	    ? codec->toUnicode( local8Bit, len )
	    : fromLatin1( local8Bit, len );
#endif
#if defined( Q_WS_MAC )
    return fromUtf8(local8Bit,len);
#endif
// Should this be OS_WIN32?
#ifdef Q_WS_WIN
    if ( len >= 0 ) {
	QByteArray s(local8Bit,len+1);
	return qt_winMB2QString(s);
    }
    return qt_winMB2QString( local8Bit );
#endif
#ifdef Q_WS_QWS
    return fromUtf8(local8Bit,len);
#endif
#endif // QT_NO_TEXTCODEC
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
    characters of \a utf8, ignoring the rest of \a utf8. If \a len is
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
    result.setLength( len ); // worst case
    unsigned short *qch = result.d->data;
    uint uc = 0;
    int need = 0;
    uchar ch;
    for (int i=0; i<len; i++) {
	ch = *utf8++;
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
			*qch++ = QChar(high);
			*qch++ = QChar(low);
		    } else {
			*qch++ = uc;
		    }
		}
	    } else {
		// error
		*qch++ = QChar::replacement;
		need = 0;
	    }
	} else {
	    if ( ch < 128 ) {
		*qch++ = ch;
	    } else if ((ch & 0xe0) == 0xc0) {
		uc = ch & 0x1f;
		need = 1;
	    } else if ((ch & 0xf0) == 0xe0) {
		uc = ch & 0x0f;
		need = 2;
	    } else if ((ch&0xf8) == 0xf0) {
		uc = ch & 0x07;
		need = 3;
	    }
	}
    }
    result.truncate( qch - result.d->data );
    return result;
}

/*!
  Constructs a string that is a deep copy of \a str, interpreted as a
  UCS2 encoded, zero terminated, Unicode string.

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
    Resizes the string to \a len characters and copies \a
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
    Fills the string with \a len characters of value \a c, and returns
    a reference to the string.

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
	int c = QString::compare(" abc", "abc");   // c == 0
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

#if defined(Q_WS_WIN)
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
#elif defined(Q_WS_X11)
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

/*!
    Safely builds a formatted string from the format string \a cformat
    and an arbitrary list of arguments. The format string supports all
    the escape sequences of printf() in the standard C library.

    The %s escape sequence expects a utf8() encoded string. The format
    string \e cformat is expected to be in latin1. If you need a
    Unicode format string, use arg() instead. For typesafe string
    building, with full Unicode support, you can use QTextOStream like
    this:

    \code
	QString str;
	QString s = ...;
	int x = ...;
	QTextOStream(&str) << s << " : " << x;
    \endcode

    For \link QObject::tr() translations,\endlink especially if the
    strings contains more than one escape sequence, you should
    consider using the arg() function instead. This allows the order
    of the replacements to be controlled by the translator, and has
    Unicode support.

    \sa arg()
*/

#ifndef QT_NO_SPRINTF
QString &QString::sprintf(const char * cformat, ...)
{
    va_list ap;
    va_start(ap, cformat);

    if (!cformat || !*cformat) {
	// Qt 1.x compat
	*this = fromLatin1("");
	return *this;
    }
    QString format = fromAscii(cformat);

    QRegExp escape("%#?0?-? ?\\+?'?[0-9*]*\\.?[0-9*]*h?l?L?q?Z?");
    QString result;
    int last = 0;
    int pos;
    int len = 0;

    for (;;) {
	pos = escape.search(format, last);
	len = escape.matchedLength();
	// Non-escaped text
	if (pos > (int)last)
	    result += format.mid(last, pos - last);
	if (pos < 0) {
	    // The rest
	    if (last < format.length())
		result += format.mid(last);
	    break;
	}
	last = pos + len + 1;

	// Escape
	QString f = format.mid(pos, len);
	int width, decimals;
	int params = 0;
	int wpos = f.find('*');
	if (wpos >= 0) {
	    params++;
	    width = va_arg(ap, int);
	    if (f.find('*', wpos + 1) >= 0) {
		decimals = va_arg(ap, int);
		params++;
	    } else {
		decimals = 0;
	    }
	} else {
	    decimals = width = 0;
	}
	QString replacement;
	if (format[pos + len] == 's' || format[pos + len] == 'S' ||
	     format[pos + len] == 'c')
	{
	    bool rightjust = (f.find('-') < 0);
	    // %-5s really means left adjust in sprintf

	    if (wpos < 0) {
		QRegExp num(fromLatin1("[0-9]+"));
		int p = num.search(f);
		int nlen = num.matchedLength();
		int q = f.find('.');
		if (q < 0 || (p < q && p >= 0))
		    width = f.mid(p, nlen).toInt();
		if (q >= 0) {
		    p = num.search(f, q);
		    // "decimals" is used to specify string truncation
		    if (p >= 0)
			decimals = f.mid(p, nlen).toInt();
		}
	    }

	    if (format[pos + len] == 's') {
		QString s = QString::fromUtf8(va_arg(ap, char*));
		replacement = (decimals <= 0) ? s : s.left(decimals);
	    } else {
		int ch = va_arg(ap, int);
		replacement = QChar((ushort)ch);
	    }
	    if (replacement.length() < width) {
		replacement = rightjust
		    ? replacement.rightJustify(width)
		    : replacement.leftJustify(width);
	    }
	} else if (format[pos+len] == '%') {
	    replacement = '%';
	} else if (format[pos+len] == 'n') {
	    int *n = va_arg(ap, int*);
	   * n = result.length();
	} else {
	    char in[64], out[330];
	    strncpy(in,f.latin1(),63);
	    out[0] = '\0';
	    char fch = format[pos+len].latin1();
	    in[f.length()] = fch;
	    switch (fch) {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
		{
		    int value = va_arg(ap, int);
		    switch (params) {
		    case 0:
			::sprintf(out, in, value);
			break;
		    case 1:
			::sprintf(out, in, width, value);
			break;
		    case 2:
			::sprintf(out, in, width, decimals, value);
		    }
		}
		break;
	    case 'e':
	    case 'E':
	    case 'f':
	    case 'g':
	    case 'G':
		{
		    double value = va_arg(ap, double);
		    switch (params) {
		    case 0:
			::sprintf(out, in, value);
			break;
		    case 1:
			::sprintf(out, in, width, value);
			break;
		    case 2:
			::sprintf(out, in, width, decimals, value);
		    }
		}
		break;
	    case 'p':
		{
		    void *value = va_arg(ap, void *);
		    switch (params) {
		    case 0:
			::sprintf(out, in, value);
			break;
		    case 1:
			::sprintf(out, in, width, value);
			break;
		    case 2:
			::sprintf(out, in, width, decimals, value);
		    }
		}
	    }
	    replacement = fromAscii(out);
	}
	result += replacement;
    }
   * this = result;

    va_end(ap);
    return *this;
}
#endif

static bool ok_in_base(QChar c, int base)
{
    if (base <= 10)
	return c.isDigit() && c.digitValue() < base;
    else
	return c.isDigit() || (c >= 'a' && c < char('a'+base-10))
			   || (c >= 'A' && c < char('A'+base-10));
}

/*!
    Returns the string converted to a \c long value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \sa number()
*/

Q_LLONG QString::toLongLong(bool *ok, int base) const
{
    const QChar *p = unicode();
    Q_LLONG val = 0;
    int l = length();
    const Q_LLONG max_mult = LLONG_MAX / base;
    bool is_ok = false;
    int neg = 0;
    if (!p)
	goto bye;
    while (l && p->isSpace())                 // skip leading space
	l--,p++;
    if (!l)
	goto bye;
    if (*p == '-') {
	l--;
	p++;
	neg = 1;
    } else if (*p == '+') {
	l--;
	p++;
    }

    // NOTE: toULongLong() code is similar
    if (!l || !ok_in_base(*p,base))
	goto bye;
    while (l && ok_in_base(*p,base)) {
	l--;
	int dv;
	if (p->isDigit()) {
	    dv = p->digitValue();
	} else {
	    if (*p >= 'a' && *p <= 'z')
		dv = *p - 'a' + 10;
	    else
		dv = *p - 'A' + 10;
	}
	if (val > max_mult ||
	    (val == max_mult && dv > (LLONG_MAX % base) + neg))
	    goto bye;
	val = base * val + dv;
	p++;
    }
    if (neg)
	val = -val;
    while (l && p->isSpace())                 // skip trailing space
	l--,p++;
    if (!l)
	is_ok = true;
bye:
    if (ok)
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to an \c {unsigned long} value to the
    base \a base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

    \sa number()
*/

Q_ULLONG QString::toULongLong(bool *ok, int base) const
{
    const QChar *p = unicode();
    Q_ULLONG val = 0;
    int l = length();
    const Q_ULLONG max_mult = ULLONG_MAX / base;
    bool is_ok = false;
    if (!p)
	goto bye;
    while (l && p->isSpace())                 // skip leading space
	l--,p++;
    if (!l)
	goto bye;
    if (*p == '+')
	l--,p++;

    // NOTE: toLongLong() code is similar
    if (!l || !ok_in_base(*p,base))
	goto bye;
    while (l && ok_in_base(*p,base)) {
	l--;
	uint dv;
	if (p->isDigit()) {
	    dv = p->digitValue();
	} else {
	    if (*p >= 'a' && *p <= 'z')
		dv = *p - 'a' + 10;
	    else
		dv = *p - 'A' + 10;
	}
	if (val > max_mult || (val == max_mult && dv > ULLONG_MAX % base))
	    goto bye;
	val = base * val + dv;
	p++;
    }

    while (l && p->isSpace())                 // skip trailing space
	l--,p++;
    if (!l)
	is_ok = true;
bye:
    if (ok)
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
    Returns the string converted to a \c long value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

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
    Returns the string converted to an \c {unsigned long} value to the
    base \a base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

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
    Returns the string converted to an \c int value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

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
    Returns the string converted to an \c{unsigned int} value to the
    base \a base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.

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
    Returns the string converted to a \c short value to the base \a
    base, which is 10 by default and must be between 2 and 36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.
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
    Returns the string converted to an \c {unsigned short} value to
    the base \a base, which is 10 by default and must be between 2 and
    36.

    If \a ok is not 0: if a conversion error occurs, \a *ok is set to
    false; otherwise \a *ok is set to true.
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
    if (base < 2 || base > 36) {
	qWarning("QString::setNum: Invalid base %d", base);
	base = 10;
    }
#endif
    char   charbuf[65*sizeof(QChar)];
    QChar *buf = (QChar*)charbuf;
    QChar *p = &buf[64];
    int  len = 0;
    bool neg;
    if (n < 0) {
	neg = true;
	if (n == LLONG_MIN) {
	    // Cannot always negate this special case
	    QString s1, s2;
	    s1.setNum(n/base, base);
	    s2.setNum((-(n+base))%base, base);
	   * this = s1 + s2;
	    return *this;
	}
	n = -n;
    } else {
	neg = false;
    }
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
	++len;
    } while (n);
    if (neg) {
	*--p = '-';
	++len;
    }
    return setUnicode(p, len);
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
    if (base < 2 || base > 36) {
	qWarning("QString::setNum: Invalid base %d", base);
	base = 10;
    }
#endif
    char   charbuf[65*sizeof(QChar)];
    QChar *buf = (QChar*)charbuf;
    QChar *p = &buf[64];
    int len = 0;
    do {
	*--p = "0123456789abcdefghijklmnopqrstuvwxyz"[((int)(n%base))];
	n /= base;
	len++;
    } while (n);
    return setUnicode(p,len);
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
#if defined(QT_CHECK_RANGE)
    if (!(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G')) {
	qWarning("QString::setNum: Invalid format char '%c'", f);
	f = 'f';
    }
#endif
    char format[20];
    char *fs = format; // generate format string: %.<prec>l<f>
   * fs++ = '%';
    if (prec >= 0) {
	if (prec > 99) // rather than crash in sprintf()
	    prec = 99;
	*fs++ = '.';
	if (prec >= 10) {
	   * fs++ = prec / 10 + '0';
	   * fs++ = prec % 10 + '0';
	} else {
	   * fs++ = prec + '0';
	}
    }
   * fs++ = 'l';
   * fs++ = f;
   * fs = '\0';
#ifndef QT_NO_SPRINTF
    sprintf(format, n);
    return *this;
#else
    char buf[512];
    ::sprintf(buf, format, n);        // snprintf is unfortunately not portable
    return setLatin1(buf);
#endif
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

/*!
    This function will return a string that replaces the lowest
    numbered occurrence of \c %1, \c %2, ..., \c %9 with \a a.

    The \a fieldWidth value specifies the minimum amount of space that
    \a a is padded to. A positive value will produce right-aligned
    text, whereas a negative value will produce left-aligned text.

    The following example shows how we could create a 'status' string
    when processing a list of files:
    \code
    QString status = QString("Processing file %1 of %2: %3")
			.arg(i)         // current file's number
			.arg(total)     // number of files to process
			.arg(fileName); // current file's name
    \endcode

    It is generally fine to use filenames and numbers as we have done
    in the example above. But note that using arg() to construct
    natural language sentences does not usually translate well into
    other languages because sentence structure and word order often
    differ between languages.

    If there is no place marker (\c %1, \c %2, etc.), a warning
    message (qWarning()) is output and the result is undefined.
*/
QString QString::arg(const QString& a, int fieldWidth) const
{
    QString paddedArg = a;
    if (fieldWidth != 0) {
	int n = QABS(fieldWidth);
	if (n > (int) a.length()) {
	    QString padding;
	    while (n > (int) a.length()) {
		padding += ' ';
		n--;
	    }
	    if (fieldWidth < 0)
		paddedArg.append(padding);
	    else
		paddedArg.prepend(padding);
	}
    }

    const QChar *uc = (const QChar*) d->data;
    const int len = d->size;
    const int end = len - 1;
    int numOccurrences = 0;
    int firstDigit = 10;
    int i;

    for (i = 0; i < end; i++) {
	if (uc[i] == '%') {
	    int digit = uc[i + 1].unicode() - '0';
	    if (digit >= 0 && digit <= firstDigit) {
		if (digit < firstDigit) {
		    firstDigit = digit;
		    numOccurrences = 0;
		}
		numOccurrences++;
	    }
	}
    }

    if (firstDigit == 10) {
	qWarning("QString::arg(): Argument missing: %s, %s", latin1(), a.latin1());
	return *this;
    } else {
	QString result;
	i = 0;
	while (i < len) {
	    if (uc[i] == '%' && i != end) {
		int digit = uc[i + 1].unicode() - '0';
		if (digit == firstDigit) {
		    result += paddedArg;
		    i += 2;
		    if (--numOccurrences == 0) {
			result += mid(i);
			return result;
		    }
		    continue;
		}
	    }
	    result += uc[i++];
	}
	return result;
    }
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

    \code
	QString str;
	str = QString("Decimal 63 is %1 in hexadecimal")
		.arg(63, 0, 16);
	// str == "Decimal 63 is 3f in hexadecimal"
    \endcode
*/
QString QString::arg(Q_LLONG a, int fieldWidth, int base) const
{
    return arg(QString::number(a, base), fieldWidth);
}

/*!
    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/
QString QString::arg(Q_ULLONG a, int fieldWidth, int base) const
{
    return arg(QString::number(a, base), fieldWidth);
}

/*!
    \fn QString QString::arg(int a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg(uint a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg(short a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/

/*!
    \fn QString QString::arg(ushort a, int fieldWidth, int base) const

    \overload

    \a a is expressed in base \a base, which is 10 by default and must
    be between 2 and 36.
*/


/*!
    \overload

    \a a is assumed to be in the Latin-1 character set.
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
*/
QString QString::arg(double a, int fieldWidth, char fmt, int prec) const
{
    return arg(QString::number(a, fmt, prec), fieldWidth);
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
    int digits[4];
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
	digits[i] = lastDigit;
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
    d->dirty = false;
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
    d->encoding = d->simpletext = d->righttoleft = 0;
    d->dirty = true;
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
	if ( ub || s.version() < 3 ) {
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
	    str.setLength( bytes/2 );
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
