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

#include <QMutex>
#include <QUrl>

#include "qabstracturiresolver.h"

QT_BEGIN_NAMESPACE

/*!
  \class QAbstractUriResolver
  \brief A callback that rewrites one URI into another.
  \threadsafe
  \since 4.4

  Uniform Resource Identifiers, URIs, are strings that identify
  or name a resource. That URI are not more specific than that, makes
  them versatelite, global identifiers.

  However, in some circumstances it can be useful to turn an URI that
  identifies something logical, into a URI that locates something physical(an URL),
  or to simply rewrite one URI into a completely different. QAbstractUriResolver, a callback,
  offers this functionality through its resolve() function.

  For instance, one could write a QAbstractUriResolver subclass that rewrites \c urn:isbn:0-345-33973-8
  into \c file:// URLs that locates XML files for actual books. Or a web server could disallow
  certain URIs, as part of protecting the user's personal files from malicious scripts.

  The user calls resolve(), but the subclass implements handleResolve(). Behind the curtains,
  QAbstractUriResolver() ensures handleResolve() gets called serially, and is therefore is threadsafe.

  \sa {http://en.wikipedia.org/wiki/Uniform_Resource_Identifier} {Wikipedia, Uniform Resource Identifier}
*/

class QAbstractUriResolverPrivate
{
public:
    QMutex lock;
};

/*!
  Constructs a QAbstractUriResolver instance.
 */
QAbstractUriResolver::QAbstractUriResolver() : d(new QAbstractUriResolverPrivate())
{
}

/*!
  Destructs this QAbstractUriResolver instance.
 */
QAbstractUriResolver::~QAbstractUriResolver()
{
    delete d;
}

/*!
  Resolve \a relative into an absolute URI and returns it.

  The caller guarantees that \a baseURI is an absolute URI and valid,
  and that \a relative is valid. \a relative, as the name implies, may
  be relative.

  resolve() calls handleResolve() serially, and passes \a relative
  and \a baseURI to it unchanged.

  \sa QUrl::isRelative(), QUrl::isValid()
 */
QUrl QAbstractUriResolver::resolve(const QUrl &relative,
                           const QUrl &baseURI) const
{
    Q_ASSERT_X(relative.isValid(), Q_FUNC_INFO,
               "The relative URI must be valid");
    Q_ASSERT_X(baseURI.isValid(), Q_FUNC_INFO,
               "The relative URI must be valid");
    Q_ASSERT_X(!baseURI.isRelative(), Q_FUNC_INFO,
               "The caller must guarantee that the baseURI is absolute");

    QMutexLocker lock(&d->lock);
    return handleResolve(relative, baseURI);
}

/*!
    \fn QUrl QAbstractUriResolver::handleResolve(const QUrl &relative,
                                                const QUrl &baseURI) const

    Resolves \a relative into an absolute URI, in a manner specified by
    the QAbstractUriResolver sub-class.

    \a baseURI is the URI that the caller would use for resolving \a relative
    into an absolute URI.

    The caller guarantees that \a baseURI is valid and absolute.

    The implementation guarantees that the returned QUrl is absolute or a
    default constructed QUrl. In other cases, effects are undefined. If a default
    constructed QUrl is returned, it signals that this QAbstractUriResolver did not accept
    the URI requested to be resolved.

    This means if this QAbstractUriResolvers isn't interested in the URI, it should simply
    return \a relative resolved against \a baseURI, which is done by:

    \code
    return baseURI.resolved(relative);
    \endcode

  \sa QUrl::isRelative(), QUrl::isValid()
 */

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
