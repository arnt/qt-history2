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

/*!
    \class QPointer
    \brief The QPointer class is a template class that provides guarded pointers to QObjects.

    \ingroup objectmodel
    \mainclass

    A guarded pointer, QPointer<T>, behaves like a normal C++
    pointer \c{T *}, except that it is automatically set to 0 when the
    referenced object is destroyed (unlike normal C++ pointers, which
    become "dangling pointers" in such cases). \c T must be a
    subclass of QObject.

    Guarded pointers are useful whenever you need to store a pointer
    to a QObject that is owned by someone else, and therefore might be
    destroyed while you still hold a reference to it. You can safely
    test the pointer for validity.

    Example:

    \quotefromfile snippets/pointer/pointer.cpp
    \skipto QPointer<QLabel> label
    \printuntil setText(
    \dots
    \printline if (label)
    \printline show()

    If the QLabel is deleted in the meantime, the \c label variable
    will hold 0 instead of an invalid address, and the last line will
    never be executed.

    The functions and operators available with a QPointer are the
    same as those available with a normal unguarded pointer, except
    the pointer arithmetic operators (\c{+}, \c{-}, \c{++}, and
    \c{--}), which are normally used only with arrays of objects.

    Use QPointers like normal pointers and you will not need to read
    this class documentation.

    For creating guarded pointers, you can construct or assign to them
    from a T* or from another guarded pointer of the same type. You
    can compare them with each other using operator==() and
    operator!=(), or test for 0 with isNull(). You can dereference
    them using either the \c *x or the \c x->member notation.

    A guarded pointer will automatically cast to a \c T *, so you can
    freely mix guarded and unguarded pointers. This means that if you
    have a QPointer<QWidget>, you can pass it to a function that
    requires a QWidget *. For this reason, it is of little value to
    declare functions to take a QPointer as a parameter; just use
    normal pointers. Use a QPointer when you are storing a pointer
    over time.

    Note that class \c T must inherit QObject, or a compilation or
    link error will result.

    \sa QObject
*/

/*!
    \fn QPointer::QPointer()

    Constructs a 0 guarded pointer.

    \sa isNull()
*/

/*!
    \fn QPointer::QPointer(T* p)

    Constructs a guarded pointer that points to same object that \a p
    points to.
*/

/*!
    \fn QPointer::QPointer(const QPointer<T> &p)

    Copies one guarded pointer from another. The constructed guarded
    pointer points to the same object that \a p points to (which may
    be 0).
*/

/*!
    \fn QPointer::~QPointer()

    Destroys the guarded pointer. Just like a normal pointer,
    destroying a guarded pointer does \e not destroy the object being
    pointed to.
*/

/*!
    \fn QPointer<T>& QPointer::operator=(const QPointer<T> &p)

    Assignment operator. This guarded pointer will now point to the
    same object that \a p points to.
*/

/*!
    \fn QPointer<T> & QPointer::operator=(T* p)

    \overload

    Assignment operator. This guarded pointer will now point to the
    same object that \a p points to.
*/

/*!
    \fn bool QPointer::operator==(const QPointer<T> &p) const

    Equality operator; implements traditional pointer semantics.
    Returns true if both \a p and this guarded pointer are 0, or if
    both \a p and this pointer point to the same object; otherwise
    returns false.

    \sa operator!=()
*/

/*!
    \fn bool QPointer::operator==(const T *p) const

    \overload

    Equality operator; implements traditional pointer semantics.
    Returns true if both \a p and this guarded pointer are 0, or if
    both \a p and this pointer point to the same object; otherwise
    returns false.

    \sa operator!=()
*/

/*!
    \fn bool QPointer::operator!= (const QPointer<T>& p) const

    Inequality operator; implements pointer semantics, the negation of
    operator==(). Returns true if \a p and this guarded pointer are
    not pointing to the same object; otherwise returns false.
*/

/*!
    \fn bool QPointer::operator!= (const T *p) const

    \overload

    Inequality operator; implements pointer semantics, the negation of
    operator==(). Returns true if \a p and this guarded pointer are
    not pointing to the same object; otherwise returns false.
*/

/*!
    \fn bool QPointer::isNull() const

    Returns \c true if the referenced object has been destroyed or if
    there is no referenced object; otherwise returns false.
*/

/*!
    \fn T* QPointer::operator->() const

    Overloaded arrow operator; implements pointer semantics. Just use
    this operator as you would with a normal C++ pointer.
*/

/*!
    \fn T& QPointer::operator*() const

    Dereference operator; implements pointer semantics. Just use this
    operator as you would with a normal C++ pointer.
*/

/*!
    \fn QPointer::operator T*() const

    Cast operator; implements pointer semantics. Because of this
    function you can pass a QPointer\<T\> to a function where a T*
    is required.
*/

/*!
    \fn bool operator==(const T *o, const QPointer<T> &p)

    Equality operator. Returns true if \a o and the guarded
    pointer \a p are pointing to the same object, otherwise
    returns false.

*/

/*!
    \fn bool operator!=(const T *o, const QPointer<T> &p)

    Inequality operator. Returns true if \a o and the guarded
    pointer \a p are not pointing to the same object, otherwise
    returns false.
*/
