#include <qtextblockiterator.h>
#include "qtextpiecetable_p.h"

int QTextBlockIterator::position() const
{
    if (!pt || !n)
	return 0;

    return pt->blockMap().key(n);
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

    int pos = pt->blockMap().key(n);
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
	b->textDirty = false;

	// ######### looks wrong is a fragment spans a block boundary!
	if (!text.isEmpty()) {
	    int lastTextPosition = 0;
	    int textLength = 0;

	    QTextPieceTable::FragmentIterator it = pt->find(position());
	    QTextPieceTable::FragmentIterator e = pt->find(position()+length());
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
    return pt->formatCollection()->charFormat(fm.fragment(fm.findNode(pt->blockMap().key(n)))->format);
}

QString QTextBlockIterator::blockText() const
{
    if (!pt || !n)
	return QString::null;

    const QString buffer = pt->buffer();
    QString text;

    int pos = pt->blockMap().key(n);
    int len = pt->blockMap().size(n);
    QTextPieceTable::FragmentIterator it = pt->find(pos);
    QTextPieceTable::FragmentIterator e = pt->find(len);

    for (; it != e; ++it) {
	const QTextFragment *fragment = it.value();
	int key = it.key();
	int add = qMax(0, pos-key);
	int l = qMin((int)fragment->size, len) - add;
	Q_ASSERT(l > 0);

	text += QConstString(buffer.unicode()+fragment->position + add, l);
	len -= l;
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

