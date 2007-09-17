/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include "qsourcelocation.h"
#include <QtDebug>


QT_BEGIN_NAMESPACE

/*!
  \class QSourceLocation
  \reentrant
  \since 4.4
  \brief Identifies a location in a resource, using a URI, line number and column number.
  \ingroup xml-tools

  QSourceLocation is a simple value based class that has three properties, uri(),
  line(), and column(), that together identifies a certain point in a resource, such
  as a file or an in-memory document.

  line() and column() refer to character count(not for instance byte count) and they both starts
  from 1, as opposed to 0.

 */

/*!
   Construct a QSourceLocation that doesn't identify anything at all.

   For a default constructed QSourceLocation(), isNull() returns \c true.
 */
QSourceLocation::QSourceLocation() : m_line(-1)
                                   , m_column(-1)
{
}

/*!
  Constructs a QSourceLocation instance that is a copy of \a other.
 */
QSourceLocation::QSourceLocation(const QSourceLocation &other) : m_line(other.m_line)
                                                               , m_column(other.m_column)
                                                               , m_uri(other.m_uri)
{
}

/*!
 Constructs a QSourceLocation with URI \a u, line \a l and column \a c.
 */
QSourceLocation::QSourceLocation(const QUrl &u, int l, int c) : m_line(l)
                                                              , m_column(c)
                                                              , m_uri(u)
{
}

/*!
  Destructs this QSourceLocation instance.
 */
QSourceLocation::~QSourceLocation()
{
}

/*!
  Returns true if this QSourceLocation is identical to \a other.

  Two QSourceLocation instances are equal if their uri(), line() and column()
  are equal.

  QSourceLocation instances that isNull() returns true for, are considered equal.
 */
bool QSourceLocation::operator==(const QSourceLocation &other) const
{
    return    m_line == other.m_line
           && m_column == other.m_column
           && m_uri == other.m_uri;
}

/*!
  Returns the opposite of applying operator==() for this QXmlName and \a other.
 */
bool QSourceLocation::operator!=(const QSourceLocation &other) const
{
    return operator==(other);
}

/*!
  Assigns this QSourceLocation instance to \a other.
 */
QSourceLocation &QSourceLocation::operator=(const QSourceLocation &other)
{
    if(this != &other)
    {
        m_line = other.m_line;
        m_column = other.m_column;
        m_uri = other.m_uri;
    }

    return *this;
}

/*!
 Returns the current column number. The column number refers to the count
 of characters, not bytes.

 The first column has number 1, not 0.

 The default value is -1, signalling that the column number is unknown.
 */
qint64 QSourceLocation::column() const
{
    return m_column;
}

/*!
 Sets the column number to \a newColumn. 0 is an invalid column number. The
 first column number is 1.
 */
void QSourceLocation::setColumn(qint64 newColumn)
{
    Q_ASSERT_X(newColumn != 0, Q_FUNC_INFO,
               "0 is an invalid column number. The first column number is 1.");
    m_column = newColumn;
}

/*!
 Returns the current line number.

 The first line number is 1, not 0.

 The default value is -1, signalling that the line number is unknown.
 */
qint64 QSourceLocation::line() const
{
    return m_line;
}

/*!
 Sets the line number to \a newLine.
 */
void QSourceLocation::setLine(qint64 newLine)
{
    m_line = newLine;
}

/*!
  Returns the resource that this QSourceLocation refers to. For instance,
  this could be a file on the local file system, if the URI scheme is \c file.
 */
QUrl QSourceLocation::uri() const
{
    return m_uri;
}

/*!
 Sets the URI to \a newUri.
 */
void QSourceLocation::setUri(const QUrl &newUri)
{
    m_uri = newUri;
}

/*!
  \relates QSourceLocation

  Prints \a sourceLocation to the debug stream \a debug.
 */
#ifndef QT_NO_DEBUG_STREAM
QDebug &operator<<(QDebug debug, const QSourceLocation &sourceLocation)
{
    debug << "QSourceLocation("
          << sourceLocation.m_uri
          << ", line:"
          << sourceLocation.m_line
          << ", column:"
          << sourceLocation.m_column
          << ")";
    return debug.space();
}
#endif

/*!
 Returns \c true if this QSourceLocation doesn't identify anything.

 For instance, for a default constructed QSourceLocation this function
 returns \c true. The same applies for any other QSourceLocation whose uri()
 is invalid.
 */
bool QSourceLocation::isNull() const
{
    return !m_uri.isValid();
}

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
