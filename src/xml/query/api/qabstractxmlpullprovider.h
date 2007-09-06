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

#ifndef QAbstractXmlPullProvider_h
#define QAbstractXmlPullProvider_h

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QAbstractXmlPullProviderPrivate;
class QXmlName;
class QString;
class QVariant;
template<typename Key, typename Value> class QHash;

class Q_XML_EXPORT QAbstractXmlPullProvider
{
public:
    QAbstractXmlPullProvider();
    virtual ~QAbstractXmlPullProvider();

    enum Event
    {
        StartOfInput            = 1,
        AtomicValue             = 1 << 1,
        StartDocument           = 1 << 2,
        EndDocument             = 1 << 3,
        StartElement            = 1 << 4,
        EndElement              = 1 << 5,
        Text                    = 1 << 6,
        ProcessingInstruction   = 1 << 7,
        Comment                 = 1 << 8,
        Attribute               = 1 << 9,
        Namespace               = 1 << 10,
        EndOfInput              = 1 << 11
    };

    virtual Event next() = 0;
    virtual Event current() const = 0;
    virtual QXmlName name() const = 0;
    virtual QVariant atomicValue() const = 0;
    virtual QString stringValue() const = 0;

    virtual QHash<QXmlName, QString> attributes();

    /* *** The functions below are internal. */
private:
    Q_DISABLE_COPY(QAbstractXmlPullProvider)
    friend class QPreparedQuery;
    QAbstractXmlPullProviderPrivate *d;
};

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
