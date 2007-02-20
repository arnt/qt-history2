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

#ifndef QXMLSTREAM_H
#define QXMLSTREAM_H

#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QVector>

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QXmlStreamReaderPrivate;
class QXmlStreamAttributes;
class Q_XML_EXPORT QXmlStreamAttribute {
    int name_buffer, name_pos, name_size;
    int namespaceUri_buffer, namespaceUri_pos, namespaceUri_size;
    int qualifiedName_buffer, qualifiedName_pos, qualifiedName_size;
    int value_buffer, value_pos, value_size;
    QString buffer[3];
    void *reserved;
    uint m_isDefault : 1;
    friend class QXmlStreamReaderPrivate;
    friend class QXmlStreamAttributes;
public:
    QXmlStreamAttribute();
    QXmlStreamAttribute(const QString &qualifiedName, const QString &value);
    QXmlStreamAttribute(const QString &namespaceUri, const QString &name, const QString &value);
    QXmlStreamAttribute(const QXmlStreamAttribute &);
    QXmlStreamAttribute& operator=(const QXmlStreamAttribute &);
    ~QXmlStreamAttribute();
    inline QStringRef namespaceUri() const
        {return QStringRef(&buffer[namespaceUri_buffer], namespaceUri_pos, namespaceUri_size);}
    inline QStringRef name() const
        {return QStringRef(&buffer[name_buffer], name_pos, name_size);}
    inline QStringRef qualifiedName() const
        {return QStringRef(&buffer[qualifiedName_buffer], qualifiedName_pos, qualifiedName_size);}
    inline QStringRef value() const
        {return QStringRef(&buffer[value_buffer], value_pos, value_size);}
    inline bool isDefault() const { return m_isDefault; }
};

Q_DECLARE_TYPEINFO(QXmlStreamAttribute, Q_MOVABLE_TYPE);

class Q_XML_EXPORT QXmlStreamAttributes : public QVector<QXmlStreamAttribute>
{
public:
    QStringRef value(const QString &namespaceUri, const QString &name) const;
    QStringRef value(const QString &namespaceUri, const QLatin1String &name) const;
    QStringRef value(const QLatin1String &namespaceUri, const QLatin1String &name) const;
    QStringRef value(const QString &qualifiedName) const;
    QStringRef value(const QLatin1String &qualifiedName) const;
    void append(const QString &namespaceUri, const QString &name, const QString &value);
    void append(const QString &qualifiedName, const QString &value);
#if !defined(Q_NO_USING_KEYWORD)
    using QVector<QXmlStreamAttribute>::append;
#else
    inline void append(const QXmlStreamAttribute &attribute)
        { QVector<QXmlStreamAttribute>::append(attribute); }
#endif
};

class Q_XML_EXPORT QXmlStreamNamespaceDeclaration {
    QString buffer;
    int prefix_pos, prefix_size;
    int namespaceUri_pos, namespaceUri_size;
    void *reserved;

    friend class QXmlStreamReaderPrivate;
public:
    QXmlStreamNamespaceDeclaration();
    QXmlStreamNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &);
    QXmlStreamNamespaceDeclaration& operator=(const QXmlStreamNamespaceDeclaration &);
    ~QXmlStreamNamespaceDeclaration();
    inline QStringRef prefix() const { return QStringRef(&buffer, prefix_pos, prefix_size); }
    inline QStringRef namespaceUri() const { return QStringRef(&buffer, namespaceUri_pos, namespaceUri_size); }
};

