/****************************************************************************
** $Id$
**
** Implementation of validator classes
**
** Created : 970610
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qvalidator.h"
#ifndef QT_NO_VALIDATOR
#include "qwidget.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>

/*!
  \class QValidator qvalidator.h

  \brief The QValidator class provides validation of input text.

  \ingroup misc
  \mainclass

  The class itself is abstract. Two subclasses, \l QIntValidator and
  \l QDoubleValidator, provide basic numeric-range checking,
  and \l QRegExpValidator provides general checking using a custom
  regular expression.

  If the built-in validators aren't sufficient, you can subclass
  QValidator. The class has two virtual functions: validate()
  and fixup().

  validate() must be implemented by every subclass.  It returns \c
  Invalid, \c Intermediate or \c Acceptable depending on whether its
  argument is valid (for the subclass's definition of valid).

  These three states require some explanation.  An \c Invalid string is
  \e clearly invalid.  \c Intermediate is less obvious - the concept
  of validity is slippery when the string is incomplete (still being
  edited).  QValidator defines \c Intermediate as the property of a
  string that is neither clearly invalid nor acceptable as a final
  result.  \c Acceptable means that the string is acceptable as a
  final result.  One might say that any string that is a plausible
  intermediate state during entry of an \c Acceptable string is \c
  Intermediate.

  Here are some examples:

  \list

  \i For a line edit that accepts integers from 0 to 999 inclusive,
  42 and 666 are \c Acceptable, the empty string and 1114 are \c
  Intermediate and asdf is \c Invalid.

  \i For an editable combobox that accepts URLs, any well-formed URL
  is \c Acceptable, "http://www.trolltech.com/," is \c Intermediate (it might
  be a cut-and-paste that accidentally took in a comma at the
  end), the empty string is valid (the user might select and delete
  all of the text in preparation of entering a new URL) and
  "http:///./" is \c Invalid.

  \i For a spin box that accepts lengths, "11cm" and "1in" are \c
  Acceptable, "11" and the empty string are \c Intermediate and
  "http://www.trolltech.com" and "hour" are \c Invalid.

  \endlist

  fixup() is provided for validators that can repair some or all user
  errors.  The default does nothing.  QLineEdit, for example, will
  call fixup() if the user presses Enter and the content is not
  currently valid, in case fixup() can do magic.  This allows some \c
  Invalid strings to be made \c Acceptable, too, spoiling the muddy
  definition even more.

  QValidator is typically used with QLineEdit, QSpinBox and QComboBox.
*/


/*! \enum QValidator::State

  This enum type defines the states in which a validated string can
  exist.  There are currently three states:

  \value Invalid  the string is \e clearly invalid.

  \value Intermediate  the string is a plausible intermediate value
  during editing.

  \value Acceptable  the string is acceptable as a final result, i.e.
  it is valid.

  The state \c Valid has been renamed \c Intermediate.  The old name
  confused too many people and is now obsolete.
*/


/*!
  Sets up the internal data structures used by the validator.  At
  the moment there aren't any. The \a parent and \a name parameters
  are passed to the QObject constructor.
*/

QValidator::QValidator( QObject * parent, const char *name )
    : QObject( parent, name )
{
}


/*!
  Destroys the validator, freeing any storage and other resources
  used.
*/

QValidator::~QValidator()
{
}


/*!
  \fn QValidator::State QValidator::validate( QString& input, int& pos ) const

  This pure virtual function returns \c Invalid if \a input is invalid
  according to this validator's rules, \c Intermediate if it is likely that a
  little more editing will make the input acceptable (e.g. the user
  types '4' into a widget which accepts integers between 10 and 99) and
  \c Acceptable if the input is valid.

  The function can change \a input and \a pos (the cursor position) if
  it wants to.
*/


/*!
  \fn void QValidator::fixup( QString & input ) const

  This function attempts to change \a input to be valid according to
  this validator's rules. It need not result in a valid string -
  callers of this function must re-test afterwards; the default does
  nothing.

  Reimplementations of this function can change \a input even if they
  do not produce a valid string.  For example, an ISBN validator might
  want to delete every character except digits and "-", even if the
  result is not a valid ISBN; a surname validator might want to
  remove whitespace from the start and end of the string, even if the
  resulting string is not in the list of accepted surnames.
*/

void QValidator::fixup( QString & ) const
{
}


