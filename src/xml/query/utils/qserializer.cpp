/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QIODevice>
#include <QTextCodec>

#include "qpatternistlocale_p.h"
#include "qitem_p.h"

#include "qserializer_p.h"

using namespace Patternist;

Serializer::Serializer(const DynamicContext::Ptr &context,
                       const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
                                                                , m_context(context)
                                                                , m_isPreviousAtomic(false)
                                                                , m_state(BeforeDocumentElement)
                                                                , m_namePool(context->namePool())
{
    Q_ASSERT(m_context);
    Q_ASSERT(m_namePool);

    m_hasClosedElement.reserve(EstimatedTreeDepth);
    m_nameCache.reserve(EstimatedNameCount);

    m_hasClosedElement.push(qMakePair(QName(), true));

    /* We push the empty namespace such that first of all namespaceBinding() won't assert
     * on an empty QStack, and such that the empty namespace is in-scope and that the
     * code doesn't attempt to declare it.
     *
     * We push the XML namespace. Although we won't receive declarations for it, we may output
     * attributes by that name. */
    NamespaceBinding::Vector defNss;
    defNss.resize(2);
    defNss[0] = NamespaceBinding(StandardPrefixes::empty, StandardNamespaces::empty);
    defNss[1] = NamespaceBinding(StandardPrefixes::xml, StandardNamespaces::xml);

    m_namespaces.push(defNss);
}

bool Serializer::atDocumentRoot() const
{
    return m_state == BeforeDocumentElement ||
           m_state == InsideDocumentElement && m_hasClosedElement.size() == 1;
}

void Serializer::startContent()
{
    if(!m_hasClosedElement.top().second)
    {
        write('>');
        m_hasClosedElement.top().second = true;
    }
}

void Serializer::writeEscaped(const QString &toEscape)
{
    if(toEscape.isEmpty()) /* Early exit. */
        return;

    QString result;
    result.reserve(int(toEscape.length() * 1.1));
    const int length = toEscape.length();

    for(int i = 0; i < length; ++i)
    {
        const QChar c(toEscape.at(i));

        if(c == QLatin1Char('<'))
            result += QLatin1String("&lt;");
        else if(c == QLatin1Char('>'))
            result += QLatin1String("&gt;");
        else if(c == QLatin1Char('&'))
            result += QLatin1String("&amp;");
        else
            result += toEscape.at(i);
    }

    write(result);
}

void Serializer::writeEscapedAttribute(const QString &toEscape)
{
    if(toEscape.isEmpty()) /* Early exit. */
        return;

    QString result;
    result.reserve(int(toEscape.length() * 1.1));
    const int length = toEscape.length();

    for(int i = 0; i < length; ++i)
    {
        const QChar c(toEscape.at(i));

        if(c == QLatin1Char('<'))
            result += QLatin1String("&lt;");
        else if(c == QLatin1Char('>'))
            result += QLatin1String("&gt;");
        else if(c == QLatin1Char('&'))
            result += QLatin1String("&amp;");
        else if(c == QLatin1Char('"'))
            result += QLatin1String("&quot;");
        else
            result += toEscape.at(i);
    }

    write(result);
}

void Serializer::write(const QString &content)
{
    m_device->write(m_codec->fromUnicode(content.constData(), content.length(), &m_converterState));
}

void Serializer::write(const char c)
{
    m_device->putChar(c);
}

void Serializer::write(const QName name)
{
    const QByteArray &cell = m_nameCache[name.code()];

    if(cell.isNull())
    {
        QByteArray &mutableCell = m_nameCache[name.code()];

        const QString content(m_namePool->toLexical(name));
        mutableCell = m_codec->fromUnicode(content.constData(), content.length(), &m_converterState);
        m_device->write(mutableCell);
    }
    else
        m_device->write(cell);
}

void Serializer::write(const char *chars)
{
    m_device->write(chars);
}

void Serializer::startElement(const QName name)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_device->isWritable());
    Q_ASSERT(m_codec);
    Q_ASSERT(!name.isNull());

    m_namespaces.push(NamespaceBinding::Vector());

    if(atDocumentRoot())
    {
        if(m_state == BeforeDocumentElement)
            m_state = InsideDocumentElement;
        else if(m_state != InsideDocumentElement)
        {
            m_context->error(tr("An XML document can only have one document element. "
                                "Therefore the element by name %1 cannot be serialized.")
                                .arg(formatKeyword(m_context->namePool(), name)),
                             ReportContext::SENR0001, this);
        }
    }

    startContent();
    write('<');
    write(name);

    /* Ensure that the namespace URI used in the name gets outputted. */
    {
        const NamespaceBinding nsb(NamespaceBinding::fromQName(name));
        namespaceBinding(nsb);
    }

    // TODO merge these two stacks. They push and pop at the same times.
    m_hasClosedElement.push(qMakePair(name, false));
    m_isPreviousAtomic = false;
}

void Serializer::endElement()
{
    QPair<QName, bool> e(m_hasClosedElement.pop());
    m_namespaces.pop();

    if(e.second)
    {
        write("</");
        write(e.first);
        write('>');
    }
    else
        write("/>");

    m_isPreviousAtomic = false;
}

