/****************************************************************************
**
** Implementation of QStringList.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qalgorithms.h"
#include "qdatastream.h"
#include "qregexp.h"
#include "qstringlist.h"

/*! \typedef QStringListIterator
    \relates QStringList

    The QStringListIterator typedef provides a Java-style const
    iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style const iterator is simply
    a typedef for QListIterator<QString>.

    \sa QStringListMutableIterator, QStringList::const_iterator
*/

/*! \typedef QStringListMutableIterator
    \relates QStringList

    The QStringListIterator typedef provides a Java-style non-const
    iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style non-const iterator is
    simply a typedef for QListMutableIterator<QString>.

    \sa QStringListIterator, QStringList::iterator
*/

/*!
    \class QStringList
    \brief The QStringList class provides a list of strings.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass
    \reentrant

    QStringList inherits from QList\<QString\>. All of QList's
    functionality also applies to QStringList. For example, you can
    use isEmpty() to test whether the list is empty, and you can call
    functions like append(), prepend(), insert(), replace(), and
    remove() to modify a QStringList. In addition, QStringList
    provides a few convenience functions that make handling lists of
    strings easier.

    Like QList, QStringList is \l{implicitly shared}. Passing string
    lists around as value parameters is both fast and safe.

    Strings can be added to a list using append(), operator+=(), or
    operator<<(). For example:
    \code
        QStringList fonts;
        fonts << "Arial" << "Helvetica" << "Times" << "Courier";
    \endcode

    To iterate over a string, you can either use index positions or
    QList's Java-style and STL-style iterator types. Here are
    examples of each approach.

    Index:
    \code
        for (int i = 0; i < fonts.size(); ++i)
	    cout << fonts.at(i).ascii() << endl;
    \endcode

    Java-style iterator:
    \code
	QStringListIterator i(fonts);
        while (i.hasNext())
	    cout << i.next().ascii() << endl;
    \endcode

    STL-style iterator:
    \code
	QStringList::const_iterator i;
        for (i = fonts.constBegin(); i != fonts.constEnd(); ++i)
	    cout << (*i).ascii() << endl;
    \endcode

    QStringListIterator and QStringListMutableIterator are simply
    typedefs for QListIterator<QString> and
    QListMutableIterator<QString>.

    You can concatenate all the strings in a string list into a single
    string (with an optional separator) using join(). For example:

    \code
        QString str = fonts.join(",");
	// str == "Arial,Helvetica,Times,Courier"
    \endcode

    To break up a string into a string list, use QString::split():

    \code
	QString str = "Arial,Helvetica,Times,Courier";
	QStringList list = str.split(",");
        // list: [ "Arial", "Helvetica", "Times", "Courier" ]
    \endcode

    The argument to split can be a single character, a string, or a
    QRegExp.

    You can sort a string list with sort(), and extract a new list
    which contains only those strings which contain a particular
    substring (or match a particular regular expression) using the
    find() functions. For example:

    \code
        QStringList monospacedFonts = fonts.find(QRegExp("Courier|Fixed"));
    \endcode

    Similarly, the replace() function calls QString::replace() on
    each string in the string list in turn. Here's an example that
    uses it to replace all occurrences of "$QTDIR" with "/usr/lib/qt"
    in a string list:

    \code
	QStringList files;
        files << "$QTDIR/src/moc/moc.y"
	      << "$QTDIR/src/moc/moc.l"
              << "$QTDIR/include/qconfig.h";

	files.replace("$QTDIR", "/usr/lib/qt");
    \endcode

    \sa QString, QStringListIterator, QStringListMutableIterator
*/

/*!
    \fn QStringList::QStringList()

    Constructs an empty string list.
*/

/*!
    \fn QStringList::QStringList(const QString &str)

    Constructs a string list consisting of the single string \a str.
    Longer lists are easily created as follows:

    \code
	list = (QStringList() << str1 << str2 << str3);
    \endcode
*/

/*!
    \fn QStringList::QStringList(const char *str)

    Constructs a string list consisting of the single string \a str.
*/

/*!
    \fn QStringList::QStringList(const QStringList &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QStringList is
    \l{implicitly shared}. This makes returning a QStringList from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*!
    \fn QStringList::QStringList(const QList<QString> &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QStringList is
    \l{implicitly shared}. This makes returning a QStringList from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*!
    Sorts the list of strings in ascending order (case sensitively).

    Sorting is performed using Qt's qHeapSort() algorithm,
    which operates in \l{linear-logarithmic time}, i.e. O(\e{n} log \e{n}).

    If you want to sort your strings in an arbitrary order, consider
    using a QMap. For example, you could use a QMap\<QString,
    QString\> to create a case-insensitive ordering (e.g. mapping the
    lower-case text to the text), or a QMap\<int, QString\> to sort
    the strings by some integer index.

    \sa qHeapSort()
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
#endif // QT_COMPAT

/*!
    Returns a list of all the strings containing the substring \a str.

    If \a cs is \l QString::CaseSensitive, the string comparison is
    case sensitive; otherwise the comparison is case insensitive.

    \code
    QStringList list;
    list << "Bill Gates" << "John Doe" << "Bill Clinton";
    list = list.find("Bill");
    // list: [ "Bill Gates", "Bill Clinton" ]
    \endcode

    \sa QString::contains()
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
    capturing parentheses \endlink, occurrences of \bold{\\1},
    \bold{\\2}, ..., in \a after are replaced with \a{rx}.cap(1),
    \a{rx}.cap(2), ...

    Example:
    \code
        QStringList list;
        list << "Bill Clinton" << "Gates, Bill";
        list.replace(QRegExp("^(.*), (.*)$"), "\\2 \\1");
        // list == ["Bill Clinton", "Bill Gates"]
    \endcode

    \sa replace()
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
/*!
    \relates QStringList

    Reads a string list from stream \a in into \a list.

    \sa \link datastreamformat.html Format of the QDataStrea operators \endlink
*/
QDataStream &operator>>(QDataStream &in, QStringList &list)
{
    return operator>>(in, static_cast<QList<QString> &>(list));
}

/*!
    \relates QStringList

    Writes the string list \a list to stream \a out.

    \sa \link datastreamformat.html Format of the QDataStrea operators \endlink
*/
QDataStream &operator<<(QDataStream &out, const QStringList &list)
{
    return operator<<(out, static_cast<const QList<QString> &>(list));
}
#endif // QT_NO_DATASTREAM
