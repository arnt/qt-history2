
#include "qtextdocumentlayout_p.h"
#include <private/qtextformat_p.h>
#include "qtextpiecetable_p.h"
#include "qtextglobal.h"
#include "qtextlistmanager_p.h"
#include "qtexttablemanager_p.h"
#include "qtexttable.h"
#include "qtextlist.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qrect.h>
#include <qpalette.h>

QTextDocumentLayout::QTextDocumentLayout(QTextPieceTable *parent)
    : QObject(parent)
{
    recreateAllBlocks();
}

QTextListFormat QTextDocumentLayout::listFormat(QTextPieceTable::BlockIterator bl) const
{
    QTextBlockFormat blockFmt = bl.blockFormat();

    int idx = blockFmt.listFormatIndex();

    return pieceTable()->formatCollection()->listFormat(idx);
}

int QTextDocumentLayout::indent(QTextPieceTable::BlockIterator bl) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    int listIdx = blockFormat.listFormatIndex();

    int indent = pieceTable()->formatCollection()->listFormat(listIdx).indent() +
	         blockFormat.indent();

    return indent * pieceTable()->config()->indentValue;
}

void QTextDocumentLayout::drawListItem(QTextPaintContext &context, QTextPieceTable::BlockIterator bl, const QTextLayout::Selection &selection)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QFontMetrics fontMetrics(blockFormat.font());
    const int style = listFormat(bl).style();
    QString itemText;
    QPoint pos = (*bl)->rect.topLeft();
    QSize size;

    switch (style) {
	case QTextListFormat::ListDecimal:
	case QTextListFormat::ListLowerAlpha:
	case QTextListFormat::ListUpperAlpha:
	    itemText = QTextListItem(bl).text();
	    size.setWidth(fontMetrics.width(itemText));
	    size.setHeight(fontMetrics.height());
	    break;

	case QTextListFormat::ListSquare:
	case QTextListFormat::ListCircle:
	case QTextListFormat::ListDisc:
	    size.setWidth(fontMetrics.lineSpacing() / 3);
	    size.setHeight(size.width());
	    break;

	case QTextListFormat::ListStyleUndefined:
	    return;
	default: return;
    }

    // ### RTL

    QRect r(pos, size);

    r.moveBy(blockFormat.leftMargin() + indent(bl) - size.width(),
	     blockFormat.topMargin() + (fontMetrics.height() / 2 - size.height() / 2));

    r.moveBy(-fontMetrics.width(" "), 0);

    QPainter &p = context.painter;

    if (selection.type() == QTextLayout::Highlight
	&& (selection.from() + selection.length() > 0)
	&& (selection.from() < 1)) {
	p.setPen(context.palette.highlightedText());

	p.fillRect(r, context.palette.highlight());
    }

    QBrush brush = context.palette.brush(QPalette::Text);

    switch (style) {
	case QTextListFormat::ListDecimal:
	case QTextListFormat::ListLowerAlpha:
	case QTextListFormat::ListUpperAlpha:
	    p.drawText(r.left(), r.top() + fontMetrics.height() - fontMetrics.descent(), itemText);
	    break;
	case QTextListFormat::ListSquare:
	    p.fillRect(r, brush);
	    break;
	case QTextListFormat::ListCircle:
	    p.drawEllipse(r);
	    break;
	case QTextListFormat::ListDisc:
	    p.setBrush(brush);
	    p.drawEllipse(r);
	    p.setBrush(Qt::NoBrush);
	    break;
	case QTextListFormat::ListStyleUndefined:
	    return;
	default: return;
    }
}

