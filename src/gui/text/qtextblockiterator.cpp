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

        if (!text.isEmpty()) {
            int lastTextPosition = 0;
            int textLength = 0;

            int pos = position();
            const int endPos = pos + length() - 1; // -1 do omit the block separator char
            int lastFormatIdx = pt->find(pos).value()->format;
            QTextPieceTable::FragmentIterator it = pt->find(pos);

            while (pos < endPos) {
                const QTextFragment *frag = it.value();

                const int formatIndex = frag->format;
                if (formatIndex != lastFormatIdx) {
                    Q_ASSERT(lastFormatIdx != -1);
                    b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);

                    lastFormatIdx = formatIndex;
                    lastTextPosition += textLength;
                    textLength = 0;
                }

                const int inFragmentOffset = pos - it.position();
                const int size = qMin(int(frag->size - inFragmentOffset), endPos - pos);

                pos += size;
                textLength += size;
                ++it;
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

    int pos = position();
    const int endPos = pos + length() - 1; // -1 do omit the block separator char
    QTextPieceTable::FragmentIterator it = pt->find(pos);

    while (pos < endPos) {
        const QTextFragment *frag = it.value();
        const QChar *fragText = buffer.constData() + frag->stringPosition;
        Q_ASSERT(pos >= it.position());
        const int inFragmentOffset = pos - it.position();
        const int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

        text += QString::fromRawData(fragText + inFragmentOffset, charsToCopy);
        pos += charsToCopy;
        ++it;
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

