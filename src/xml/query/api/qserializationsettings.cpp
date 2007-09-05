/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QTextCodec>

#include "qserializationsettings.h"
#include "qserializationsettings_p.h"

/* Comments
 * - TODO Implementation indentation
 */

/*!
  \class QSerializationSettings
  \reentrant
  \since 4.4
  \brief Provides information for how serializing XML should be done.

  QPreparedQuery::serialize() evaluates the query, and writes the result as XML to a QIODevice.
  Sometimes it may be of interest to customize this step, such as to change the codec that
  is used to turn codepoints into bytes, or indentation. This class provides those settings.
 */

/*!
  Constructs a QSerializationSettings instance.
 */
QSerializationSettings::QSerializationSettings() : d(new QSerializationSettingsPrivate())
{
}

/*!
 Constructs a QSerializationSettings instance that is a copy of \a other.
 */
QSerializationSettings::QSerializationSettings(const QSerializationSettings &other) : d(new QSerializationSettingsPrivate(*other.d))
{
}

/*!
  Destructs this QSerializationSettings instance.
 */
QSerializationSettings::~QSerializationSettings()
{
    delete d;
}

/*!
 Sets the codec to use to \a c. If none is set, \c UTF-8 will be used.

 Remember that XML parsers are only required to support \c UTF-8 and \c UTF-16. For that reason one
 can run into interoperability problems if another encoding is chosen. \c UTF-8 and \c UTF-16
 can also represent all characters that can appear in the result of a query; this is not true for
 \c ASCII or \c ISO-8859-1, for instance.

 QSerializationSettings does not own the codec, unless the default codec is in use.
 */
void QSerializationSettings::setCodec(QTextCodec *c)
{
    d->codec = c;
}

/*!
   Returns the codec in use.

   If no codec has been set, the codec for \c UTF-8 is returned.

   \sa setCodec()
 */
QTextCodec *QSerializationSettings::codec() const
{
    if(d->codec)
        return d->codec;
    else
        return QTextCodec::codecForMib(106);
}

/*!
  Enables indentation if \a value is \c true, otherwise
  indentation is disabled.

  \sa indentationEnabled()
 */
void QSerializationSettings::setIndentationEnabled(bool value)
{
    d->indentationEnabled = value;
}

/*!
  Returns \c true if indentation is enabled, otherwise \c false.

  Enabling indentation gains readability, at the cost of storage size
  and compuational overhead.
  
  Indentation works by modifying and adding text nodes consisting of only
  whitespace in the resulting document. For many popular formats, such as XHTML,
  SVG and Docbook, this does not affect significant data and therefore does not
  affect semantics. However, for some formats it do, and it may therefore be
  of interest to consider if indentation should be enabled for the data that
  is being serialized.

  The default value is \c true.

  \sa setIndentationEnabled()
 */
bool QSerializationSettings::indentationEnabled() const
{
    return d->indentationEnabled;
}

// vim: et:ts=4:sw=4:sts=4
