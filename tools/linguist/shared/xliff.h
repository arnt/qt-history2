/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metatranslator.h"
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtXml>

class XLIFFHandler : public QXmlDefaultHandler
{
public:
    XLIFFHandler( MetaTranslator *translator );

    virtual bool startElement( const QString& namespaceURI,
                               const QString& localName, const QString& qName,
                               const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
                             const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

    virtual bool endDocument();

private:
    enum XliffContext {
        XC_xliff,
        XC_group,
        XC_context_group,
        XC_context,
        XC_context_linenumber,
        XC_ph,
        XC_restype_plurals
    };
    void pushContext(XliffContext ctx);
    bool popContext(XliffContext ctx);
    XliffContext currentContext() const;
    bool hasContext(XliffContext ctx) const;

private:
    MetaTranslator *tor;
    MetaTranslatorMessage::Type m_type;
    QString m_language;
    QString m_context;
    QString m_source;
    QString m_comment;
    QStringList translations;
    QString m_fileName;
    int     m_lineNumber;

    QString accum;
    QString m_ctype;
    int ferrorCount;
    bool contextIsUtf8;
    bool messageIsUtf8;
    const QString m_URI;  // convenience and efficiency; urn:oasis:names:tc:xliff:document:1.1
    QStack<int> m_contextStack;
};


