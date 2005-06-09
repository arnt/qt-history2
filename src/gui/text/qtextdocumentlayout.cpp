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

#include "qabstracttextdocumentlayout_p.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qdebug.h>
#include <qvarlengtharray.h>
#include <limits.h>
#include <qstyle.h>

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
    QPointF position;
    QSizeF size;

    // contents starts at (margin+border/margin+border)
    qreal margin;
    qreal border;
    qreal padding;
    // contents width includes padding (as we need to treat this on a per cell basis for tables)
    qreal contentsWidth;
    qreal contentsHeight;

    qreal minimumWidth;
    qreal maximumWidth;

    QTextFrameFormat::Position flow_position;

    LayoutStruct *currentLayoutStruct;

    bool sizeDirty;
    bool layoutDirty;

    QList<QTextFrame *> floats;
};

struct LayoutStruct {
    LayoutStruct() : widthUsed(0), minimumWidth(0), maximumWidth(INT_MAX), fullLayout(false), pageHeight(0.0), pageBottom(0.0) {}
    QTextFrame *frame;
    qreal x_left;
    qreal x_right;
    qreal y;
    qreal widthUsed;
    qreal minimumWidth;
    qreal maximumWidth;
    bool fullLayout;
    QList<QTextFrame *> pendingFloats;
    qreal pageHeight;
    qreal pageBottom;

    inline void newPage()
    { pageBottom += pageHeight; y = pageBottom - pageHeight; }
};

class QTextTableData : public QTextFrameData
{
public:
    inline QTextTableData() : cellSpacing(0), cellPadding(0) {}
    qreal cellSpacing, cellPadding;
    QVector<qreal> minWidths;
    QVector<qreal> maxWidths;
    QVector<qreal> widths;
    QVector<qreal> heights;
    QVector<qreal> columnPositions;
    QVector<qreal> rowPositions;
    // rows that appear at the top of a page after a page break
    QVector<int> rowPageBreaks;

    inline qreal cellWidth(int column, int colspan) const
    { return columnPositions.at(column + colspan - 1) + widths.at(column + colspan - 1)
             - columnPositions.at(column) - 2 * cellPadding; }

    inline void calcRowPosition(int row)
    {
        if (row > 0)
            rowPositions[row] = rowPositions.at(row - 1) + heights.at(row - 1) + border + cellSpacing + border;
    }

    QRectF cellRect(int row, int rowSpan, int colum, int colSpan) const;
    inline QRectF cellRect(const QTextTableCell &cell) const
    { return cellRect(cell.row(), cell.rowSpan(), cell.column(), cell.columnSpan()); }

    inline QPointF cellPosition(int row, int col) const
    { return QPointF(columnPositions.at(col) + cellPadding, rowPositions.at(row) + cellPadding); }

    void updateSize();
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

void QTextTableData::updateSize()
{
    const qreal effectiveMargin = this->margin + border + padding;
    qreal height = contentsHeight == -1
                   ? rowPositions.last() + heights.last() + padding + border + cellSpacing + effectiveMargin
                   : contentsHeight + 2*effectiveMargin;
    size = QSizeF(contentsWidth + 2*effectiveMargin, height);
}

QRectF QTextTableData::cellRect(int row, int rowSpan, int column, int colSpan) const
{
    QRectF r(columnPositions.at(column),
             rowPositions.at(row),
             columnPositions.at(column + colSpan - 1) + widths.at(column + colSpan - 1) - columnPositions.at(column),
             rowPositions.at(row + rowSpan - 1) + heights.at(row + rowSpan - 1) - rowPositions.at(row));
    return r;
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

struct CheckPoint
{
    qreal y;
    int positionInFrame;
};

static bool operator<(const CheckPoint &checkPoint, qreal y)
{
    return checkPoint.y < y;
}

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
        : blockTextFlags(0), wordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere),
          fixedColumnWidth(-1)
    { }

    bool pagedLayout;

    int blockTextFlags;
    QTextOption::WrapMode wordWrapMode;
#ifdef LAYOUT_DEBUG
    mutable QString debug_indent;
#endif

    int fixedColumnWidth;

    qreal indent(QTextBlock bl) const;

    enum DrawResult { OutsideClipRect, Drawn };

    DrawResult drawFrame(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                         QTextFrame *f) const;
    DrawResult drawBlock(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                         QTextBlock bl) const;
    void drawListItem(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                      QTextBlock bl, const QTextCharFormat *selectionFormat) const;

    enum HitPoint {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextFrame *frame, const QPointF &point, int *position) const;
    HitPoint hitTest(QTextBlock bl, const QPointF &point, int *position) const;

    void relayoutDocument();

    LayoutStruct layoutCell(QTextTable *t, const QTextTableCell &cell, qreal width,
                            int layoutFrom, int layoutTo);
    void setCellPosition(QTextTable *t, const QTextTableCell &cell, const QPointF &pos);
    void layoutTable(QTextTable *t, int layoutFrom, int layoutTo);

    void positionFloat(QTextFrame *frame, QTextLine *currentLine = 0);

    // calls the next one
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo);
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, qreal frameWidth, qreal frameHeight);

    void layoutBlock(const QTextBlock &block, LayoutStruct *layoutStruct, int layoutFrom, int layoutTo,
                     bool isFirstBlockInFlow, bool isLastBlockInFlow);
    void layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct, int layoutFrom, int layoutTo);
    void pageBreakInsideTable(QTextTable *table, LayoutStruct *layoutStruct);


    void floatMargins(qreal y, const LayoutStruct *layoutStruct, qreal *left, qreal *right) const;
    qreal findY(qreal yFrom, const LayoutStruct *layoutStruct, qreal requiredWidth) const;

    QVector<CheckPoint> checkPoints;

    QTextFrame::Iterator iteratorForYPosition(qreal y) const;
};

