/****************************************************************************
**
** Implementation of QStringList.
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

#include "qstringlist.h"

#include "qregexp.h"
#include "qalgorithms.h"

/*!
    \class QStringList qstringlist.h
    \reentrant
    \brief The QStringList class provides a list of strings.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass

    It is used to store and manipulate strings that logically belong
    together. Essentially QStringList is a QList of QString
    objects. Unlike QStrList, which stores pointers to characters,
    QStringList holds real QString objects. It is the class of choice
    whenever you work with Unicode strings. QStringList is part of the
    \link qtl.html Qt Template Library\endlink.

    Like QString itself, QStringList objects are implicitly shared, so
    passing them around as value-parameters is both fast and safe.

    Strings can be added to a list using append(), operator+=() or
    operator<<(), e.g.
    \code
    QStringList fonts;
    fonts.append("Times");
    fonts += "Courier";
    fonts += "Courier New";
    fonts << "Helvetica [Cronyx]" << "Helvetica [Adobe]";
    \endcode

    String lists have an iterator, QStringList::Iterator(), e.g.
    \code
    for (QStringList::Iterator it = fonts.begin(); it != fonts.end(); ++it) {
	cout << *it << ":";
    }
    cout << endl;
    // Output:
    //	Times:Courier:Courier New:Helvetica [Cronyx]:Helvetica [Adobe]:
    \endcode

    Many Qt functions return string lists by value; to iterate over
    these you should make a copy and iterate over the copy.

    You can concatenate all the strings in a string list into a single
    string (with an optional separator) using join(), e.g.
    \code
    QString allFonts = fonts.join(", ");
    cout << allFonts << endl;
    // Output:
    //	Times, Courier, Courier New, Helvetica [Cronyx], Helvetica [Adobe]
    \endcode

    You can sort the list with sort(), and extract a new list which
    contains only those strings which contain a particular substring
    (or match a particular regular expression) using the find()
    functions, e.g.
    \code
    fonts.sort();
    cout << fonts.join(", ") << endl;
    // Output:
    //	Courier, Courier New, Helvetica [Adobe], Helvetica [Cronyx], Times

    QStringList helveticas = fonts.find("Helvetica");
    cout << helveticas.join(", ") << endl;
    // Output:
    //	Helvetica [Adobe], Helvetica [Cronyx]
    \endcode

    Existing strings can be split into string lists with character,
    string or regular expression separators, e.g.
    \code
    QString s = "Red\tGreen\tBlue";
    QStringList colors = s.split("\t");
    cout << colors.join(", ") << endl;
    // Output:
    //	Red, Green, Blue
    \endcode
*/

/*!
    \fn QStringList::QStringList()

    Creates an empty string list.
*/

/*!
    \fn QStringList::QStringList(const QStringList& l)

    Creates a copy of the list \a l. This function is very fast
    because QStringList is implicitly shared. In most situations this
    acts like a deep copy, for example, if this list or the original
    one or some other list referencing the same shared data is
    modified, the modifying list first makes a copy, i.e.
    copy-on-write.
    In a threaded environment you may require a real deep copy
    \omit see \l QDeepCopy\endomit.
*/

/*!
    \fn QStringList::QStringList (const QString & i)

    Constructs a string list consisting of the single string \a i.
    Longer lists are easily created as follows:

    \code
    QStringList items;
    items << "Buy" << "Sell" << "Update" << "Value";
    \endcode
*/

/*!
    \fn QStringList::QStringList (const char *i)

    Constructs a string list consisting of the single Latin1 string \a i.
*/

/*!
    \fn QStringList::QStringList(const QList<QString>& l)

    Constructs a new string list that is a copy of \a l.
*/

/*!
    Sorts the list of strings in ascending case-sensitive order.

    Sorting is very fast. It uses the \link qtl.html Qt Template
    Library's\endlink efficient HeapSort implementation that has a
    time complexity of O(n*log n).

    If you want to sort your strings in an arbitrary order consider
    using a QMap. For example you could use a QMap\<QString,QString\>
    to create a case-insensitive ordering (e.g. mapping the lowercase
    text to the text), or a QMap\<int,QString\> to sort the strings by
    some integer index, etc.
*/
void QStringList::sort()
{
    qHeapSort(*this);
}

#ifdef QT_COMPAT
/*! \fn QStringList QStringList::split(const QChar &sep, const QString &str, bool allowEmptyEntries)

    \overload

    This version of the function uses a QChar as separator, rather
    than a regular expression.

    \sa join() QString::section()
*/

