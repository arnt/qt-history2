#include "qtextdocumentlayout_p.h"
#include <private/qtextformat_p.h>
#include "qtextpiecetable_p.h"
#include "qtexttablemanager_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qrect.h>
#include <qpalette.h>

QTextDocumentLayout::QTextDocumentLayout()
    : QAbstractTextDocumentLayout()
{
    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QTextBlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it)
        drawBlock(painter, context, it);
}

void QTextDocumentLayout::documentChange(int from, int oldLength, int length)
{
    recreateAllBlocks();
}

int QTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextBlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it) {
        QRect r = QTextPieceTable::block(it)->rect;
        if (r.contains(point))
            return hitTest(it, point, accuracy);
    }

    if (accuracy == QText::FuzzyHit && pieceTable()->numBlocks()) {
        it = pieceTable()->blocksEnd();
        --it;
        QRect r = QTextPieceTable::block(it)->rect;
        QPoint relative(point.x(), r.bottom() - 1);

        return hitTest(it, relative, accuracy);
    }
    return -1;
}



QTextListFormat QTextDocumentLayout::listFormat(QTextBlockIterator bl) const
{
    return bl.blockFormat().listFormat();
}

int QTextDocumentLayout::indent(QTextBlockIterator bl) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    int indent = blockFormat.listFormat().indent() + blockFormat.indent();

    return indent * pieceTable()->config()->indentValue;
}

void QTextDocumentLayout::drawListItem(QPainter *painter, const PaintContext &context, QTextBlockIterator bl, const QTextLayout::Selection &selection)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextCharFormat charFormat = bl.charFormat();
    QFontMetrics fontMetrics(charFormat.font());
    const int style = listFormat(bl).style();
    QString itemText;
    QPoint pos = QTextPieceTable::block(bl)->rect.topLeft();
    QSize size;

    switch (style) {
        case QTextListFormat::ListDecimal:
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
            itemText = static_cast<QTextList *>(blockFormat.group())->itemText(bl);
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

    if (selection.type() == QTextLayout::Highlight
        && (selection.from() + selection.length() > 0)
        && (selection.from() < 1)) {
        painter->setPen(context.palette.highlightedText());

        painter->fillRect(r, context.palette.highlight());
    }

    QBrush brush = context.palette.brush(QPalette::Text);

    switch (style) {
        case QTextListFormat::ListDecimal:
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
            painter->drawText(r.left(), r.top() + fontMetrics.height() - fontMetrics.descent(), itemText);
            break;
        case QTextListFormat::ListSquare:
            painter->fillRect(r, brush);
            break;
        case QTextListFormat::ListCircle:
            painter->drawEllipse(r);
            break;
        case QTextListFormat::ListDisc:
            painter->setBrush(brush);
            painter->drawEllipse(r);
            painter->setBrush(Qt::NoBrush);
            break;
        case QTextListFormat::ListStyleUndefined:
            return;
        default: return;
    }
}

void QTextDocumentLayout::drawBlock(QPainter *painter, const PaintContext &context, QTextBlockIterator bl)
{
    const QTextLayout *tl = bl.layout();
    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.showCursor ? context.cursor.position() : -1;

    if (bl.contains(cursor))
        cursor -= bl.position();
    else
        cursor = -1;

    QPoint pos = QTextPieceTable::block(bl)->rect.topLeft();

    QColor bgCol = blockFormat.backgroundColor();
    if (bgCol.isValid())
        painter->fillRect(QTextPieceTable::block(bl)->rect, bgCol);

    QTextLayout::Selection s;
    int nSel = 0;
    if (context.cursor.hasSelection()) {
        int selStart = context.cursor.selectionStart();
        int selEnd = context.cursor.selectionEnd();
        s.setRange(selStart - bl.position(), selEnd - selStart);
        s.setType(QTextLayout::Highlight);
        ++nSel;
    }

    if (listFormat(bl).style() != QTextListFormat::ListStyleUndefined)
        drawListItem(painter, context, bl, s);

    if (tl->numLines() == 0) {

        pos.rx() += blockFormat.leftMargin() + indent(bl);
        pos.ry() += blockFormat.topMargin();

        if (cursor != -1)
            painter->drawLine(pos.x(), pos.y(),
                                     pos.x(), pos.y() + QFontMetrics(bl.charFormat().font()).height());
        return;
    }

    const_cast<QTextLayout *>(tl)->setPalette(context.palette);

    tl->draw(painter, pos, cursor, &s, nSel, painter->clipRegion().boundingRect());
}

int QTextDocumentLayout::totalHeight() const
{
    int height = 0;
    QTextBlockIterator it = pieceTable()->blocksBegin();
    for (; it != pieceTable()->blocksEnd(); ++it)
        height = qMax(height, QTextPieceTable::block(it)->rect.bottom());

    return height;
}

void QTextDocumentLayout::recreateAllBlocks()
{
    QPoint pos(0, 0);
    QTextBlockIterator it = pieceTable()->blocksBegin();
    int width = pageSize().width();
    while (!it.atEnd()) {
        QTextPieceTable::block(it)->layoutDirty = true;

        // check if we are at a table
        QTextTableFormat fmt = it.blockFormat().tableFormat();
        if (fmt.isValid()) {
            it = layoutTable(it, &pos, width);
        } else {
            layoutBlock(it, pos, width);
            pos.setY(QTextPieceTable::block(it)->rect.bottom());
            ++it;
        }
    }
}

