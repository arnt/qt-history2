/*!
    \class QQueue
    \brief The QQueue class is a template class that provides a queue.

    \ingroup qtl
    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    Define a template instance QQueue\<X\> to create a queue of
    values of type X.

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
	    cout << "Item: " << queue.dequeue() << endl;

	// Output:
	//	Item: 1
	//	Item: 2
	//	Item: 3
    \endcode

    QQueue is a specialized QList provided for convenience.  All of
    QList's functionality also applies to QList, for example the
    facility to iterate over all elements using QList<T>::Iterator,
    or the possibility to reserve extra capacity with reserve().

    Some classes cannot be used within a QQueue directly, for example
    everything derived from QObject and thus all classes that
    implement widgets. If you need a queue of QWidgets you would
    instantiate a QQueue<QWidget *> instead. Only values can be used in
    a QQueue, including pointers to objects.  To qualify as a value,
    the class must provide
    \list
    \i a copy constructor;
    \i an assignment operator;
    \i a default constructor, i.e. a constructor that does not take any arguments.
    \endlist

    Note that C++ defaults to field-by-field assignment operators and
    copy constructors if no explicit version is supplied. In many
    cases this is sufficient.

    \important isEmpty()
*/

/*!
    \fn QQueue::QQueue()

    Constructs an empty queue.
*/

/*!
    \fn QQueue::~QQueue()

    Destroys the queue. References to the values in the queue and all
    mutable iterators on this queue become invalidated. Because QQueue
    is highly tuned for performance, you won't see warnings if you use
    invalid iterators because it is impossible for an iterator to
    check whether or not it is valid.
*/

/*!
    \fn void QQueue::enqueue(const T& t)

    Adds element \a t to the tail of the queue. Last in, last out.

    \sa dequeue(), head()
*/

/*!
    \fn T &QQueue::head()

    Returns a reference to the head item of the queue, if there is any.

    \sa dequeue(), enqueue()
*/

/*!
    \fn const T &QQueue::head() const

    \overload

    Returns a reference to the head item of the queue, if there is any.

    \sa dequeue(), enqueue()
*/

/*!
    \fn T QQueue::dequeue()

    Removes the head item from the queue, if there is any, and returns it.

    \sa head(), enqueue()
*/