QTextFrame::Iterator QTextDocumentLayoutPrivate::iteratorForYPosition(qreal y) const
{
    Q_Q(const QTextDocumentLayout);

    const QTextDocumentPrivate *doc = q->document()->docHandle();
    QTextFrame *rootFrame = doc->rootFrame();

    if (checkPoints.isEmpty()
        || y < 0 || y > data(rootFrame)->size.height())
        return rootFrame->begin();

    QVector<CheckPoint>::ConstIterator checkPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), y);
    if (checkPoint == checkPoints.end())
        return rootFrame->begin();

    if (checkPoint != checkPoints.begin())
        --checkPoint;

    const int position = rootFrame->firstPosition() + checkPoint->positionInFrame;
    const QTextDocumentPrivate::BlockMap &map = doc->blockMap();
    const int begin = map.findNode(rootFrame->firstPosition());
    const int end = map.findNode(rootFrame->lastPosition()+1);

    const int block = map.findNode(position);
    const int blockPos = map.position(block);

    QTextFrame::iterator it(rootFrame, block, begin, end);

    QTextFrame *containingFrame = doc->frameAt(blockPos);
    if (containingFrame != rootFrame) {
        while (containingFrame->parentFrame() != rootFrame) {
            containingFrame = containingFrame->parentFrame();
            Q_ASSERT(containingFrame);
        }

        it.cf = containingFrame;
        it.cb = 0;
    }

    return it;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QPointF &point, int *position) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    Q_ASSERT(!fd->layoutDirty);
    Q_ASSERT(!fd->sizeDirty);
    const QPointF relativePoint = point - fd->position;

    QTextFrame *rootFrame = q->document()->rootFrame();

//     LDEBUG << "checking frame" << frame->firstPosition() << "point=" << point
//            << "position" << fd->position << "size" << fd->size;
    if (frame != rootFrame) {
        if (relativePoint.y() < 0 || relativePoint.x() < 0) {
            *position = frame->firstPosition() - 1;
//             LDEBUG << "before pos=" << *position;
            return PointBefore;
        } else if (relativePoint.y() > fd->size.height() || relativePoint.x() > fd->size.width()) {
            *position = frame->lastPosition() + 1;
//             LDEBUG << "after pos=" << *position;
            return PointAfter;
        }
    }
    INC_INDENT;

    HitPoint hit = PointInside;

    QTextTable *table = qobject_cast<QTextTable *>(frame);
    QTextTableData *td = 0;
    int row = 0;
    int col = 0;

    if (table)
        td = static_cast<QTextTableData *>(fd);

    int pos = -1;

    do {
        QTextFrame::Iterator it = frame->begin();
        QTextFrame::Iterator end = frame->end();

        QPointF p = relativePoint;

        if (frame == rootFrame) {
            it = iteratorForYPosition(p.y());

            Q_ASSERT(it.parentFrame() == frame);

            if (it.currentFrame())
                pos = it.currentFrame()->firstPosition();
            else
                pos = it.currentBlock().position();
            *position = pos;
            hit = PointBefore;
        }

        if (table) {
            QTextTableCell cell = table->cellAt(row, col);
            const QRectF rect = td->cellRect(cell);
            if (rect.contains(p)) {
                it = cell.begin();
                end = cell.end();
                p -= rect.topLeft();
            } else {
                it = QTextFrame::Iterator();
                end = it;
            }
        }

        for (; it != end; ++it) {
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
                table = 0; // be sure to exit from the while loop, too
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

        if (table) {
            ++col;
            if (col >= table->columns()) {
                col = 0;
                ++row;
            }
        }

    } while (table && row < table->rows());

    DEC_INDENT;
//     LDEBUG << "inside=" << hit << " pos=" << *position;
    return hit;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextBlock bl, const QPointF &point, int *position) const
{
    const QTextLayout *tl = bl.layout();
    QRectF textrect = tl->boundingRect();
    textrect.translate(tl->position());
//     LDEBUG << "    checking block" << bl.position() << "point=" << point
//            << "    tlrect" << textrect;
    if (point.y() < textrect.top()) {
        *position = bl.position();
//             LDEBUG << "    before pos=" << *position;
        return PointBefore;
    } else if (point.y() > textrect.bottom()) {
        *position += bl.length();
//             LDEBUG << "    after pos=" << *position;
        return PointAfter;
    }

    QPointF pos = point - textrect.topLeft();

    // ### rtl?

    HitPoint hit = PointInside;
    *position = bl.position();
    int off = 0;
    for (int i = 0; i < tl->lineCount(); ++i) {
        QTextLine line = tl->lineAt(i);
        const QRectF lr = line.naturalTextRect();
        if (lr.top() > pos.y()) {
            off = qMin(off, line.textStart());
        } else if (lr.bottom() <= pos.y()) {
            off = qMax(off, line.textStart() + line.textLength());
        } else {
            if (lr.left() > pos.x()) {
                off = line.textStart();
            } else if (lr.right() < pos.x()) {
                off = line.textStart() + line.textLength();
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
qreal QTextDocumentLayoutPrivate::indent(QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    QTextBlockFormat blockFormat = bl.blockFormat();
    qreal indent = blockFormat.indent();

    QTextObject *object = q->document()->objectForFormat(blockFormat);
    if (object)
        indent += object->format().toListFormat().indent();

    qreal scale = 1;
    if (q->paintDevice()) {
        extern int qt_defaultDpi();
        scale = q->paintDevice()->logicalDpiY() / qt_defaultDpi();
    }

    return indent * TextIndentValue * scale;
}

static void drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect)
{
    if (fd->border) {
        painter->save();
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        const qreal margin = fd->margin + fd->border;
        const qreal w = rect.width() - 2*margin;
        const qreal h = rect.height() - 2*margin;
        // left
        painter->drawRect(QRectF(rect.left() + fd->margin, rect.top() + fd->margin, fd->border, h + 2 * fd->border));
        // top
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border, rect.top() + fd->margin, w + fd->border, fd->border));

        painter->setBrush(Qt::darkGray);
        // right
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border + w, rect.top() + fd->margin + fd->border, fd->border, h));
        // bottom
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border, rect.top() + fd->margin + fd->border + h, w + fd->border, fd->border));

        painter->restore();
    }

    const QBrush bg = frame->frameFormat().background();
    if (bg != Qt::NoBrush) {
        QRectF bgRect = rect;
        const qreal margin = fd->margin + fd->border;
        bgRect.adjust(margin, margin, -margin, -margin);

        if (!frame->parentFrame())
            bgRect = clip;

        painter->fillRect(bgRect, bg);
    }
}