void Serializer::attribute(const QName name,
                           const QString &value)
{
    Q_ASSERT(!name.isNull());

    /* Ensure that the namespace URI used in the name gets outputted. */
    {
        const NamespaceBinding nsb(NamespaceBinding::fromQName(name));

        /* Since attributes doesn't pick up the default namespace, a
         * namespace declaration would cause trouble if we output it. */
        if(nsb.prefix() != StandardPrefixes::empty)
            namespaceBinding(nsb);
    }

    if(atDocumentRoot())
    {
        m_context->error(tr("An attribute cannot be a toplevel node. Hence the attribute "
                                          "by name %1 cannot be serialized.")
                                          .arg(formatKeyword(m_namePool, name)),
                                     ReportContext::SENR0001, this);
    }
    else
    {
        write(' ');
        write(name);
        write("=\"");
        writeEscapedAttribute(value);
        write('"');
    }
}

bool Serializer::isBindingInScope(const NamespaceBinding nb) const
{
    const int levelLen = m_namespaces.size();

    if(nb.prefix() == StandardPrefixes::empty)
    {
        for(int lvl = levelLen - 1; lvl >= 0; --lvl)
        {
            const NamespaceBinding::Vector &scope = m_namespaces.at(lvl);
            const int vectorLen = scope.size();

            for(int s = vectorLen - 1; s >= 0; --s)
            {
                const NamespaceBinding &nsb = scope.at(s);

                if(nsb.prefix() == StandardPrefixes::empty)
                    return nsb.namespaceURI() == nb.namespaceURI();
            }
        }
    }
    else
    {
        // TODO do forward it here
        for(int lvl = levelLen - 1; lvl >= 0; --lvl)
        {
            const NamespaceBinding::Vector &scope = m_namespaces.at(lvl);
            const int vectorLen = scope.size();

            for(int s = 0; s < vectorLen; ++s)
            {
                if(scope.at(s) == nb)
                    return true;
            }
        }
    }

    return false;
}

void Serializer::namespaceBinding(const NamespaceBinding nb)
{
    Q_ASSERT_X(!nb.isNull(), Q_FUNC_INFO,
               "It makes no sense to pass a null NamespaceBinding.");

    Q_ASSERT_X(nb.namespaceURI() != StandardNamespaces::empty || nb.prefix() == StandardPrefixes::empty, Q_FUNC_INFO,
               "Undeclarations of prefixes aren't allowed in XML 1.0 and aren't supposed to be received.");

    if(isBindingInScope(nb))
        return;

    m_namespaces.top().append(nb);

    if(nb.prefix() == StandardPrefixes::empty)
        write(" xmlns");
    else
    {
        write(" xmlns:");
        write(m_namePool->stringForPrefix(nb.prefix()));
    }

    write("=\"");
    writeEscapedAttribute(m_namePool->stringForNamespace(nb.namespaceURI()));
    write('"');
}

void Serializer::comment(const QString &value)
{
    Q_ASSERT_X(!value.contains(QLatin1String("--")), Q_FUNC_INFO,
               "Invalid input; it's the caller's responsibility to ensure the input is correct.");

    startContent();
    write("<!--");
    write(value);
    write("-->");
    m_isPreviousAtomic = false;
}

void Serializer::characters(const QString &value)
{
    m_isPreviousAtomic = false;
    startContent();
    writeEscaped(value);
}

void Serializer::processingInstruction(const QName name,
                                       const QString &value)
{
    Q_ASSERT_X(!value.contains(QLatin1String("?>")), Q_FUNC_INFO,
               "Invalid input; it's the caller's responsibility to ensure the input is correct.");

    startContent();
    write("<?");
    write(name);
    write(' ');
    write(value);
    write("?>");

    m_isPreviousAtomic = false;
}

void Serializer::item(const Item &outputItem)
{
    if(outputItem.isAtomicValue())
    {
        if(m_isPreviousAtomic)
        {
            startContent();
            write(' ');
            writeEscaped(outputItem.stringValue());
        }
        else
        {
            m_isPreviousAtomic = true;
            const QString value(outputItem.stringValue());

            if(!value.isEmpty())
            {
                startContent();
                writeEscaped(value);
            }
        }
    }
    else
    {
        startContent();
        Q_ASSERT(outputItem.isNode());
        sendAsNode(outputItem);
    }
}

void Serializer::startDocument()
{
    m_isPreviousAtomic = false;
}

void Serializer::endDocument()
{
    m_isPreviousAtomic = false;
}

void Serializer::setOutputDevice(QIODevice *const device)
{
    m_device = device;
}

QIODevice *Serializer::outputDevice() const
{
    return m_device;
}

void Serializer::setCodec(QTextCodec *const outputCodec)
{
    m_codec = outputCodec;

    /* If we don't set this flag, QTextCodec will generate a BOM. */
    m_converterState.flags = QTextCodec::IgnoreHeader;
}

// vim: et:ts=4:sw=4:sts=4
