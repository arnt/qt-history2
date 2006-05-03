/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qstringlist.h>

/*! \typedef QStringListIterator
    \relates QStringList

    The QStringListIterator type definition provides a Java-style const
    iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style const iterator is simply
    a type definition for QListIterator<QString>.

    \sa QMutableStringListIterator, QStringList::const_iterator
*/

/*! \typedef QMutableStringListIterator
    \relates QStringList

    The QStringListIterator type definition provides a Java-style
    non-const iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style non-const iterator is
    simply a type definition for QMutableListIterator<QString>.

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

    QStringList inherits from QList<QString>. Like QList, QStringList
    is \l{implicitly shared}. It provides fast index-based access as
    well as fast insertions and removals. Passing string lists as
    value parameters is both fast and safe.

    All of QList's functionality also applies to QStringList. For
    example, you can use isEmpty() to test whether the list is empty,
    and you can call functions like append(), prepend(), insert(),
    replace(), and remove() to modify a QStringList. In addition,
    QStringList provides a few convenience functions that make
    handling lists of strings easier:

    \tableofcontents

    \section1 Adding strings

    Strings can be added to a list using the \l
    {QList::append()}{append()}, \l
    {QList::operator+=()}{operator+=()} and \l
    {QStringList::operator<<()}{operator<<()} functions. For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList fonts
    \printuntil Courier

    \section1 Iterating over the strings

    To iterate over a list, you can either use index positions or
    QList's Java-style and STL-style iterator types:

    Indexing:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto for (int i = 0; i < fonts.size(); ++i)
    \printuntil cout << fonts.at(i).toLocal8Bit().constData() << end

    Java-style iterator:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringListIterator
    \printuntil cout << javaStyleIterator

    STL-style iterator:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList::const_iterator
    \printuntil cout << (*constIterator)

    The QStringListIterator class is simply a type definition for
    QListIterator<QString>. QStringList also provide the
    QMutableStringListIterator class which is a type definition for
    QMutableListIterator<QString>.

    \section1 Manipulating the strings

    QStringList provides several functions allowing you to manipulate
    the contents of a list. You can concatenate all the strings in a
    string list into a single string (with an optional separator)
    using the join() function. For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto fonts.join
    \printuntil // str

    To break up a string into a string list, use the QString::split()
    function:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList list;
    \printuntil // list

    The argument to split can be a single character, a string, or a
    QRegExp.

    In addition, the \l {QStringList::operator+()}{operator+()}
    function allows you to concatenate two string lists into one. To
    sort a string list, use the sort() function.

    QString list also provides the filter() function which lets you
    to extract a new list which contains only those strings which
    contain a particular substring (or match a particular regular
    expression):

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto monospacedFonts
    \printline monospacedFonts

    The contains() function tells you whether the list contains a
    given string, while the indexOf() function returns the index of
    the first occurrence of the given string. The lastIndexOf()
    function on the other hand, returns the index of the last
    occurrence of the string.

    Finally, the replaceInStrings() function calls QString::replace()
    on each string in the string list in turn. For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList files
    \printuntil // files

    \sa QString
*/

/*!
    \fn QStringList::QStringList()

    Constructs an empty string list.
*/

/*!
    \fn QStringList::QStringList(const QString &str)

    Constructs a string list that contains the given string, \a
    str. Longer lists are easily created like this:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto  longerList
    \printline  longerList

    \sa append()
*/

/*!
    \fn QStringList::QStringList(const QStringList &other)

    Constructs a copy of the \a other string list.

    This operation takes \l{constant time} because QStringList is
    \l{implicitly shared}, making the process of returning a
    QStringList from a function very fast. If a shared instance is
    modified, it will be copied (copy-on-write), and that takes
    \l{linear time}.

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
    \fn void QStringList::sort()

    Sorts the list of strings in ascending order (case sensitively).

    Sorting is performed using Qt's qSort() algorithm,
    which operates in \l{linear-logarithmic time}, i.e. O(\e{n} log \e{n}).

    If you want to sort your strings in an arbitrary order, consider
    using the QMap class. For example, you could use a QMap<QString,
    QString> to create a case-insensitive ordering (e.g. with the keys
    being lower-case versions of the strings, and the values being the
    strings), or a QMap<int, QString> to sort the strings by some
    integer index.

    \sa qSort()
*/
void QtPrivate::QStringList_sort(QStringList *that)
{
    qSort(*that);
}


