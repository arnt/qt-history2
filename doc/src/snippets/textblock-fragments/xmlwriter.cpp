#include <QtGui>

#include "xmlwriter.h"

QDomDocument *XmlWriter::toXml()
{
    QDomImplementation implementation;
    QDomDocumentType docType = implementation.createDocumentType(
        "scribe-document", "scribe", "www.trolltech.com/scribe");

    document = new QDomDocument(docType);

    // ### This processing instruction is required to ensure that any kind
    // of encoding is given when the document is written.
    QDomProcessingInstruction process = document->createProcessingInstruction(
        "xml", "version=\"1.0\" encoding=\"utf-8\"");
    document->appendChild(process);

    QDomElement documentElement = document->createElement("document");
    document->appendChild(documentElement);

    QTextBlock currentBlock = textDocument->begin();

    while (currentBlock.isValid()) {
        QDomElement blockElement = document->createElement("block");
        document->appendChild(blockElement);

        readFragment(currentBlock, blockElement, document);
        /* Include some text here for easy quoting:
        processBlock(currentBlock);
        */
        currentBlock = currentBlock.next();
    }

    return document;
}

void XmlWriter::readFragment(const QTextBlock &currentBlock,
                             QDomElement blockElement,
                             QDomDocument *document)
{
    QTextBlock::iterator it;
    for (it = currentBlock.begin(); !(it.atEnd()); ++it) {

        QTextFragment fragment = it.fragment();
        if (fragment.isValid()) {
            QDomElement fragmentElement = document->createElement("fragment");
            blockElement.appendChild(fragmentElement);

            fragmentElement.setAttribute("length", fragment.length());
            QDomText fragmentText = document->createTextNode(fragment.text());

            fragmentElement.appendChild(fragmentText);
            /* Include some text here for easy quoting:
            processFragment(currentBlock);
            ...
            */
        }
    }
}
