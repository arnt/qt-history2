/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#include "QtCore/qobject.h"
#include "QtCore/qbytearray.h"

#ifndef QT_NO_TRANSLATION

class QTranslatorPrivate;
template <typename T> class QList;

class Q_CORE_EXPORT QTranslatorMessage
{
public:
    QTranslatorMessage();
    QTranslatorMessage(const char * context, const char * sourceText,
                       const char * comment,
                       const QString& translation = QString());
    explicit QTranslatorMessage(QDataStream &);
    QTranslatorMessage(const QTranslatorMessage & m);

    QTranslatorMessage & operator=(const QTranslatorMessage & m);

    uint hash() const { return h; }
    const char *context() const { return cx.isNull() ? 0 : cx.constData(); }
    const char *sourceText() const { return st.isNull() ? 0 : st.constData(); }
    const char *comment() const { return cm.isNull() ? 0 : cm.constData(); }

    inline void setTranslation(const QString & translation);
    QString translation() const { return tn; }

    enum Prefix { NoPrefix, Hash, HashContext, HashContextSourceText,
                  HashContextSourceTextComment };
    void write(QDataStream & s, bool strip = false,
                Prefix prefix = HashContextSourceTextComment) const;
    Prefix commonPrefix(const QTranslatorMessage&) const;

    bool operator==(const QTranslatorMessage& m) const;
    bool operator!=(const QTranslatorMessage& m) const
    { return !operator==(m); }
    bool operator<(const QTranslatorMessage& m) const;
    bool operator<=(const QTranslatorMessage& m) const
    { return !m.operator<(*this); }
    bool operator>(const QTranslatorMessage& m) const
    { return m.operator<(*this); }
    bool operator>=(const QTranslatorMessage& m) const
    { return !operator<(m); }

private:
    uint h;
    QByteArray cx;
    QByteArray st;
    QByteArray cm;
    QString tn;

    enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
               Tag_Hash, Tag_SourceText, Tag_Context, Tag_Comment,
               Tag_Obsolete1 };
};
Q_DECLARE_TYPEINFO(QTranslatorMessage, Q_MOVABLE_TYPE);

inline void QTranslatorMessage::setTranslation(const QString & atranslation)
{ tn = atranslation; }

class Q_CORE_EXPORT QTranslator : public QObject
{
    Q_OBJECT
public:
    explicit QTranslator(QObject *parent = 0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QTranslator(QObject * parent, const char * name);
#endif
    ~QTranslator();

    virtual QTranslatorMessage findMessage(const char *, const char *,
                                            const char * = 0) const;

    bool load(const QString & filename,
               const QString & directory = QString(),
               const QString & search_delimiters = QString(),
               const QString & suffix = QString());
    bool load(const uchar *data, int len);

    void clear();

#ifndef QT_NO_TRANSLATION_BUILDER
    enum SaveMode { Everything, Stripped };

    bool save(const QString & filename, SaveMode mode = Everything);

    void insert(const QTranslatorMessage&);
    inline void insert(const char *context, const char *sourceText, const QString &translation) {
        insert(QTranslatorMessage(context, sourceText, "", translation));
    }
    void remove(const QTranslatorMessage&);
    inline void remove(const char *context, const char *sourceText) {
        remove(QTranslatorMessage(context, sourceText, ""));
    }
    bool contains(const char *, const char *, const char * comment = 0) const;

    void squeeze(SaveMode = Everything);
    void unsqueeze();

    QList<QTranslatorMessage> messages() const;
#endif

    bool isEmpty() const;

#ifdef QT3_SUPPORT
    QT3_SUPPORT QString find(const char *context, const char *sourceText, const char * comment = 0) const
        { return findMessage(context, sourceText, comment).translation(); }
#endif

private:
    Q_DISABLE_COPY(QTranslator)
    Q_DECLARE_PRIVATE(QTranslator)
};

#endif // QT_NO_TRANSLATION

#endif // QTRANSLATOR_H