QTextDocumentLayoutPrivate::DrawResult
QTextDocumentLayoutPrivate::drawFrame(const QPointF &offset, QPainter *painter,
                                      const QAbstractTextDocumentLayout::PaintContext &context,
                                      QTextFrame *frame) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    Q_ASSERT(!fd->sizeDirty);
    Q_ASSERT(!fd->layoutDirty);

    const QPointF off = offset + fd->position;
    if (context.clip.isValid()
        && (off.y() > context.clip.bottom() || off.y() + fd->size.height() < context.clip.top()
            || off.x() > context.clip.right() || off.x() + fd->size.width() < context.clip.left()))
        return OutsideClipRect;

//     LDEBUG << debug_indent << "drawFrame" << frame->firstPosition() << "--" << frame->lastPosition() << "at" << offset;
//     INC_INDENT;

    QTextTable *table = qobject_cast<QTextTable *>(frame);
    const QRectF frameRect(off, fd->size);

    if (table) {
        const int rows = table->rows();
        const int columns = table->columns();
        QTextTableData *td = static_cast<QTextTableData *>(data(table));

        if (td->rowPageBreaks.isEmpty()) {
            drawFrameDecoration(painter, frame, fd, context.clip, frameRect);
        } else {
            Q_ASSERT(td->rowPageBreaks.first() > 0);
            QRectF rect = frameRect;

            const qreal extraTableHeight = td->padding + td->border + td->cellSpacing // inter cell spacing
                                           + td->margin + td->border + td->padding; // effective table margin

            int lastVisibleRow = td->rowPageBreaks.first() - 1;

            rect.setHeight(td->rowPositions.at(lastVisibleRow) + td->heights.at(lastVisibleRow) + extraTableHeight);
            drawFrameDecoration(painter, frame, fd, context.clip, rect);

            for (int i = 0; i < td->rowPageBreaks.count(); ++i) {
                const int firstVisibleRow = td->rowPageBreaks.at(i);
                if (i < td->rowPageBreaks.count() - 1)
                    lastVisibleRow = td->rowPageBreaks.at(i + 1) - 1;
                else
                    lastVisibleRow = rows - 1;

                rect.setTop(off.y() + td->rowPositions.at(firstVisibleRow) - extraTableHeight);
                rect.setBottom(off.y() + td->rowPositions.at(lastVisibleRow) + td->heights.at(lastVisibleRow) + extraTableHeight);

                drawFrameDecoration(painter, frame, fd, context.clip, rect);
            }
        }

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

                QRectF cellRect = td->cellRect(r, rspan, c, cspan);

                cellRect.translate(off);
                if (context.clip.isValid() && !cellRect.intersects(context.clip))
                    continue;

                if (fd->border) {
                    const QBrush oldBrush = painter->brush();
                    const QPen oldPen = painter->pen();

                    painter->setBrush(Qt::darkGray);
                    painter->setPen(Qt::NoPen);

                    // top border
                    painter->drawRect(QRectF(cellRect.left(), cellRect.top() - fd->border,
                                             cellRect.width() + fd->border, fd->border));
                    // left border
                    painter->drawRect(QRectF(cellRect.left() - fd->border, cellRect.top() - fd->border,
                                             fd->border, cellRect.height() + 2 * fd->border));

                    painter->setBrush(Qt::lightGray);

                    // bottom border
                    painter->drawRect(QRectF(cellRect.left(), cellRect.top() + cellRect.height(),
                                             cellRect.width() + fd->border, fd->border));
                    // right border
                    painter->drawRect(QRectF(cellRect.left() + cellRect.width(), cellRect.top(),
                                             fd->border, cellRect.height()));

                    painter->setBrush(oldBrush);
                    painter->setPen(oldPen);
                }

                {
                    const QBrush bg = cell.format().background();
                    if (bg != Qt::NoBrush)
                        painter->fillRect(cellRect, bg);
                }

                QAbstractTextDocumentLayout::PaintContext cell_context = context;
                for (int i = 0; i < context.selections.size(); ++i) {
                    const QAbstractTextDocumentLayout::Selection &s = context.selections.at(i);
                    int row_start = -1, col_start = -1, num_rows = -1, num_cols = -1;

                    if (s.cursor.currentTable() == table)
                        s.cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);
                    if (row_start != -1) {
                        if (r >= row_start && r < row_start + num_rows
                            && c >= col_start && c < col_start + num_cols) {
                            cell_context.selections[i].cursor.setPosition(cell.firstPosition());
                            cell_context.selections[i].cursor.setPosition(cell.lastPosition(), QTextCursor::KeepAnchor);
                        } else {
                            cell_context.selections[i].cursor.clearSelection();
                        }
                    }
                }
                const QPointF cellPos = off + td->cellPosition(r, c);
                for (QTextFrame::Iterator it = cell.begin(); !it.atEnd(); ++it) {
                    QTextFrame *c = it.currentFrame();
                    if (c)
                        drawFrame(cellPos, painter, cell_context, c);
                    else
                        drawBlock(cellPos, painter, cell_context, it.currentBlock());
                }
            }
        }

    } else {
        drawFrameDecoration(painter, frame, fd, context.clip, frameRect);

        DrawResult previousDrawResult = OutsideClipRect;

        QTextFrame::Iterator it = frame->begin();

        if (frame == q->document()->rootFrame())
            it = iteratorForYPosition(context.clip.top());

        for (; !it.atEnd(); ++it) {
            QTextFrame *c = it.currentFrame();
            DrawResult r;
            if (c)
                r = drawFrame(off, painter, context, c);
            else
                r = drawBlock(off, painter, context, it.currentBlock());

            // floats do not necessarily follow vertical ordering, so don't
            // let them influence the optimization below
            if (c && c->frameFormat().position() != QTextFrameFormat::InFlow)
                continue;

            // assume vertical ordering and thus stop when we reached
            // unreachable parts
            if (previousDrawResult == Drawn && r == OutsideClipRect)
                break;

            previousDrawResult = r;
        }
    }