void QTextDocumentLayout::drawBlock(QTextPaintContext &context, QTextPieceTable::BlockIterator bl)
{
    const QTextLayout *tl = bl.layout();
    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.showCursor ? context.cursor.position() : -1;

    if (bl.contains(cursor))
	cursor -= bl.start();
    else
	cursor = -1;

    QPoint pos = (*bl)->rect.topLeft();

    QColor bgCol = blockFormat.backgroundColor();
    if (bgCol.isValid())
	context.painter.fillRect((*bl)->rect, bgCol);

    QTextLayout::Selection s;
    int nSel = 0;
    if (context.cursor.hasSelection()) {
	int selStart = context.cursor.selectionStart();
	int selEnd = context.cursor.selectionEnd();
	s.setRange(selStart - bl.start(), selEnd - selStart);
	s.setType(QTextLayout::Highlight);
	++nSel;
    }

    if (listFormat(bl).style() != QTextListFormat::ListStyleUndefined)
	drawListItem(context, bl, s);

    if (tl->numLines() == 0) {

	pos.rx() += blockFormat.leftMargin() + indent(bl);
	pos.ry() += blockFormat.topMargin();

	if (cursor != -1)
	    context.painter.drawLine(pos.x(), pos.y(),
				     pos.x(), pos.y() + QFontMetrics(blockFormat.font()).height());
	return;
    }

    const_cast<QTextLayout *>(tl)->setPalette(context.palette);

    tl->draw(&context.painter, pos, cursor, &s, nSel, context.clipRect);
}

void QTextDocumentLayout::draw(QTextPaintContext &context)
{
    QTextPieceTable::BlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it)
	drawBlock(context, it);
}

int QTextDocumentLayout::totalHeight() const
{
    int height = 0;
    QTextPieceTable::BlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it)
	height = qMax(height, (*it)->rect.bottom());

    return height;
}

void QTextDocumentLayout::recreateAllBlocks()
{
    QPoint pos(0, 0);
    QTextPieceTable::BlockIterator it = pieceTable()->blocksBegin();
    while (!it.atEnd()) {
	(*it)->layoutDirty = true;

	// check if we are at a table
	int fmtIdx = (*pieceTable()->find(it.key()))->format;
	QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
	if (fmt.tableFormatIndex() != -1) {
	    it = layoutTable(it, &pos);
	} else {
	    layoutBlock(it, pos);
	    pos.setY((*it)->rect.bottom());
	    ++it;
	}
    }
}

QTextPieceTable::BlockIterator QTextDocumentLayout::layoutCell(QTextPieceTable::BlockIterator it, QPoint *pos)
{
    int fmtIdx = (*pieceTable()->find(it.key()))->format;
    QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
    int tableIdx = fmt.tableFormatIndex();
//     qDebug() << "layoutCell at" << it.key() << "tableIdx" << tableIdx;

    (*it)->layoutDirty = true;
    layoutBlock(it, *pos);
    pos->setY((*it)->rect.bottom());
    ++it;

    while (1) {
	Q_ASSERT(it != pieceTable()->blocksEnd());
	(*it)->layoutDirty = true;

	// check if we are at a table
	int fmtIdx = (*pieceTable()->find(it.key()))->format;
	QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
	int ti = fmt.tableFormatIndex();
	if (ti == tableIdx) {
// 	    qDebug() << "end layoutCell";
	    return it;
	}

	if (fmt.tableFormatIndex() != -1) {
	    it = layoutTable(it, pos);
	} else {
	    layoutBlock(it, *pos);
	    pos->setY((*it)->rect.bottom());
	    ++it;
	}
    }
}