/*!
  \class QIntValidator qvalidator.h

  \brief The QIntValidator class provides a validator which ensures that
  a string contains a valid integer within a specified range.

  \ingroup misc

  The validate() function returns \c Acceptable, \c Intermediate or \c
  Invalid. \c Acceptable means that the string is a valid integer within
  the specified range. \c Intermediate means that the string is a valid
  integer but is not within the specified range. \c Invalid means that
  the string is not a valid integer.

  Example of use:

  \code
    //...
    #include <qlineedit.h>
    #include <qvalidator.h>
    //...
    QIntValidator v( 0, 100, this );
    QLineEdit* edit = new QLineEdit( this );
    // the edit lineedit will only accept integers between 0 and 100
    edit->setValidator( &v );
  \endcode

  Below we present some examples of validators. In practice they would
  normally be associated with a widget as in the example above.

  \code
    QString s;
    // a validator that will only accept integers between 0 and 100
    QIntValidator v( 0, 100, this );

    s = "10";
    v.validate( a, 0 );	// Returns Acceptable
    s = "35";
    v.validate( a, 0 );	// Returns Acceptable

    s = "105";
    v.validate( a, 0 );	// Returns Intermediate
    s = "-763";
    v.validate( a, 0 );	// Returns Intermediate

    s = "abc";
    v.validate( a, 0 );	// Returns Invalid;
    s = "12v";
    v.validate( a, 0 );	// Returns Invalid;
  \endcode

  The minimum and maximum values are set in one call with setRange() or
  individually with setBottom() and setTop().

  \sa QDoubleValidator QRegExpValidator
*/


/*!
  Constructs a validator that accepts all integers and has parent
  \a parent and name \a name.
*/

QIntValidator::QIntValidator( QObject * parent, const char *name )
    : QValidator( parent, name )
{
    b = INT_MIN;
    t = INT_MAX;
}


/*!
  Constructs a validator that accepts all integers from and including \a
  minimum up to and including \a maximum with parent \a parent and
  name \a name.
*/

QIntValidator::QIntValidator( int minimum, int maximum,
			      QObject * parent, const char* name )
    : QValidator( parent, name )
{
    b = minimum;
    t = maximum;
}


/*!
  Destroys the validator, freeing any resources allocated.
*/

QIntValidator::~QIntValidator()
{
    // nothing
}


/*!
  Returns \c Acceptable if the \a input is an integer within the valid
  range, \c Intermediate if the \a input is an integer outside the
  valid range and \c Invalid if the \a input is not an integer.

  \code
    s = "35";
    v.validate( a, 0 );	// Returns Acceptable

    s = "105";
    v.validate( a, 0 );	// Returns Intermediate

    s = "abc";
    v.validate( a, 0 );	// Returns Invalid;
  \endcode

*/

QValidator::State QIntValidator::validate( QString & input, int & ) const
{
    QRegExp empty( QString::fromLatin1("^ *-? *$") );
    if ( empty.search(input) == 0 )
	return QValidator::Intermediate;
    bool ok;
    long int tmp = input.toLong( &ok );
    if ( !ok )
	return QValidator::Invalid;
    else if ( tmp < b || tmp > t )
	return QValidator::Intermediate;
    else
	return QValidator::Acceptable;
}


/*!
  Sets the range of the validator to accept only integers between \a minimum and
  \a maximum inclusive.
*/

void QIntValidator::setRange( int minimum, int maximum )
{
    b = minimum;
    t = maximum;
}


/*! \property QIntValidator::bottom
    \brief the validator's lowest acceptable value

  \sa setRange()
*/
void QIntValidator::setBottom( int bottom )
{
    setRange( bottom, top() );
}

/*! \property QIntValidator::top
    \brief the validator's highest acceptable value

  \sa setRange()
*/
void QIntValidator::setTop( int top )
{
    setRange( bottom(), top );
}



/*!
  \class QDoubleValidator qvalidator.h

  \brief The QDoubleValidator class provides range checking of
  floating-point numbers.

  \ingroup misc

  QDoubleValidator provides an upper bound, a lower bound and a limit
  on the number of digits after the decimal point.  It does not
  provide a fixup() function.

  You can set the acceptable range in one call with setRange(), or with
  setBottom() and setTop(). Set the number of decimal places with
  setDecimals(). The validate() function returns the validation state.

  \sa QIntValidator QRegExpValidator
*/

