/*!
    \class QQueue
    \brief The QQueue class is a generic container that provides a queue.

    \ingroup qtl
    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QQueue\<T\> is one of Qt's generic \l{container classes}. It
    implements a queue data structure for items of a same type.

    A queue is a first in, first out (FIFO) structure. Items are
    added to the tail of the queue using enqueue() and retrieved from
    the head using dequeue(). The head() function provides access to
    the head item without removing it.

    Example:
    \code
	QQueue<int> queue;
	queue.enqueue(1);
	queue.enqueue(2);
	queue.enqueue(3);
	while (!queue.isEmpty())
	    cout << queue.dequeue() << endl;
    \endcode

    The example will output 1, 2, 3 in that order.

    QQueue inherits from QList. All of QList's functionality also
    applies to QList. For example, you can use isEmpty() to test
    whether the queue is empty, and you can traverse a QQueue using
    QList's iterator classes (for example, QListIterator). But in
    addition, QQueue provides three convenience functions that make
    it easy to implement FIFO semantics: enqueue(), dequeue(), and
    head().

    \sa QList, QStack
*/

/*!
    \fn QQueue::QQueue()

    Constructs an empty queue.
*/

/*!
    \fn QQueue::~QQueue()

    Destroys the queue. References to the values in the queue and all
    iterators of this queue become invalid.
*/

/*!
    \fn void QQueue::enqueue(const T& t)

    Adds value \a t to the tail of the queue.

    This is the same as QList::append().

    \sa dequeue(), head()
*/

/*!
    \fn T &QQueue::head()

    Returns a reference to the queue's head item. This function
    assumes that the queue isn't empty.

    This is the same as QList::first().

    \sa dequeue(), enqueue(), isEmpty()
*/

/*!
    \fn const T &QQueue::head() const

    \overload
*/

/*!
    \fn T QQueue::dequeue()

    Removes the head item in the queue and returns it. This function
    assumes that the queue isn't empty.

    This is the same as QList::takeFirst().

    \sa head(), enqueue(), isEmpty()
*/
