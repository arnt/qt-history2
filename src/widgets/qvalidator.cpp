/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.cpp#7 $
**
** C++ file skeleton
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qvalidator.h"
#include "qwidget.h"

#include <limits.h> // *_MIN, *_MAX

RCSTAG("$Id: //depot/qt/main/src/widgets/qvalidator.cpp#7 $");


/*!  \class QValidator qvalidator.h

  \brief The QValidator class provides ways to validate an input text.

  The class itself is abstract; two subclasses provide rudimentary
  numeric range checking.

  The class includes two virtual functions, isValid() and fixup().

  isValid() is pure virtual, so it must be implemented by every
  subclass.  It returns TRUE or FALSE depending on whether its
  argument is valid (for the class' definition of valid).

  fixup() is provided for validators that can repair some or all user
  errors.  The default does nothing.  QLineEdit, for example, will
  call fixup() if the user presses Return and the content is not
  currently valid, in case fixup() can do do magic.

  QValidator is generally used with QLineEdit, QSpinBox and QComboBox.
*/

/*!  Sets up the internal data structures used by the validator.  At
  the moment there aren't any.
*/

QValidator::QValidator( QWidget * parent, const char * name )
    : QObject( parent, name )
{
}


/*!  Deletes the validator and frees any storage and other resources
  used.
*/

QValidator::~QValidator()
{
}


/*! \fn bool QValidator::isValid( const char * input )

  This pure virtual function returns TRUE if \a input is valid
  according to this validator's rules, and FALSE else.
*/


/*!  Attempts to change \a to be valid according to this validator's
  rules.  Need not result in a valid string - callers of this function
  must re-test afterwards.  The default does nothing.

  Reimplementation notes:

  Note that \a input may not be the only QString object referencing
  this string, so it's almost always necessary to detach() the string
  at the start of the code:

  \code
    input.detach();
  \endcode

  You can change \a input even if you aren't able to produce a valid
  string.  For example an ISBN validator might want to delete every
  character except digits and "-", even if the result is not a valid
  ISBN, and a last-name validator might want to remove white space
  from the start and end of the string, even if the resulting string
  is not in the list of known last names.
*/

void QValidator::fixup( QString & input )
{
    NOT_USED(input);
}


/*! \class QIntValidator qvalidator.h

  \brief The QIntValidator class provides range checking of integers.

  QIntValidator provides a lower and an upper bound.  It does not
  provide a fixup() function.

  \sa QDoubleValidator
*/


/*!  Creates a validator object which accepts all integers.
*/

QIntValidator::QIntValidator( QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = INT_MIN;
    t = INT_MAX;
}


/*!  Creates a validator object which accepts all integers from \a
  bottom up to and including \a top.
*/

QIntValidator::QIntValidator( int bottom, int top,
			      QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = bottom;
    t = top;
}


/*!  Deletes the validator and frees up any storage used.
*/

QIntValidator::~QIntValidator()
{
    // nothing
}


/*!  Returns TRUE if \a input contains a number in the legal range.
*/

bool QIntValidator::isValid( const char * input )
{
    bool ok;
    QString s( input );
    long int tmp = s.toLong( &ok );
    return ok && tmp >= b && tmp <= t;
}


/*!  Sets the validator to accept only number from \a botton up to an
  including \a top.
*/

void QIntValidator::setRange( int bottom, int top )
{
    b = bottom;
    t = top;
}


/*! \fn int QIntValidator::bottom() const

  Returns the lowest valid number according to this validator.

  \sa top() setRange()

*/


/*! \fn int QIntValidator::top() const

  Returns the highest valid number according to this validator.

  \sa bottom() setRange()
*/


/*! \class QDoubleValidator qvalidator.h

  \brief The QDoubleValidator class provides range checking of integers.

  QDoubleValidator provides an upper bound, a lower bound, and a limit
  on the number of digits after the decimal point.  It does not
  provide a fixup() function.

  \sa QIntValidator
*/

/*!  Creates a validator object which accepts all double from 2.7182818
  to 3.1415926 (please, no bug reports) with at most seven digits after
  the decimal point.

  This constructor is not meant to be useful; it is provided for
  completeness.
*/

QDoubleValidator::QDoubleValidator( QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = 2.7182818;
    t = 3.1415926;
    d = 7;
}


/*!  Creates a validator object which accepts all doubles from \a
  bottom up to and including \a top with at most \a decimals digits
  after the decimal point.
*/

QDoubleValidator::QDoubleValidator( double bottom, double top, int decimals,
				    QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = bottom;
    t = top;
    d = decimals;
}


/*!  Deletes the validator and frees any storage and other resources
  used.
*/

QDoubleValidator::~QDoubleValidator()
{
    // nothing
}


/*!  Returns TRUE if \a input contains a number in the legal range and
  format.
*/

bool QDoubleValidator::isValid( const char * input )
{
    bool ok;
    QString s( input );
    double tmp = s.toDouble( &ok );
    // check the number of decimals here!
    return ok && tmp >= b && tmp <= t;
}


/*!  Sets the validator to accept numbers from \a bottom up to and
  including \a top with at most \a decimals digits after the decimal
  point.
*/

void QDoubleValidator::setRange( double bottom, double top, int decimals )
{
    b = bottom;
    t = top;
    d = decimals;
}


/*! \fn double QDoubleValidator::bottom() const

  Returns the lowest valid number according to this validator.

  \sa top() decimals() setRange()

*/


/*! \fn double QDoubleValidator::top() const

  Returns the highest valid number according to this validator.

  \sa bottom() decimals setRange()
*/


/*! \fn int QDoubleValidator::decimals() const

  Returns the largest number of digits a valid number can have after
  its decimal point.

  \sa bottom() top() setRange()
*/
