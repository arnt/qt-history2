/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QByteArray>
#include <QTranslator>

class TranslatorPrivate;
template <typename T> class QList;

class TranslatorMessage
{
public:
    TranslatorMessage();
    TranslatorMessage(const char * context, const char * sourceText,
                       const char * comment,
                       const QString &fileName,
                       int lineNumber,                       
                       const QString& translation = QString());
    explicit TranslatorMessage(QDataStream &);
    TranslatorMessage(const TranslatorMessage & m);

    TranslatorMessage & operator=(const TranslatorMessage & m);

    uint hash() const { return m_hash; }
    const char *context() const { return m_context.isNull() ? 0 : m_context.constData(); }
    const char *sourceText() const { return m_sourcetext.isNull() ? 0 : m_sourcetext.constData(); }
    const char *comment() const { return m_comment.isNull() ? 0 : m_comment.constData(); }

    inline void setTranslation(const QString & translation);
    QString translation() const { return m_translation; }

    enum Prefix { NoPrefix, Hash, HashContext, HashContextSourceText,
                  HashContextSourceTextComment };
    void write(QDataStream & s, bool strip = false,
                Prefix prefix = HashContextSourceTextComment) const;
    Prefix commonPrefix(const TranslatorMessage&) const;

    bool operator==(const TranslatorMessage& m) const;
    bool operator!=(const TranslatorMessage& m) const
    { return !operator==(m); }
    bool operator<(const TranslatorMessage& m) const;
    bool operator<=(const TranslatorMessage& m) const
    { return !m.operator<(*this); }
    bool operator>(const TranslatorMessage& m) const
    { return m.operator<(*this); }
    bool operator>=(const TranslatorMessage& m) const
    { return !operator<(m); }

    QString fileName(void) const { return m_fileName; }
    void setFileName(const QString &fileName) { m_fileName = fileName; }
    int lineNumber(void) const { return m_lineNumber; }
    void setLineNumber(int lineNumber) { m_lineNumber = lineNumber; }
    bool isNull() const { return m_sourcetext.isNull() && m_lineNumber == -1 && m_translation.isNull(); }

private:
    uint        m_hash;
    QByteArray  m_context;
    QByteArray  m_sourcetext;
    QByteArray  m_comment;
    QString     m_translation;
    QString     m_fileName;
    int         m_lineNumber;

    enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
               Tag_Hash, Tag_SourceText, Tag_Context, Tag_Comment,
               Tag_Obsolete1 };
};
Q_DECLARE_TYPEINFO(TranslatorMessage, Q_MOVABLE_TYPE);

inline void TranslatorMessage::setTranslation(const QString & atranslation)
{ m_translation = atranslation; }

class Translator : public QTranslator
{
    Q_OBJECT
public:
    explicit Translator(QObject *parent = 0);
    ~Translator();

    virtual TranslatorMessage findMessage(const char *context, const char *sourceText,
                                          const char *comment, 
                                          const QString &fileName = 0, int lineNumber = -1) const;
    virtual QString translate(const char *context, const char *sourceText,
                              const char *comment = 0) const
        { return findMessage(context, sourceText, 0, comment).translation(); }

    bool load(const QString & filename,
               const QString & directory = QString(),
               const QString & search_delimiters = QString(),
               const QString & suffix = QString());
    bool load(const uchar *data, int len);

    void clear();

#ifndef QT_NO_TRANSLATION_BUILDER
    enum SaveMode { Everything, Stripped };

    bool save(const QString & filename, SaveMode mode = Everything);

    void insert(const TranslatorMessage&);
    inline void insert(const char *context, const char *sourceText, const QString &fileName, int lineNo, const QString &translation) {
        insert(TranslatorMessage(context, sourceText, "", fileName, lineNo, translation ));
    }
    void remove(const TranslatorMessage&);
    inline void remove(const char *context, const char *sourceText) {
        remove(TranslatorMessage(context, sourceText, "", QLatin1String(""), -1));
    }
    bool contains(const char *context, const char *sourceText, const char * comment = 0) const;

    bool contains(const char *context, const char *comment, const QString &fileName, int lineNumber) const;

    void squeeze(SaveMode = Everything);
    void unsqueeze();

    QList<TranslatorMessage> messages() const;
#endif

    bool isEmpty() const;

private:
    Q_DISABLE_COPY(Translator)
    TranslatorPrivate *d;
};

#endif // TRANSLATOR_H
