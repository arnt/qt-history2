#include "qtextdocumentlayout_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include "qtextblockiterator.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qrect.h>
#include <qpalette.h>
#include <qdebug.h>
#include <limits.h>

#include "qabstracttextdocumentlayout_p.h"

/*

Optimisation strategies:

HTML layout:

* Distinguish between normal and special flow. For normal flow the condition:
  y1 > y2 holds for all blocks with b1.key() > b2.key().
* Special flow is: floats, table cells

* Normal flow within table cells. Tables (not cells) are part of the normal flow.


* If blocks grows/shrinks in height and extends over whole page width at the end, move following blocks.
* If height doesn't change, no need to do anything

Table cells:

* If minWidth of cell changes, recalculate table width, relayout if needed.
* What about maxWidth when doing auto layout?

Floats:
* need fixed or proportional width, otherwise don't float!
* On width/height change relayout surrounding paragraphs.

Document width change:
* full relayout needed


Float handling:

* Floats are specified by a special format group.
* currently only floating images are implemented.

*/

enum {
    TextIndentValue = 40
};

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout)
public:
    struct Float {
        QTextBlockIterator block;
        QRect rect;
    };
    QHash<QTextGroup *, Float> floats;

#if 0
    struct Page {
        QTextBlockIterator first;
        QTextBlockIterator last;
    };
    QList<Page> pages;
#endif

    QSize pageSize;
    bool pagedLayout;
    mutable QTextBlockIterator currentBlock;
    int currentYPos;

    void drawListItem(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                             QTextBlockIterator bl, const QTextLayout::Selection &selection) const;
    void drawBlock(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextBlockIterator bl) const;
    static int indent(QTextBlockIterator bl);
    int hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const;

    void relayoutDocument();
    void layoutBlock(const QTextBlockIterator block);

    void floatMargins(int y, int *left, int *right);
    int findY(int yfrom, int requiredWidth);
    void removeFloatsForBlock(const QTextBlockIterator &b);
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

// ### could be moved to QTextBlockIterator
int QTextDocumentLayoutPrivate::indent(QTextBlockIterator bl)
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
    currentBlock = bl;
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

void QTextDocumentLayoutPrivate::relayoutDocument()
{
    currentYPos = 0;
    QTextBlockIterator it = q->begin();
    while (!it.atEnd()) {
        // check if we are at a table
//         QTextTableFormat fmt = it.blockFormat().tableFormat();
//         if (fmt.isValid()) {
//             it = layoutTable(it, &pos, width);
//         } else
        {
            layoutBlock(it);
            ++it;
        }
    }
}

void QTextDocumentLayoutPrivate::layoutBlock(QTextBlockIterator bl)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();
    currentBlock = bl;

    tl->setTextFlags(blockFormat.alignment()|Qt::IncludeTrailingSpaces|Qt::WordBreak);
    tl->useDesignMetrics(true);
//     tl.enableKerning(true);
    tl->clearLines();

    int cy = currentYPos;
    currentYPos += blockFormat.topMargin();

    int l = blockFormat.leftMargin() + indent(bl);
    int r = d->pageSize.width() - blockFormat.rightMargin();

    removeFloatsForBlock(bl);
    tl->setPosition(QPoint(0, cy));

    while (1) {
        QTextLine line = tl->createLine();
        if (!line.isValid())
            break;

        int left, right;
        floatMargins(d->currentYPos, &left, &right);

        left = qMax(left, l);
        right = qMax(right, r);
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;
        line.layout(right-left);

        floatMargins(d->currentYPos, &left, &right);
        if (line.width() > right-left) {
            // float has been added in the meantime, redo
            line.layout(right-left);
//             qDebug() << "    redo: left=" << left << " right=" << right;
            if (line.textWidth() > right-left) {
                // lines min width more than what we have
                d->currentYPos = findY(d->currentYPos, line.textWidth());
                floatMargins(d->currentYPos, &left, &right);
                line.layout(right-left);
            }
        }

        line.setPosition(QPoint(left, currentYPos-cy));
        currentYPos += line.ascent() + line.descent() + 1;
    }

    currentYPos += blockFormat.bottomMargin();
}

void QTextDocumentLayoutPrivate::floatMargins(int y, int *left, int *right)
{
    *left = 0;
    *right = d->pageSize.width();
    for (QHash<QTextGroup *, Float>::const_iterator it = floats.constBegin(); it != floats.constEnd(); ++it) {
        QRect r = it->rect;
        r.moveBy(it->block.layout()->position());
        if (r.y() <= y && r.bottom() > y) {
            QTextFrameFormat::Position pos = it.key()->commonFormat().toFrameFormat().position();
            if (pos == QTextFrameFormat::Left)
                *left = qMax(*left, it->rect.right());
            else
                *right = qMin(*right, it->rect.left());
        }
    }
}