//     DEC_INDENT;

    return Drawn;
}

QTextDocumentLayoutPrivate::DrawResult
QTextDocumentLayoutPrivate::drawBlock(const QPointF &offset, QPainter *painter,
                                      const QAbstractTextDocumentLayout::PaintContext &context,
                                      QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextLayout *tl = bl.layout();
    QRectF r = tl->boundingRect();
    r.translate((offset + tl->position()).toPoint());
    if (context.clip.isValid() && !r.intersects(context.clip))
        return OutsideClipRect;
//      LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset << "br" << tl->boundingRect();

    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.cursorPosition;

    if (bl.contains(cursor))
        cursor -= bl.position();
    else
        cursor = -1;

    QBrush bg = blockFormat.background();
    if (bg != Qt::NoBrush) {
        QRectF r = bl.layout()->boundingRect();
        r.translate(bl.layout()->position() + offset);
        painter->fillRect(r, bg);
    }

    QVector<QTextLayout::FormatRange> selections;
    int blpos = bl.position();
    int bllen = bl.length();
    const QTextCharFormat *selFormat = 0;
    for (int i = 0; i < context.selections.size(); ++i) {
        const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
        const int selStart = range.cursor.selectionStart() - blpos;
        const int selEnd = range.cursor.selectionEnd() - blpos;
        if (selStart < bllen && selEnd > 0) {
            QTextLayout::FormatRange o;
            o.start = selStart;
            o.length = selEnd - selStart;
            o.format = range.format;
            selections.append(o);
        }
        if (selStart <= 0 && selEnd >= 1)
            selFormat = &range.format;
    }

    QTextObject *object = q->document()->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(offset, painter, context, bl, selFormat);

    QPen oldPen = painter->pen();
    painter->setPen(context.palette.color(QPalette::Text));

    tl->draw(painter, offset, selections, context.clip);
    if (cursor >= 0 && tl->preeditAreaText().isEmpty())
        tl->drawCursor(painter, offset, cursor);

    painter->setPen(oldPen);

    return Drawn;
}


