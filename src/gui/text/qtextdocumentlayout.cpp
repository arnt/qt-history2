/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextdocumentlayout_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qrect.h>
#include <qpalette.h>
#include <qdebug.h>
#include <qvarlengtharray.h>
#include <limits.h>
#include <qstyle.h>

#include "qabstracttextdocumentlayout_p.h"

#include <qdebug.h>

// #define LAYOUT_DEBUG

#ifdef LAYOUT_DEBUG
#define LDEBUG qDebug()
#define INC_INDENT debug_indent += "  "
#define DEC_INDENT debug_indent = debug_indent.left(debug_indent.length()-2)
#else
#define LDEBUG if(0) qDebug()
#define INC_INDENT do {} while(0)
#define DEC_INDENT do {} while(0)
#endif

// ################ should probably add frameFormatChange notification!

struct LayoutStruct;

class QTextFrameData : public QTextFrameLayoutData
{
public:
    QTextFrameData()
        : minimumWidth(0), maximumWidth(INT_MAX), currentLayoutStruct(0),
          sizeDirty(true), layoutDirty(true)
        {}

    // relative to parent frame
    QPoint position;
    QSize size;

    // contents starts at (margin+border/margin+border)
    int margin;
    int border;
    int padding;
    // contents width includes padding (as we need to treat this on a per cell basis for tables)
    int contentsWidth;
    int contentsHeight;

    int minimumWidth;
    int maximumWidth;

    QTextFrameFormat::Position flow_position;

    LayoutStruct *currentLayoutStruct;

    bool sizeDirty;
    bool layoutDirty;

    QList<QTextFrame *> floats;
};

struct LayoutStruct {
    LayoutStruct() : widthUsed(0), minimumWidth(0), maximumWidth(INT_MAX), fullLayout(false) {}
    QTextFrame *frame;
    int x_left;
    int x_right;
    int y;
    int widthUsed;
    int minimumWidth;
    int maximumWidth;
    bool fullLayout;
    QList<QTextFrame *> pendingFloats;
};

class QTextTableData : public QTextFrameData
{
public:
    inline QTextTableData() : cellSpacing(0), cellPadding(0) {}
    int cellSpacing, cellPadding;
    QVector<int> minWidths;
    QVector<int> maxWidths;
    QVector<int> widths;
    QVector<int> heights;
    QVector<int> columnPositions;
    QVector<int> rowPositions;

    inline int cellWidth(int column, int colspan) const
    { return columnPositions.at(column + colspan - 1) + widths.at(column + colspan - 1)
             - columnPositions.at(column) - 2 * cellPadding; }

    inline void calcRowPosition(int row)
    {
        if (row > 0)
            rowPositions[row] = rowPositions.at(row - 1) + heights.at(row - 1) + border + cellSpacing + border;
    }
};

static QTextFrameData *createData(QTextFrame *f)
{
    QTextFrameData *data;
    if (qobject_cast<QTextTable *>(f))
        data = new QTextTableData;
    else
        data = new QTextFrameData;
    f->setLayoutData(data);
    return data;
}

static inline QTextFrameData *data(QTextFrame *f)
{
    QTextFrameData *data = static_cast<QTextFrameData *>(f->layoutData());
    if (!data)
        data = createData(f);
    return data;
}

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

* Floats are specified by a special format object.
* currently only floating images are implemented.

*/

/*

   On the table layouting:

   +---[ table border ]-------------------------
   |      [ cell spacing ]
   |  +------[ cell border ]-----+  +--------
   |  |                          |  |
   |  |
   |  |
   |  |
   |

   rowPositions[i] and columnPositions[i] point at the cell content
   position. So for example the left border is drawn at
   x = columnPositions[i] - fd->border and similar for y.

*/

enum {
    TextIndentValue = 40
};

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout)
public:

#if 0
    struct Page {
        QTextBlock first;
        QTextBlock last;
    };
    QList<Page> pages;
#endif

    QTextDocumentLayoutPrivate()
        : fixedColumnWidth(-1)
    { }

    QSize pageSize;
    bool pagedLayout;

    int blockTextFlags;
#ifdef LAYOUT_DEBUG
    mutable QString debug_indent;
#endif

    int fixedColumnWidth;

    int indent(QTextBlock bl) const;

    void drawFrame(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextFrame *f) const;
    void drawBlock(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextBlock bl) const;
    void drawListItem(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                      QTextBlock bl, bool highlight) const;

    enum HitPoint {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextFrame *frame, const QPoint &point, int *position) const;
    HitPoint hitTest(QTextBlock bl, const QPoint &point, int *position) const;

    void relayoutDocument();

    LayoutStruct layoutCell(QTextTable *t, const QTextTableCell &cell, int width,
                            int layoutFrom, int layoutTo);
    void setCellPosition(QTextTable *t, const QTextTableCell &cell, const QPoint &pos);
    void layoutTable(QTextTable *t, int layoutFrom, int layoutTo);

    void positionFloat(QTextFrame *frame, QTextLine *currentLine = 0);

    // calls the next one
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo);
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, int frameWidth, int frameHeight);

    void layoutBlock(const QTextBlock &block, LayoutStruct *layoutStruct, int layoutFrom, int layoutTo);
    void layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct, int layoutFrom, int layoutTo);


    void floatMargins(int y, const LayoutStruct *layoutStruct, int *left, int *right) const;
    int findY(int yFrom, const LayoutStruct *layoutStruct, int requiredWidth) const;
};

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QPoint &point, int *position) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    Q_ASSERT(!fd->layoutDirty);
    Q_ASSERT(!fd->sizeDirty);
    QPoint p = point - fd->position;