#ifdef QT3_SUPPORT
/*!
    \fn QStringList QStringList::split(const QChar &sep, const QString &str, bool allowEmptyEntries)

    \overload

    This version of the function uses a QChar as separator.

    \sa join() QString::section()
*/

/*!
    \fn QStringList QStringList::split(const QString &sep, const QString &str, bool allowEmptyEntries)

    \overload

    This version of the function uses a QString as separator.

    If \a sep is an empty string, the return value is a list of
    one-character strings: split(QString(""), "four") returns the
    four-item list, "f", "o", "u", "r".

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    \sa join() QString::section()
*/
#ifndef QT_NO_REGEXP
/*!
    \fn QStringList QStringList::split(const QRegExp &sep, const QString &str, bool allowEmptyEntries)

    Splits the string \a str into strings wherever the regular
    expression \a sep occurs, and returns the list of those strings.

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    For example, if you split the string "a,,b,c" on commas, split()
    returns the three-item list "a", "b", "c" if \a allowEmptyEntries
    is false (the default), and the four-item list "a", "", "b", "c"
    if \a allowEmptyEntries is true.

    Use \c{split(QRegExp("\\s+"), str)} to split on arbitrary amounts
    of whitespace.

    If \a sep does not match anywhere in \a str, split() returns a
    single element list with the element containing the original
    string, \a str.

    \sa join() QString::section()
*/
#endif
#endif // QT3_SUPPORT

/*!
    \fn QStringList QStringList::filter(const QString &str, Qt::CaseSensitivity cs) const

    Returns a list of all the strings containing the substring \a str.

    If \a cs is \l Qt::CaseSensitive (the default), the string
    comparison is case sensitive; otherwise the comparison is case
    insensitive.

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList list;
    \printline QStringList list;
    \skipto list << "Bill Murray"
    \printuntil  // result

    This is equivalent to

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList result;
    \printline QStringList result;
    \skipto foreach
    \printuntil }

    \sa contains()
*/
QStringList QtPrivate::QStringList_filter(const QStringList *that, const QString &str,
                                          Qt::CaseSensitivity cs)
{
    QStringMatcher matcher(str, cs);
    QStringList res;
    for (int i = 0; i < that->size(); ++i)
        if (matcher.indexIn(that->at(i)) != -1)
            res << that->at(i);
    return res;
}


/*!
    \fn QBool QStringList::contains(const QString &str, Qt::CaseSensitivity cs) const

    Returns true if the list contains the string \a str; otherwise
    returns false. The search is case insensitive if \a cs is
    Qt::CaseInsensitive; the search is case sensitive by default.

    \sa indexOf(), lastIndexOf(), QString::contains()
 */
QBool QtPrivate::QStringList_contains(const QStringList *that, const QString &str,
                                      Qt::CaseSensitivity cs)
{
    QStringMatcher matcher(str, cs);
    for (int i = 0; i < that->size(); ++i) {
        QString string(that->at(i));
        if (string.length() == str.length() && matcher.indexIn(string) == 0)
            return QBool(true);
    }
    return QBool(false);
}

#ifndef QT_NO_REGEXP
/*!
    \fn QStringList QStringList::filter(const QRegExp &rx) const

    \overload

    Returns a list of all the strings that match the regular
    expression \a rx.
*/
QStringList QtPrivate::QStringList_filter(const QStringList *that, const QRegExp &rx)
{
    QStringList res;
    for (int i = 0; i < that->size(); ++i)
        if (that->at(i).contains(rx))
            res << that->at(i);
    return res;
}
#endif

/*!
    \fn QStringList &QStringList::replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs)

    Returns a string list where every string has had the \a before
    text replaced with the \a after text wherever the \a before text
    is found. The \a before text is matched case-sensitively or not
    depending on the \a cs flag.

    For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList list;
    \printline QStringList list;
    \skipto list << "alpha"
    \printuntil  // list

    \sa QString::replace()
*/
void QtPrivate::QStringList_replaceInStrings(QStringList *that, const QString &before,
                                             const QString &after, Qt::CaseSensitivity cs)
{
    for (int i = 0; i < that->size(); ++i)
        (*that)[i].replace(before, after, cs);
}


