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
    static QTextDocumentFragment fromXML(const QString &xml);
    static QTextDocumentFragment fromPlainText(const QString &plainText);
    static QTextDocumentFragment fromHTML(const QString &html);
    static QTextDocumentFragment fromHTML(const QByteArray &html);

private:
    QTextDocumentFragmentPrivate *d;
};

#endif // QTEXTDOCUMENTFRAGMENT_H
