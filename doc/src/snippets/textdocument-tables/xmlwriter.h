#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <QDomDocument>

class QTextDocument;

class XmlWriter
{
public:
    XmlWriter(QTextDocument *document) : textDocument(document) {}
    QDomDocument *toXml();

private:
    void createItems(QDomElement &parent, const QTextBlock &block);
    void createItems(QDomElement &parent, QTextFrame *frame);

    QDomDocument *document;
    QTextDocument *textDocument;
};

#endif