QTextPieceTable::BlockIterator QTextDocumentLayout::layoutTable(QTextPieceTable::BlockIterator it, QPoint *pos)
{
    int fmtIdx = (*pieceTable()->find(it.key()))->format;
    QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
    int tableIdx = fmt.tableFormatIndex();
    Q_ASSERT(tableIdx != -1);

    QTextTableFormat format = pieceTable()->formatCollection()->tableFormat(tableIdx);
    QTextTable *table = pieceTable()->tableManager()->table(tableIdx);

    int rows = table->rows();
    int cols = table->cols();

    QSize ps = pgSize;

    int width = (ps.width()-5)/cols;
    pgSize.setWidth(width);
    int y = 0;

    for (int i = 0; i < rows; ++i) {
	int rowHeight = 0;
	for (int j = 0; j < cols; ++j) {
	    int fmtIdx = (*pieceTable()->find(it.key()))->format;
	    QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
	    int ti = fmt.tableFormatIndex();
	    Q_ASSERT(ti == tableIdx);
	    Q_ASSERT(!fmt.tableCellEndOfRow());

	    QPoint point = QPoint(j*width, y) + *pos;
	    it = layoutCell(it, &point);

	    rowHeight = qMax(rowHeight, point.y() - pos->y() - y);
	    rowHeight = qMax(rowHeight, QFontMetrics(fmt.font()).height());
// 	    qDebug() << "rowHeight" << rowHeight;
	}
	int fmtIdx = (*pieceTable()->find(it.key()))->format;
	QTextBlockFormat fmt = pieceTable()->formatCollection()->blockFormat(fmtIdx);
	int ti = fmt.tableFormatIndex();
	Q_ASSERT(ti == tableIdx);
	Q_ASSERT(fmt.tableCellEndOfRow());

	(*it)->layoutDirty = true;
	QPoint point = QPoint(cols*width, y) + *pos;
	layoutBlock(it, point);
	++it;

	y += rowHeight;
    }
//     qDebug() << "end layoutTable";
    pgSize = ps;
    pos->setY(pos->y() + y);

    return it;
}


void QTextDocumentLayout::layoutBlock(QTextPieceTable::BlockIterator bl, const QPoint &p)
{
    if (!(*bl)->layoutDirty) {
	(*bl)->rect = QRect(p.x(), p.y(), (*bl)->rect.width(), (*bl)->rect.height());
	return;
    }

    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    tl->setTextFlags(blockFormat.alignment()|Qt::IncludeTrailingSpaces|Qt::WordBreak);
//     tl.enableKerning(true);

    int x = blockFormat.leftMargin() + indent(bl);
    int y = blockFormat.topMargin();

    const int lw = pageSize().width() - blockFormat.rightMargin() - x;

    int from = 0;
    int len = tl->text().length();
    while (from < len) {
	QTextLine l = tl->createLine(from, y, x, x + lw);
	y += l.ascent() + l.descent() + 1;
	from += l.length();
    }

    y += blockFormat.bottomMargin();

    (*bl)->rect = QRect(p.x(), p.y(), lw, y);
    (*bl)->layoutDirty = false;
}

int QTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextPieceTable::BlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it) {
	QRect r = (*it)->rect;
	if (r.contains(point))
	    return hitTest(it, point, accuracy);
    }

    if (accuracy == QText::FuzzyHit && pieceTable()->numBlocks()) {
	it = pieceTable()->blocksEnd();
	--it;
	QRect r = (*it)->rect;
	QPoint relative(point.x(), r.bottom() - 1);

	return hitTest(it, relative, accuracy);
    }
    return -1;
}

int QTextDocumentLayout::hitTest(QTextPieceTable::BlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    const QTextLayout *tl = bl.layout();
    QPoint pos = point - (*bl)->rect.topLeft();

    QTextBlockFormat blockFormat = bl.blockFormat();

    // ### rtl?

    int textStartPos = blockFormat.leftMargin() + indent(bl);

    if (pos.x() < textStartPos) {
	if (accuracy == QText::ExactHit)
	    return -1;

	return bl.start();
    }

    QRect textrect = bl.layout()->boundingRect();

    if (pos.y() < textrect.top())
	return bl.start();
    if (pos.y() > textrect.bottom())
	return bl.end();

    for (int i = 0; i < tl->numLines(); ++i) {
	QTextLine line = tl->lineAt(i);

	if (line.rect().contains(pos))
	    return bl.start() + line.xToCursor(point.x());
    }

    return -1;
}
