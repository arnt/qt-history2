/*!
    \class QStack qstack.h
    \brief The QStack class is a template class that provides a stack.

    \ingroup qtl
    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    Define a template instance QStack\<X\> to create a stack of values
    that all have the class X.

    A stack is a last in, first out (LIFO) structure. Items are added
    to the top of the stack with push() and retrieved from the top
    with pop(). The top() function provides access to the topmost item
    without removing it.

    Example:
    \code
	QStack<int> stack;
	stack.push(1);
	stack.push(2);
	stack.push(3);
	while (! stack.isEmpty())
	    cout << "Item: " << stack.pop() << endl;

	// Output:
	//	Item: 3
	//	Item: 2
	//	Item: 1
    \endcode

    QStack is a specialized QVector provided for convenience.  All of
    QVector's functionality also applies to QStack, for example the
    facility to iterate over all elements using QVectorIterator<T>, or
    the possibility to reserve extra capacity with reserve().

    Some classes cannot be used within a QStack directly, for example
    everything derived from QObject and thus all classes that
    implement widgets. If you need a stack of QWidgets you would
    instantiate a QStack<QWidget*> instead. Only values can be used in
    a QStack, including pointers to objects.  To qualify as a value,
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
    \fn QStack::QStack()

    Constructs an empty stack.
*/

/*!
    \fn QStack::~QStack()

    Destroys the stack. References to the values in the stack and all
    mutable iterators on this stack become invalidated. Because QStack
    is highly tuned for performance, you won't see warnings if you use
    invalid iterators because it is impossible for an iterator to
    check whether or not it is valid.
*/


/*!
    \fn void  QStack::push(const T& t)

    Adds element, \a t, to the top of the stack. Last in, first out.

    \sa pop(), top()
*/

/*!
    \fn T& QStack::top()

    Returns a reference to the top item of the stack, if there is any.

    \sa pop(), push()
*/


/*!
    \fn const T& QStack::top() const

    \overload

    Returns a reference to the top item of the stack, if there is any.

    \sa pop(), push()
*/

/*!
    \fn T QStack::pop()

    Removes the top item from the stack, if there is any, and returns it.

    \sa top() push()
*/