void QTextDocumentLayoutPrivate::drawListItem(const QPointF &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlock bl, const QTextCharFormat *selectionFormat) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextBlockFormat blockFormat = bl.blockFormat();
    const QTextCharFormat charFormat = bl.charFormat();
    const QFont font(charFormat.font(), painter->device());
    const QFontMetrics fontMetrics(font);
    QTextObject * const object = q->document()->objectForFormat(blockFormat);
    const QTextListFormat lf = object->format().toListFormat();
    const int style = lf.style();
    QString itemText;
    QSizeF size;

    QTextLayout *layout = bl.layout();
    QTextLine firstLine = layout->lineAt(0);
    Q_ASSERT(firstLine.isValid());
    QPointF pos = (offset + layout->boundingRect().topLeft() + layout->position()).toPoint();
    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    {
        QRectF textRect = firstLine.naturalTextRect();
        pos += textRect.topLeft().toPoint();
        if (dir == Qt::RightToLeft)
            pos.rx() += textRect.width();
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

    QRectF r(pos, size);

    qreal xoff = fontMetrics.width(QLatin1Char(' '));
    if (dir == Qt::LeftToRight)
        xoff = -xoff - size.width();
    r.translate( xoff, (fontMetrics.height() / 2 - size.height() / 2));

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    if (selectionFormat) {
        painter->setPen(QPen(selectionFormat->foreground(), 0));
        painter->fillRect(r, selectionFormat->background());
    } else {
        QBrush fg = charFormat.foreground();
        if (fg == Qt::NoBrush)
            fg = context.palette.text();
        painter->setPen(QPen(fg, 0));
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
    q->documentChanged(0, 0, doc->docHandle()->length());
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

LayoutStruct QTextDocumentLayoutPrivate::layoutCell(QTextTable *t, const QTextTableCell &cell, qreal width,
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
            cd->sizeDirty = true;
                layoutFrame(frame, frame->firstPosition(), frame->lastPosition(), width, -1);
            layoutStruct.minimumWidth = qMax(layoutStruct.minimumWidth, cd->minimumWidth);
            layoutStruct.maximumWidth = qMin(layoutStruct.maximumWidth, cd->maximumWidth);

            if (cd->flow_position != QTextFrameFormat::InFlow)
                floats.append(frame);
        }
    }

    qreal floatMinWidth = layoutStruct.minimumWidth;

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

    // as floats in cells get added to the table's float list but must not affect
    // floats in other cells we must clear the list here.
    data(t)->floats.clear();

//    qDebug() << "layoutCell done";

    return layoutStruct;
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
    if (columnWidthConstraints.size() != columns)
        columnWidthConstraints.resize(columns);
    Q_ASSERT(columnWidthConstraints.count() == columns);

    const qreal cellSpacing = td->cellSpacing = fmt.cellSpacing();
    td->cellPadding = fmt.cellPadding();
    const qreal margin = td->margin + td->border + td->padding;

    qreal totalWidth = fmt.width().value(td->contentsWidth);
    // two (vertical) borders per cell per column
    totalWidth -= columns * 2 * td->border;
    // inter-cell spacing
    totalWidth -= (columns - 1) * cellSpacing;
    // cell spacing at the left and right hand side
    totalWidth -= 2 * cellSpacing;
    // remember the width used to distribute to percentaged columns
    qreal initialTotalWidth = totalWidth;

    td->widths.resize(columns);
    td->widths.fill(0);

    td->minWidths.resize(columns);
    // start with a minimum width of 0. totally empty
    // cells of default created tables are invisible otherwise
    // and therefore hardly editable
    td->minWidths.fill(1);

    td->maxWidths.resize(columns);
    td->maxWidths.fill(INT_MAX);

    td->rowPageBreaks.clear();

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
            qreal widthToDistribute = layoutStruct.minimumWidth + 2 * td->cellPadding;
            for (int n = 0; n < cspan; ++n) {
                const int col = i + n;
                qreal w = widthToDistribute / (cspan - n);
                td->minWidths[col] = qMax(td->minWidths.at(col), w);
                widthToDistribute -= td->minWidths.at(col);
                if (widthToDistribute <= 0)
                    break;
            }

            // ### colspans
            qreal maxW = td->maxWidths.at(i);
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
    qreal totalPercentage = 0;
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
        const qreal totalPercentagedWidth = initialTotalWidth * totalPercentage / 100;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::PercentageLength) {
                const qreal percentWidth = totalPercentagedWidth * columnWidthConstraints.at(i).rawValue() / totalPercentage;
                td->widths[i] = qMax(percentWidth, td->minWidths.at(i));
                totalWidth -= td->widths.at(i);
            }
    }

    // for variable columns distribute the remaining space
    if (variableCols > 0 && totalWidth > 0) {
        QVarLengthArray<int> columnsWithProperMaxSize;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::VariableLength
                && td->maxWidths.at(i) != INT_MAX)
                columnsWithProperMaxSize.append(i);

        qreal lastTotalWidth = totalWidth;
        while (totalWidth > 0) {
            for (int k = 0; k < columnsWithProperMaxSize.count(); ++k) {
                const int col = columnsWithProperMaxSize[k];
                const int colsLeft = columnsWithProperMaxSize.count() - k;
                const qreal w = qMin(td->maxWidths.at(col) - td->widths.at(col), totalWidth / colsLeft);
                td->widths[col] += w;
                totalWidth -= w;
            }
            if (totalWidth == lastTotalWidth)
                break;
            lastTotalWidth = totalWidth;
        }

        if (totalWidth > 0
            // don't unnecessarily grow variable length sized tables
            && fmt.width().type() != QTextLength::VariableLength) {
            const qreal widthPerAnySizedCol = totalWidth / variableCols;
            for (int col = 0; col < columns; ++col) {
                if (columnWidthConstraints.at(col).type() == QTextLength::VariableLength)
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

            const qreal width = td->cellWidth(c, cspan);
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

                const qreal width = td->cellWidth(c, cspan);
                LayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo);

                // the last row gets all the remaining space
                qreal heightToDistribute = layoutStruct.y + 2 * td->cellPadding;
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

    // - margin to compensate the + margin in columnPositions[0]
//    td->contentsWidth = qMax(td->contentsWidth,
//                             td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin);
    td->contentsWidth = td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin;

    td->minimumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i) {
        td->minimumWidth += td->minWidths.at(i) + td->border + cellSpacing + td->border;
    }
    td->minimumWidth += margin - td->border;

    td->maximumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i)
        if (td->maxWidths.at(i) != INT_MAX)
            td->maximumWidth += td->maxWidths.at(i) + td->border + cellSpacing + td->border;
    td->maximumWidth += margin - td->border;

    td->updateSize();
    td->sizeDirty = false;
}

