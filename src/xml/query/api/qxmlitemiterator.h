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

#ifndef QXMLITEMITERATOR_H
#define QXMLITEMITERATOR_H

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Xml)

class QVariant;

class QXmlItemIteratorPrivate;

class Q_XML_EXPORT QXmlItemIterator
{
public:
    QXmlItemIterator();
    QXmlItemIterator(const QXmlItemIterator &other);
    ~QXmlItemIterator();
    QXmlItemIterator &operator=(const QXmlItemIterator &other);

    void next();
    bool isAtomicValue() const;
    bool isNode() const;
    bool hasNext() const;

    QVariant atomicValue() const;
    const void *toNodePointer() const;

protected:
    friend class QXmlQuery;
    QXmlItemIterator(QXmlItemIteratorPrivate *const p);
private:
    QXmlItemIteratorPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
