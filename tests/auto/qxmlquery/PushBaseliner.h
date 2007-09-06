/****************************************************************************
 * ** * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
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

#ifndef Patternist_PushBaseliner_h
#define Patternist_PushBaseliner_h

#include <QTextStream>

#include "qabstractxmlpushcallback.h"
#include "qxmlname.h"
#include "qxmlquery.h"

class PushBaseliner : public QAbstractXmlPushCallback
{
public:
    PushBaseliner(QTextStream &out,
                  const QXmlQuery query) : m_out(out)
                                         , m_query(query)
    {
        Q_ASSERT(m_out.codec());
    }

    virtual void startElement(const QXmlName&);
    virtual void endElement();
    virtual void attribute(const QXmlName&, const QString&);
    virtual void comment(const QString&);
    virtual void characters(const QString&);
    virtual void startDocument();
    virtual void endDocument();
    virtual void processingInstruction(const QXmlName&, const QString&);
    virtual void atomicValue(const QVariant&);
    virtual void namespaceBinding(const QXmlName&);

private:
    QTextStream &   m_out;
    const QXmlQuery m_query;
};

void PushBaseliner::startElement(const QXmlName &name)
{
    m_out << "startElement(" << name.toClarkName(m_query) << ')'<< endl;
}

void PushBaseliner::endElement()
{
    m_out << "endElement()" << endl;
}

void PushBaseliner::attribute(const QXmlName &name, const QString &value)
{
    m_out << "attribute(" << name.toClarkName(m_query) << ", " << value << ')'<< endl;
}

void PushBaseliner::comment(const QString &value)
{
    m_out << "comment(" << value << ')' << endl;
}

void PushBaseliner::characters(const QString &value)
{
    m_out << "characters(" << value << ')' << endl;
}

void PushBaseliner::startDocument()
{
    m_out << "startDocument()" << endl;
}

void PushBaseliner::endDocument()
{
    m_out << "endDocument()" << endl;
}

void PushBaseliner::processingInstruction(const QXmlName &name, const QString &data)
{
    m_out << "processingInstruction(" << name.toClarkName(m_query) << ", " << data << ')' << endl;
}

void PushBaseliner::atomicValue(const QVariant &val)
{
    m_out << "atomicValue(" << val.toString() << ')' << endl;
}

void PushBaseliner::namespaceBinding(const QXmlName &name)
{
    m_out << "namespaceBinding(" << name.toClarkName(m_query) << ')' << endl;
}

#endif