#ifndef QT_NO_REGEXP
/*!
    \fn QStringList &QStringList::replaceInStrings(const QRegExp &rx, const QString &after)
    \overload

    Replaces every occurrence of the regexp \a rx, in each of the
    string lists's strings, with \a after. Returns a reference to the
    string list.

    For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList list;
    \printline QStringList list;
    \skipto list << "alpha"
    \skipto list.clear()
    \skipto list << "alpha"
    \printuntil  // list

    For regular expressions that contain \l{capturing parentheses},
    occurrences of \bold{\\1}, \bold{\\2}, ..., in \a after are
    replaced with \a{rx}.cap(1), \a{rx}.cap(2), ...

    For example:

    \quotefromfile snippets/qstringlist/main.cpp
    \skipto QStringList list;
    \printline QStringList list;
    \skipto list << "Bill Clinton" << "Murray, Bill"
    \printuntil  // list
*/
void QtPrivate::QStringList_replaceInStrings(QStringList *that, const QRegExp &rx, const QString &after)
{
    for (int i = 0; i < that->size(); ++i)
        (*that)[i].replace(rx, after);
}
#endif

/*!
    \fn QString QStringList::join(const QString &separator) const

    Joins all the string list's strings into a single string with each
    element separated by the the given \a separator (which can be an
    empty string).

    \sa QString::split()
*/
QString QtPrivate::QStringList_join(const QStringList *that, const QString &sep)
{
    QString res;
    for (int i = 0; i < that->size(); ++i) {
        if (i)
            res += sep;
        res += that->at(i);
    }
    return res;
}

/*!
    \fn QStringList QStringList::operator+(const QStringList &other) const

    Returns a string list that is the concatenation of this string
    list with the \a other string list.

    \sa append()
*/

/*!
    \fn QStringList &QStringList::operator<<(const QString &str)

    Appends the given string, \a str, to this string list and returns
    a reference to the string list.

    \sa append()
*/

/*!
    \fn QStringList &QStringList::operator<<(const QStringList &other)

    \overload

    Appends the \a other string list to the string list and returns a reference to
    the latter string list.
*/

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator>>(QDataStream &in, QStringList &list)
    \relates QStringList

    Reads a string list from the given \a in stream into the specified
    \a list.

    \sa {Format of the QDataStream Operators}
*/

/*!
    \fn QDataStream &operator<<(QDataStream &out, const QStringList &list)
    \relates QStringList

    Writes the given string \a list to the specified \a out stream.

    \sa {Format of the QDataStream Operators}
*/
#endif // QT_NO_DATASTREAM

/*!
    \fn QStringList QStringList::grep(const QString &str, bool cs = true) const

    Use find() instead.
*/

/*!
    \fn QStringList QStringList::grep(const QRegExp &rx) const

    Use find() instead.
*/

/*!
    \fn QStringList &QStringList::gres(const QString &before, const QString &after, bool cs = true)

    Use replace() instead.
*/

/*!
    \fn QStringList &QStringList::gres(const QRegExp &rx, const QString &after)

    Use replace() instead.
*/

/*!
    \fn Iterator QStringList::fromLast()

    Use end() instead.

    \oldcode
    QStringList::Iterator i = list.fromLast();
    \newcode
    QStringList::Iterator i = list.isEmpty() ? list.end() : --list.end();
    \endcode
*/

/*!
    \fn ConstIterator QStringList::fromLast() const

    Use end() instead.

    \oldcode
    QStringList::ConstIterator i = list.fromLast();
    \newcode
    QStringList::ConstIterator i = list.isEmpty() ? list.end() : --list.end();
    \endcode
*/


#ifndef QT_NO_REGEXP
/*!
    \fn int QStringList::indexOf(const QRegExp &rx, int from) const

    Returns the index position of the first exact match of \a rx in
    the list, searching forward from index position \a from. Returns
    -1 if no item matched.

    \sa lastIndexOf(), contains(), QRegExp::exactMatch()
*/
int QtPrivate::QStringList_indexOf(const QStringList *that, const QRegExp &rx, int from)
{
   if (from < 0)
       from = qMax(from + that->size(), 0);
   for (int i = from; i < that->size(); ++i) {
        if (rx.exactMatch(that->at(i)))
            return i;
    }
    return -1;
}

/*!
    \fn int QStringList::lastIndexOf(const QRegExp &rx, int from) const

    Returns the index position of the last exact match of \a rx in
    the list, searching backward from index position \a from. If \a
    from is -1 (the default), the search starts at the last item.
    Returns -1 if no item matched.

    \sa indexOf(), contains(), QRegExp::exactMatch()
*/
int QtPrivate::QStringList_lastIndexOf(const QStringList *that, const QRegExp &rx, int from)
{
    if (from < 0)
        from += that->size();
    else if (from >= that->size())
        from = that->size() - 1;
    for (int i = from; i >= 0; --i) {
        if (rx.exactMatch(that->at(i)))
            return i;
        }
    return -1;
}
#endif