void QTextDocumentLayoutPrivate::positionFloat(QTextFrame *frame, QTextLine *currentLine)
{
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
    qreal y = pd->currentLayoutStruct->y;
    if (currentLine) {
        qreal left, right;
        floatMargins(y, pd->currentLayoutStruct, &left, &right);
//         qDebug() << "have line: right=" << right << "left=" << left << "textWidth=" << currentLine->textWidth();
        if (right - left < currentLine->naturalTextWidth() + fd->size.width()) {
            pd->currentLayoutStruct->pendingFloats.append(frame);
//             qDebug() << "    adding to pending list";
            return;
        }
    }

    if (!parent->parentFrame() /* float in root frame */
        && y + fd->size.height() > pd->currentLayoutStruct->pageBottom) {
        y = pd->currentLayoutStruct->pageBottom;
    }

    y = findY(y, pd->currentLayoutStruct, fd->size.width());

    qreal left, right;
    floatMargins(y, pd->currentLayoutStruct, &left, &right);

    if (fd->flow_position == QTextFrameFormat::FloatLeft)
        fd->position = QPointF(left, y);
    else
        fd->position = QPointF(right - fd->size.width(), y);

//     qDebug()<< "float positioned at " << fd->position;
    fd->layoutDirty = false;
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutFrame (pre)";
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->frameFormat();

    QTextFrame *parent = f->parentFrame();
    const QTextFrameData *pd = parent ? data(parent) : 0;

    const qreal maximumWidth = pd ? pd->contentsWidth : q_func()->document()->pageSize().width();

    const qreal width = fformat.width().value(maximumWidth);

    QTextLength height = fformat.height();
    qreal h = height.value(pd ? pd->contentsHeight : -1);

    layoutFrame(f, layoutFrom, layoutTo, width, h);
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, qreal frameWidth, qreal frameHeight)
{
    LDEBUG << "layoutFrame from=" << layoutFrom << "to=" << layoutTo;
    Q_Q(QTextDocumentLayout);
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameData *fd = data(f);

    {
        QTextFrameFormat fformat = f->frameFormat();
        // set sizes of this frame from the format
        fd->margin = fformat.margin();
        fd->border = fformat.border();
        fd->padding = fformat.padding();

        fd->contentsWidth = frameWidth - 2*(fd->margin + fd->border + fd->padding);

        if (frameHeight != -1) {
            fd->contentsHeight = frameHeight - 2*(fd->margin + fd->border + fd->padding);
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
            fd->size = iface->intrinsicSize(q->document(), startPos - 1, format).toSize();
        fd->sizeDirty = false;
        return;
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
        layoutTable(table, layoutFrom, layoutTo);
        return;
    }

    // needed for child frames with a minimum width that is
    // more than what we can offer
    qreal newContentsWidth = fd->contentsWidth;

    // layout child frames
    QList<QTextFrame *> children = f->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = data(c);
        if (cd->sizeDirty) {
//            QSizeF oldsize = cd->size;
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

    qreal margin = fd->margin + fd->border;
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

    if (!f->parentFrame()) {
        layoutStruct.pageHeight = q->document()->pageSize().height();
        layoutStruct.pageBottom = layoutStruct.pageHeight;
    }

    QTextFrame::Iterator it = f->begin();
    layoutFlow(it, &layoutStruct, layoutFrom, layoutTo);

    fd->contentsWidth = qMax(fd->contentsWidth, layoutStruct.widthUsed);
    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    qreal height = fd->contentsHeight == -1
                 ? layoutStruct.y + margin + fd->padding
                 : fd->contentsHeight + 2*margin;
    fd->size = QSizeF(fd->contentsWidth + 2*margin, height);
    fd->sizeDirty = false;
}

void QTextDocumentLayoutPrivate::layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct,
                                            int layoutFrom, int layoutTo)
{
    Q_Q(QTextDocumentLayout);
    LDEBUG << "layoutFlow from=" << layoutFrom << "to=" << layoutTo;
    QTextFrameData *fd = data(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    const bool inRootFrame = (it.parentFrame() == q->document()->rootFrame());
    if (inRootFrame) {
        checkPoints.clear();
        CheckPoint cp;
        cp.y = 0;
        cp.positionInFrame = 0;
        checkPoints << cp;
    }

    bool firstIteration = true;

    while (!it.atEnd()) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame && qAbs(layoutStruct->y - checkPoints.last().y) > 2000) {
            qreal left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            if (left == layoutStruct->x_left && right == layoutStruct->x_right) {
                CheckPoint p;
                p.y = layoutStruct->y;
                if (it.currentFrame()) {
                    p.positionInFrame = it.currentFrame()->firstPosition();
                } else {
                    p.positionInFrame = it.currentBlock().position();
                }

                checkPoints.append(p);
            }
        }

        if (c) {
            // position child frame
            QTextFrameData *cd = data(c);
            Q_ASSERT(!cd->sizeDirty);
            if (cd->flow_position == QTextFrameFormat::InFlow) {
                qreal left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, layoutStruct->x_left);
                right = qMin(right, layoutStruct->x_right);

                if (right - left < cd->size.width()) {
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, cd->size.width());
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                }

                QPointF pos(left, layoutStruct->y);

                Qt::Alignment align = Qt::AlignLeft;

                if (QTextTable *table = qobject_cast<QTextTable *>(c))
                    align = table->format().alignment();

                if (align == Qt::AlignRight)
                    pos.rx() += layoutStruct->x_right - cd->size.width();
                else if (align == Qt::AlignHCenter)
                    pos.rx() += (layoutStruct->x_right - cd->size.width()) / 2;

                cd->position = pos;
                layoutStruct->y += cd->size.height();
                cd->layoutDirty = false;

                if (inRootFrame
                    && cd->position.y() + cd->size.height() > layoutStruct->pageBottom) {

                    QTextTable *table = qobject_cast<QTextTable *>(c);
                    if (table && cd->size.height() > layoutStruct->pageHeight / 2) {
                        pageBreakInsideTable(table, layoutStruct);
                    } else {
                        layoutStruct->newPage();
                        cd->position.setY(layoutStruct->y);
                        layoutStruct->y += cd->size.height();
                    }
                }
            } else {
                positionFloat(c);
            }
            ++it;
        } else {
            QTextBlock block = it.currentBlock();
            ++it;
            bool lastBlockInFlow = (it.atEnd());

            // layout and position child block
            layoutBlock(block, layoutStruct, layoutFrom, layoutTo, firstIteration, lastBlockInFlow);
        }

        firstIteration = false;
    }

    if (inRootFrame) {
        CheckPoint cp;
        cp.y = layoutStruct->y;
        cp.positionInFrame = q->document()->docHandle()->length();
        checkPoints.append(cp);
    }


    fd->currentLayoutStruct = 0;
}

void QTextDocumentLayoutPrivate::layoutBlock(const QTextBlock &bl, LayoutStruct *layoutStruct,
                                             int layoutFrom, int layoutTo, bool firstBlockInFlow, bool lastBlockInFlow)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    LDEBUG << "layoutBlock from=" << layoutFrom << "to=" << layoutTo;

    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    if (blockTextFlags & ((int)QTextDocumentLayout::LTR|(int)QTextDocumentLayout::RTL)) {
        if (!blockFormat.hasProperty(QTextFormat::LayoutDirection))
            dir = blockTextFlags & QTextDocumentLayout::LTR ? Qt::LeftToRight : Qt::RightToLeft;
    }
    Qt::Alignment align = QStyle::visualAlignment(dir, blockFormat.alignment());
    if (blockTextFlags & Qt::AlignHorizontal_Mask) {
        if (!blockFormat.hasProperty(QTextFormat::BlockAlignment))
            align = (Qt::Alignment)(blockTextFlags & Qt::AlignHorizontal_Mask);
    }
    QTextOption option(align);
    option.setTextDirection(dir);
    if (blockTextFlags & Qt::TextSingleLine || blockFormat.nonBreakableLines())
        option.setWrapMode(QTextOption::ManualWrap);
    else
        option.setWrapMode(wordWrapMode);
    tl->setTextOption(option);

    const bool haveWordOrAnyWrapMode = (option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere);