//     LDEBUG << "checking frame" << frame->firstPosition() << "point=" << point
//            << "position" << fd->position << "size" << fd->size;
    if (frame != q->document()->rootFrame()) {
        if (p.y() < 0 || p.x() < 0) {
            *position = frame->firstPosition() - 1;
//             LDEBUG << "before pos=" << *position;
            return PointBefore;
        } else if (p.y() > fd->size.height() || p.x() > fd->size.width()) {
            *position = frame->lastPosition() + 1;
//             LDEBUG << "after pos=" << *position;
            return PointAfter;
        }
    }
    INC_INDENT;

    int pos = -1;
    HitPoint hit = PointInside;
    QTextFrame::Iterator end = frame->end();
    for (QTextFrame::Iterator it = frame->begin(); it != end; ++it) {
        QTextFrame *c = it.currentFrame();
        HitPoint hp;
        if (c) {
            hp = hitTest(c, p, &pos);
        } else {
            hp = hitTest(it.currentBlock(), p, &pos);
        }
        if (hp >= PointInside) {
            hit = hp;
            *position = pos;
            break;
        }
        if (hp == PointBefore && pos < *position) {
            *position = pos;
            hit = hp;
        } else if (hp == PointAfter && pos > *position) {
            *position = pos;
            hit = hp;
        }
    }

    DEC_INDENT;
//     LDEBUG << "inside=" << hit << " pos=" << *position;
    return hit;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextBlock bl, const QPoint &point, int *position) const
{
    const QTextLayout *tl = bl.layout();
    QRect textrect = tl->rect().toRect();
//     LDEBUG << "    checking block" << bl.position() << "point=" << point
//            << "    tlrect" << textrect;
    if (!textrect.contains(point)) {
        *position = bl.position();
        if (point.y() < textrect.top()) {
//             LDEBUG << "    before pos=" << *position;
            return PointBefore;
        } else {
            *position += bl.length();
//             LDEBUG << "    after pos=" << *position;
            return PointAfter;
        }
    }

    QPoint pos = point - textrect.topLeft();

    // ### rtl?

    HitPoint hit = PointInside;
    *position = bl.position();
    int off = 0;
    for (int i = 0; i < tl->numLines(); ++i) {
        QTextLine line = tl->lineAt(i);
        // don't use line.rect() as it uses line.width() and we want textWidth() for the actual used
        // width
        QRect lr(qRound(line.x()), qRound(line.y()), qRound(line.textWidth()), qRound(line.height()));
        if (lr.top() > pos.y()) {
            off = qMin(off, line.from());
        } else if (lr.bottom() <= pos.y()) {
            off = qMax(off, line.from() + line.length());
        } else {
            if (lr.left() > pos.x()) {
                off = line.from();
            } else if (lr.right() < pos.x()) {
                off = line.from() + line.length();
            } else {
                hit = PointExact;
                off = line.xToCursor(pos.x());
            }
            break;
        }
    }
    *position += off;

//     LDEBUG << "    inside=" << hit << " pos=" << *position;
    return hit;
}

// ### could be moved to QTextBlock
int QTextDocumentLayoutPrivate::indent(QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    QTextBlockFormat blockFormat = bl.blockFormat();
    int indent = blockFormat.indent();

    QTextObject *object = q->document()->objectForFormat(blockFormat);
    if (object)
        indent += object->format().toListFormat().indent();

    return indent * TextIndentValue;
}

void QTextDocumentLayoutPrivate::drawFrame(const QPoint &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextFrame *frame) const
{
    QTextFrameData *fd = data(frame);
    Q_ASSERT(!fd->sizeDirty);
    Q_ASSERT(!fd->layoutDirty);

    const QPoint off = offset + fd->position;
    if (context.rect.isValid()
        && (off.y() > context.rect.bottom() || off.y() + fd->size.height() < context.rect.top()
            || off.x() > context.rect.right() || off.x() + fd->size.width() < context.rect.left()))
        return;

//     LDEBUG << debug_indent << "drawFrame" << frame->firstPosition() << "--" << frame->lastPosition() << "at" << offset;
//     INC_INDENT;

    QTextTable *table = qobject_cast<QTextTable *>(frame);

    // draw frame decoration
    if (fd->border) {
        painter->save();
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        const int margin = fd->margin + fd->border;
        const int w = fd->size.width() - 2*margin;
        const int h = fd->size.height() - 2*margin;
        // left
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin, fd->border, h + 2 * fd->border);
        // top
        painter->drawRect(off.x() + fd->margin + fd->border, off.y() + fd->margin, w + fd->border, fd->border);

        painter->setBrush(Qt::darkGray);

        // right
        painter->drawRect(off.x() + fd->margin + fd->border + w, off.y() + fd->margin + fd->border, fd->border, h);
        // bottom
        painter->drawRect(off.x() + fd->margin + fd->border, off.y() + fd->margin + fd->border + h, w + fd->border, fd->border);
        /*
        if (table) {
            QTextTableData *td = static_cast<QTextTableData *>(fd);
            int rows = table->rows();
            int columns = table->columns();
            for (int i = 1; i < rows; ++i) {
//                 qDebug("drawing row at %d/%d %d %d", off.x() + fd->margin, off.y() + td->rowPositions.at(i) - fd->border, w, fd->border);
                painter->drawRect(off.x() + fd->margin, off.y() + td->rowPositions.at(i) - fd->border, w, fd->border);
            }
            for (int i = 1; i < columns; ++i) {
//                 qDebug("drawing column at %d/%d %d %d", off.x() + td->columnPositions.at(i), off.y() + fd->margin, fd->border, h);
                painter->drawRect(off.x() + td->columnPositions.at(i) - fd->border, off.y() + fd->margin, fd->border, h);
            }
        }
        */
        painter->restore();
    }

    if (table) {
        const QColor bgCol = table->format().backgroundColor();
        if (bgCol.isValid()) {
            QRect bgRect = QRect(off, fd->size);
            const int margin = fd->margin + fd->border;
            bgRect.addCoords(margin, margin, -margin, -margin);
            painter->fillRect(bgRect, bgCol);
        }

        const int rows = table->rows();
        const int columns = table->columns();
        QTextTableData *td = static_cast<QTextTableData *>(data(table));

        int row_start = -1, col_start = -1, num_rows = -1, num_cols = -1;

        if (context.cursor.currentTable() == table)
            context.cursor.selectedTableCells(&row_start, &num_rows, &col_start,  &num_cols);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < columns; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                QRect cellRect;
                cellRect.setLeft(td->columnPositions.at(c));
                cellRect.setTop(td->rowPositions.at(r));
                cellRect.setWidth(td->columnPositions.at(c + cspan - 1) + td->widths.at(c + cspan - 1) - cellRect.left());
                cellRect.setHeight(td->rowPositions.at(r + rspan - 1) + td->heights.at(r + rspan - 1) - cellRect.top());

                cellRect.translate(off);
                if (context.rect.isValid() && !cellRect.intersects(context.rect))
                    continue;

                if (fd->border) {
                    const QBrush oldBrush = painter->brush();
                    const QPen oldPen = painter->pen();

                    painter->setBrush(Qt::darkGray);
                    painter->setPen(Qt::NoPen);

                    // top border
                    painter->drawRect(cellRect.left(), cellRect.top() - fd->border,
                                      cellRect.width() + fd->border, fd->border);
                    // left border
                    painter->drawRect(cellRect.left() - fd->border, cellRect.top() - fd->border,
                                      fd->border, cellRect.height() + 2 * fd->border);

                    painter->setBrush(Qt::lightGray);

                    // bottom border
                    painter->drawRect(cellRect.left(), cellRect.top() + cellRect.height(),
                                      cellRect.width() + fd->border, fd->border);
                    // right border
                    painter->drawRect(cellRect.left() + cellRect.width(), cellRect.top(),
                                      fd->border, cellRect.height());

                    painter->setBrush(oldBrush);
                    painter->setPen(oldPen);
                }

                {
                    const QColor bgCol = cell.format().tableCellBackgroundColor();
                    if (bgCol.isValid())
                        painter->fillRect(cellRect, bgCol);
                }

                QAbstractTextDocumentLayout::PaintContext cell_context = context;
                if (row_start != -1) {
                    if (r >= row_start && r < row_start + num_rows
                        && c >= col_start && c < col_start + num_cols) {
                        cell_context.cursor.setPosition(cell.firstPosition());
                        cell_context.cursor.setPosition(cell.lastPosition(), QTextCursor::KeepAnchor);
                    } else {
                        cell_context.cursor.clearSelection();
                    }
                }
                for (QTextFrame::Iterator it = cell.begin(); !it.atEnd(); ++it) {
                    QTextFrame *c = it.currentFrame();
                    if (c)
                        drawFrame(off, painter, cell_context, c);
                    else
                        drawBlock(off, painter, cell_context, it.currentBlock());
                }
            }
        }

    } else {
        for (QTextFrame::Iterator it = frame->begin(); !it.atEnd(); ++it) {
            QTextFrame *c = it.currentFrame();
            if (c)
                drawFrame(off, painter, context, c);
            else
                drawBlock(off, painter, context, it.currentBlock());
        }
    }