/*!
  Constructs a validator object with parent \a parent, called \a name, which
  accepts any double.
*/

QDoubleValidator::QDoubleValidator( QObject * parent, const char *name )
    : QValidator( parent, name )
{
    b = -HUGE_VAL;
    t = HUGE_VAL;
    d = 1000;
}


/*!
  Constructs a validator object with parent \a parent, called \a name. This
  validator will accept doubles from \a bottom to \a top inclusive, with up to
  \a decimals digits after the decimal point.
*/

QDoubleValidator::QDoubleValidator( double bottom, double top, int decimals,
				    QObject * parent, const char* name )
    : QValidator( parent, name )
{
    b = bottom;
    t = top;
    d = decimals;
}


/*!
  Destroys the validator, freeing any resources used.
*/

QDoubleValidator::~QDoubleValidator()
{
}


/*!
    Returns \c Acceptable if the string \a input contains a double that is
    within the valid range and is in the correct format.

    Returns \c Intermediate if \a input contains a double that is outside the
    range or is in the wrong format, e.g. with too many digits after the
    decimal point or is empty.

    Returns \c Invalid if the \a input is not a double.
*/

QValidator::State QDoubleValidator::validate( QString & input, int & ) const
{
    QRegExp empty( QString::fromLatin1("^ *-?\\.? *$") );
    if ( empty.search(input) == 0 )
	return QValidator::Intermediate;
    bool ok = TRUE;
    double tmp = input.toDouble( &ok );
    if ( !ok ) {
	QRegExp expexpexp( QString::fromLatin1("e-?\\d*$"), FALSE );
	int eeePos = expexpexp.search( input ); // EXPlicit EXPonent regEXP
	int nume = input.contains( 'e', FALSE );
	if ( eeePos > 0 && nume < 2 ) {
	    QString mantissa = input.left( eeePos );
	    tmp = mantissa.toDouble( &ok );
	    if ( !ok )
		return QValidator::Invalid;
	}
	else if ( eeePos == 0 ) {
	    return QValidator::Intermediate;
	}
	else {
	    return QValidator::Invalid;
	}
    }

    int i = input.find( '.' );
    if ( i >= 0 ) {
	// has decimal point, now count digits after that
	i++;
	int j = i;
	while( input[j].isDigit() )
	    j++;
	if ( j - i > d )
	    return QValidator::Intermediate;
    }

    if ( tmp < b || tmp > t )
	return QValidator::Intermediate;
    else
	return QValidator::Acceptable;
}


/*!
  Sets the validator to accept doubles from \a minimum up to and
  including \a maximum with at most \a decimals digits after the decimal
  point.
*/

void QDoubleValidator::setRange( double minimum, double maximum, int decimals )
{
    b = minimum;
    t = maximum;
    d = decimals;
}

/*! \property QDoubleValidator::bottom
    \brief the validator's minimum acceptable value

    \sa setRange()
*/

void QDoubleValidator::setBottom( double bottom )
{
    setRange( bottom, top(), decimals() );
}


/*! \property QDoubleValidator::top
    \brief the validator's maximum acceptable value

    \sa setRange()
*/

void QDoubleValidator::setTop( double top )
{
    setRange( bottom(), top, decimals() );
}

/*! \property QDoubleValidator::decimals
    \brief the validator's maximum number of digits after the decimal point

    \sa setRange()
*/

void QDoubleValidator::setDecimals( int decimals )
{
    setRange( bottom(), top(), decimals );
}