QTextBlockIterator QTextDocumentLayout::layoutCell(QTextBlockIterator it, QPoint *pos, int width)
{
    QTextFormatGroup *group = it.blockFormat().group();

    QTextPieceTable::block(it)->layoutDirty = true;
    layoutBlock(it, *pos, width);
    pos->setY(QTextPieceTable::block(it)->rect.bottom());
    ++it;

    while (1) {
        Q_ASSERT(it != pieceTable()->blocksEnd());
        QTextPieceTable::block(it)->layoutDirty = true;

        // check if we are at a table
        QTextBlockFormat fmt = it.blockFormat();
        QTextFormatGroup *g = fmt.group();
        if (g == group) {
//             qDebug() << "end layoutCell";
            return it;
        }

        if (g && g->commonFormat().toTableFormat().isValid()) {
            it = layoutTable(it, pos, width);
        } else {
            layoutBlock(it, *pos, width);
            pos->setY(QTextPieceTable::block(it)->rect.bottom());
            ++it;
        }
    }
}

QTextBlockIterator QTextDocumentLayout::layoutTable(QTextBlockIterator it, QPoint *pos, int width)
{

    QTextFormatGroup *group = it.blockFormat().group();
    Q_ASSERT(group && group->commonFormat().toTableFormat().isValid());
    QTextTable *table = pieceTable()->tableManager()->table(group);

    int rows = table->rows();
    int cols = table->cols();
    Q_ASSERT(rows > 0 && cols > 0); // also avoid division by zero later

    QSize ps = pageSize();

    int cellWidth = (width-5)/cols;
    int y = 0;

    for (int i = 0; i < rows; ++i) {
        int rowHeight = 0;
        for (int j = 0; j < cols; ++j) {
            QTextBlockFormat fmt = it.blockFormat();
            Q_ASSERT(fmt.group() == group);
            Q_ASSERT(!fmt.tableCellEndOfRow());

            QPoint point = QPoint(j*width, y) + *pos;
            it = layoutCell(it, &point, cellWidth);

            rowHeight = qMax(rowHeight, point.y() - pos->y() - y);
            rowHeight = qMax(rowHeight, QFontMetrics(it.charFormat().font()).height());
//             qDebug() << "rowHeight" << rowHeight;
        }
        Q_ASSERT(it.blockFormat().group() == group);
        Q_ASSERT(it.blockFormat().tableCellEndOfRow());

        QTextPieceTable::block(it)->layoutDirty = true;
        QPoint point = QPoint(cols*width, y) + *pos;
        layoutBlock(it, point, cellWidth);
        ++it;

        y += rowHeight;
    }
//     qDebug() << "end layoutTable";
    pos->setY(pos->y() + y);

    return it;
}


void QTextDocumentLayout::layoutBlock(QTextBlockIterator bl, const QPoint &p, int width)
{
    if (!QTextPieceTable::block(bl)->layoutDirty) {
        QTextPieceTable::block(bl)->rect = QRect(p.x(), p.y(),
                                                 QTextPieceTable::block(bl)->rect.width(), QTextPieceTable::block(bl)->rect.height());
        return;
    }

    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    tl->setTextFlags(blockFormat.alignment()|Qt::IncludeTrailingSpaces|Qt::WordBreak);
    tl->useDesignMetrics(true);
//     tl.enableKerning(true);

    int x = blockFormat.leftMargin() + indent(bl);
    int y = blockFormat.topMargin();

    const int lw = width - blockFormat.rightMargin() - x;

    int from = 0;
    int len = tl->text().length();
    if (len) {
        while (from < len) {
            QTextLine l = tl->createLine(from, y, x, x + lw);
            y += l.ascent() + l.descent() + 1;
            from += l.length();
        }
    } else {
        y += QFontMetrics(bl.charFormat().font()).lineSpacing();
    }

    y += blockFormat.bottomMargin();

    QTextPieceTable::block(bl)->rect = QRect(p.x(), p.y(), lw, y);
    QTextPieceTable::block(bl)->layoutDirty = false;
}

int QTextDocumentLayout::hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    const QTextLayout *tl = bl.layout();
    QPoint pos = point - QTextPieceTable::block(bl)->rect.topLeft();

    QTextBlockFormat blockFormat = bl.blockFormat();

    // ### rtl?

    int textStartPos = blockFormat.leftMargin() + indent(bl);

    if (pos.x() < textStartPos) {
        if (accuracy == QText::ExactHit)
            return -1;

        return bl.position();
    }

    QRect textrect = bl.layout()->boundingRect();

    if (pos.y() < textrect.top())
        return bl.position();
    if (pos.y() > textrect.bottom())
        return bl.position() + bl.length() - 1;

    for (int i = 0; i < tl->numLines(); ++i) {
        QTextLine line = tl->lineAt(i);

        if (line.rect().contains(pos))
            return bl.position() + line.xToCursor(point.x());
    }

    return -1;
}