//     DEC_INDENT;
}

void QTextDocumentLayoutPrivate::drawBlock(const QPoint &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextLayout *tl = bl.layout();
    QRect r = tl->boundingRect().toRect();
    r.translate((offset + tl->position()).toPoint());
    if (context.rect.isValid() && !r.intersects(context.rect))
        return;
//      LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset << "br" << tl->boundingRect();

    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.showCursor ? context.cursor.position() : -1;

    if (bl.contains(cursor))
        cursor -= bl.position();
    else
        cursor = -1;

    QColor bgCol = blockFormat.backgroundColor();
    if (bgCol.isValid()) {
        QRect r = bl.layout()->rect().toRect();
        r.translate(offset);
        painter->fillRect(r , bgCol);
    }

    QList<QTextLayout::FormatOverride> overrides = tl->formatOverrides();
    bool highlightListItem = false;
    if (context.cursor.hasSelection()) {
        int blpos = bl.position();
        int bllen = bl.length();
        const int selStart = context.cursor.selectionStart() - blpos;
        const int selEnd = context.cursor.selectionEnd() - blpos;
        if (selStart < bllen && selEnd > 0) {
            QTextLayout::FormatOverride o;
            o.from = selStart;
            o.length = selEnd - selStart;
            o.format.setBackgroundColor(context.palette.color(QPalette::Highlight));
            o.format.setTextColor(context.palette.color(QPalette::HighlightedText));
            QList<QTextLayout::FormatOverride> newOverrides = overrides;
            newOverrides.append(o);
            const_cast<QTextLayout *>(tl)->setFormatOverrides(newOverrides);
        }
        if (selStart <= 0 && selEnd >= 1)
            highlightListItem = true;
    }

    QTextObject *object = q->document()->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(offset, painter, context, bl, highlightListItem);

    const_cast<QTextLayout *>(tl)->setPalette(context.palette);

    tl->draw(painter, offset, context.rect);
    if (cursor >= 0)
        tl->drawCursor(painter, offset, cursor);
    const_cast<QTextLayout *>(tl)->setFormatOverrides(overrides);
}


void QTextDocumentLayoutPrivate::drawListItem(const QPoint &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlock bl, bool highlight) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextBlockFormat blockFormat = bl.blockFormat();
    const QTextCharFormat charFormat = bl.charFormat();
    const QFont font = charFormat.font();
    const QFontMetrics fontMetrics(font);
    QTextObject * const object = q->document()->objectForFormat(blockFormat);
    const QTextListFormat lf = object->format().toListFormat();
    const int style = lf.style();
    QString itemText;
    QSize size;

    QTextLine firstLine = bl.layout()->lineAt(0);
    Q_ASSERT(firstLine.isValid());
    QPoint pos = offset + bl.layout()->rect().toRect().topLeft();
    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    {
        QRectF textRect = firstLine.textRect();
        pos += textRect.origin().toPoint();
        if (dir == Qt::RightToLeft)
            pos.rx() += qRound(textRect.width());
    }

    switch (style) {
    case QTextListFormat::ListDecimal:
    case QTextListFormat::ListLowerAlpha:
    case QTextListFormat::ListUpperAlpha:
        itemText = static_cast<QTextList *>(object)->itemText(bl);
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

    QRect r(pos, size);

    int xoff = fontMetrics.width(QLatin1Char(' '));
    if (dir == Qt::LeftToRight)
        xoff = -xoff - size.width();
    r.translate( xoff, (fontMetrics.height() / 2 - size.height() / 2));

    painter->save();

    if (highlight) {
        painter->setPen(context.palette.highlightedText().color());

        painter->fillRect(r, context.palette.highlight());
    } else {
        QColor col = charFormat.textColor();
        if (!col.isValid())
            col = context.palette.text().color();
        painter->setPen(col);
    }

    QBrush brush = context.palette.brush(QPalette::Text);

    switch (style) {
    case QTextListFormat::ListDecimal:
    case QTextListFormat::ListLowerAlpha:
    case QTextListFormat::ListUpperAlpha:
        painter->setFont(font);
        painter->drawText(QPointF(r.left(), pos.y() + fontMetrics.ascent()), itemText);
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
        break;
    default:
        break;
    }

    painter->restore();
}


