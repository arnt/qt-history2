#include "qxpath.h"


/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
/*!
  Creates an empty and therefore invalid XPath.

  \sa isValid() setPath()
*/
QXPath::QXPath()
{
    d = 0;
}

/*!
  Creates XPath for \a path. The string \a path is parsed and stroed in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa isValid() setPath()
*/
QXPath::QXPath( const QString& path )
{
    d = 0;
    setPath( path );
}

/*!
  Destructor.
*/
QXPath::~QXPath()
{
}


/*!
  Sets the location path \a p. The string \a p is parsed and stored in an
  internal format.

  If an error occured during the parsing, isValid() will return FALSE.

  \sa path() isValid()
*/
void QXPath::setPath( const QString& p )
{
    steps.clear();
    parse( p );
}

/*!
  Returns a string representation of the location path. This string is probably
  different from that one you constructed the path with. But it is semantically
  equal to that string.

  If the XPath is invalid, a null string is returned.

  \sa setPath(), isValid()
*/
QString QXPath::path() const
{
    return QString::null;
}


/*!
  Returns TRUE if the XPath is valid, otherwise FALSE. An XPath is invalid when
  it was constructed by a malformed string.
*/
bool QXPath::isValid() const
{
    return TRUE;
}

/*!
  Returns TRUE if the XPath is an absolute path, otherwise FALSE.
*/
bool QXPath::isAbsolutePath() const
{
    if ( isValid() )
	return absPath;
    else
	return FALSE;
}


/*!
  Parses the string \a path and returns TRUE if the parsing was ok, otherwise
  returns FALSE.
*/
bool QXPath::parse( const QString& path )
{
    QString p = path.simplifyWhiteSpace(); // ### or only stripWhiteSpace()?
    int pos = 0;

    if ( p[0] == '/' ) {
	pos ++;
	absPath = TRUE;
    } else {
	absPath = FALSE;
    }

    return TRUE;
}


/***************************************************************
 *
 * QXPath
 *
 ***************************************************************/
/*!
  Contructs a default location step.
*/
QXPathStep::QXPathStep()
{
    stepAxis = QXPath::Child;
}

/*!
  Creates an location step with...
*/
QXPathStep::QXPathStep( QXPath::Axis axis )
{
    stepAxis = axis;
}

/*!
  Copy constructor.
*/
QXPathStep::QXPathStep( const QXPathStep& step )
{
    stepAxis = step.stepAxis;
}

/*!
  Destructor.
*/
QXPathStep::~QXPathStep()
{
}

/*!
*/
void QXPathStep::setAxis( QXPath::Axis axis )
{
    stepAxis = axis;
}

/*!
*/
QXPath::Axis QXPathStep::axis() const
{
    return stepAxis;
}
