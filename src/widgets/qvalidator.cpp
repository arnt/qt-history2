/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.cpp#3 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qvalidator.h"

#include "qregexp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qvalidator.cpp#3 $");


struct QValidatorPrivate {
    bool useInt;
    bool useDouble;
    bool cs;

    int iBottom, iTop;
    double dBottom, dTop;
    int decimals;

    QString prefix;
    QString postfix;
};


/*!  \class QValidator qvalidator.h

  \brief The QValidator class provides ways to validate an input text.

  The class provides validation of integers or floating-point
  numbers, optionally with \link QRegExp regular expression \endlink
  prefixes or suffixes.  It is also possible to reimplement a number
  of three virtual functions and do your own testing.

  QValidator is generally used with QLineEdit, QSpinBox and QComboBox.
*/

/*!
  Creates a validator object which accepts everything.
  If a parent object is given, the validator will be destroyed
  when that object is destroyed.
*/

QValidator::QValidator( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = 0;
}


/*!  Deletes the validator and frees any storage and other resources
  used.
*/

QValidator::~QValidator()
{
    delete d;
    d = 0;
}


/*! \define QValidator::Result

  Validates \a input and returns a Result indicating whether \a input
  was sound.  Does not change \a input.

  If \a input is 0, validate() returns Good.

  The enum type Result has three values: <ul> <li> \c Good if the
  input string matches the validator's criteria, <li> \c Uncertain if
  the input string does not match the validator's criteria in some way
  fixup() can fix, <li> \c Bad if the input string cannot even be made
  syntactically correct by fixup().</ul>





  You may reimplement validate() if you need to use a custom
  validation algorithm, and not even use prefixes or suffixes.
  Normally it's better to reimplement validateContent().

  \sa validateContent()
*/

QValidator::Result QValidator::validate( const char * input )
{
    if ( !input || !d )
	return Good;

    if ( validateContent( content ) )
	return Good;
    content = fixup( content );
    if ( content.isNull() )
	return Bad;
    else if ( validateContent( content ) )
	return Uncertain;
    return Bad;
}




/*!  Validates the prefix and postfix of \a input, and returns
  a string containing the data between the end of the prefix and the
  start of the postfix.

  If \a does not contain a valid prefix and a valid postfix,
  validateAffixes() returns a null QString (one constructed using the
  default QStrong constructur).

  If no {pre,post}fix as been set, {pre,post}fix() returns "", which
  is a always {pre,post}fix.

  \sa prefix() postfix() setPrefix() setPostfix() QString::QString()
  validateContent()
*/

QString QValidator::validateAffixes( const QString input )
{
    QString nullString;

    QString s1( "^" );
    s1.append( prefix() );
    QRegExp r1( s1, caseSensitive(), FALSE );
    int p1 = 0;
    if ( r1.match( input, 0, &p1 ) < 0 )
	return nullString;

    QString s2( postfix() );
    s2.append( "$" );
    QRegExp r2( s2, caseSensitive(), FALSE );
    int p2 = r2.match( input, p1 );

    if ( p2 < 0 )
	return nullString;

    return input.mid( p1, p2-p1 );
}



/*!  Validates the string \a s which must be stripped of any prefix or
  suffix, and returns TRUE if it is valid (for some meaning of
  "valid") or FALSE if it is not.

  validate() calls this function if its input string has the correct
  prefix and/or suffix.

  This is probably the best function to reimplement if you need a
  custom validation algorithm.  You may also want to reimplement
  fixup()
*/

bool QValidator::validateContent( const QString s )
{
    if ( d && d->useInt ) {
	bool ok;
	long int tmp = s.toLong( &ok );
	return ok && tmp >= d->iBottom && tmp <= d->iTop;
    } else if ( d && d->useDouble ) {
	bool ok;
	double tmp = s.toDouble( &ok );
	// check the number of decimals here!
	return ok && tmp >= d->dBottom && tmp <= d->dTop;
    }
    return TRUE;
}


/*!  Tries to change the string to make it legal.
*/

void QValidator::fixup( QString & )
{
    return Good;
}


/*!  Sets the validator to operate in integer mode, and accept any
  integer from \a bottom to \a top inclusive.

  This functions has no effect if validateContent() is reimplemented.
*/

void QValidator::setInteger( int bottom, int top )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->useInt = TRUE;
    d->useDouble = FALSE;
    d->iBottom = bottom;
    d->iTop = top;
}


/*!  Sets the validator to operate in floating-point mode, and accept
  any floating-point number from \a bottom to \a top inclusive, with
  up to \a decimals decimals.

  QValidator accepts integers too in floating-point mode.

  This functions has no effect if validateContent() is reimplemented.
*/

void QValidator::setDouble( double bottom, double top, int decimals )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->useInt = FALSE;
    d->useDouble = TRUE;
    d->dBottom = bottom;
    d->dTop = top;
    d->decimals = decimals;
}


/*!  Set the prefix regulat expression to \a p.

  \a p must be a regular expression as defined in the QRegExp
  documentation.  setPrefix() prepends "^" to the supplied prefix.

*/

void QValidator::setPrefix( const char * p )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->prefix = p;
}


/*!  Returns the prefix for this validator, or 0 if no prefix has
  been set.

  The returned value does not include the implicit "^".
*/

const char * QValidator::prefix() const
{
    if (d && d->prefix.length())
	return d->prefix;
    return 0;
}


/*!  Set the postfix regular expression to \a p.

  \a p must be a regular expression as defined in the QRegExp
  documentation.  setPostfix() appends "$" to the supplied postfix.
*/

void QValidator::setPostfix( const char * p )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->postfix = p;
}


/*!  Returns the postfix for this validator, or 0 if no postfix has
  been set.

  The returned value does not include the implicit "$".

*/

const char * QValidator::postfix() const
{
    if (d && d->postfix.length())
	return d->postfix;
    return 0;
}


/*!  Sets the prefix and/or postfix parsing to be case sensitive if
  \yes is TRUE, and to be case insensitive if \a yes is FALSE.

  We recommend that subclasses use caseSensitive() wherever
  appropriate.
*/

void QValidator::setCaseSensitive( bool yes )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->cs = yes;
}


/*!  Returns TRUE if this validator matches the prefix and/or postvix
  case sensitively, or else FALSE.

  We recommend that subclasses use caseSensitive() wherever
  appropriate.
*/

bool QValidator::caseSensitive() const
{
    return d ? d->cs : TRUE;
}
