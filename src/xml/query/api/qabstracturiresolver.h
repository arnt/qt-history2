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

#ifndef QAbstractUriResolver_h
#define QAbstractUriResolver_h

#include <QtCore/QSharedData>

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QUrl;
class QAbstractUriResolverPrivate;

class Q_DECL_EXPORT QAbstractUriResolver : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<QAbstractUriResolver> Ptr;

    QAbstractUriResolver();
    virtual ~QAbstractUriResolver();

    QUrl resolve(const QUrl &relative,
                 const QUrl &baseURI) const;

protected:
    virtual QUrl handleResolve(const QUrl &relative,
                               const QUrl &baseURI) const = 0;

private:
    Q_DISABLE_COPY(QAbstractUriResolver)
    QAbstractUriResolverPrivate *d;
};

QT_END_HEADER
#endif
// vim: et:ts=4:sw=4:sts=4