int QTextDocumentLayoutPrivate::findY(int yfrom, int requiredWidth)
{
    int right, left;
    requiredWidth = qMin(requiredWidth, pageSize.width());

//     qDebug() << "findY:";
    while (1) {
        floatMargins(yfrom, &left, &right);
//         qDebug() << "    right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        int newY = INT_MAX;
        for (QHash<QTextGroup *, Float>::const_iterator it = floats.constBegin(); it != floats.constEnd(); ++it) {
            QRect r = it->rect;
            r.moveBy(it->block.layout()->position());
            if (r.y() <= yfrom && r.bottom() > yfrom) {
                newY = qMin(newY, r.bottom());
            }
        }
        if (newY == INT_MAX)
            break;
        yfrom = newY;
    }
    return yfrom;
}

void QTextDocumentLayoutPrivate::removeFloatsForBlock(const QTextBlockIterator &b)
{
    for (QHash<QTextGroup *, Float>::iterator it = floats.begin(); it != floats.constEnd();) {
        if (it->block == b)
            it = floats.erase(it);
        else
            ++it;
    }
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
//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);
    QTextBlockIterator it = findBlock(from);
    QTextBlockIterator end = findBlock(from + length - 1);
    if (!end.atEnd()) // can happen when removing text from the beginning: from = 0, oldLength = something, length = 0
        ++end;
    d->currentYPos = 0;
    if (it != begin()) {
        QTextBlockIterator prev = it;
        --prev;
        d->currentYPos = prev.layout()->rect().bottom();
    }

    bool move = false;
    while (it != end) {
//         qDebug("layouting block at pos %d", it.position());
        int old_bottom = it.layout()->rect().bottom();
        d->layoutBlock(it);
        int bottom = it.layout()->rect().bottom();
        move = (old_bottom != bottom);
        ++it;
    }
    if (move) {
        end = this->end();
        while (it != end) {
//             qDebug("moving block at pos %d", it.position());
            it.layout()->setPosition(QPoint(0, d->currentYPos));
            d->currentYPos += it.layout()->rect().bottom();
            ++it;
        }
    }
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

void QTextDocumentLayout::setSize(QTextObject item, const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;
    QTextFrameFormat::Position pos = QTextFrameFormat::None;
    QTextGroup *group = f.group();
    if (group)
        pos = group->commonFormat().toFrameFormat().position();

    item.setDescent(0);
    QSize inlineSize = (pos == QTextFrameFormat::None ? handler.iface->intrinsicSize(format) : QSize(0, 0));
    item.setWidth(inlineSize.width());
    item.setAscent(inlineSize.height());
}

void QTextDocumentLayout::layoutObject(QTextObject item, const QTextFormat &format)
{
    if (item.width())
        return;

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;
    QTextFrameFormat::Position pos = QTextFrameFormat::None;
    QTextGroup *group = f.group();
    if (group)
        pos = group->commonFormat().toFrameFormat().position();
    if (pos == QTextFrameFormat::None)
        return;

    if (d->floats.contains(group))
        return;

    QSize s = handler.iface->intrinsicSize(format);

    int left, right;
    int floaty = d->findY(d->currentYPos, s.width());
    d->floatMargins(floaty, &left, &right);

    QTextLayout *layout = d->currentBlock.layout();

#if 0
    QTextLine cline = layout->lineAt(layout->numLines()-1);
    Q_ASSERT(cline.isValid());
    if (cline.textWidth() > right-left - s.width()) {
        // ##### place float on next line
    }
    cline.setWidth(right-left-s.width());
#endif

    QTextDocumentLayoutPrivate::Float fl;
    QPoint fposition = QPoint((pos == QTextFrameFormat::Left ? left : right - s.width()), floaty);
    fl.block = d->currentBlock;
    fposition -= layout->position();
    fl.rect = QRect(fposition.x(), fposition.y(), s.width(), s.height());
//     qDebug() << "layoutObject: " << fl.rect;

    d->floats[group] = fl;
}

void QTextDocumentLayout::drawObject(QPainter *p, const QRect &rect, QTextObject item,
                                     const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrameFormat::Position pos = QTextFrameFormat::None;
    QTextGroup *group = f.group();
    QRect r = rect;
    if (group) {
        pos = group->commonFormat().toFrameFormat().position();
        if (pos != QTextFrameFormat::None) {
            const QTextDocumentLayoutPrivate::Float &f = d->floats.value(group);
            r = f.rect;
            r.moveBy(f.block.layout()->position());
        }
    }
//     qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawObject(p, r, item, format, selType);
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
#if 0
    if (!d->pagedLayout)
        return 1;
    return d->pages.count();
#endif
    return 1;
}

void QTextDocumentLayout::setPageSize(const QSize &size)
{
    d->pageSize = size;
    d->relayoutDocument();
}

QSize QTextDocumentLayout::pageSize() const
{
    return d->pageSize;
}