//    qDebug() << "layoutBlock; width" << layoutStruct->x_right - layoutStruct->x_left << "(maxWidth is btw" << tl->maximumWidth() << ")";

    if (!firstBlockInFlow)
        layoutStruct->y += blockFormat.topMargin();

    //QTextFrameData *fd = data(layoutStruct->frame);

    const qreal indent = this->indent(bl);

    tl->setPosition(QPointF(layoutStruct->x_left, layoutStruct->y));
    const QRectF tlBoundingRect = tl->boundingRect();

    if (layoutStruct->fullLayout
        || (bl.position() + bl.length() > layoutFrom && bl.position() <= layoutTo)
        // force relayout if we cross a page boundary
        || (layoutStruct->pageHeight > 0.0 && layoutStruct->y + tlBoundingRect.height() > layoutStruct->pageBottom)) {

//         qDebug() << "    layouting block at" << bl.position();
        const qreal cy = layoutStruct->y;
        const qreal l = layoutStruct->x_left  + blockFormat.leftMargin()  + (dir == Qt::RightToLeft ? 0 : indent);
        const qreal r = layoutStruct->x_right - blockFormat.rightMargin() - (dir == Qt::RightToLeft ? indent : 0);

        tl->beginLayout();
        bool firstLine = true;
        while (1) {
            QTextLine line = tl->createLine();
            if (!line.isValid())
                break;

            qreal left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            qreal text_indent = 0;
            if (firstLine) {
                text_indent = blockFormat.textIndent();
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
                firstLine = false;
            }
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;

            if (fixedColumnWidth != -1)
                line.setNumColumns(fixedColumnWidth);
            else
                line.setLineWidth(right - left);

//        qDebug() << "layoutBlock; layouting line with width" << right - left << "->textWidth" << line.textWidth();
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            if (dir == Qt::LeftToRight)
                left += text_indent;
            else
                right -= text_indent;

            if (fixedColumnWidth == -1 && line.naturalTextWidth() > right-left) {
                // float has been added in the meantime, redo
                layoutStruct->pendingFloats.clear();

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WrapAnywhere);
                    tl->setTextOption(option);
                }

                line.setLineWidth(right-left);
                if (line.naturalTextWidth() > right-left) {
                    layoutStruct->pendingFloats.clear();
                    // lines min width more than what we have
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, line.naturalTextWidth());
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                    left = qMax(left, l);
                    right = qMin(right, r);
                    if (dir == Qt::LeftToRight)
                        left += text_indent;
                    else
                        right -= text_indent;
                    line.setLineWidth(qMax<qreal>(line.naturalTextWidth(), right-left));
                }

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WordWrap);
                    tl->setTextOption(option);
                }
            }

            qreal lineHeight = line.height();
            if (layoutStruct->pageHeight > 0.0 && layoutStruct->y + lineHeight > layoutStruct->pageBottom) {
                layoutStruct->newPage();

                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, l);
                right = qMin(right, r);
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
            }

            line.setPosition(QPointF(left - layoutStruct->x_left, layoutStruct->y - cy));
            layoutStruct->y += lineHeight;
            layoutStruct->widthUsed
                = qMax<qreal>(layoutStruct->widthUsed, left - layoutStruct->x_left + line.naturalTextWidth());

            // position floats
            for (int i = 0; i < layoutStruct->pendingFloats.size(); ++i) {
                QTextFrame *f = layoutStruct->pendingFloats.at(i);
                positionFloat(f);
            }
            layoutStruct->pendingFloats.clear();
        }
        tl->endLayout();
    } else {
        layoutStruct->y += tlBoundingRect.height();
    }

    if (!lastBlockInFlow)
        layoutStruct->y += blockFormat.bottomMargin();

    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, tl->minimumWidth() + blockFormat.leftMargin() + indent);

    const qreal maxW = tl->maximumWidth() + blockFormat.leftMargin() + indent;
    if (maxW > 0) {
        if (layoutStruct->maximumWidth == INT_MAX)
            layoutStruct->maximumWidth = maxW;
        else
            layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maxW);
    }


}

void QTextDocumentLayoutPrivate::pageBreakInsideTable(QTextTable *table, LayoutStruct *layoutStruct)
{
    QTextTableData *td = static_cast<QTextTableData *>(data(table));
    const int rows = table->rows();
    Q_ASSERT(rows > 0);
    qreal origY = layoutStruct->y - td->size.height();
    // y positions relative to top of table, as rowPositions is relative, too
    qreal pageBottom = layoutStruct->pageBottom - td->position.y();
    // distance from cell content boundary (top or bottom) to end of table (top or bottom)
    const qreal extraTableHeight = td->padding + td->border + td->cellSpacing // inter cell spacing
                                   + td->margin + td->border + td->padding; // effective table margin

    td->rowPageBreaks.clear();

    // if first row is already taller than the remaining height move the whole table
    // to the next page
    if (td->rowPositions.first() + td->heights.first() + extraTableHeight > pageBottom) {
        layoutStruct->newPage();
        origY = layoutStruct->y;
        td->position.setY(layoutStruct->y);
        pageBottom = layoutStruct->pageBottom - td->position.y();
    }

    qreal offset = 0.0;
    for (int r = 2; r < rows; ++r) {
        if (td->rowPositions[r] + offset > pageBottom) {
            offset += pageBottom - td->rowPositions[r - 1];
            offset += extraTableHeight; // make sure there's enough space for the table margin/border
            layoutStruct->newPage();
            td->rowPositions[r - 1] = layoutStruct->y + extraTableHeight - td->position.y();

            td->rowPageBreaks.append(r - 1);
            pageBottom = layoutStruct->pageBottom - td->position.y();
        }
        td->rowPositions[r] += offset;
    }

    if (rows > 1 && td->rowPositions.last() + td->heights.last() + extraTableHeight > pageBottom) {
        td->rowPageBreaks.append(rows - 1);
        layoutStruct->newPage();
        td->rowPositions.last() = layoutStruct->y + extraTableHeight - td->position.y();
    }

    // calc new total height of table
    td->updateSize();
    layoutStruct->y = origY + td->size.height();
}

