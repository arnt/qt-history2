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
    void readFragment(const QTextBlock &currentBlock, QDomElement blockElement,
                      QDomDocument *document);

    QDomDocument *document;
    QTextDocument *textDocument;
};

#endif
