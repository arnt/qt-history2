#include <qtextblockiterator.h>
#include "qtextpiecetable_p.h"

int QTextBlockIterator::position() const
{
    if (!pt || !n)
        return 0;

    return pt->blockMap().position(n);
}

int QTextBlockIterator::length() const
{
    if (!pt || !n)
        return 0;

    return pt->blockMap().size(n);
}

bool QTextBlockIterator::contains(int position) const
{
    if (!pt || !n)
        return 0;

    int pos = pt->blockMap().position(n);
    int len = pt->blockMap().size(n);
    return position >= pos && position < pos + len;
}

QTextLayout *QTextBlockIterator::layout() const
{
    if (!pt || !n)
        return 0;

    const QTextBlock *b = pt->blockMap().fragment(n);
    if (!b->layout) {
        b->layout = new QTextLayout();
        b->layout->setFormatCollection(const_cast<QTextPieceTable *>(pt)->formatCollection());
        b->layout->setDocumentLayout(pt->layout());
    }
    if (b->textDirty) {
        QString text = blockText();
        b->layout->setText(text);

        // ######### looks wrong if a fragment spans a block boundary!
        if (!text.isEmpty()) {
            int lastTextPosition = 0;
            int textLength = 0;

            QTextPieceTable::FragmentIterator it = pt->find(position());
            QTextPieceTable::FragmentIterator e = pt->find(position() + length() - 1);
            int lastFormatIdx = it.value()->format;
            for (; it != e; ++it) {
                const QTextFragment *fragment = it.value();
                int formatIndex = fragment->format;

                if (formatIndex != lastFormatIdx) {
                    Q_ASSERT(lastFormatIdx != -1);
                    b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);

                    lastFormatIdx = formatIndex;
                    lastTextPosition += textLength;
                    textLength = 0;
                }

                textLength += fragment->size;
            }

            Q_ASSERT(lastFormatIdx != -1);
            b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);
        }
        b->textDirty = false;
    }
    return b->layout;
}

QTextBlockFormat QTextBlockIterator::blockFormat() const
{
    if (!pt || !n)
        return QTextFormat().toBlockFormat();

    return pt->formatCollection()->blockFormat(pt->blockMap().fragment(n)->format);
}

QTextCharFormat QTextBlockIterator::charFormat() const
{
    if (!pt || !n)
        return QTextFormat().toCharFormat();

    const QTextPieceTable::FragmentMap &fm = pt->fragmentMap();
    return pt->formatCollection()->charFormat(fm.fragment(fm.findNode(pt->blockMap().position(n)))->format);
}

QString QTextBlockIterator::blockText() const
{
    if (!pt || !n)
        return QString::null;

    const QString buffer = pt->buffer();
    QString text;

    // ######### looks wrong if a fragment spans a block boundary!
    QTextPieceTable::FragmentIterator it = pt->find(position());
    QTextPieceTable::FragmentIterator e = pt->find(position() + length() - 1);

    for (; it != e; ++it) {
        const QTextFragment *fragment = it.value();

        text += QString::fromRawData(buffer.unicode() + fragment->stringPosition, fragment->size);
    }

    return text;
}

QTextBlockIterator& QTextBlockIterator::operator++()
{
    if (pt)
        n = pt->blockMap().next(n);
    return *this;
}

QTextBlockIterator& QTextBlockIterator::operator--()
{
    if (pt)
        n = pt->blockMap().prev(n);
    return *this;
}