void QTextDocumentLayoutPrivate::floatMargins(qreal y, const LayoutStruct *layoutStruct,
                                              qreal *left, qreal *right) const
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


qreal QTextDocumentLayoutPrivate::findY(qreal yFrom, const LayoutStruct *layoutStruct, qreal requiredWidth) const
{
    qreal right, left;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

//     qDebug() << "findY:" << yFrom;
    while (1) {
        floatMargins(yFrom, layoutStruct, &left, &right);
//         qDebug() << "    yFrom=" << yFrom<<"right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        qreal newY = INT_MAX;
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
    d->drawFrame(QPointF(), painter, context, frame);
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

void QTextDocumentLayout::documentChanged(int from, int oldLength, int length)
{
    Q_D(QTextDocumentLayout);
    QSizeF pageSize = document()->pageSize();
    if (pageSize.isNull() || !pageSize.isValid())
        return;


//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(document()->rootFrame(), from, oldLength, length);

    const QSizeF oldSize = documentSize();
    const int oldPageCount = pageCount();

    QTextFrame *root = document()->rootFrame();
    if(data(root)->sizeDirty)
        d->layoutFrame(root, from, from + length);
    data(root)->layoutDirty = false;

    const QSizeF newSize = documentSize();
    if (newSize != oldSize)
        emit documentSizeChanged(newSize);
    const int newPageCount = pageCount();
    if (oldPageCount != newPageCount)
        emit pageCountChanged(newPageCount);

    emit update();
}

int QTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
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

void QTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QSizeF intrinsic = handler.iface->intrinsicSize(document(), posInDocument, format);

    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    if (frame) {
        pos = frame->frameFormat().position();
        data(frame)->sizeDirty = false;
        data(frame)->size = intrinsic.toSize();
    }

    item.setDescent(0);
    QSizeF inlineSize = (pos == QTextFrameFormat::InFlow ? intrinsic : QSizeF(0, 0));
    item.setWidth(inlineSize.width());
    item.setAscent(inlineSize.height());
}

void QTextDocumentLayout::positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    Q_UNUSED(posInDocument);
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
        line = b.layout()->lineAt(b.layout()->lineCount()-1);
//     qDebug() << "layoutObject: line.isValid" << line.isValid() << b.position() << b.length() <<
//         frame->firstPosition() << frame->lastPosition();
    d->positionFloat(frame, line.isValid() ? &line : 0);
}

void QTextDocumentLayout::drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                                           int posInDocument, const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    QRectF r = rect;
    if (frame) {
        QTextFrameData *fd = data(frame);
        if (fd->flow_position != QTextFrameFormat::InFlow) {
            r = QRectF(fd->position, fd->size);
            r.translate(data(frame->parentFrame())->position);
        }
    }
//    qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawInlineObject(p, r, item, posInDocument, format);
}


int QTextDocumentLayout::pageCount() const
{
    return (int)(documentSize().height()/document()->pageSize().height()) + 1;
}

QSizeF QTextDocumentLayout::documentSize() const
{
    return data(document()->rootFrame())->size;
}

// Pull this private function in from qglobal.cpp
Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n);

/* used from QLabel */
void QTextDocumentLayout::adjustSize()
{
    QFont f = document()->defaultFont();
    QFontMetrics fm(f);
    int mw =  fm.width('x') * 80;
    int w = mw;
    QTextDocument *doc = document();
    doc->setPageSize(QSizeF(w, INT_MAX));
    QSizeF size = documentSize();
    if (size.width() != 0) {
        w = qt_int_sqrt((uint)(5 * size.height() * size.width() / 3));
        doc->setPageSize(QSizeF(qMin(w, mw), INT_MAX));

        size = documentSize();
        if (w*3 < 5*size.height()) {
            w = qt_int_sqrt((uint)(2 * size.height() * size.width()));
            doc->setPageSize(QSizeF(qMin(w, mw), INT_MAX));
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

void QTextDocumentLayout::setWordWrapMode(QTextOption::WrapMode mode)
{
    Q_D(QTextDocumentLayout);
    d->wordWrapMode = mode;
}

QTextOption::WrapMode QTextDocumentLayout::wordWrapMode() const
{
    Q_D(const QTextDocumentLayout);
    return d->wordWrapMode;
}

void QTextDocumentLayout::setFixedColumnWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->fixedColumnWidth = width;
}

QRectF QTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    QPointF pos;
    QTextFrame *f = frame;
    while (f) {
        pos += data(f)->position;
        f = f->parentFrame();
    }
    return QRectF(pos, data(frame)->size);
}

QRectF QTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    QTextFrame *frame = document()->frameAt(block.position());
    QPointF offset;
    const int blockPos = block.position();

    while (frame) {
        QTextFrameData *fd = data(frame);
        offset += fd->position;

        if (QTextTable *table = qobject_cast<QTextTable *>(frame)) {
            QTextTableCell cell = table->cellAt(blockPos);
            if (cell.isValid())
                offset += static_cast<QTextTableData *>(fd)->cellPosition(cell.row(), cell.column());
        }

        frame = frame->parentFrame();
    }

    const QTextLayout *layout = block.layout();
    QRectF rect = layout->boundingRect();
    rect.translate(layout->position() + offset);
    return rect;
}

