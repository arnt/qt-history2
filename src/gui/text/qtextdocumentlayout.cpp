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


*/

enum {
    TextIndentValue = 40
};

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout);
public:
#if 0
    struct Float {
        // document position
        int from;
        int length;

        QRect rect;
    };

    struct Page {
        QTextBlockIterator first;
        QTextBlockIterator last;
    };
    QList<Page> pages;
#endif

    QSize pageSize;
    bool pagedLayout;

    void drawListItem(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                             QTextBlockIterator bl, const QTextLayout::Selection &selection) const;
    void drawBlock(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextBlockIterator bl) const;
    int indent(QTextBlockIterator bl) const;
    int hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const;

    void relayoutDocument();
    void layoutBlock(const QTextBlockIterator block, const QPoint &p, int width);
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

void QTextDocumentLayoutPrivate::relayoutDocument()
{
    QPoint pos(0, 0);
    QTextBlockIterator it = q->begin();
    int width = pageSize.width();
    while (!it.atEnd()) {
        // check if we are at a table
//         QTextTableFormat fmt = it.blockFormat().tableFormat();
//         if (fmt.isValid()) {
//             it = layoutTable(it, &pos, width);
//         } else
        {
            layoutBlock(it, pos, width);
            pos.setY(it.layout()->rect().bottom());
            ++it;
        }
    }
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
//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);
    QTextBlockIterator it = findBlock(from);
    QTextBlockIterator end = findBlock(from + length - 1);
    if (!end.atEnd())
        ++end;
    QPoint pos;

    if (it != begin()) {
        QTextBlockIterator prev = it;
        --prev;
        pos = prev.layout()->rect().bottomLeft();
    }

    bool move = false;
    while (it != end) {
//         qDebug("layouting block at pos %d", it.position());
        int old_bottom = it.layout()->rect().bottom();
        d->layoutBlock(it, pos, d->pageSize.width());
        int bottom = it.layout()->rect().bottom();
        pos.setY(bottom);
        move = (old_bottom != bottom);
        ++it;
    }
    if (move) {
        end = this->end();
        while (it != end) {
//             qDebug("moving block at pos %d", it.position());
            it.layout()->setPosition(pos);
            pos.setY(it.layout()->rect().bottom());
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
