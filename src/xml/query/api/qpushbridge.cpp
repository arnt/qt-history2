
#include "QName.h"
#include "Item.h"
#include "qabstractxmlpushcallback.h"
#include "qxmlquery_p.h"

#include "PushBridge_p.h"

void PushBridge::startElement(const Patternist::QName name)
{
    m_receiver->startElement(QXmlQueryPrivate::fromPoolName(name));
}

void PushBridge::namespaceBinding(const Patternist::NamespaceBinding nb)
{
    Q_UNUSED(nb);
    // TODO
    //m_receiver->namespaceBinding();
}

void PushBridge::endElement()
{
    m_receiver->endElement();
}

void PushBridge::attribute(const Patternist::QName name,
                           const QString &value)
{

    m_receiver->attribute(QXmlQueryPrivate::fromPoolName(name), value);
}


void PushBridge::processingInstruction(const Patternist::QName name,
                                       const QString &value)
{
    m_receiver->processingInstruction(QXmlQueryPrivate::fromPoolName(name), value);
}

void PushBridge::comment(const QString &value)
{
    m_receiver->comment(value);
}

void PushBridge::item(const Patternist::Item &i)
{
    if(i.isAtomicValue())
        /* TODO */;
    else
        sendAsNode(i);
}

void PushBridge::characters(const QString &value)
{
    m_receiver->characters(value);
}

void PushBridge::whitespaceOnly(const QStringRef &value)
{
    m_receiver->characters(value.toString());
}

void PushBridge::startDocument()
{
    m_receiver->startDocument();
}

void PushBridge::endDocument()
{
    m_receiver->endDocument();
}

// vim: et:ts=4:sw=4:sts=4