void QTextDocumentLayoutPrivate::relayoutDocument()
{
    Q_Q(QTextDocumentLayout);
    const QTextDocument *doc = q->document();
    q->documentChange(0, 0, doc->docHandle()->length());
}

static bool isFrameInCell(const QTextTableCell &cell, QTextFrame *frame)
{
    const int cellStart = cell.firstPosition();
    const int cellEnd = cell.lastPosition();
    const int frameStart = frame->firstPosition();
    const int frameEnd = frame->lastPosition();

    return cellStart <= frameStart && cellStart <= frameEnd
           && cellEnd >= frameStart && cellEnd >= frameEnd;
}

LayoutStruct QTextDocumentLayoutPrivate::layoutCell(QTextTable *t, const QTextTableCell &cell, int width,
                                                    int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutCell";
    LayoutStruct layoutStruct;
    layoutStruct.frame = t;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = INT_MAX;
    layoutStruct.y = 0;
    layoutStruct.x_left = 0;
    layoutStruct.x_right = width;

    QList<QTextFrame *> floats;

    // ### speed up
    // layout out child frames in that cell first
    for (int i = 0; i < t->childFrames().size(); ++i){
        QTextFrame *frame = t->childFrames().at(i);
        if (isFrameInCell(cell, frame)) {
            QTextFrameData *cd = data(frame);
            if (cd->sizeDirty)
                layoutFrame(frame, frame->firstPosition(), frame->lastPosition(), width, -1);
            layoutStruct.minimumWidth = qMax(layoutStruct.minimumWidth, cd->size.width());

            if (cd->flow_position != QTextFrameFormat::InFlow)
                floats.append(frame);
        }
    }

    int floatMinWidth = layoutStruct.minimumWidth;

    layoutFlow(cell.begin(), &layoutStruct, layoutFrom, layoutTo);

    // floats that are located inside the text (like inline images) aren't taken into account by
    // layoutFlow with regards to the cell height (layoutStruct->y), so for a safety measure we
    // do that here. For example with <td><img align="right" src="..." />blah</td>
    // when the image happens to be higher than the text
    for (int i = 0; i < floats.size(); ++i)
        layoutStruct.y = qMax(layoutStruct.y, data(floats.at(i))->size.height());

    // constraint the maximumWidth by the minimum width of the fixed size floats, to
    // keep them visible
    layoutStruct.maximumWidth = qMax(layoutStruct.maximumWidth, floatMinWidth);

//    qDebug() << "layoutCell done";

    return layoutStruct;
}

// ##### shouldn't we just add a global offset to each cell instead?
void QTextDocumentLayoutPrivate::setCellPosition(QTextTable *t, const QTextTableCell &cell, const QPoint &pos)
{
    // #### don't we add the offset twice here????
    for (int i = 0; i < t->childFrames().size(); ++i) {
        QTextFrame *frame = t->childFrames().at(i);
        if (isFrameInCell(cell, frame)) {
            QTextFrameData *cd = data(frame);
            cd->position += pos;
        }
    }

    for (QTextFrame::Iterator it = cell.begin(); !it.atEnd(); ++it) {
        if (QTextFrame *c = it.currentFrame()) {
            QTextFrameData *cd = data(c);
            cd->position += pos;
        } else {
            QTextBlock bl = it.currentBlock();
            QTextLayout *tl = bl.layout();
            tl->setPosition(tl->position() + pos);
        }
    }
}

