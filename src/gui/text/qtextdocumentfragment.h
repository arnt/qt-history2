#ifndef QTEXTDOCUMENTFRAGMENT_H
#define QTEXTDOCUMENTFRAGMENT_H

#include <qstring.h>

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;

class QTextDocumentFragment
{
    friend class QTextCursor;
public:
    QTextDocumentFragment();
    QTextDocumentFragment(QTextDocument *document);
    QTextDocumentFragment(const QTextCursor &range);
    QTextDocumentFragment(const QTextDocumentFragment &rhs);
    QTextDocumentFragment &operator=(const QTextDocumentFragment &rhs);
    ~QTextDocumentFragment();

    bool isNull() const;

    QString toPlainText() const;
    // ### reconsider
    QString toXML() const;

    void save(QTextStream &stream) const;
    static QTextDocumentFragment loadFromXML(const QString &xml);
    static QTextDocumentFragment loadFromPlainText(const QString &plainText);
    static QTextDocumentFragment loadFromHTML(const QString &html);
    static QTextDocumentFragment loadFromHTML(const QByteArray &html);

private:
    QTextDocumentFragmentPrivate *d;
};

#endif // QTEXTDOCUMENTFRAGMENT_H