/*!
  \class QRegExpValidator qvalidator.h

  \brief The QRegExpValidator class is used to check a string against a
  regular expression.

  \ingroup misc

  QRegExpValidator contains a regular expression, "regexp", used to
  determine whether an input string is \c Acceptable, \c Intermediate or
  \c Invalid.

  The regexp is treated as if it begins with the start of string
  assertion, <tt>^</tt>, and ends with the end of string assertion
  <tt>$</tt> so the match is against the entire input string, or from
  the given position if a start position greater than zero is given.

  For a brief introduction to Qt's regexp engine see QRegExp().

  Example of use:
  \code
    #include <qlineedit.h>
    #include <qregexp.h>
    #include <qvalidator.h>
    //...
    // Regexp: optional '-' followed by between 1 and 3 digits
    QRegExp rx( "-?\\d{1,3}" );
    QRegExpValidator validator( rx, 0 );
    QLineEdit *edit   = new QLineEdit( split );
    // edit widget will only accept numbers -999 to 999
    edit->setValidator( &validator );
  \endcode

  Below we present some examples of validators. In practice they would
  normally be associated with a widget as in the example above.

  \code
    // Integers 1 to 9999, i.e. a digit between 1 and 9 followed by up to 3 digits
    QRegExp rx( "[1-9]\\d{0,3}" );
    // The validator treats the regexp as "^[1-9]\\d{0,3}$"
    QRegExpValidator v( rx, 0 );
    QString s;

    s = "0";     v.validate( s, 0 ); // Returns Invalid
    s = "12345"; v.validate( s, 0 ); // Returns Invalid
    s = "1";     v.validate( s, 0 ); // Returns Acceptable

    rx.setPattern( "\S+" );	    // One or more non-whitespace characters
    v.setRegExp( rx );
    v.validate( "myfile.txt", 0 );  // Returns Acceptable
    v.validate( "my file.txt", 0 ); // Returns Invalid

    // A, B or C followed by exactly five digits followed by W, X, Y or Z
    rx.setPattern( "[A-C]\\d{5}[W-Z]" );
    v.setRegExp( rx );
    v.validate( "a12345Z", 0 );	// Returns Invalid
    v.validate( "A12345Z", 0 );	// Returns Acceptable
    v.validate( "B12", 0 );	// Returns Intermediate

    // Match most 'readme' files
    rx.setPattern( "read\\S?me(\.(txt|asc|1st))?" );
    rx.setCaseSensitive( FALSE );
    v.setRegExp( rx );
    v.validate( "readme", 0 );	    // Returns Acceptable
    v.validate( "README.1ST", 0 );  // Returns Acceptable
    v.validate( "read me.txt", 0 ); // Returns Invalid
    v.validate( "readm", 0 );	    // Returns Intermediate
  \endcode

  \sa QRegExp QIntValidator QDoubleValidator
*/

/*!
  Constructs a validator that accepts any string (including an
  empty one) as valid. The object's parent is \a parent and its name is \a
  name.
*/

QRegExpValidator::QRegExpValidator( QObject *parent, const char *name )
    : QValidator( parent, name ), r( QString::fromLatin1(".*") )
{
}

/*!
  Constructs a validator which accepts all strings that match the
  regular expression \a rx. The object's parent is \a parent and its name is
  \a name.

  The match is made against the entire string, e.g. if the regexp is
  <b>[A-Fa-f0-9]+</b> it will be treated as <b>^[A-Fa-f0-9]+$</b>.
*/

QRegExpValidator::QRegExpValidator( const QRegExp& rx, QObject *parent,
				    const char *name )
    : QValidator( parent, name ), r( rx )
{
}

/*!
  Destroys the validator, freeing any resources allocated.
*/

QRegExpValidator::~QRegExpValidator()
{
}

/*!
  Returns \c Acceptable if \a input is matched by the regular expression
  for this validator, \c Intermediate if it has matched partially (i.e.
  could be a valid match if additional valid characters are added), and
  \c Invalid if \a input is not matched.

  The start position is the beginning of the string unless \a pos is
  given and is > 0 in which case the regexp is matched from \a pos until
  the end of the string.

  For example, if the regular expression is
  <b>\\w\\d\\d</b> (that is, word-character, digit, digit) then
  "A57" is \c Acceptable, "E5" is \c Intermediate and "+9" is \c Invalid.

  \sa QRegExp::match()
*/

QValidator::State QRegExpValidator::validate( QString& input, int& pos ) const
{
    if ( ((QRegExp&) r).exactMatch(input) ) {
	return Acceptable;
    } else if ( ((QRegExp&) r).matchedLength() == (int) input.length() ) {
	return Intermediate;
    } else {
	pos = input.length();
	return Invalid;
    }
}

/*!
  Sets the regular expression used for validation to \a rx.

  \sa regExp()
*/

void QRegExpValidator::setRegExp( const QRegExp& rx )
{
    r = rx;
}

/*! \fn const QRegExp& QRegExpValidator::regExp() const

  Returns the regular expression used for validation.

  \sa setRegExp()
*/

#endif