void QTextDocumentLayoutPrivate::layoutTable(QTextTable *table, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutTable";
    QTextTableData *td = static_cast<QTextTableData *>(data(table));
    Q_ASSERT(td->sizeDirty);
    const int rows = table->rows();
    const int columns = table->columns();

    const QTextTableFormat fmt = table->format();

    QVector<QTextLength> columnWidthConstraints = fmt.columnWidthConstraints();
    if (columnWidthConstraints.isEmpty()) {
        columnWidthConstraints.resize(columns);
        columnWidthConstraints.fill(QTextLength());
    }
    Q_ASSERT(columnWidthConstraints.count() == columns);

    const int cellSpacing = td->cellSpacing = fmt.cellSpacing();
    td->cellPadding = fmt.cellPadding();
    const int margin = td->margin + td->border + td->padding;

    int initialTotalWidth = fmt.width().value(td->contentsWidth);
    int totalWidth = initialTotalWidth;
    // two (vertical) borders per cell per column
    totalWidth -= columns * 2 * td->border;
    // inter-cell spacing
    totalWidth -= (columns - 1) * cellSpacing;
    // cell spacing at the left and right hand side
    totalWidth -= 2 * cellSpacing;

    td->widths.resize(columns);
    td->widths.fill(0);

    td->minWidths.resize(columns);
    // start with a minimum width of 0. totally empty
    // cells of default created tables are invisible otherwise
    // and therefore hardly editable
    td->minWidths.fill(1);

    td->maxWidths.resize(columns);
    td->maxWidths.fill(INT_MAX);

    // calculate minimum and maximum sizes of the columns
    for (int i = 0; i < columns; ++i) {
        for (int row = 0; row < rows; ++row) {
            const QTextTableCell cell = table->cellAt(row, i);
            const int cspan = cell.columnSpan();

            if (cspan > 1 && i != cell.column())
                continue;

            // to figure out the min and the max width lay out the cell at
            // maximum width. otherwise the maxwidth calculation sometimes
            // returns wrong values
            LayoutStruct layoutStruct = layoutCell(table, cell, INT_MAX, layoutFrom, layoutTo);

            // distribute the minimum width over all columns the cell spans
            int widthToDistribute = layoutStruct.minimumWidth + 2 * td->cellPadding;
            for (int n = 0; n < cspan; ++n) {
                const int col = i + n;
                int w = widthToDistribute / (cspan - n);
                td->minWidths[col] = qMax(td->minWidths.at(col), w);
                widthToDistribute -= td->minWidths.at(col);
                if (widthToDistribute <= 0)
                    break;
            }

            // ### colspans
            int maxW = td->maxWidths.at(i);
            if (layoutStruct.maximumWidth != INT_MAX) {
                if (maxW == INT_MAX)
                    maxW = layoutStruct.maximumWidth + 2 * td->cellPadding;
                else
                    maxW = qMax(maxW, layoutStruct.maximumWidth + 2 * td->cellPadding);
            }
            td->maxWidths[i] = qMax(td->minWidths.at(i), maxW);
        }
    }

    // set fixed values, figure out total percentages used and number of
    // variable length cells. Also assign the minimum width for variable columns.
    int totalPercentage = 0;
    int variableCols = 0;
    for (int i = 0; i < columns; ++i) {
        const QTextLength &length = columnWidthConstraints.at(i);
        if (length.type() == QTextLength::FixedLength) {
            td->widths[i] = qMax(length.rawValue(), td->minWidths.at(i));
            totalWidth -= td->widths.at(i);
        } else if (length.type() == QTextLength::PercentageLength) {
            totalPercentage += length.rawValue();
        } else if (length.type() == QTextLength::VariableLength) {
            variableCols++;

            td->widths[i] = td->minWidths.at(i);
            totalWidth -= td->minWidths.at(i);
        }
    }

    // set percentage values
    {
        const int totalPercentagedWidth = initialTotalWidth * totalPercentage / 100;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::PercentageLength) {
                const int percentWidth = totalPercentagedWidth * columnWidthConstraints.at(i).rawValue() / totalPercentage;
                td->widths[i] = qMax(percentWidth, td->minWidths.at(i));
                totalWidth -= td->widths.at(i);
            }
    }

    // for variable columns distribute the remaining space
    if (variableCols > 0 && totalWidth > 0) {
        QVarLengthArray<int> anySizeColumns;
        QVarLengthArray<int> columnsWithProperMaxSize;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::VariableLength) {
                if (td->maxWidths.at(i) == INT_MAX)
                    anySizeColumns.append(i);
                else
                    columnsWithProperMaxSize.append(i);
            }

        int lastTotalWidth = totalWidth;
        while (totalWidth > 0) {
            for (int k = 0; k < columnsWithProperMaxSize.count(); ++k) {
                const int col = columnsWithProperMaxSize[k];
                const int colsLeft = columnsWithProperMaxSize.count() - k;
                const int w = qMin(td->maxWidths.at(col) - td->widths.at(col), totalWidth / colsLeft);
                td->widths[col] += w;
                totalWidth -= w;
            }
            if (totalWidth == lastTotalWidth)
                break;
            lastTotalWidth = totalWidth;
        }

        if (totalWidth > 0 && !anySizeColumns.isEmpty()
                // don't unnecessarily grow variable length sized tables
                && fmt.width().type() != QTextLength::VariableLength) {
            const int widthPerAnySizedCol = totalWidth / anySizeColumns.count();
            for (int k = 0; k < anySizeColumns.count(); ++k) {
                const int col = anySizeColumns[k];
                td->widths[col] += widthPerAnySizedCol;
            }
        }
    }


    td->columnPositions.resize(columns);
    td->columnPositions[0] = margin /*includes table border*/ + cellSpacing + td->border;

    for (int i = 1; i < columns; ++i)
        td->columnPositions[i] = td->columnPositions.at(i-1) + td->widths.at(i-1) + td->border + cellSpacing + td->border;

    td->heights.resize(rows);
    td->heights.fill(0);

    td->rowPositions.resize(rows);
    td->rowPositions[0] = margin /*includes table border*/ + cellSpacing + td->border;

    bool haveRowSpannedCells = false;

    // now that we have the column widths we can lay out all cells with the right
    // width, to calculate the row heights. we have to use two passes though, cells
    // which span more than one row have to be processed later to avoid them enlarging
    // other calls too much
    //
    // ### this could be made faster by iterating over the cells array of QTextTable
    for (int r = 0; r < rows; ++r) {
        td->calcRowPosition(r);

        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            const int rspan = cell.rowSpan();
            const int cspan = cell.columnSpan();

            if (cspan > 1 && cell.column() != c)
                continue;

            if (rspan > 1) {
                haveRowSpannedCells = true;
                continue;
            }

            const int width = td->cellWidth(c, cspan);
//            qDebug() << "layoutCell for cell at row" << r << "col" << c;
            LayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo);

            td->heights[r] = qMax(td->heights.at(r), layoutStruct.y + 2 * td->cellPadding);
        }
    }

    if (haveRowSpannedCells) {
        for (int r = 0; r < rows; ++r) {
            td->calcRowPosition(r);

            for (int c = 0; c < columns; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                const int rspan = cell.rowSpan();
                const int cspan = cell.columnSpan();

                if (cspan > 1 && cell.column() != c)
                    continue;

                if (rspan == 1)
                    continue;

                if (cell.row() != r)
                    continue;

                const int width = td->cellWidth(c, cspan);
                LayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo);

                // the last row gets all the remaining space
                int heightToDistribute = layoutStruct.y + 2 * td->cellPadding;
                for (int n = 0; n < rspan - 1; ++n) {
                    const int row = r + n;
                    heightToDistribute -= td->heights.at(row) + td->border + cellSpacing + td->border;
                    if (heightToDistribute <= 0)
                        break;
                }

                if (heightToDistribute > 0) {
                    const int lastRow = r + rspan - 1;
                    td->heights[lastRow] = qMax(td->heights.at(lastRow), heightToDistribute);
                }
            }
        }
    }

    // finally position all cells
    for (int r = 0; r < rows; ++r) {
        const int y = td->rowPositions.at(r) + td->cellPadding;
        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            const int rspan = cell.rowSpan();
            const int cspan = cell.columnSpan();

            if (cspan > 1 && cell.column() != c)
                continue;

            if (rspan > 1 && cell.row() != r)
                    continue;

            const int x = td->columnPositions.at(c) + td->cellPadding;
            setCellPosition(table, cell, QPoint(x, y));
        }
    }

    // - margin to compensate the + margin in columnPositions[0]
