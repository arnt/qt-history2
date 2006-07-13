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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QTranslator>

#include <private/qtranslator_p.h>

class QIODevice;
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
                       const QStringList& translations = QStringList());
    TranslatorMessage(const TranslatorMessage & m);

    TranslatorMessage & operator=(const TranslatorMessage & m);

    uint hash() const { return m_hash; }
    const char *context() const { return m_context.isNull() ? 0 : m_context.constData(); }
    const char *sourceText() const { return m_sourcetext.isNull() ? 0 : m_sourcetext.constData(); }
    const char *comment() const { return m_comment.isNull() ? 0 : m_comment.constData(); }

    inline void setTranslations(const QStringList &translations);
    QStringList translations() const { return m_translations; }
    void setTranslation(const QString &translation) { m_translations = QStringList(translation); }
    QString translation() const { return m_translations.value(0); }
    bool isTranslated() const { return m_translations.count() > 1 || !m_translations.value(0).isEmpty(); }

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
    bool isNull() const { return m_sourcetext.isNull() && m_lineNumber == -1 && m_translations.isEmpty(); }

private:
    uint        m_hash;
    QByteArray  m_context;
    QByteArray  m_sourcetext;
    QByteArray  m_comment;
    QStringList m_translations;
    QString     m_fileName;
    int         m_lineNumber;

    enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
               Tag_Obsolete1, Tag_SourceText, Tag_Context, Tag_Comment,
               Tag_Obsolete2 };
};
Q_DECLARE_TYPEINFO(TranslatorMessage, Q_MOVABLE_TYPE);

inline void TranslatorMessage::setTranslations(const QStringList &translations)
{ m_translations = translations; }

class Translator : public QObject
{
    Q_OBJECT

public:
    explicit Translator(QObject *parent = 0);
    ~Translator();

    virtual TranslatorMessage findMessage(const char *context, const char *sourceText,
                                          const char *comment, 
                                          const QString &fileName = 0, int lineNumber = -1) const;

    void clear();

    enum SaveMode { Everything, Stripped };

    bool save(const QString & filename, SaveMode mode = Everything);
    bool save(QIODevice *iod, SaveMode mode = Everything);

    void insert(const TranslatorMessage&);
    inline void insert(const char *context, const char *sourceText, const QString &fileName, int lineNo, const QStringList &translations) {
        insert(TranslatorMessage(context, sourceText, "", fileName, lineNo, translations ));
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

    bool isEmpty() const;

    void setNumerusRules(const QByteArray &rules);

private:
    Q_DISABLE_COPY(Translator)
    TranslatorPrivate *d;
};

static const uchar hungarianRules[] = { };
static const uchar danishRules[] =
    { EQ, 1 };
static const uchar frenchRules[] =
    { LEQ, 1 };
static const uchar latvianRules[] =
    { MOD_10 | EQ, 1, AND, MOD_100 | NEQ, 11, NEWRULE,
      NEQ, 0 };
static const uchar gaeligeRules[] =
    { EQ, 1, NEWRULE,
      EQ, 2 };
static const uchar lithuanianRules[] =
    { MOD_10 | EQ, 1, AND, MOD_100 | NEQ, 11, NEWRULE,
      MOD_10 | EQ, 2, AND, MOD_100 | NOT_IN, 10, 19 };
static const uchar croatianRules[] =
    { MOD_10 | EQ, 1, AND, MOD_100 | NEQ, 11, NEWRULE,
      MOD_10 | IN, 2, 4, AND, MOD_100 | NOT_IN, 10, 19 };
static const uchar polishRules[] =
    { EQ, 1, NEWRULE,
      MOD_10 | IN, 2, 4, AND, MOD_100 | NOT_IN, 10, 19 };
static const uchar slovenianRules[] =
    { MOD_100 | EQ, 1, NEWRULE,
      MOD_100 | EQ, 2, NEWRULE,
      MOD_100 | IN, 3, 4 };

#endif // TRANSLATOR_H