/*! \fn QStringList QStringList::split(const QString &sep, const QString &str, bool allowEmptyEntries)

    \overload

    This version of the function uses a QString as separator, rather
    than a regular expression.

    If \a sep is an empty string, the return value is a list of
    one-character strings: split(QString(""), "four") returns the
    four-item list, "f", "o", "u", "r".

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    \sa join() QString::section()
*/

#ifndef QT_NO_REGEXP
/*! \fn QStringList QStringList::split(const QRegExp &sep, const QString &str, bool allowEmptyEntries)

    Splits the string \a str into strings wherever the regular
    expression \a sep occurs, and returns the list of those strings.

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    For example, if you split the string "a,,b,c" on commas, split()
    returns the three-item list "a", "b", "c" if \a allowEmptyEntries
    is false (the default), and the four-item list "a", "", "b", "c"
    if \a allowEmptyEntries is true.

    If \a sep does not match anywhere in \a str, split() returns a
    single element list with the element containing the single string
    \a str.

    \sa join() QString::section()
*/
#endif
#endif //QT_COMPAT

/*!
    Returns a list of all the strings containing the substring \a str.

    If \a cs is \l QString::CaseSensitive, the string comparison is
    case sensitive; otherwise case is ignored.

    \code
    QStringList list;
    list << "Bill Gates" << "John Doe" << "Bill Clinton";
    list = list.find("Bill");
    // list == ["Bill Gates", "Bill Clinton"]
    \endcode

    \sa QString::indexOf()
*/

QStringList QStringList::find(const QString &str, QString::CaseSensitivity cs) const
{
    QStringList res;
    for (int i = 0; i < size(); ++i)
	if (at(i).contains(str, cs))
	    res << at(i);
    return res;
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns a list of all the strings that match the regular
    expression \a rx.

    \sa QString::indexOf()
*/

QStringList QStringList::find(const QRegExp &rx) const
{
    QStringList res;
    for (int i = 0; i < size(); ++i)
	if (at(i).contains(rx))
	    res << at(i);
    return res;
}
#endif

/*!
    Replaces every occurrence of the string \a before in the strings
    that constitute the string list with the string \a after. Returns
    a reference to the string list.

    If \a cs is \l QString::CaseSensitive, the search is case
    sensitive; otherwise the search is case insensitive.

    Example:
    \code
    QStringList list;
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.replace("a", "o");
    // list == ["olpho", "beto", "gommo", "epsilon"]
    \endcode

    \sa QString::replace()
*/
QStringList& QStringList::replace(const QString &before, const QString &after, QString::CaseSensitivity cs)
{
    for (int i = 0; i < size(); ++i)
	(*this)[i].replace(before, after, cs);
    return *this;
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Replaces every occurrence of the regexp \a rx in the string
    with \a after. Returns a reference to the string list.

    Example:
    \code
    QStringList list;
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.replace(QRegExp("^a"), "o");
    // list == ["olpha", "beta", "gamma", "epsilon"]
    \endcode

    For regexps containing \link qregexp.html#capturing-text
    capturing parentheses \endlink, occurrences of <b>\\1</b>,
    <b>\\2</b>, ..., in \a after are replaced with \a{rx}.cap(1),
    cap(2), ...

    Example:
    \code
    QStringList list;
    list << "Bill Clinton" << "Gates, Bill";
    list.replace(QRegExp("^(.*), (.*)$"), "\\2 \\1");
    // list == ["Bill Clinton", "Bill Gates"]
    \endcode

    \sa QString::replace()
*/
QStringList& QStringList::replace(const QRegExp &rx, const QString &after)
{
    for (int i = 0; i < size(); ++i)
	(*this)[i].replace(rx, after);
    return *this;
}

#endif

/*!
    Joins the string list into a single string with each element
    separated by the string \a sep (which can be empty).

    \sa QString::split()
*/
QString QStringList::join(const QString &sep) const
{
    QString res;
    for (int i = 0; i < size(); ++i) {
	if (i)
	    res += sep;
	res += at(i);
    }
    return res;
}

#ifndef QT_NO_DATASTREAM
template <class T>
QDataStream& operator>>( QDataStream& s, QStringList& l )
{
    return operator>>(s, (QList<T>&)l);
}

template <class T>
QDataStream& operator<<( QDataStream& s, const QStringList& l )
{
    return operator<<(s, (QList<T>&)l);
}
#endif // QT_NO_DATASTREAM