//    td->contentsWidth = qMax(td->contentsWidth,
//                             td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin);
    td->contentsWidth = td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin;
    int height = td->contentsHeight == -1
                 ? td->rowPositions.last() + td->heights.last() + td->padding + td->border + cellSpacing + margin
                 : td->contentsHeight + 2*margin;
    td->size = QSize(td->contentsWidth + 2*margin, height);
    td->sizeDirty = false;
}

void QTextDocumentLayoutPrivate::positionFloat(QTextFrame *frame, QTextLine *currentLine)
{
    Q_D(QTextDocumentLayout);
    QTextFrameData *fd = data(frame);

    QTextFrame *parent = frame->parentFrame();
    Q_ASSERT(parent);
    QTextFrameData *pd = data(parent);
    Q_ASSERT(pd && pd->currentLayoutStruct);

    if (!pd->floats.contains(frame))
        pd->floats.append(frame);
    fd->layoutDirty = true;
    Q_ASSERT(!fd->sizeDirty);

//     qDebug() << "positionFloat:" << frame << "width=" << fd->size.width();
    int y = pd->currentLayoutStruct->y;
    if (currentLine) {
        int left, right;
        d->floatMargins(y, pd->currentLayoutStruct, &left, &right);
//         qDebug() << "have line: right=" << right << "left=" << left << "textWidth=" << currentLine->textWidth();
        if (right - left < currentLine->textWidth() + fd->size.width()) {
            pd->currentLayoutStruct->pendingFloats.append(frame);
//             qDebug() << "    adding to pending list";
            return;
        }
    }
    y = findY(y, pd->currentLayoutStruct, fd->size.width());

    int left, right;
    d->floatMargins(y, pd->currentLayoutStruct, &left, &right);

    if (fd->flow_position == QTextFrameFormat::FloatLeft)
        fd->position = QPoint(left, y);
    else
        fd->position = QPoint(right - fd->size.width(), y);

//     qDebug()<< "float positioned at " << fd->position;
    fd->layoutDirty = false;
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutFrame (pre)";
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->format();

    QTextFrame *parent = f->parentFrame();
    const QTextFrameData *pd = parent ? data(parent) : 0;

    const int maximumWidth = pd ? pd->contentsWidth : pageSize.width();

    const int width = fformat.width().value(maximumWidth);

    QTextLength height = fformat.height();
    int h = height.value(pd ? pd->contentsHeight : -1);

    layoutFrame(f, layoutFrom, layoutTo, width, h);
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, int frameWidth, int frameHeight)
{
    LDEBUG << "layoutFrame from=" << layoutFrom << "to=" << layoutTo;
    Q_Q(QTextDocumentLayout);
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameData *fd = data(f);

    {
        QTextFrameFormat fformat = f->format();
        // set sizes of this frame from the format
        fd->margin = fformat.margin();
        fd->border = fformat.border();
        fd->padding = fformat.padding();

        fd->contentsWidth = frameWidth - 2*(fd->margin + fd->border);

        if (frameHeight != -1) {
            fd->contentsHeight = frameHeight - 2*(fd->margin + fd->border);
        } else {
            fd->contentsHeight = frameHeight;
        }

        fd->flow_position = fformat.position();
    }

    int startPos = f->firstPosition();
    int endPos = f->lastPosition();
    if (startPos > endPos) {
        // inline image
        QTextCharFormat format = q->format(startPos - 1);
        QTextObjectInterface *iface = q->handlerForObject(format.objectType());
        if (iface)
            fd->size = iface->intrinsicSize(q->document(), format).toSize();
        fd->sizeDirty = false;
        return;
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
        layoutTable(table, layoutFrom, layoutTo);
        return;
    }

    // needed for child frames with a minimum width that is
    // more than what we can offer
    int newContentsWidth = fd->contentsWidth;

    // layout child frames
    QList<QTextFrame *> children = f->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = data(c);
        if (cd->sizeDirty) {
//            QSize oldsize = cd->size;
            layoutFrame(c, layoutFrom, layoutTo);
//             if (oldsize != cd->size)
//                 fullLayout = true;
            newContentsWidth = qMax(newContentsWidth, cd->size.width());
        }
    }
    // #### might need to relayout everything
//     if (fd->contentsWidth != newContentsWidth) {
//         ;
//     }

    int margin = fd->margin + fd->border;
    LayoutStruct layoutStruct;
    layoutStruct.frame = f;
    layoutStruct.x_left = margin + fd->padding;
    layoutStruct.x_right = margin + fd->contentsWidth - fd->padding;
    layoutStruct.y = margin + fd->padding;
    layoutStruct.widthUsed = 0;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = INT_MAX;
    layoutStruct.fullLayout = fd->contentsWidth != newContentsWidth;
    LDEBUG << "layoutStruct: x_left" << layoutStruct.x_left << "x_right" << layoutStruct.x_right
           << "fullLayout" << layoutStruct.fullLayout;

    fd->contentsWidth = newContentsWidth;

    QTextFrame::Iterator it = f->begin();
    layoutFlow(it, &layoutStruct, layoutFrom, layoutTo);

    fd->contentsWidth = qMax(fd->contentsWidth, layoutStruct.widthUsed);
    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    int height = fd->contentsHeight == -1
                 ? layoutStruct.y + margin + fd->padding
                 : fd->contentsHeight + 2*margin;
    fd->size = QSize(fd->contentsWidth + 2*margin, height);
    fd->sizeDirty = false;
}

