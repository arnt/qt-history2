#ifndef QTEXTDOCUMENTFRAGMENT_H
#define QTEXTDOCUMENTFRAGMENT_H

#include <qstring.h>

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;
class QByteArray;

class Q_GUI_EXPORT QTextDocumentFragment
{
    friend class QTextCursor;
public:
    QTextDocumentFragment();
    QTextDocumentFragment(QTextDocument *document);
    QTextDocumentFragment(const QTextCursor &range);
    QTextDocumentFragment(const QTextDocumentFragment &rhs);
    QTextDocumentFragment &operator=(const QTextDocumentFragment &rhs);
    ~QTextDocumentFragment();

    bool isEmpty() const;

    QString toPlainText() const;

    QByteArray toBinary() const;

    void save(QDataStream &stream) const;
    static QTextDocumentFragment fromBinary(const QByteArray &data);
    static QTextDocumentFragment fromPlainText(const QString &plainText);
    static QTextDocumentFragment fromHTML(const QString &html);
    static QTextDocumentFragment fromHTML(const QByteArray &html);

private:
    QTextDocumentFragmentPrivate *d;
};

#endif // QTEXTDOCUMENTFRAGMENT_H
