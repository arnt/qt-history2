/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_Serializer_H
#define Patternist_Serializer_H

class QIODevice;

#include <QPair>
#include <QStack>
#include <QTextCodec>

#include "DynamicContext.h"
#include "NamespaceBinding.h"
#include "SequenceReceiver.h"
#include "SourceLocationReflection.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Receives SequenceReceiver events and produces XML that
     * is sent to a QIODevice.
     *
     * @ingroup Patternist_xdm
     * @see <a href="http://www.w3.org/TR/xslt-xquery-serialization/">XSLT 2.0
     * and XQuery 1.0 Serialization</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Serializer : public SequenceReceiver
                     , public DelegatingSourceLocationReflection
    {
    public:
        typedef PlainSharedPtr<Serializer> Ptr;

        Serializer(const DynamicContext::Ptr &context,
                   const SourceLocationReflection *const r);

        /**
         * Writes out @p nb.
         *
         * Namespace bindings aren't looked up in a cache, because
         * we typically receive very few.
         */
        virtual void namespaceBinding(const NamespaceBinding nb);

        virtual void characters(const QString &value);
        virtual void comment(const QString &value);

        virtual void startElement(const QName name);

        virtual void endElement();

        virtual void attribute(const QName name,
                               const QString &value);

        virtual void processingInstruction(const QName name,
                                           const QString &value);

        virtual void item(const Item &item);

        virtual void startDocument();
        virtual void endDocument();

        void setOutputDevice(QIODevice *const device);
        QIODevice *outputDevice() const;

        void setCodec(QTextCodec *const codec);

    private:
        inline bool isBindingInScope(const NamespaceBinding nb) const;

        /**
         * Where in the document the Serializer is currently working.
         */
        enum State
        {
            /**
             * Before the document element. This is the XML prolog where the
             * XML declaration, and possibly comments and processing
             * instructions are found.
             */
            BeforeDocumentElement,

            /**
             * This is inside the document element, at any level.
             */
            InsideDocumentElement
        };

        enum Constants
        {
            EstimatedTreeDepth = 10,
            EstimatedNameCount = 35
        };

        /**
         * If the current state is neither BeforeDocumentElement or
         * AfterDocumentElement.
         */
        inline bool atDocumentRoot() const;

        /**
         * Closes any open element start tag. Must be called before outputting
         * any element content.
         */
        inline void startContent();

        /**
         * Escapes content intended as text nodes for elements.
         */
        void writeEscaped(const QString &toEscape);

        /**
         * Identical to writeEscaped(), but also escapes quotes.
         */
        inline void writeEscapedAttribute(const QString &toEscape);

        /**
         * Writes out @p name.
         */
        inline void write(const QName name);

        inline void write(const char *chars);
        /**
         * Encodes and writes out @p content.
         */
        inline void write(const QString &content);

        inline void write(const char c);

        DynamicContext::Ptr                 m_context;
        QStack<QPair<QName, bool> >         m_hasClosedElement;
        bool                                m_isPreviousAtomic;
        State                               m_state;
        const NamePool::Ptr                 m_namePool;
        QStack<NamespaceBinding::Vector>    m_namespaces;
        QIODevice *                         m_device;
        QTextCodec *                        m_codec;
        QTextCodec::ConverterState          m_converterState;
        /**
         * Name cache. Since encoding QStrings are rather expensive
         * operations to do, and we on top of that would have to do
         * it each time a name appears, we here map names to their
         * encoded equivalents.
         *
         * This means that when writing out large documents, the serialization
         * of names after a while is reduced to a hash lookup and passing an
         * existing byte array.
         *
         * We use QName::Code as key as opposed to merely QName, because the
         * prefix is of significance.
         */
        QHash<QName::Code, QByteArray>      m_nameCache;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