void QTextDocumentLayoutPrivate::layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct,
                                            int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutFlow from=" << layoutFrom << "to=" << layoutTo;
    QTextFrameData *fd = data(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    for ( ; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();

        if (c) {
            // position child frame
            QTextFrameData *cd = data(c);
            Q_ASSERT(!cd->sizeDirty);
            if (cd->flow_position == QTextFrameFormat::InFlow) {
                Qt::Alignment align = Qt::AlignLeft;

                if (QTextTable *table = qobject_cast<QTextTable *>(c))
                    align = table->format().alignment();

                QPoint pos(layoutStruct->x_left, layoutStruct->y);

                if (align == Qt::AlignRight)
                    pos.rx() += layoutStruct->x_right - cd->size.width();
                else if (align == Qt::AlignHCenter)
                    pos.rx() += (layoutStruct->x_right - cd->size.width()) / 2;

                cd->position = pos;
                layoutStruct->y += cd->size.height();
                cd->layoutDirty = false;
            } else {
                positionFloat(c);
            }
        } else {
//            QTextBlock block = it.currentBlock();
//             if (block.position() + block.length() < layoutFrom)
//                 continue;
            // layout and position child block
            layoutBlock(it.currentBlock(), layoutStruct, layoutFrom, layoutTo);
        }
    }

    fd->currentLayoutStruct = 0;
}

void QTextDocumentLayoutPrivate::layoutBlock(const QTextBlock &bl, LayoutStruct *layoutStruct,
                                             int layoutFrom, int layoutTo)
{
    Q_D(QTextDocumentLayout);
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    LDEBUG << "layoutBlock from=" << layoutFrom << "to=" << layoutTo;

    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    Qt::Alignment align = QStyle::visualAlignment(dir, blockFormat.alignment());
    QTextOption option(align);
    option.setLayoutDirection(dir);
    if (d->blockTextFlags & Qt::TextSingleLine || blockFormat.nonBreakableLines())
        option.setWrapMode(QTextOption::ManualWrap);
    tl->setTextOption(option);

//    qDebug() << "layoutBlock; width" << layoutStruct->x_right - layoutStruct->x_left << "(maxWidth is btw" << tl->maximumWidth() << ")";

    layoutStruct->y += blockFormat.topMargin();

    //QTextFrameData *fd = data(layoutStruct->frame);

    const int indent = this->indent(bl);

    tl->setPosition(QPoint(layoutStruct->x_left, layoutStruct->y));

    if (layoutStruct->fullLayout || (bl.position() + bl.length() > layoutFrom && bl.position() <= layoutTo)) {

//         qDebug() << "    layouting block at" << bl.position();
        const int cy = layoutStruct->y;
        int l = layoutStruct->x_left + blockFormat.leftMargin();
        int r = layoutStruct->x_right - blockFormat.rightMargin();
        if (dir == Qt::RightToLeft)
            r -= indent;
        else
            l += indent;

//    tl->useDesignMetrics(true);
//     tl->enableKerning(true);
        tl->setLayoutMode(QTextLayout::NoGlyphCache);
        tl->beginLayout();
        while (1) {
            QTextLine line = tl->createLine();
            if (!line.isValid())
                break;

            int left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);

            left = qMax(left, l);
            right = qMin(right, r);
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;

            if (d->fixedColumnWidth != -1)
                line.layoutFixedColumnWidth(d->fixedColumnWidth);
            else
                line.layout(right - left);

//        qDebug() << "layoutBlock; layouting line with width" << right - left << "->textWidth" << line.textWidth();
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);

            if (d->fixedColumnWidth == -1 && line.textWidth() > right-left) {
                // float has been added in the meantime, redo
                layoutStruct->pendingFloats.clear();
                line.layout(right-left);
                if (line.textWidth() > right-left) {
                    layoutStruct->pendingFloats.clear();
                    // lines min width more than what we have
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, qRound(line.textWidth()));
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                    line.layout(qMax(line.textWidth(), right-left));
                }
            }

            line.setPosition(QPoint(left - layoutStruct->x_left, layoutStruct->y - cy));
            layoutStruct->y += qRound(line.ascent() + line.descent() + 1);
            layoutStruct->widthUsed = qRound(qMax(layoutStruct->widthUsed, left - layoutStruct->x_left + line.textWidth()));

            // position floats
            for (int i = 0; i < layoutStruct->pendingFloats.size(); ++i) {
                QTextFrame *f = layoutStruct->pendingFloats.at(i);
                positionFloat(f);
            }
            layoutStruct->pendingFloats.clear();
        }
        tl->endLayout();
    } else {
        layoutStruct->y += qRound(tl->boundingRect().height());
    }

    layoutStruct->y += blockFormat.bottomMargin();
    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, tl->minimumWidth() + blockFormat.leftMargin() + indent);

    const int maxW = tl->maximumWidth() + blockFormat.leftMargin() + indent;
    if (maxW > 0) {
        if (layoutStruct->maximumWidth == INT_MAX)
            layoutStruct->maximumWidth = maxW;
        else
            layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maxW);
    }


}

void QTextDocumentLayoutPrivate::floatMargins(int y, const LayoutStruct *layoutStruct,
                                              int *left, int *right) const
{
//     qDebug() << "floatMargins y=" << y;
    *left = layoutStruct->x_left;
    *right = layoutStruct->x_right;
    QTextFrameData *lfd = data(layoutStruct->frame);
    for (int i = 0; i < lfd->floats.size(); ++i) {
        QTextFrameData *fd = data(lfd->floats.at(i));
        if (!fd->layoutDirty) {
            if (fd->position.y() <= y && fd->position.y() + fd->size.height() > y) {
//                 qDebug() << "adjusting with float" << f << fd->position.x()<< fd->size.width();
                if (fd->flow_position == QTextFrameFormat::FloatLeft)
                    *left = qMax(*left, fd->position.x() + fd->size.width());
                else
                    *right = qMin(*right, fd->position.x());
            }
        }
    }
//     qDebug() << "floatMargins: left="<<*left<<"right="<<*right<<"y="<<y;
}


int QTextDocumentLayoutPrivate::findY(int yFrom, const LayoutStruct *layoutStruct, int requiredWidth) const
{
    int right, left;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

//     qDebug() << "findY:" << yFrom;
    while (1) {
        floatMargins(yFrom, layoutStruct, &left, &right);
//         qDebug() << "    yFrom=" << yFrom<<"right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        int newY = INT_MAX;
        QTextFrameData *lfd = data(layoutStruct->frame);
        for (int i = 0; i < lfd->floats.size(); ++i) {
            QTextFrameData *fd = data(lfd->floats.at(i));
            if (!fd->layoutDirty) {
                if (fd->position.y() <= yFrom && fd->position.y() + fd->size.height() > yFrom)
                    newY = qMin(newY, fd->position.y() + fd->size.height());
            }
        }
        if (newY == INT_MAX)
            break;
        yFrom = newY;
    }
    return yFrom;
}


