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

/*
 * QXmlName is conceptually identical to Patternist::QName. The difference is that the latter
 * is elegant, powerfull and fast.
 *
 * However, it is too powerfull and too open and not at all designed for being public. QXmlName,
 * in constrast, is only a public marker, that for instance uses a qint64 instead of qint32, such
 * that we in the future can use that, if needed.
 */

#include "qxmlquery.h"
#include "qxmlquery_p.h"

#include "qxmlname.h"

QT_BEGIN_NAMESPACE

/*!
  \class QXmlName
  \brief Represents a name for an XML node in a namespace-aware and efficient way.
  \reentrant
  \since 4.4
  \ingroup xml-tools

  QXmlName represents a name in XML, such as \c body in the namespace http://www.w3.org/1999/xhtml/, in
  an efficient and safe way. Instances are created by passing strings to the QXmlQuery::createName() factory
  function, and localName(), namespaceUri() and prefix()  can be used to access the components as strings.
  QXmlName's default constructor constructs null names. 

  QXmlName represents names by using a pooling mechanism behind the curtains. One of the side
  effects of this mechanism, is that QXmlName instances are tied to the QXmlQuery it
  was created with. For instance, one cannot call QXmlQuery::nameToComponents() with a name
  that was created with another QXmlQuery; if one do, behavior is undefined. This makes QXmlName
  efficient to pass around, and its comparison operators are swift too.

  \target What is a Node Name?
  \section1 What is a Node Name?
  The name of an element or attribute has three components: its namespace URI, a local name,
  and a prefix. For the purpose of what a name refers to the prefix is insignficant, and is only
  used to associate the correct namespace with the node. For instance, this element node:

  \code
  <svg xmlns="http://www.w3.org/2000/svg"/>
  \endcode
  
  and:

  \code
  <prefix:svg xmlns:prefix="http://www.w3.org/2000/svg"/>
  \endcode

  both represents the same name and compares equal.
 */

/*!
  Returns true if this QXmlName is uninitialized and does not contain a name at all.
 */
bool QXmlName::isNull() const
{
    return m_code == QXmlNamePrivateDetails::InvalidCode;
}

/*!
  \fn QXmlName::QXmlName();

  Constructs a QXmlName instance that is null.

  \sa isNull()
 */

/*!
  \fn bool QXmlName::operator==(const QXmlName &other) const

  Returns \c true if this QXmlName is equal to \a other, otherwise \c false.

  Two QXmlNames are equal if the namespace URIs and local names they represents
  are equal. The prefix is ignored.
 */

/*!
  \fn bool QXmlName::operator!=(const QXmlName &other) const

  Returns the opposite of applying operator==() on this QXmlName and \a other.
 */
 
/**
 \fn uint QXmlName::qHash(const QXmlName &name)
 \relates QXmlName

 Computes a hash key based on the local name combined with the namespace URI.

 That is, the prefix, which only is a syntactical distriction, is ignored.
 */

/*!
 Returns the namespace URI.

 \a query is used internally for looking up the string.
 */
QString QXmlName::namespaceUri(const QXmlQuery &query) const
{
    if(isNull())
        return QString();
    else
        return query.d->namePool->stringForNamespace(QXmlQueryPrivate::toPoolName(*this).namespaceURI());
}

/*!
 Returns the prefix.

 \a query is used internally for looking up the string.
 */
QString QXmlName::prefix(const QXmlQuery &query) const
{
    if(isNull())
        return QString();
    else
        return query.d->namePool->stringForPrefix(QXmlQueryPrivate::toPoolName(*this).prefix());
}

/*!
 Returns the local name.

 \a query is used internally for looking up the string.
 */
QString QXmlName::localName(const QXmlQuery &query) const
{
    if(isNull())
        return QString();
    else
        return query.d->namePool->stringForLocalName(QXmlQueryPrivate::toPoolName(*this).localName());
}

/*!
 Returns a string that is this QXmlName formatted as a so called Clark Name. For instance,
 the local name "html" with prefix "x" while residing in the XHTML namespace, would be
 returned as "{http://www.w3.org/1999/xhtml/}x:html". The local name "QWidget" with an empty namespace,
 would be returned as "QWidget".

 This function can be useful for debugging, or when a name needs to be represented as a string in an
 informal way. As implemented, a Clark Name cannot be deterministically parsed.

 \a query is used internally for looking up the strings.

 \sa {http://www.jclark.com/xml/xmlns.htm} {XML Namespaces, James Clark}
 */
QString QXmlName::toClarkName(const QXmlQuery &query) const
{
    if(isNull())
        return QLatin1String("QXmlName(null)");
    else
    {
        const QString ns(namespaceUri(query));

        if(ns.isEmpty())
            return localName(query);
        else
        {
            const QString p(prefix(query));
            const QString l(localName(query));

            return   QChar::fromLatin1('{')
                   + ns
                   + QChar::fromLatin1('}')
                   + (p.isEmpty() ? l : p + QChar::fromLatin1(':') + l);
        }
    }
}

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
