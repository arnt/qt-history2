#include "qtextdocumentlayout_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include "qtextblockiterator.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qrect.h>
#include <qpalette.h>

#include "qabstracttextdocumentlayout_p.h"

enum {
    TextIndentValue = 40
};

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout);
public:
    struct Page {
        QTextBlockIterator first;
        QTextBlockIterator last;
    };
    QList<Page> pages;

    QSize pageSize;
    bool pagedLayout;

    void drawListItem(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                             QTextBlockIterator bl, const QTextLayout::Selection &selection) const;
    void drawBlock(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextBlockIterator bl) const;
    int indent(QTextBlockIterator bl) const;
    int hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const;

    void recreateAllBlocks();
    void layoutBlock(const QTextBlockIterator block, const QPoint &p, int width);
    QTextBlockIterator layoutCell(QTextBlockIterator block, QPoint *pos, int width);
    QTextBlockIterator layoutTable(QTextBlockIterator block, QPoint *pos, int width);
};

#define d d_func()
#define q q_func()

int QTextDocumentLayoutPrivate::hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    const QTextLayout *tl = bl.layout();
    QPoint pos = point - tl->rect().topLeft();

    QTextBlockFormat blockFormat = bl.blockFormat();

    // ### rtl?

    int textStartPos = blockFormat.leftMargin() + indent(bl);

    if (pos.x() < textStartPos) {
        if (accuracy == QText::ExactHit)
            return -1;

        return bl.position();
    }

    QRect textrect = bl.layout()->rect();

    if (point.y() < textrect.top())
        return bl.position();
    if (point.y() > textrect.bottom())
        return bl.position() + bl.length() - 1;

    for (int i = 0; i < tl->numLines(); ++i) {
        QTextLine line = tl->lineAt(i);

        if (line.rect().contains(pos))
            return bl.position() + line.xToCursor(point.x());
    }

    return -1;
}

int QTextDocumentLayoutPrivate::indent(QTextBlockIterator bl) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    int indent = blockFormat.listFormat().indent() + blockFormat.indent();

    return indent * TextIndentValue;
}

void QTextDocumentLayoutPrivate::drawListItem(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlockIterator bl, const QTextLayout::Selection &selection) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextCharFormat charFormat = bl.charFormat();
    QFontMetrics fontMetrics(charFormat.font());
    const int style = bl.blockFormat().listFormat().style();
    QString itemText;
    QPoint pos = bl.layout()->rect().topLeft();
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

void QTextDocumentLayoutPrivate::drawBlock(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextBlockIterator bl) const
{
    const QTextLayout *tl = bl.layout();
    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.showCursor ? context.cursor.position() : -1;

    if (bl.contains(cursor))
        cursor -= bl.position();
    else
        cursor = -1;

    QColor bgCol = blockFormat.backgroundColor();
    if (bgCol.isValid())
        painter->fillRect(bl.layout()->rect(), bgCol);

    QTextLayout::Selection s;
    int nSel = 0;
    if (context.cursor.hasSelection()) {
        int selStart = context.cursor.selectionStart();
        int selEnd = context.cursor.selectionEnd();
        s.setRange(selStart - bl.position(), selEnd - selStart);
        s.setType(QTextLayout::Highlight);
        ++nSel;
    }

    if (bl.blockFormat().listFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(painter, context, bl, s);

    if (tl->numLines() == 0) {

        QPoint pos = tl->rect().topLeft();

        pos.rx() += blockFormat.leftMargin() + indent(bl);
        pos.ry() += blockFormat.topMargin();

        if (cursor != -1)
            painter->drawLine(pos.x(), pos.y(),
                              pos.x(), pos.y() + QFontMetrics(bl.charFormat().font()).height());
        return;
    }

    const_cast<QTextLayout *>(tl)->setPalette(context.palette);

    tl->draw(painter, QPoint(0, 0), cursor, &s, nSel, painter->clipRegion().boundingRect());
}

void QTextDocumentLayoutPrivate::recreateAllBlocks()
{
    QPoint pos(0, 0);
    QTextBlockIterator it = q->begin();
    int width = pageSize.width();
    while (!it.atEnd()) {
        // check if we are at a table
        QTextTableFormat fmt = it.blockFormat().tableFormat();
        if (fmt.isValid()) {
            it = layoutTable(it, &pos, width);
        } else {
            layoutBlock(it, pos, width);
            pos.setY(it.layout()->rect().bottom());
            ++it;
        }
    }
}


QTextBlockIterator QTextDocumentLayoutPrivate::layoutCell(QTextBlockIterator it, QPoint *pos, int width)
{
    QTextFormatGroup *group = it.blockFormat().group();

    layoutBlock(it, *pos, width);
    pos->setY(it.layout()->rect().bottom());
    ++it;

    while (1) {
        Q_ASSERT(!it.atEnd());

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
            pos->setY(it.layout()->rect().bottom());
            ++it;
        }
    }
}

QTextBlockIterator QTextDocumentLayoutPrivate::layoutTable(QTextBlockIterator it, QPoint *pos, int width)
{

    QTextFormatGroup *group = it.blockFormat().group();
    Q_ASSERT(group && group->commonFormat().toTableFormat().isValid());
    QTextTable *table = qt_cast<QTextTable *>(group);
    Q_ASSERT(table);

    int rows = table->rows();
    int cols = table->cols();
    Q_ASSERT(rows > 0 && cols > 0); // also avoid division by zero later

    QSize ps = pageSize;

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

        QPoint point = QPoint(cols*width, y) + *pos;
        layoutBlock(it, point, cellWidth);
        ++it;

        y += rowHeight;
    }
//     qDebug() << "end layoutTable";
    pos->setY(pos->y() + y);

    return it;
}

void QTextDocumentLayoutPrivate::layoutBlock(QTextBlockIterator bl, const QPoint &p, int width)
{
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
        tl->createLine(0, y, x, x + lw);
    }
    tl->setPosition(p);
}




QTextDocumentLayout::QTextDocumentLayout()
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate)
{
    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QTextBlockIterator it = begin();
    for (; it != end(); ++it)
        d->drawBlock(painter, context, it);
}

void QTextDocumentLayout::documentChange(int from, int oldLength, int length)
{
    qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);
    d->recreateAllBlocks();
}

int QTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextBlockIterator it = begin();
    for (; it != end(); ++it) {
        QRect r = it.layout()->rect();
        if (r.contains(point))
            return d->hitTest(it, point, accuracy);
    }

    if (accuracy == QText::FuzzyHit) {
        it = end();
        --it;
        QRect r = it.layout()->rect();
        QPoint relative(point.x(), r.bottom() - 1);

        return d->hitTest(it, relative, accuracy);
    }
    return -1;
}

int QTextDocumentLayout::totalHeight() const
{
    int height = 0;
    QTextBlockIterator it = begin();
    for (; it != end(); ++it)
        height = qMax(height, it.layout()->rect().bottom());

    return height;
}

int QTextDocumentLayout::numPages() const
{
    if (!d->pagedLayout)
        return 1;
    return d->pages.count();
}

void QTextDocumentLayout::setPageSize(const QSize &size)
{
    d->pageSize = size;
    d->recreateAllBlocks();
}

QSize QTextDocumentLayout::pageSize() const
{
    return d->pageSize;
}