QTextDocumentLayout::QTextDocumentLayout(QTextDocument *doc)
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate, doc)
{
    Q_D(QTextDocumentLayout);
    d->blockTextFlags = Qt::TextIncludeTrailingSpaces|Qt::TextWordWrap;

    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    Q_D(QTextDocumentLayout);
    QTextFrame *frame = document()->rootFrame();
    if(data(frame)->sizeDirty)
        return;
    d->drawFrame(QPoint(), painter, context, frame);
}

static void markFrames(QTextFrame *current, int from, int oldLength, int length)
{
    int end = qMax(oldLength, length) + from;

    if (current->firstPosition() >= end || current->lastPosition() < from)
        return;

    QTextFrameData *fd = data(current);
    for (int i = 0; i < fd->floats.size(); ++i) {
        QTextFrame *f = fd->floats[i];
        if (!f->parentFrame()) {
            // float got removed in editing operation
            fd->floats.removeAt(i);
            --i;
        }
    }

    fd->layoutDirty = true;
    fd->sizeDirty = true;

//     qDebug("    marking frame (%d--%d) as dirty", current->firstPosition(), current->lastPosition());
    QList<QTextFrame *> children = current->childFrames();
    for (int i = 0; i < children.size(); ++i)
        markFrames(children.at(i), from, oldLength, length);
}

void QTextDocumentLayout::documentChange(int from, int oldLength, int length)
{
    Q_D(QTextDocumentLayout);
    if (d->pageSize.isNull() || !d->pageSize.isValid())
        return;


//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(document()->rootFrame(), from, oldLength, length);

    const QSize oldSize = sizeUsed();

    QTextFrame *root = document()->rootFrame();
    if(data(root)->sizeDirty)
        d->layoutFrame(root, from, from + length);
    data(root)->layoutDirty = false;

    const QSize newSize = sizeUsed();
    if (newSize != oldSize)
        emit usedSizeChanged();

    emit update();
}

int QTextDocumentLayout::hitTest(const QPoint &point, Qt::HitTestAccuracy accuracy) const
{
    Q_D(const QTextDocumentLayout);
    QTextFrame *f = document()->rootFrame();
    int position = 0;
    QTextDocumentLayoutPrivate::HitPoint p = d->hitTest(f, point, &position);
    if (accuracy == Qt::ExactHit && p < QTextDocumentLayoutPrivate::PointExact)
        return -1;

    // ensure we stay within document bounds
    if (position > f->lastPosition())
        position = f->lastPosition();
    else if (position < 0)
        position = 0;

    return position;
}

void QTextDocumentLayout::setSize(QTextInlineObject item, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QSizeF intrinsic = handler.iface->intrinsicSize(document(), format);

    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    if (frame) {
        pos = frame->format().position();
        data(frame)->sizeDirty = false;
        data(frame)->size = intrinsic.toSize();
    }

    item.setDescent(0);
    QSizeF inlineSize = (pos == QTextFrameFormat::InFlow ? intrinsic : QSizeF(0, 0));
    item.setWidth(inlineSize.width());
    item.setAscent(inlineSize.height());
}

void QTextDocumentLayout::layoutObject(QTextInlineObject item, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    if (item.width() != 0)
        // inline
        return;

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    if (!frame)
        return;

    QTextBlock b = document()->findBlock(frame->firstPosition());
    QTextLine line;
    if (b.position() <= frame->firstPosition() && b.position() + b.length() > frame->lastPosition())
        line = b.layout()->lineAt(b.layout()->numLines()-1);
//     qDebug() << "layoutObject: line.isValid" << line.isValid() << b.position() << b.length() <<
//         frame->firstPosition() << frame->lastPosition();
    d->positionFloat(frame, line.isValid() ? &line : 0);
}

void QTextDocumentLayout::drawObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                                     const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    QRect r = rect.toRect();
    if (frame) {
        QTextFrameData *fd = data(frame);
        if (fd->flow_position != QTextFrameFormat::InFlow) {
            r = QRect(fd->position, fd->size);
            r.translate(data(frame->parentFrame())->position);
        }
    }
//    qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawObject(p, r, item, format);
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
    Q_D(QTextDocumentLayout);
    d->pageSize = size;
    d->relayoutDocument();
}

QSize QTextDocumentLayout::pageSize() const
{
    Q_D(const QTextDocumentLayout);
    return d->pageSize;
}

QSize QTextDocumentLayout::sizeUsed() const
{
    Q_Q(const QTextDocumentLayout);
    return data(q->document()->rootFrame())->size;
}

// Pull this private function in from qglobal.cpp
Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n);

void QTextDocumentLayout::adjustSize()
{
    // ##### use default doc font
    QFont f;
    QFontMetrics fm(f);
    int mw =  fm.width('x') * 80;
    int w = mw;
    setPageSize(QSize(w, INT_MAX));
    QSize size = sizeUsed();
    if (size.width() != 0) {
        w = qt_int_sqrt(5 * size.height() * size.width() / 3);
        setPageSize(QSize(qMin(w, mw), INT_MAX));

        if (w*3 < 5*size.height()) {
            w = qt_int_sqrt(2 * size.height() * size.width());
            setPageSize(QSize(qMin(w, mw), INT_MAX));
        }
    }
}

void QTextDocumentLayout::setBlockTextFlags(int flags)
{
    Q_D(QTextDocumentLayout);
    d->blockTextFlags = flags;
}

int QTextDocumentLayout::blockTextFlags() const
{
    Q_D(const QTextDocumentLayout);
    return d->blockTextFlags;
}

void QTextDocumentLayout::setFixedColumnWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->fixedColumnWidth = width;
}

QRect QTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    return QRect(data(frame)->position, data(frame)->size);
}