Q_DECLARE_TYPEINFO(QXmlStreamNamespaceDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamNamespaceDeclaration> QXmlStreamNamespaceDeclarations;

class Q_XML_EXPORT QXmlStreamNotationDeclaration {
    QString buffer;
    int name_pos, name_size;
    int systemId_pos, systemId_size;
    int publicId_pos, publicId_size;
    void *reserved;

    friend class QXmlStreamReaderPrivate;
public:
    QXmlStreamNotationDeclaration();
    ~QXmlStreamNotationDeclaration();
    QXmlStreamNotationDeclaration(const QXmlStreamNotationDeclaration &);
    QXmlStreamNotationDeclaration& operator=(const QXmlStreamNotationDeclaration &);
    inline QStringRef name() const { return QStringRef(&buffer, name_pos, name_size); }
    inline QStringRef systemId() const { return QStringRef(&buffer, systemId_pos, systemId_size); }
    inline QStringRef publicId() const { return QStringRef(&buffer, publicId_pos, publicId_size); }
};

Q_DECLARE_TYPEINFO(QXmlStreamNotationDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamNotationDeclaration> QXmlStreamNotationDeclarations;

class Q_XML_EXPORT QXmlStreamEntityDeclaration {
    QString buffer;
    int name_pos, name_size;
    int notationName_pos, notationName_size;
    int systemId_pos, systemId_size;
    int publicId_pos, publicId_size;
    int value_pos, value_size;
    void *reserved;

    friend class QXmlStreamReaderPrivate;
public:
    QXmlStreamEntityDeclaration();
    ~QXmlStreamEntityDeclaration();
    QXmlStreamEntityDeclaration(const QXmlStreamEntityDeclaration &);
    QXmlStreamEntityDeclaration& operator=(const QXmlStreamEntityDeclaration &);
    inline QStringRef name() const { return QStringRef(&buffer, name_pos, name_size); }
    inline QStringRef notationName() const { return QStringRef(&buffer, notationName_pos, notationName_size); }
    inline QStringRef systemId() const { return QStringRef(&buffer, systemId_pos, systemId_size); }
    inline QStringRef publicId() const { return QStringRef(&buffer, publicId_pos, publicId_size); }
    inline QStringRef value() const { return QStringRef(&buffer, value_pos, value_size); }
};

Q_DECLARE_TYPEINFO(QXmlStreamEntityDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamEntityDeclaration> QXmlStreamEntityDeclarations;



class Q_XML_EXPORT QXmlStreamReader {
public:
    enum TokenType {
        NoToken = 0,
        Invalid,
        StartDocument,
        EndDocument,
        StartElement,
        EndElement,
        Characters,
        Comment,
        DTD,
        EntityReference,
        ProcessingInstruction,
    };


    QXmlStreamReader();
    QXmlStreamReader(QIODevice *device);
    QXmlStreamReader(const QByteArray &data);
    QXmlStreamReader(const QString &data);
    QXmlStreamReader(const char * data);
    ~QXmlStreamReader();

    void setDevice(QIODevice *device);
    QIODevice *device() const;
    void addData(const QByteArray &data);
    void addData(const QString &data);
    void addData(const char *data);
    void clear();


    bool atEnd() const;
    TokenType readNext();

    TokenType tokenType() const;
    QString tokenString() const;

    inline bool isStartDocument() const { return tokenType() == StartDocument; }
    inline bool isEndDocument() const { return tokenType() == EndDocument; }
    inline bool isStartElement() const { return tokenType() == StartElement; }
    inline bool isEndElement() const { return tokenType() == EndElement; }
    inline bool isCharacters() const { return tokenType() == Characters; }
    bool isWhitespace() const;
    bool isCDATA() const;
    inline bool isComment() const { return tokenType() == Comment; }
    inline bool isDTD() const { return tokenType() == DTD; }
    inline bool isEntityReference() const { return tokenType() == EntityReference; }
    inline bool isProcessingInstruction() const { return tokenType() == ProcessingInstruction; }

    bool isStandaloneDocument() const;

    qint64 lineNumber() const;
    qint64 columnNumber() const;
    qint64 characterOffset() const;

    QXmlStreamAttributes attributes() const;
    QString readElementText();

    QStringRef name() const;
    QStringRef namespaceUri() const;
    QStringRef qualifiedName() const;

    QStringRef processingInstructionTarget() const;
    QStringRef processingInstructionData() const;

    QStringRef text() const;

    QXmlStreamNamespaceDeclarations namespaceDeclarations() const;
    QXmlStreamNotationDeclarations notationDeclarations() const;
    QXmlStreamEntityDeclarations entityDeclarations() const;


    enum Error {
        NoError,
        CustomError,
        UnexpectedElementError,
        InvalidDocumentError,
        XmlNotWellFormedError,
        PrematureEndOfDocumentError
    };
    void raiseError(const QString& message = QString());
    QString errorString() const;
    Error error() const;

private:
    Q_DISABLE_COPY(QXmlStreamReader)
    Q_DECLARE_PRIVATE(QXmlStreamReader)
    QXmlStreamReaderPrivate *d_ptr;

};



class QXmlStreamWriterPrivate;

class Q_XML_EXPORT QXmlStreamWriter
{
    QDOC_PROPERTY(bool autoFormatting READ autoFormatting WRITE setAutoFormatting)
public:
    QXmlStreamWriter();
    QXmlStreamWriter(QIODevice *device);
    QXmlStreamWriter(QByteArray *array);
    QXmlStreamWriter(QString *string);
    ~QXmlStreamWriter();

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setCodec(QTextCodec *codec);
    void setCodec(const char *codecName);
    QTextCodec *codec() const;

    void setAutoFormatting(bool);
    bool autoFormatting() const;

    void writeAttribute(const QString &qualifiedName, const QString &value);
    void writeAttribute(const QString &namespaceUri, const QString &name, const QString &value);
    void writeAttribute(const QXmlStreamAttribute& attribute);
    void writeAttributes(const QXmlStreamAttributes& attributes);

    void writeCDATA(const QString &text);
    void writeCharacters(const QString &text);
    void writeComment(const QString &text);

    void writeDTD(const QString &dtd);

    void writeEmptyElement(const QString &qualifiedName);
    void writeEmptyElement(const QString &namespaceUri, const QString &name);

    void writeTextElement(const QString &qualifiedName, const QString &text);
    void writeTextElement(const QString &namespaceUri, const QString &name, const QString &text);

    void writeEndDocument();
    void writeEndElement();

    void writeEntityReference(const QString &name);
    void writeNamespace(const QString &namespaceUri, const QString &prefix = QString());
    void writeDefaultNamespace(const QString &namespaceUri);
    void writeProcessingInstruction(const QString &target, const QString &data = QString());

    void writeStartDocument();
    void writeStartDocument(const QString &version);
    void writeStartElement(const QString &qualifiedName);
    void writeStartElement(const QString &namespaceUri, const QString &name);

    void writeCurrentToken(const QXmlStreamReader &reader);

private:
    Q_DISABLE_COPY(QXmlStreamWriter)
    Q_DECLARE_PRIVATE(QXmlStreamWriter);
    QXmlStreamWriterPrivate *d_ptr;
};

QT_END_HEADER

#endif // QXMLSTREAM_H
