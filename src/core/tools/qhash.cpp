#include "qhash.h"

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
    int n = key.length();
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
    1, 21,  3, 21,  7, 15,  9,  5,  3, 29, 15
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
    0, 0, Q_ATOMIC_INIT(1), 0, MinNumBits, 0, 0
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
    d->ref = 1;
    d->size = size;
    d->userNumBits = userNumBits;
    d->numBits = numBits;
    d->numBuckets = numBuckets;

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

QHashData::Node *QHashData::prevNode(Node *node)
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

void QHashData::rehash(int hint)
{
    if (hint < 0) {
	hint = countBits(-hint);
	if (hint < MinNumBits)
	    hint = MinNumBits;
	userNumBits = hint;
	if (size > (1 << userNumBits))
	    return;
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
    \brief The QHash class is a generic container that provides a hash-table-based dictionary.

    QHash\<Key, T\> is one of Qt's \l{generic container classes}. It
    stores (key, value) pairs and provides very fast lookup of the
    value associated with a key.

    QHash provides very similar functionality to QMap. The
    differences are:

    \list
    \i QHash provides faster lookups than QMap.
    \i When iterating over a QMap, the items are always sorted by
       key. With QHash, the items can be in any order.
    \i The key type of a QMap must provide operator<(). The key
       type of a QHash must provide operator==() and qHash().
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

    If you want to check whether the hash contains a certain key, use
    contains():

    \code
	int timeout = 30;
        if (hash.contains("TIMEOUT"))
	    timeout = hash.value("TIMEOUT");
    \endcode
    
    There is also a value() overload that returns its second argument
    if there is no item with the specified key:

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
	for (int i = 0; i < 1000; ++i) {
	    if (hash[i] == okButton)
		cout << "Found button at index " << i << endl;
        }
    \endcode

    To avoid that problem, replace \c hash[i] with \c hash.value(i)
    in the code above.

    If you want to navigate through all the (key, value) pairs stored
    in a QHash, you can use an iterator. Here's how to iterate over a
    QHash<QString, int> using a \l{Java-style iterator}:

    \code
	QHashIterator<QString, int> it(hash);
        while (it.hasNext()) {
	    it.next();
            cout << it.key() << ": " << it.value() << endl;
        }
    \endcode

    Here's the same code, but using an \l{STL-style iterator} this time:

    \code
	QHash<QString, int>::ConstIterator it = hash.begin();
        while (it != hash.end()) {
	    cout << it.key() << ": " << it.value() << endl;
	    ++it;
        }
    \endcode

    Normally, a QHash allows only one value per key. If you call
    insert() with a key that already exists in the QHash, the
    previous value will be erased. For example:

    \code
	hash.insert("plenty", 100);
        hash.insert("plenty", 2000);
        // hash.value("plenty") == 2000
    \endcode

    However, you can store multiple values per key by using
    insertMulti() instead of insert(). If you want to retrieve all
    the values for a single key, you can use values(const T &key),
    which returns a QList<T>:

    \code
	QList<int> values = hash.values("plenty");
        for (int i = 0; i < values.size(); ++i)
	    cout << values.at(i) << endl;
    \endcode

    Alternatively, you can call find()
    to get the STL-style iterator for the first item with a key
    and iterate from there:

    \code
	QHash<QString, int>::Iterator it = hash.find("plenty");
        while (it != hash.end() && it.key() == "plenty") {
	    cout << it.value() << endl;
	    ++it;
        }
    \endcode

    The items that share the same key are available from most
    recently to less recently inserted.

    If all you need is the values stored in a hash (not the keys),
    you can also use \l{foreach}:

    \code
	QHash<QString, int> hash;
        ...
	foreach (int value, hash)
	    cout << value << endl;
    \endcode

    Items can be removed from the hash in several ways. One way is to
    call remove(); this will remove any item with a certain key.
    Another way is to use QHashMutableIterator::remove(). In
    addition, you can clear the entire hash using clear().

    QHash's key and value data types must be \l{assignable data
    types}. You cannot, for example, store a QWidget as a value;
    instead, store a QWidget *. In addition, QHash's key type must
    provide operator==(), and there must exist a global qHash()
    function that returns a hash value for the type.

    Here's a list of the C++ and Qt types that can serve as keys in a
    QHash: any integer type (char, unsigned long, etc.), any pointer
    type, QString, and QByteArray. For all of these, \c <qhash.h>
    defines a qHash() function that computes an adequate hash value.
    If you want to use other types as the key, make sure that you
    provide operator==() and a qHash() implementation.

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
    largest possible extent.

    Internally, QHash uses a hash table to perform lookups. Unlike
    QDict in Qt 3, which required to be initialized with a prime
    number, QHash's hash table automatically grows and shrinks to
    provide fast lookups without wasting too much memory. You can
    still control the size of the hash table by calling reserve() if
    you already know approximately how many items the QHash may
    contain, but this isn't necessary to obtain good performance. You
    can also call capacity() to retreive the hash table size.

    \sa QMap
*/

/*! \fn QHash::QHash()

    Constructs an empty hash.

    \sa clear()
*/

/*! \fn QHash::QHash(const QHash<Key, T> &other)

    Constructs a copy of \a other.

    This operation occurs in \l{constant time}, because QHash is
    \l{implicitly shared}. This makes return a QHash from a function
    very fast. If a shared instance is modified, it will be copied
    (copy-on-write), and this takes \l{linear time}.

    \sa operator=()
*/

/*! \fn QHash::~QHash()

    Destroys the hash. References to the values in the hash and all
    iterators of this hash become invalidated.
*/

/*! \fn QHash<Key, T> &QHash::operator=(const QHash<Key, T> &other)

    Assigns \a other to this hash and returns a reference to this hash.

    This operation occurs in \l{constant time}, because QHash is
    \l{implicitly shared}.

    All iterators in the current hash become invalidated by this
    operation.
*/

/*! \fn bool QHash::operator==(const QHash<Key, T> &other) const

    Returns true if \a other is equal to this hash; otherwise returns
    false.

    Two hashes are considered equal if they contain the same (key,
    value) pairs.

    \sa operator!=()
*/

/*! \fn bool QHash::operator!=(const QHash<Key, T> &other) const

    Returns true if \a other is not equal to this hash; otherwise
    returns false.

    Two hashes are considered equal if they contain the same (key,
    value) pairs.

    \sa operator==()
*/

/*! \fn int QHash::size() const

    Returns the number of items in the hash.

    \sa isEmpty(), count()
*/

/*! \fn bool QHash::isEmpty() const

    Returns true if the hash contains no items; returns false
    otherwise.

    \sa size()
*/

/*! \fn bool QHash::operator!() const

    ### needed?
*/

/*! \fn QHash::operator QSafeBool() const

    Returns true if the hash contains some items; returns true
    otherwise.

    Example:
    \code
	static QHash<QString, int> hash;
        ...
        if (!hash)
	    fillWithData(&hash);
    \endcode

    This is the same as \c{!hash.isEmpty()}.
*/

/*! \fn void QHash::reserve(int size)

    Ensures that the QHash's internal hash table consists of at least
    \a size buckets. Ideally, \a size should reflect the maximum
    number of items expected in the hash.

    This function is useful for code that needs to build a huge hash
    and wants to avoid repeated reallocation. Example:

    \code
	QHash<QString, int> hash;
        hash.reserve(5000);
	for (int i = 0; i < 5000; ++i)
	    hash.insert(keys[i], values[i]);
    \endcode

    The \a size parameter can be any number. Internally, QHash will
    compute a prime number to use for the number of buckets.

    If \a size is an underestimate, the worse that will happen is
    that the QHash will be a bit slower.

    In general, you will rarely ever need to call this function.
    QHash's internal hash table automatically shrinks or grows to
    provide good performance without wasting too much memory.

    \sa capacity()
*/

/*! \fn int QHash::capacity() const

    Returns the number of buckets in the QHash's internal hash table.

    This function is only useful if you are trying to tune QHash's
    memory and CPU usage using reserve(). If all you want to know is
    the number of items in the hash, call size().

    \sa reserve()
*/

/*! \fn void QHash::detach()

    \internal

    Detaches this hash from other hashes with which it may share
    data.

    \sa isDetached()
*/

/*! \fn bool QHash::isDetached() const

    \internal

    Returns true if the hash's internal data isn't shared with any
    other hash object; returns false otherwise.

    \sa detach()
*/

/*! \fn void QHash::clear()

    Removes all items from the hash.

    \sa remove()
*/

/*! \fn int QHash::remove(const Key &key)

    Removes all items with the key \a key from the hash.

    \sa clear(), take()
*/

/*! \fn T QHash::take(const Key &key)

    Removes the item with the key \a key from the hash and returns
    the value associated with it.

    If the item does not exist in the hash, the function simply
    returns a \l{default-constructed} value. If there are multiple
    items for \a key in the hash, only the most recently inserted one
    is removed.

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
    returns a \l{default-constructed} value. If there are multiple
    items for \a key in the hash, the value of the most recently
    inserted one is returned.

    \sa contains()
*/

/*! \fn const T QHash::value(const Key &key, const T &defaultValue) const

    \overload

    If the hash contains no item with key \a key, the function returns
    \a defaultValue.
*/

/*! \fn T &QHash::operator[](const Key &key)

    Returns the value associated with the key \a key as a modifiable
    reference.

    If the hash contains no item with key \a key, the function
    inserts a \l{default-constructed} in the hash and returns a
    reference to it. If the hash contains multiple items with key \a
    key, this function returns the most recently inserted value.

    \sa insert(), value()
*/

/*! \fn const T QHash::operator[](const Key &key) const

    \overload

    Same as value().
*/

/*! \fn QList<Key> QHash::keys() const

    Returns a list of all the keys in the hash, in the order in which
    they are stored in the hash.

    \sa values()
*/

/*! \fn QList<T> QHash::values() const

    Returns a list of all the values in the hash, in the order in
    which they are stored in the hash.

    \sa keys()
*/

/*! \fn QList<T> QHash::values(const Key &key) const

    \overload

    Returns a list of all the values associated with a given key,
    from the most recently inserted to the less recently inserted
    one.

    \sa insertMulti()
*/

/*! \fn int QHash::count(const Key &key) const

    Returns the number of items associated with key \a key.

    \sa contains(), insertMulti()
*/

/*! \fn int QHash::count() const

    \overload

    Same as size().
*/

/*! \fn QHash::Iterator QHash::begin()

    Returns a \l{STL-style iterator} pointing to the first item in
    the hash.

    \sa constBegin(), end()
*/

/*! \fn QHash::ConstIterator QHash::begin() const

    \overload
*/

/*! \fn QHash::ConstIterator QHash::constBegin() const

    Returns a const \l{STL-style iterator} pointing to the first item
    in the hash.

    \sa begin(), constEnd()
*/

/*! \fn QHash::Iterator QHash::end()

    Returns a \l{STL-style iterator} pointing to the next-to-last item
    in the hash.

    \sa begin(), constEnd()
*/

/*! \fn QHash::ConstIterator QHash::end() const

    \overload
*/

/*! \fn QHash::ConstIterator QHash::constEnd() const

    Returns a const \l{STL-style iterator} pointing to the
    next-to-last item.

    \sa constBegin(), end()
*/

/*! \fn Iterator QHash::erase(Iterator it)

    Removes the item associated with the iterator \a it from the
    hash.

    \sa remove()
*/

/*! \fn Iterator QHash::find(const Key &key)

    Returns an iterator pointing to the item with key \a key in the
    hash.

    If the hash contains no item with key \a key, the function
    returns end().

    If the hash contains multiple items with key \a key, this
    function returns an iterator that points to the most recently
    inserted value. The other values are accessible by incrementing
    that iterator. For example, here's some code that iterates over
    all the items with the same key:

    \code
	QHash<QString, int> hash;
        ...
	QHash<QString, int>::Iterator it = hash.find("HDR");
        while (it != hash.end() && it.key() == "HDR") {
	    cout << it.value() << endl;
	    ++it;
        }
    \endcode

    \sa value()
*/

/*! \fn ConstIterator QHash::find(const Key &key) const

    \overload
*/

/*! \fn Iterator QHash::insert(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already one item with the key \a key, that item's
    value is replaced with \a value.

    If there are multiple items with the key \a key, the most
    recently inserted item's value is replaced with \a value.

    \sa insertMulti()
*/

/*! \fn Iterator QHash::insertMulti(const Key &key, const T &value)

    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the same key in the hash, this
    function will simply create a new one. (This behavior is
    different from insert(), which overwrites the value of an
    existing item.)

    \sa insert(), values()
*/

/*! \fn QHash<Key, T> &QHash::operator+=(const QHash<Key, T> &other)

    Inserts all the items in \a others into this hash.

    \sa insertMulti()
*/

/*! \fn QHash<Key, T> QHash::operator+(const QHash<Key, T> &other) const

    Returns a hash that contain all items in this hash in addition to
    all items in \a other.

    \sa insertMulti()
*/

/*! \typedef iterator

    STL-compatibility typedef.
*/

/*! \typedef const_iterator

    STL-compatibility typedef.
*/

/*! \fn bool QHash::empty() const

    STL-compatibility function. Same as isEmpty().
*/

/*! \fn bool QHash::ensure_constructed()

    \internal
*/

/*! \class QHash::Iterator

    ###
*/

/*! \fn QHash::Iterator::operator Node *() const

    \internal
*/

/*! \fn QHash::Iterator::Iterator()

    Creates an unitialized iterator.

    Functions like key(), value(), and operator*() should not be
    called on an unitialized iterator. Use operator=() to assign a
    value to it before using it.
*/

/*! \fn QHash::Iterator::Iterator(void *node)

    \internal
*/

/*! \fn const Key &QHash::Iterator::key() const

    Returns the current item's key as a const reference.

    There is no direct way of changing an item's key through an
    iterator. You need to call QHash::erase() followed by
    QHash::insert() or QHash::insertMulti().

    \sa value()
*/	

/*! \fn T &QHash::Iterator::value() const

    Returns a non-const reference to the current item's value.

    You can change the value of an item by using value() on
    the left side of an assignment, for example:

    \code
	if (it.key() == "Hello")
	    it.value() = "Bonjour";
    \endcode

    \sa key()
*/

/*! \fn T &QHash::Iterator::operator*() const

    Returns a non-const reference to the current item's value.

    Same as value().

    \sa key()
*/	

/*! \fn bool QHash::Iterator::operator==(const Iterator &other)

    Returns true if \a other points to the same item as this
    iterator; otherwise returns false.

    \sa operator!=()
*/	

/*! \fn bool QHash::Iterator::operator!=(const Iterator &other)

    Returns true if \a other points to a different item than this
    iterator; otherwise returns false.

    \sa operator==()
*/

/*! \fn QHash::Iterator &QHash::Iterator::operator++()

    The prefix ++ operator (\c{++it}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.

    Calling this function on QHash::end() leads to undefined results.

    \sa operator--()
*/	

/*! \fn QHash::Iterator QHash::Iterator::operator++(int)

    \overload

    The postfix ++ operator (\c{it++}) advances the iterator to the
    next item in the hash and returns an iterator to the previously
    current item.

    Calling this function on QHash::end() leads to undefined results.
*/

/*! \fn QHash::Iterator &QHash::Iterator::operator--()

    The prefix -- operator (\c{--it}) makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on QHash::begin() leads to undefined
    results.

    \sa operator++()
*/	

/*! \fn QHash::Iterator QHash::Iterator::operator--(int)

    \overload

    The postfix -- operator (\c{it--}) makes the preceding item
    current and returns an iterator pointing to the previously
    current item.

    Calling this function on QHash::begin() leads to undefined
    results.
*/

/*! \class QHash::ConstIterator

    ###
*/

/*! \fn QHash::ConstIterator::operator Node *() const

    \internal
*/

/*! \fn QHash::ConstIterator::ConstIterator()

    Creates an uinitialized iterator.

    Functions like key(), value(), and operator*() should not be
    called on an unitialized iterator. Use operator=() to assign a
    value to it before using it.
*/

/*! \fn QHash::ConstIterator::ConstIterator(void *node)

    \internal
*/

/*! \fn QHash::ConstIterator::ConstIterator(const Iterator &other)

    Constructs a copy of \a other.
*/

/*! \fn const Key &QHash::ConstIterator::key() const

    Returns the current item's key.

    \sa value()
*/

/*! \fn const T &QHash::ConstIterator::value() const

    Returns the current item's value.

    \sa key()
*/

/*! \fn const T &QHash::ConstIterator::operator*() const

    Returns the current item's value.

    Same as value().

    \sa key()
*/

/*! \fn bool QHash::ConstIterator::operator==(const ConstIterator &other)

    Returns true if \a other points to the same item as this
    iterator; otherwise returns false.

    \sa operator!=()
*/

/*! \fn bool QHash::ConstIterator::operator!=(const ConstIterator &other)

    Returns true if \a other points to a different item than this
    iterator; otherwise returns false.

    \sa operator==()
*/

/*! \fn QHash::ConstIterator QHash::ConstIterator::operator++()

    The prefix ++ operator (\c{++it}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.

    Calling this function on QHash::end() leads to undefined results.

    \sa operator--()
*/

/*! \fn QHash::ConstIterator QHash::ConstIterator::operator++(int)

    \overload

    The postfix ++ operator (\c{it++}) advances the iterator to the
    next item in the hash and returns an iterator to the previously
    current item.

    Calling this function on QHash::end() leads to undefined results.
*/

/*! \fn QHash::ConstIterator &QHash::ConstIterator::operator--()

    The prefix -- operator (\c{--it}) makes the preceding item
    current and returns an iterator pointing to the new current item.

    Calling this function on QHash::begin() leads to undefined
    results.

    \sa operator++()
*/

/*! \fn QHash::ConstIterator QHash::ConstIterator::operator--(int)

    \overload

    The postfix -- operator (\c{it--}) makes the preceding item
    current and returns an iterator pointing to the previously
    current item.

    Calling this function on QHash::begin() leads to undefined
    results.
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

/*! \fn uint qHash(Q_ULLONG key)
    \relates QHash

    \overload
*/

/*! \fn uint qHash(Q_LLONG key)
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

/*! \fn template <class T> uint qHash(const T *key)
    \relates QHash

    \overload
*/

/*! \fn template <class T> uint qHash(T *key)
    \relates QHash

    \overload
*/
