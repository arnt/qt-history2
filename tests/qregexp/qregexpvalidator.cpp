/*
  qregexpvalidator.cpp
*/

#include "qregexpvalidator.h"

// this will appear in qvalidator.cpp
// ### make sure to add \sa QRegExpValidator in Q{Int,Double,}Validator!

/*!
  \class QRegExpValidator qregexpvalidator.h

  \brief The QRegExpValidator class provides checking of a string against a
  regular expression.

  \ingroup misc

  QRegExpValidator contains a regular expression used to determine whether an
  input string is acceptable, intermediate or invalid.

  Example:
  \code
    QRegExp rx( "[+-]?0*[0-9]{1,3}" );  // integers -999 to +999
    QRegExpValidator v( rx, 0 );
    QString s;
    int i = 0;

    s = "++";   v.validate( s, i );     // Invalid
    s = "1234"; v.validate( s, i );     // Invalid
    s = "-";    v.validate( s, i );     // Intermediate
    s = "-1";   v.validate( s, i );     // Valid
  \endcode

  \sa QRegExp QIntValidator QDoubleValidator
*/

/*!  Constructs a validator object that accepts any string.
*/

QRegExpValidator::QRegExpValidator( QWidget *parent, const char *name )
    : QValidator( parent, name ), r( QString::fromLatin1(".*") )
{
}

/*!  Constructs a validator object which accepts all strings that match the
  regular expression \a rx.
*/

QRegExpValidator::QRegExpValidator( const QRegExp& rx, QWidget *parent,
				    const char *name )
    : QValidator( parent, name ), r( rx )
{
}

/*!  Destroys the validator, freeing any storage and other resources used.
*/

QRegExpValidator::~QRegExpValidator()
{
}

/*!  Returns \c Acceptable if \a input is matched precisely by the regular
  expression for this validator, \c Intermediate if it's matched partially, and
  \c Invalid if \a input is not matched.

  For example, suppose the regular expression is <b>abc</b>.  Then, input string
  <tt>abc</tt> is \c Acceptable, <tt>ab</tt> is \c Intermediate, and
  <tt>hab</tt> is \c Invalid.

  \sa QRegExp::match() QRegExp::partialMatch()
*/

QValidator::State QRegExpValidator::validate( QString& input, int& pos ) const
{
    Q_UNUSED( pos );
    if ( r.partialMatch(input) ) {
	if ( ((QRegExp&) r).match(input) == 0 &&
	     r.matchedLength() == (int) input.length() )
	    return Acceptable;
	else
	    return Intermediate;
    } else
	return Invalid;
}

/*!  Sets the regular expression used for validation to \a rx.

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
