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

#ifndef QABSTRACTXMLPUSHCALLBACK_H
#define QABSTRACTXMLPUSHCALLBACK_H

#include <QtCore/QtGlobal>
#include <QtCore/QVariant>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Xml)

class QAbstractXmlPushCallbackPrivate;
class QXmlName;

class Q_XML_EXPORT QAbstractXmlPushCallback
{
public:
    QAbstractXmlPushCallback();

    virtual ~QAbstractXmlPushCallback();

    virtual void startElement(const QXmlName &name) = 0;
    virtual void endElement() = 0;
    virtual void attribute(const QXmlName &name,
                           const QString &value) = 0;
    virtual void comment(const QString &value) = 0;
    virtual void characters(const QString &value) = 0;
    virtual void startDocument() = 0;
    virtual void endDocument() = 0;

    virtual void processingInstruction(const QXmlName &target,
                                       const QString &value) = 0;

    virtual void atomicValue(const QVariant &value) = 0;
    virtual void namespaceBinding(const QXmlName &name) = 0;

protected:
    QAbstractXmlPushCallbackPrivate *d_ptr;
private:
    Q_DISABLE_COPY(QAbstractXmlPushCallback)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
