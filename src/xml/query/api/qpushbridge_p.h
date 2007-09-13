/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.  * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef PATTERNIST_PUSHBRIDGE_P_H
#define PATTERNIST_PUSHBRIDGE_P_H

#include "qsequencereceiver_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QAbstractXmlPushCallback;

class PushBridge : public Patternist::SequenceReceiver
{
public:
    /**
     * PushBridge does not own @p rec.
     */
    inline PushBridge(QAbstractXmlPushCallback *rec) : m_receiver(rec)
    {
        Q_ASSERT(m_receiver);
    }

    virtual void startElement(const Patternist::QName name);
    virtual void namespaceBinding(const Patternist::NamespaceBinding nb);
    virtual void endElement();
    virtual void attribute(const Patternist::QName name,
                           const QString &value);

    virtual void processingInstruction(const Patternist::QName name,
                                       const QString &value);
    virtual void comment(const QString &value);
    virtual void item(const Patternist::Item &item);
    virtual void characters(const QString &value);
    virtual void whitespaceOnly(const QStringRef &value);
    virtual void startDocument();
    virtual void endDocument();

private:
    QAbstractXmlPushCallback *const m_receiver;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
