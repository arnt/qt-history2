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

#include "qhash.h"

#ifdef truncate
#undef truncate
#endif

#include <qbytearray.h>
#include <qstring.h>

/*
    These functions are based on Peter J. Weinberger's hash function
    (from the Dragon Book). The constant 24 in the original function
    was replaced with 23 to produce fewer collisions on input such as
    "a", "aa", "aaa", "aaaa", ...
*/

uint qHash(const QByteArray &key)
{
    const uchar *p = reinterpret_cast<const uchar *>(key.data());
    int n = key.size();
    uint h = 0;
    uint g;

    while (n--) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}

uint qHash(const QString &key)
{
    const QChar *p = key.unicode();
    int n = key.size();
    uint h = 0;
    uint g;

    while (n--) {
        h = (h << 4) + (*p++).unicode();
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}

/*
    The prime_deltas array is a table of selected prime values, even
    though it doesn't look like one. The primes we are using are 1,
    2, 5, 11, 17, 37, 67, 131, 257, ..., i.e. primes in the immediate
    surrounding of a power of two.

    The primeForNumBits() function returns the prime associated to a
    power of two. For example, primeForNumBits(8) returns 257.
*/

static const uchar prime_deltas[] = {
    0,  0,  1,  3,  1,  5,  3,  3,  1,  9,  7,  5,  3,  9, 25,  3,
    1, 21,  3, 21,  7, 15,  9,  5,  3, 29, 15,  0,  0,  0,  0,  0
};

static inline int primeForNumBits(int numBits)
{
    return (1 << numBits) + prime_deltas[numBits];
}

/*
    Returns the smallest integer n such that
    primeForNumBits(n) >= hint.
*/
static int countBits(int hint)
{
    int numBits = 0;
    int bits = hint;

    while (bits > 1) {
        bits >>= 1;
        numBits++;
    }

    if (numBits >= (int)sizeof(prime_deltas)) {
        numBits = sizeof(prime_deltas) - 1;
    } else if (primeForNumBits(numBits) < hint) {
        ++numBits;
    }
    return numBits;
}

/*
    A QHash has initially around pow(2, MinNumBits) buckets. For
    example, if MinNumBits is 4, it has 17 buckets.
*/
const int MinNumBits = 4;

QHashData QHashData::shared_null = {
    0, 0, Q_ATOMIC_INIT(1), 0, MinNumBits, 0, 0, true
};

QHashData *QHashData::detach_helper(Node *(*node_duplicate)(Node *))
{
    union {
        QHashData *d;
        Node *e;
    };
    d = new QHashData;
    d->fakeNext = 0;
    d->buckets = 0;
    d->ref.init(1);
    d->size = size;
    d->userNumBits = userNumBits;
    d->numBits = numBits;
    d->numBuckets = numBuckets;
    d->sharable = true;

    if (numBuckets) {
        d->buckets = new Node *[numBuckets];
        Node *this_e = reinterpret_cast<Node *>(this);
        for (int i = 0; i < numBuckets; ++i) {
            Node **nextNode = &d->buckets[i];
            Node *oldNode = buckets[i];
            while (oldNode != this_e) {
                Node *dup = node_duplicate(oldNode);
                dup->h = oldNode->h;
                *nextNode = dup;
                nextNode = &dup->next;
                oldNode = oldNode->next;
            }
            *nextNode = e;
        }
    }
    return d;
}

QHashData::Node *QHashData::nextNode(Node *node)
{
    union {
        Node *next;
        Node *e;
        QHashData *d;
    };
    next = node->next;
    if (next->next)
        return next;

    int start = (node->h % d->numBuckets) + 1;
    Node **bucket = d->buckets + start;
    int n = d->numBuckets - start;
    while (n--) {
        if (*bucket != e)
            return *bucket;
        ++bucket;
    }
    return e;
}

QHashData::Node *QHashData::previousNode(Node *node)
{
    union {
        Node *e;
        QHashData *d;
    };

    e = node;
    while (e->next)
        e = e->next;

    int start;
    if (node == e)
        start = d->numBuckets - 1;
    else
        start = node->h % d->numBuckets;

    Node *sentinel = node;
    Node **bucket = d->buckets + start;
    while (start >= 0) {
        if (*bucket != sentinel) {
            Node *prev = *bucket;
            while (prev->next != sentinel)
                prev = prev->next;
            return prev;
        }

        sentinel = e;
        --bucket;
        --start;
    }
    return e;
}

/*
    If hint is negative, -hint gives the approximate number of
    buckets that should be used for the hash table. If hint is
    nonnegative, (1 << hint) gives the approximate number
    of buckets that should be used.
*/
void QHashData::rehash(int hint)
{
    if (hint < 0) {
        hint = countBits(-hint);
        if (hint < MinNumBits)
            hint = MinNumBits;
        userNumBits = hint;
        while (primeForNumBits(hint) < (size >> 1))
            ++hint;
    } else if (hint < MinNumBits) {
        hint = MinNumBits;
    }

    if (numBits != hint) {
        Node *e = reinterpret_cast<Node *>(this);
        Node **oldBuckets = buckets;
        int oldNumBuckets = numBuckets;

        numBits = hint;
        numBuckets = primeForNumBits(hint);
        buckets = new Node *[numBuckets];
        for (int i = 0; i < numBuckets; i++)
            buckets[i] = e;

        for (int i = 0; i < oldNumBuckets; i++) {
            Node *node = oldBuckets[i];
            while (node != e) {
                Node *oldNext = node->next;
                Node **nextNode = &buckets[node->h % numBuckets];
                while (*nextNode != e)
                    nextNode = &(*nextNode)->next;
                node->next = *nextNode;
                *nextNode = node;
                node = oldNext;
            }
        }
        delete [] oldBuckets;
    }
}

void QHashData::free()
{
    delete [] buckets;
    delete this;
}

/*!
    \class QHash
    \brief The QHash class is a template class that provides a hash-table-based dictionary.

    \ingroup qtl
    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QHash\<Key, T\> is one of Qt's generic \l{container classes}. It
    stores (key, value) pairs and provides very fast lookup of the
    value associated with a key.

    QHash provides very similar functionality to QMap. The
    differences are:

    \list
    \i QHash provides faster lookups than QMap.
    \i When iterating over a QMap, the items are always sorted by
       key. With QHash, the items are arbitrarily ordered.
    \i The key type of a QMap must provide operator<(). The key
       type of a QHash must provide operator==() and a global
       qHash(Key) function.
    \endlist

    Here's an example QHash with QString keys and int values:
    \code
        QHash<QString, int> hash;
    \endcode

    To insert a (key, value) pair into the hash, you can use operator[]():

    \code
        hash["one"] = 1;
        hash["three"] = 3;
        hash["seven"] = 7;
    \endcode

    This inserts the following three (key, value) pairs into the
    QHash: ("one", 1), ("three", 3), and ("seven", 7).

    Another way to insert items into the hash is to use insert():

    \code
        hash.insert("twelve", 12);
    \endcode

    To look up a value, use operator[]() or value():

    \code
        int num1 = hash["thirteen"];
        int num2 = hash.value("thirteen");
    \endcode

    If there is no item with the specified key in the hash, these
    functions return a \l{default-constructed value}.

    If you want to check whether the hash contains a particular key,
    use contains():

    \code
        int timeout = 30;
        if (hash.contains("TIMEOUT"))
            timeout = hash.value("TIMEOUT");
    \endcode

    There is also a value() overload that uses its second argument as
    a default value if there is no item with the specified key:

    \code
        int timeout = hash.value("TIMEOUT", 30);
    \endcode

    In general, we recommend that you use contains() and value()
    rather than operator[]() for looking up a key in a hash. The
    reason is that operator[]() silently inserts an item into the
    hash if no item exists with the same key (unless the hash is
    const). For example, the following code snippet will create 1000
    items in memory:

    \code
        // WRONG
        QHash<int, QWidget *> hash;
        ...
        for (int i = 0; i < 1000; ++i) {
            if (hash[i] == okButton)
                cout << "Found button at index " << i << endl;
        }
    \endcode

    To avoid this problem, replace \c hash[i] with \c hash.value(i)
    in the code above.

    If you want to navigate through all the (key, value) pairs stored
    in a QHash, you can use an iterator. QHash provides both
    \l{Java-style iterators} (QHashIterator and QMutableHashIterator)
    and \l{STL-style iterators} (QHash::const_iterator and
    QHash::iterator). Here's how to iterate over a QHash<QString,
    int> using a Java-style iterator:

    \code
        QHashIterator<QString, int> i(hash);
        while (i.hasNext()) {
            i.next();
            cout << i.key() << ": " << i.value() << endl;
        }
    \endcode

    Here's the same code, but using an STL-style iterator:

    \code
        QHash<QString, int>::const_iterator i = hash.constBegin();
        while (i != hash.constEnd()) {
            cout << i.key() << ": " << i.value() << endl;
            ++i;
        }
    \endcode

    QHash is unordered, so an iterator's sequence cannot be assumed
    to be predictable. If ordering by key is required, use a QMap.

    Normally, a QHash allows only one value per key. If you call
    insert() with a key that already exists in the QHash, the
    previous value is erased. For example:

    \code
        hash.insert("plenty", 100);
        hash.insert("plenty", 2000);
        // hash.value("plenty") == 2000
    \endcode

    However, you can store multiple values per key by using
    insertMulti() instead of insert() (or using the convenience
    subclass QMultiHash). If you want to retrieve all
    the values for a single key, you can use values(const Key &key),
    which returns a QList<T>:

    \code
        QList<int> values = hash.values("plenty");
        for (int i = 0; i < values.size(); ++i)
            cout << values.at(i) << endl;
    \endcode

    The items that share the same key are available from most
    recently to least recently inserted.

    A more efficient approach is to use QHashIterator::findNextKey() or
    QMutableHashIterator::findNextKey():

    \code
        QHashIterator<QString, int> i(hash);
        while (i.findNextKey("plenty"))
            cout << i.value() << endl;
    \endcode

    If you prefer the STL-style iterators, you can call find() to get
    the iterator for the first item with a key and iterate from
    there:

    \code
        QHash<QString, int>::iterator i = hash.find("plenty");
        while (i != hash.end() && i.key() == "plenty") {
            cout << i.value() << endl;
            ++i;
        }
    \endcode

    If you only need to extract the values from a hash (not the keys),
    you can also use \l{foreach}:

    \code
        QHash<QString, int> hash;
        ...
        foreach (int value, hash)
            cout << value << endl;
    \endcode

    Items can be removed from the hash in several ways. One way is to
    call remove(); this will remove any item with the given key.
    Another way is to use QMutableHashIterator::remove(). In addition,
    you can clear the entire hash using clear().

    QHash's key and value data types must be \l{assignable data
    types}. You cannot, for example, store a QWidget as a value;
    instead, store a QWidget *. In addition, QHash's key type must
    provide operator==(), and there must also be a global qHash()
    function that returns a hash value for an argument of the key's
    type.

    Here's a list of the C++ and Qt types that can serve as keys in a
    QHash: any integer type (char, unsigned long, etc.), any pointer
    type, QChar, QString, and QByteArray. For all of these, \c
    <qhash.h> defines a qHash() function that computes an adequate
    hash value. If you want to use other types as the key, make sure
    that you provide operator==() and a qHash() implementation.

    Example:
    \code
        #ifndef EMPLOYEE_H
        #define EMPLOYEE_H

        class Employee
        {
        public:
            Employee() {}
            Employee(const QString &name, const QDate &dateOfBirth);
            ...

        private:
            QString myName;
            QDate myDateOfBirth;
        };

        inline bool operator==(const Employee &e1, const Employee &e2)
        {
            return e1.name() == e2.name()
                   && e1.dateOfBirth() == e2.dateOfBirth();
        }

        inline uint qHash(const Employee &key)
        {
            return qHash(key.name()) ^ key.dateOfBirth().day();
        }

        #endif // EMPLOYEE_H
    \endcode

    The qHash() function computes a numeric value based on a key. It
    can use any algorithm imaginable, as long as it always returns
    the same value if given the same argument. In other words, if
    \c{e1 == e2}, then \c{qHash(e1) == qHash(e2)} must hold as well.
    However, to obtain good performance, the qHash() function should
    attempt to return different hash values for different keys to the
    largest extent possible.

    In the example above, we've relied on Qt's global qHash(const
    QString&) to give us a hash value for the employee's name, and
    XOR'ed this with the day they were born to help produce unique
    hashes for people with the same name.

    Internally, QHash uses a hash table to perform lookups. Unlike Qt
    3's QDict, which needed to be initialized with a prime number,
    QHash's hash table automatically grows and shrinks to provide fast
    lookups without wasting too much memory. You can still control the
    size of the hash table by calling reserve() if you already know
    approximately how many items the QHash will contain, but this isn't
    necessary to obtain good performance. You can also call capacity()
    to retrieve the hash table's size.

    \sa QHashIterator, QMutableHashIterator, QMap
*/

/*! \fn QHash::QHash()

    Constructs an empty hash.

    \sa clear()
*/

/*! \fn QHash::QHash(const QHash<Key, T> &other)

    Constructs a copy of \a other.

    This operation occurs in \l{constant time}, because QHash is
    \l{implicitly shared}. This makes returning a QHash from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and this takes \l{linear time}.

    \sa operator=()
*/

/*! \fn QHash::~QHash()

    Destroys the hash. References to the values in the hash and all
    iterators of this hash become invalid.
*/

/*! \fn QHash<Key, T> &QHash::operator=(const QHash<Key, T> &other)

    Assigns \a other to this hash and returns a reference to this hash.
*/

/*! \fn bool QHash::operator==(const QHash<Key, T> &other) const

    Returns true if \a other is equal to this hash; otherwise returns
    false.

    Two hashes are considered equal if they contain the same (key,
    value) pairs.

    This function requires the value type to implement \c operator==().

    \sa operator!=()
*/

/*! \fn bool QHash::operator!=(const QHash<Key, T> &other) const

    Returns true if \a other is not equal to this hash; otherwise
    returns false.

    Two hashes are considered equal if they contain the same (key,
    value) pairs.

    This function requires the value type to implement \c operator==().

    \sa operator==()
*/

/*! \fn int QHash::size() const

    Returns the number of items in the hash.

    \sa isEmpty(), count()
*/

/*! \fn bool QHash::isEmpty() const

    Returns true if the hash contains no items; otherwise returns
    false.

    \sa size()
*/

/*! \fn int QHash::capacity() const

    Returns the number of buckets in the QHash's internal hash table.

    The sole purpose of this function is to provide a means of fine
    tuning QHash's memory usage. In general, you will rarely ever
    need to call this function. If you want to know how many items are
    in the hash, call size().

    \sa reserve(), squeeze()
*/

/*! \fn void QHash::reserve(int size)

    Ensures that the QHash's internal hash table consists of at least
    \a size buckets.

    This function is useful for code that needs to build a huge hash
    and wants to avoid repeated reallocation. For example:

    \code
        QHash<QString, int> hash;
        hash.reserve(20000);
        for (int i = 0; i < 20000; ++i)
            hash.insert(keys[i], values[i]);
    \endcode

    Ideally, \a size should be slightly more than the maximum number
    of items expected in the hash. \a size doesn't have to be prime,
    because QHash will use a prime number internally anyway. If \a size
    is an underestimate, the worst that will happen is that the QHash
    will be a bit slower.

    In general, you will rarely ever need to call this function.
    QHash's internal hash table automatically shrinks or grows to
    provide good performance without wasting too much memory.

    \sa squeeze(), capacity()
*/

/*! \fn void QHash::squeeze()

    Reduces the size of the QHash's internal hash table to save
    memory.

    The sole purpose of this function is to provide a means of fine
    tuning QHash's memory usage. In general, you will rarely ever
    need to call this function.

    \sa reserve(), capacity()
*/

/*! \fn void QHash::detach()

    \internal

    Detaches this hash from any other hashes with which it may share
    data.

    \sa isDetached()
*/

/*! \fn bool QHash::isDetached() const

    \internal

    Returns true if the hash's internal data isn't shared with any
    other hash object; otherwise returns false.

    \sa detach()
*/

/*! \fn void QHash::clear()

    Removes all items from the hash.

    \sa remove()
*/

/*! \fn int QHash::remove(const Key &key)

    Removes all the items that have the key \a key from the hash.
    Returns the number of items removed which is usually 1 but will be
    0 if the key isn't in the hash, or \> 1 if insertMulti() has been
    used with the \a key.

    \sa clear(), take()
*/

/*! \fn T QHash::take(const Key &key)

    Removes the item with the key \a key from the hash and returns
    the value associated with it.

    If the item does not exist in the hash, the function simply
    returns a \l{default-constructed value}. If there are multiple
    items for \a key in the hash, only the most recently inserted one
    is removed.

    If you don't use the return value, remove() is more efficient.

    \sa remove()
*/

/*! \fn bool QHash::contains(const Key &key) const

    Returns true if the hash contains an item with key \a key;
    otherwise returns false.

    \sa count()
*/

/*! \fn const T QHash::value(const Key &key) const

    Returns the value associated with the key \a key.

    If the hash contains no item with key \a key, the function
    returns a \l{default-constructed value}. If there are multiple
    items for \a key in the hash, the value of the most recently
    inserted one is returned.

    \sa key(), values(), contains(), operator[]()
*/

/*! \fn const T QHash::value(const Key &key, const T &defaultValue) const

    \overload

    If the hash contains no item with the given \a key, the function returns
    \a defaultValue.
*/

/*! \fn T &QHash::operator[](const Key &key)

    Returns the value associated with the key \a key as a modifiable
    reference.

    If the hash contains no item with key \a key, the function inserts
    a \l{default-constructed value} into the hash with key \a key, and
    returns a reference to it. If the hash contains multiple items
    with key \a key, this function returns a reference to the most
    recently inserted value.

    \sa insert(), value()
*/

/*! \fn const T QHash::operator[](const Key &key) const

    \overload

    Same as value().
*/

/*! \fn QList<Key> QHash::keys() const

    Returns a list containing all the keys in the hash, in an
    arbitrary order. Keys that occur multiple times in the hash
    (because items were inserted with insertMulti(), or merge() was
    used), also occur multiple times in the list.

    The order is guaranteed to be the same as that used by values().

    \sa values(), key()
*/

/*! \fn QList<Key> QHash::keys(const T &value) const

    \overload

    Returns a list containing all the keys associated with value \a
    value, in an arbitrary order.

    This function can be slow (\l{linear time}), because QHash's
    internal data structure is optimized for fast lookup by key, not
    by value.
*/

/*! \fn QList<T> QHash::values() const

    Returns a list containing all the values in the hash, in an
    arbitrary order. If a key is associated multiple values, all of
    its values will be in the list, and not just the most recently
    inserted one.

    The order is guaranteed to be the same as that used by keys().

    \sa keys()
*/

/*! \fn QList<T> QHash::values(const Key &key) const

    \overload

    Returns a list of all the values associated with key \a key,
    from the most recently inserted to the least recently inserted.

    \sa count(), insertMulti()
*/

/*! \fn Key QHash::key(const T &value) const

    Returns the first key with value \a value.

    If the hash contains no item with value \a value, the function
    returns a \link {default-constructed value} default-constructed
    key \endlink.

    This function can be slow (\l{linear time}), because QHash's
    internal data structure is optimized for fast lookup by key, not
    by value.

    \sa value(), values()
*/

/*! \fn int QHash::count(const Key &key) const

    Returns the number of items associated with key \a key.

    \sa contains(), insertMulti()
*/

/*! \fn int QHash::count() const

    \overload

    Same as size().
*/

/*! \fn QHash::iterator QHash::begin()

    Returns a \l{STL-style iterator} pointing to the first item in
    the hash.

    \sa constBegin(), end()
*/

/*! \fn QHash::const_iterator QHash::begin() const

    \overload
*/

/*! \fn QHash::const_iterator QHash::constBegin() const

    Returns a const \l{STL-style iterator} pointing to the first item
    in the hash.

    \sa begin(), constEnd()
*/

/*! \fn QHash::iterator QHash::end()

    Returns a \l{STL-style iterator} pointing to the imaginary item
    after the last item in the hash.

    \sa begin(), constEnd()
*/

/*! \fn QHash::const_iterator QHash::end() const

    \overload
*/

/*! \fn QHash::const_iterator QHash::constEnd() const

    Returns a const \l{STL-style iterator} pointing to the imaginary
    item after the last item in the hash.

    \sa constBegin(), end()
*/

/*! \fn QHash::iterator QHash::erase(iterator pos)

    Removes the (key, value) pair associated with the iterator \a pos
    from the hash, and returns an iterator to the next item in the
    hash.

    \sa remove()
*/

/*! \fn QHash::iterator QHash::find(const Key &key)

    Returns an iterator pointing to the item with key \a key in the
    hash.

    If the hash contains no item with key \a key, the function
    returns end().

    If the hash contains multiple items with key \a key, this
    function returns an iterator that points to the most recently
    inserted value. The other values are accessible by incrementing
    the iterator. For example, here's some code that iterates over all
    the items with the same key:

    \code
        QHash<QString, int> hash;
        ...
        QHash<QString, int>::const_iterator i = hash.find("HDR");
        while (i != hash.end() && i.key() == "HDR") {
            cout << i.value() << endl;
            ++i;
        }
    \endcode

    \sa value(), values()
*/

/*! \fn QHash::const_iterator QHash::find(const Key &key) const

    \overload
*/

/*! \fn QHash::iterator QHash::insert(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the key \a key, that item's value
    is replaced with \a value.

    If there are multiple items with the key \a key, the most
    recently inserted item's value is replaced with \a value.

    \sa insertMulti()
*/

/*! \fn QHash::iterator QHash::insertMulti(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the same key in the hash, this
    function will simply create a new one. (This behavior is
    different from insert(), which overwrites the value of an
    existing item.)

    \sa insert(), values()
*/

/*! \fn QHash<Key, T> &QHash::merge(const QHash<Key, T> &other)

    Inserts all the items in the \a other hash into this hash. If a
    key is common to both hashes, the resulting hash will contain the
    key multiple times.

    \sa insertMulti()
*/

/*! \fn bool QHash::empty() const

    This function is provided for STL compatibility. It is equivalent
    to isEmpty().
*/

/*! \fn bool QHash::sameKey(const Key &key1, const Key &key2)

    \internal
*/

/*! \typedef QHash::ConstIterator

    Qt-style synonym for QHash::const_iterator.
*/

/*! \typedef QHash::Iterator

    Qt-style synonym for QHash::iterator.
*/


/*! \typedef QHash::iterator::difference_type
    \internal
*/

/*! \typedef QHash::iterator::iterator_category
    \internal
*/

/*! \typedef QHash::iterator::pointer
    \internal
*/

/*! \typedef QHash::iterator::reference
    \internal
*/

/*! \typedef QHash::iterator::value_type
    \internal
*/

/*! \typedef QHash::const_iterator::difference_type
    \internal
*/

/*! \typedef QHash::const_iterator::iterator_category
    \internal
*/

/*! \typedef QHash::const_iterator::pointer
    \internal
*/

/*! \typedef QHash::const_iterator::reference
    \internal
*/

/*! \typedef QHash::const_iterator::value_type
    \internal
*/

/*! \class QHash::iterator
    \brief The QHash::iterator class provides an STL-style non-const iterator for QHash and QMultiHash.

    QHash features both \l{STL-style iterators} and \l{Java-style
    iterators}. The STL-style iterators are more low-level and more
    cumbersome to use; on the other hand, they are slightly faster
    and, for developers who already know STL, have the advantage of
    familiarity.

    QHash\<Key, T\>::iterator allows you to iterate over a QHash (or
    QMultiHash) and to modify the value (but not the key) associated
    with a particular key. If you want to iterate over a const QHash,
    you should use QHash::const_iterator. It is generally good
    practice to use QHash::const_iterator on a non-const QHash as
    well, unless you need to change the QHash through the iterator.
    Const iterators are slightly faster, and can improve code
    readability.

    The default QHash::iterator constructor creates an uninitialized
    iterator. You must initialize it using a QHash function like
    QHash::begin(), QHash::end(), or QHash::find() before you can
    start iterating. Here's a typical loop that prints all the (key,
    value) pairs stored in a hash:

    \code
        QHash<QString, int> hash;
        hash.insert("January", 1);
        hash.insert("February", 2);
        ...
        hash.insert("December", 12);

        QHash<QString, int>::iterator i;
        for (i = hash.begin(); i != hash.end(); ++i)
            cout << i.key() << ": " << i.value() << endl;
    \endcode

    Unlike QMap, which orders its items by key, QHash stores its
    items in an arbitrary order. The only guarantee is that items that
    share the same key (because they were inserted using
    QHash::insertMulti()) will appear consecutively, from the most
    recently to the least recently inserted value.

    Let's see a few examples of things we can do with a
    QHash::iterator that we cannot do with a QHash::const_iterator.
    Here's an example that increments every value stored in the QHash
    by 2:

    \code
        QHash<QString, int>::iterator i;
        for (i = hash.begin(); i != hash.end(); ++i)
            i.value() += 2;
    \endcode

    Here's an example that removes all the items whose key is a
    string that starts with an underscore character:

    \code
        QHash<QString, int>::iterator i = hash.begin();
        while (i != hash.end()) {
            if (i.key().startsWith("_"))
                i = hash.erase(i);
            else
                ++i;
        }
    \endcode

    The call to QHash::erase() removes the item pointed to by the
    iterator from the hash, and returns an iterator to the next item.
    Here's another way of removing an item while iterating:

    \code
        QHash<QString, int>::iterator i = hash.begin();
        while (i != hash.end()) {
            QHash<QString, int>::iterator prev = i;
            ++i;
            if (prev.key().startsWith("_"))
                hash.erase(prev);
        }
    \endcode

    It might be tempting to write code like this:

    \code
        // WRONG
        while (i != hash.end()) {
            if (i.key().startsWith("_"))
                hash.erase(i);
            ++i;
        }
    \endcode

    However, this will potentially crash in \c{++i}, because \c i is
    a dangling iterator after the call to erase().

    Multiple iterators can be used on the same hash. However, be
    aware that any modification performed directly on the QHash has
    the potential of dramatically changing the order in which the
    items are stored in the hash, as they might cause QHash to rehash
    its internal data structure. There is one notable exception:
    QHash::erase(). This function can safely be called while
    iterating, and won't affect the order of items in the hash. If you
    need to keep iterators over a long period of time, we recommend
    that you use QMap rather than QHash.

    \sa QHash::const_iterator, QMutableHashIterator
*/

/*! \fn QHash::iterator::operator Node *() const

    \internal
*/

/*! \fn QHash::iterator::iterator()

    Constructs an uninitialized iterator.

    Functions like key(), value(), and operator++() must not be
    called on an uninitialized iterator. Use operator=() to assign a
    value to it before using it.

    \sa QHash::begin() QHash::end()
*/

/*! \fn QHash::iterator::iterator(void *node)

    \internal
*/

/*! \fn const Key &QHash::iterator::key() const

    Returns the current item's key as a const reference.

    There is no direct way of changing an item's key through an
    iterator, although it can be done by calling QHash::erase()
    followed by QHash::insert() or QHash::insertMulti().

    \sa value()
*/

/*! \fn T &QHash::iterator::value() const

    Returns a modifiable reference to the current item's value.

    You can change the value of an item by using value() on
    the left side of an assignment, for example:

    \code
        if (i.key() == "Hello")
            i.value() = "Bonjour";
    \endcode

    \sa key(), operator*()
*/

/*! \fn T &QHash::iterator::operator*() const

    Returns a modifiable reference to the current item's value.

    Same as value().

    \sa key()
*/

/*! \fn T *QHash::iterator::operator->() const

    Returns a pointer to the current item's value.

    \sa value()
*/

/*! \fn bool QHash::iterator::operator==(const iterator &other) const

    Returns true if \a other points to the same item as this
    iterator; otherwise returns false.

    \sa operator!=()
*/

/*! \fn bool QHash::iterator::operator!=(const iterator &other) const

    Returns true if \a other points to a different item than this
    iterator; otherwise returns false.

    \sa operator==()
*/

/*!
    \fn QHash::iterator QHash::iterator::operator++()

    The prefix ++ operator (\c{++i}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.

    Calling this function on QHash::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn QHash::iterator QHash::iterator::operator++(int)

    \overload

    The postfix ++ operator (\c{i++}) advances the iterator to the
    next item in the hash and returns an iterator to the previously
    current item.
*/

/*!
    \fn QHash::iterator QHash::iterator::operator--()

    The prefix -- operator (\c{--i}) makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on QHash::begin() leads to undefined
    results.

    \sa operator++()
*/

/*!
    \fn QHash::iterator QHash::iterator::operator--(int)

    \overload

    The postfix -- operator (\c{i--}) makes the preceding item
    current and returns an iterator pointing to the previously
    current item.
*/

/*! \fn QHash::iterator QHash::iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. (If \a j is negative, the iterator goes backward.)

    This operation can be slow for large \a j values.

    \sa operator-()

*/

/*! \fn QHash::iterator QHash::iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. (If \a j is negative, the iterator goes forward.)

    This operation can be slow for large \a j values.

    \sa operator+()
*/

/*! \fn QHash::iterator &QHash::iterator::operator+=(int j)

    Advances the iterator by \a j items. (If \a j is negative, the
    iterator goes backward.)

    \sa operator-=(), operator+()
*/

/*! \fn QHash::iterator &QHash::iterator::operator-=(int j)

    Makes the iterator go back by \a j items. (If \a j is negative,
    the iterator goes forward.)

    \sa operator+=(), operator-()
*/

/*! \class QHash::const_iterator
    \brief The QHash::const_iterator class provides an STL-style const iterator for QHash and QMultiHash.

    QHash features both \l{STL-style iterators} and \l{Java-style
    iterators}. The STL-style iterators are more low-level and more
    cumbersome to use; on the other hand, they are slightly faster
    and, for developers who already know STL, have the advantage of
    familiarity.

    QHash\<Key, T\>::const_iterator allows you to iterate over a
    QHash (or a QMultiHash). If you want to modify the QHash as you
    iterate over it, you must use QHash::iterator instead. It is
    generally good practice to use QHash::const_iterator on a
    non-const QHash as well, unless you need to change the QHash
    through the iterator. Const iterators are slightly faster, and
    can improve code readability.

    The default QHash::const_iterator constructor creates an
    uninitialized iterator. You must initialize it using a QHash
    function like QHash::constBegin(), QHash::constEnd(), or
    QHash::find() before you can start iterating. Here's a typical
    loop that prints all the (key, value) pairs stored in a hash:

    \code
        QHash<QString, int> hash;
        hash.insert("January", 1);
        hash.insert("February", 2);
        ...
        hash.insert("December", 12);

        QHash<QString, int>::const_iterator i;
        for (i = hash.constBegin(); i != hash.constEnd(); ++i)
            cout << i.key() << ": " << i.value() << endl;
    \endcode

    Unlike QMap, which orders its items by key, QHash stores its
    items in an arbitrary order. The only guarantee is that items that
    share the same key (because they were inserted using
    QHash::insertMulti()) will appear consecutively, from the most
    recently to the least recently inserted value.

    Multiple iterators can be used on the same hash. However, be aware
    that any modification performed directly on the QHash has the
    potential of dramatically changing the order in which the items
    are stored in the hash, as they might cause QHash to rehash its
    internal data structure. If you need to keep iterators over a long
    period of time, we recommend that you use QMap rather than QHash.

    \sa QHash::iterator, QHashIterator
*/

/*! \fn QHash::const_iterator::operator Node *() const

    \internal
*/

/*! \fn QHash::const_iterator::const_iterator()

    Constructs an uninitialized iterator.

    Functions like key(), value(), and operator++() must not be
    called on an uninitialized iterator. Use operator=() to assign a
    value to it before using it.

    \sa QHash::constBegin() QHash::constEnd()
*/

/*! \fn QHash::const_iterator::const_iterator(void *node)

    \internal
*/

/*! \fn QHash::const_iterator::const_iterator(const iterator &other)

    Constructs a copy of \a other.
*/

/*! \fn const Key &QHash::const_iterator::key() const

    Returns the current item's key.

    \sa value()
*/

/*! \fn const T &QHash::const_iterator::value() const

    Returns the current item's value.

    \sa key(), operator*()
*/

/*! \fn const T &QHash::const_iterator::operator*() const

    Returns the current item's value.

    Same as value().

    \sa key()
*/

/*! \fn const T *QHash::const_iterator::operator->() const

    Returns a pointer to the current item's value.

    \sa value()
*/

/*! \fn bool QHash::const_iterator::operator==(const const_iterator &other) const

    Returns true if \a other points to the same item as this
    iterator; otherwise returns false.

    \sa operator!=()
*/

/*! \fn bool QHash::const_iterator::operator!=(const const_iterator &other) const

    Returns true if \a other points to a different item than this
    iterator; otherwise returns false.

    \sa operator==()
*/

/*!
    \fn QHash::const_iterator QHash::const_iterator::operator++()

    The prefix ++ operator (\c{++i}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.

    Calling this function on QHash::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn QHash::const_iterator QHash::const_iterator::operator++(int)

    \overload

    The postfix ++ operator (\c{i++}) advances the iterator to the
    next item in the hash and returns an iterator to the previously
    current item.
*/

/*! \fn QHash::const_iterator &QHash::const_iterator::operator--()

    The prefix -- operator (\c{--i}) makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on QHash::begin() leads to undefined
    results.

    \sa operator++()
*/

/*! \fn QHash::const_iterator QHash::const_iterator::operator--(int)

    \overload

    The postfix -- operator (\c{i--}) makes the preceding item
    current and returns an iterator pointing to the previously
    current item.
*/

/*! \fn QHash::const_iterator QHash::const_iterator::operator+(int j) const

    Returns an iterator to the item at \a j positions forward from
    this iterator. (If \a j is negative, the iterator goes backward.)

    This operation can be slow for large \a j values.

    \sa operator-()
*/

/*! \fn QHash::const_iterator QHash::const_iterator::operator-(int j) const

    Returns an iterator to the item at \a j positions backward from
    this iterator. (If \a j is negative, the iterator goes forward.)

    This operation can be slow for large \a j values.

    \sa operator+()
*/

/*! \fn QHash::const_iterator &QHash::const_iterator::operator+=(int j)

    Advances the iterator by \a j items. (If \a j is negative, the
    iterator goes backward.)

    This operation can be slow for large \a j values.

    \sa operator-=(), operator+()
*/

/*! \fn QHash::const_iterator &QHash::const_iterator::operator-=(int j)

    Makes the iterator go back by \a j items. (If \a j is negative,
    the iterator goes forward.)

    This operation can be slow for large \a j values.

    \sa operator+=(), operator-()
*/

/*! \fn uint qHash(char key)
    \relates QHash

    Returns the hash value for \a key.
*/

/*! \fn uint qHash(uchar key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(signed char key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(ushort key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(short key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(uint key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(int key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(ulong key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(long key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(quint64 key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(qint64 key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(QChar key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(const QByteArray &key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(const QString &key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(const T *key)
    \relates QHash

    \overload
*/

/*! \fn QDataStream &operator<<(QDataStream &out, const QHash<Key, T>& hash)
    \relates QHash

    Writes the hash \a hash to stream \a out.

    This function requires the key and value types to implement \c
    operator<<().

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

/*! \fn QDataStream &operator>>(QDataStream &in, QHash<Key, T> &hash)
    \relates QHash

    Reads a hash from stream \a in into \a hash.

    This function requires the key and value types to implement \c
    operator>>().

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

/*! \class QMultiHash
    \brief The QMultiHash class is a convenience QHash subclass that provides multi-valued hashes.

    \ingroup qtl
    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QMultiHash\<Key, T\> is one of Qt's generic \l{container classes}.
    It inherits QHash and extends it with a few convenience functions
    that make it more suitable than QHash for storing multi-valued
    hashes. A multi-valued hash is a hash that allows multiple values
    with the same key; QHash normally doesn't allow that, unless you
    call QHash::insertMulti().

    Because QMultiHash inherits QHash, all of QHash's functionality also
    applies to QMultiHash. For example, you can use isEmpty() to test
    whether the hash is empty, and you can traverse a QMultiHash using
    QHash's iterator classes (for example, QHashIterator). But in
    addition, it provides an insert() function that corresponds to
    QHash::insertMulti(), and a replace() function that corresponds to
    QHash::insert(). It also provides convenient operator+() and
    operator+=().

    Example:
    \code
        QMultiHash<QString, int> hash1, hash2, hash3;

        hash1.insert("plenty", 100);
        hash1.insert("plenty", 2000);
        // hash1.size() == 2

        hash2.insert("plenty", 5000);
        // hash2.size() == 1

        hash3 = hash1 + hash2;
        // hash3.size() == 3
    \endcode

    Unlike QHash, QMultiHash provides no operator[]. Use value() or
    replace() if you want to access the most recently inserted item
    with a certain key.

    If you want to retrieve all the values for a single key, you can
    use values(const Key &key), which returns a QList<T>:

    \code
        QList<int> values = hash.values("plenty");
        for (int i = 0; i < values.size(); ++i)
            cout << values.at(i) << endl;
    \endcode

    The items that share the same key are available from most
    recently to least recently inserted.

    A more efficient approach is to use QHashIterator::findNextKey() or
    QMutableHashIterator::findNextKey():

    \code
        QHashIterator<QString, int> i(hash);
        while (i.findNextKey("plenty"))
            cout << i.value() << endl;
    \endcode

    If you prefer the STL-style iterators, you can call find() to get
    the iterator for the first item with a key and iterate from
    there:

    \code
        QMultiHash<QString, int>::iterator i = hash.find("plenty");
        while (i != hash.end() && i.key() == "plenty") {
            cout << i.value() << endl;
            ++i;
        }
    \endcode

    QMultiHash's key and value data types must be \l{assignable data
    types}. You cannot, for example, store a QWidget as a value;
    instead, store a QWidget *. In addition, QMultiHash's key type
    must provide operator==(), and there must also be a global
    qHash() function that returns a hash value for an argument of the
    key's type. See the QHash documentation for details.

    \sa QHash, QHashIterator, QMutableHashIterator, QMultiMap
*/

/*! \fn QMultiHash::QMultiHash()

    Constructs an empty hash.
*/

/*! \fn QMultiHash::QMultiHash(const QHash<Key, T> &other)

    Constructs a copy of \a other (which can be a QHash or a
    QMultiHash).

    \sa operator=()
*/

/*! \fn QMultiHash::iterator QMultiHash::replace(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the key \a key, that item's value
    is replaced with \a value.

    If there are multiple items with the key \a key, the most
    recently inserted item's value is replaced with \a value.

    \sa insert()
*/

/*! \fn QMultiHash::iterator QMultiHash::insert(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the same key in the hash, this
    function will simply create a new one. (This behavior is
    different from replace(), which overwrites the value of an
    existing item.)

    \sa replace()
*/

/*! \fn QMultiHash &QMultiHash::operator+=(const QMultiHash &other)

    Inserts all the items in the \a other hash into this hash
    and returns a reference to this hash.

    \sa insert()
*/

/*! \fn QMultiHash QMultiHash::operator+(const QMultiHash &other) const

    Returns a hash that contains all the items in this hash in
    addition to all the items in \a other. If a key is common to both
    hashes, the resulting hash will contain the key multiple times.

    \sa operator+=()
*/
