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

#include "qabstracttextdocumentlayout_p.h"

#include <qdebug.h>

//#define LAYOUT_DEBUG

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
    QTextFrameData() : minimumWidth(0), maximumWidth(INT_MAX), currentLayoutStruct(0) {}

    // relative to parent frame
    QRect boundingRect;

    // contents starts at (margin+border/margin+border)
    int margin;
    int border;
    int padding;
    // contents width includes padding (as we need to treat this on a per cell basis for tables)
    int contentsWidth;
    int contentsHeight;

    int minimumWidth;
    int maximumWidth;

    QTextFrameFormat::Position position;
    QList<QTextFrame *> layoutedFrames;

    LayoutStruct *currentLayoutStruct;

    bool dirty;
};

struct LayoutStruct {
    LayoutStruct() : widthUsed(0), minimumWidth(0), maximumWidth(INT_MAX) {}
    QTextFrame *frame;
    int x_left;
    int x_right;
    int y;
    int widthUsed;
    int minimumWidth;
    int maximumWidth;
};

class QTextTableData : public QTextFrameData
{
public:
    inline QTextTableData() : cellSpacing(0) {}
    int cellSpacing;
    QVector<int> minWidths;
    QVector<int> maxWidths;
    QVector<int> widths;
    QVector<int> heights;
    QVector<int> columnPositions;
    QVector<int> rowPositions;

    inline int cellWidth(int column, int colspan) const
    { return columnPositions.at(column + colspan - 1) + widths.at(column + colspan - 1)
             - columnPositions.at(column) - 2 * padding; }

    inline void calcRowPosition(int row)
    { 
        if (row > 0)
            rowPositions[row] = rowPositions.at(row - 1) + heights.at(row - 1) + border + cellSpacing + border;
    }
};

static QTextFrameData *createData(QTextFrame *f)
{
    QTextFrameData *data;
    if (qt_cast<QTextTable *>(f))
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
                      QTextBlock bl, const QTextLayout::Selection &selection) const;

    enum HitPoint {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextFrame *frame, const QPoint &point, int *position) const;
    HitPoint hitTest(QTextBlock bl, const QPoint &point, int *position) const;

    void relayoutDocument();

    LayoutStruct layoutCell(QTextTable *t, const QTextTableCell &cell, int width);
    void setCellPosition(QTextTableData *td, const QTextTableCell &cell, const QPoint &pos);
    void layoutTable(QTextTable *t, int layoutFrom, int layoutTo);

    void positionFloat(QTextFrame *frame);

    // calls the next one
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo);
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, int frameWidth, int frameHeight);

    void layoutBlock(const QTextBlock block, LayoutStruct *layoutStruct);
    void layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct);


    void floatMargins(LayoutStruct *layoutStruct, int *left, int *right);
    void findY(LayoutStruct *layoutStruct, int requiredWidth);
};

#define d d_func()
#define q q_func()

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QPoint &point, int *position) const
{
    QTextFrameData *fd = data(frame);

    LDEBUG << "checking frame" << frame->firstPosition() << "point=" << point
           << "boundingRect" << fd->boundingRect;
    if (!fd->boundingRect.contains(point) && frame != q->document()->rootFrame()) {
        if (point.y() < fd->boundingRect.top()) {
            *position = frame->firstPosition() - 1;
            LDEBUG << "before pos=" << *position;
            return PointBefore;
        } else {
            *position = frame->lastPosition() + 1;
            LDEBUG << "after pos=" << *position;
            return PointAfter;
        }
    }
    INC_INDENT;

    QPoint p = point - fd->boundingRect.topLeft();

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
    LDEBUG << "inside=" << hit << " pos=" << *position;
    return hit;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextBlock bl, const QPoint &point, int *position) const
{
    const QTextLayout *tl = bl.layout();
    QRect textrect = tl->rect();
    LDEBUG << "    checking block" << bl.position() << "point=" << point
           << "    tlrect" << textrect;
    if (!textrect.contains(point)) {
        *position = bl.position();
        if (point.y() < textrect.top()) {
            LDEBUG << "    before pos=" << *position;
            return PointBefore;
        } else {
            *position += bl.length();
            LDEBUG << "    after pos=" << *position;
            return PointAfter;
        }
    }

    QPoint pos = point - textrect.topLeft();

    QTextBlockFormat blockFormat = bl.blockFormat();

    // ### rtl?

    HitPoint hit = PointInside;
    *position = bl.position();
    int off = 0;
    for (int i = 0; i < tl->numLines(); ++i) {
        QTextLine line = tl->lineAt(i);
        QRect lr = line.rect();
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

    LDEBUG << "    inside=" << hit << " pos=" << *position;
    return hit;
}

// ### could be moved to QTextBlock
int QTextDocumentLayoutPrivate::indent(QTextBlock bl) const
{
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
    if (painter->hasClipping()
        && !fd->boundingRect.intersects(painter->clipRegion().boundingRect()))
        return;
//     LDEBUG << debug_indent << "drawFrame" << frame->firstPosition() << "--" << frame->lastPosition() << "at" << offset;
//     INC_INDENT;

    QTextTable *table = qt_cast<QTextTable *>(frame);
    const QPoint off = offset + fd->boundingRect.topLeft();

    // draw frame decoration
    if (fd->border) {
        painter->save();
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        const int margin = fd->margin + fd->border;
        const int w = fd->boundingRect.width() - 2*margin;
        const int h = fd->boundingRect.height() - 2*margin;
        // left
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin + fd->border, fd->border, h);
        // top
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin, w + 2 * fd->border, fd->border);
        // right
        painter->drawRect(off.x() + fd->margin + fd->border + w, off.y() + fd->margin + fd->border, fd->border, h);
        // bottom
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin + fd->border + h, w + 2 * fd->border, fd->border);
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

                cellRect.moveBy(off);
                if (!cellRect.intersects(painter->clipRegion().boundingRect()))
                    continue;

                if (fd->border) {
                    const QBrush oldBrush = painter->brush();
                    const QPen oldPen = painter->pen();

                    painter->setBrush(Qt::black);
                    painter->setPen(Qt::NoPen);

                    // top border
                    painter->drawRect(cellRect.left() - fd->border, cellRect.top() - fd->border,
                                      cellRect.width() + 2 * fd->border, fd->border);
                    // bottom border
                    painter->drawRect(cellRect.left() - fd->border, cellRect.top() + cellRect.height(),
                                      cellRect.width() + 2 * fd->border, fd->border);
                    // left border
                    painter->drawRect(cellRect.left() - fd->border, cellRect.top(),
                                      fd->border, cellRect.height());
                    // right border
                    painter->drawRect(cellRect.left() + cellRect.width(), cellRect.top(),
                                      fd->border, cellRect.height());

                    painter->setBrush(oldBrush);
                    painter->setPen(oldPen);
                }

                {
                    QColor bgCol = cell.format().tableCellBackgroundColor();
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
//     LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset;
    const QTextLayout *tl = bl.layout();
    QTextBlockFormat blockFormat = bl.blockFormat();

    int cursor = context.showCursor ? context.cursor.position() : -1;

    if (bl.contains(cursor))
        cursor -= bl.position();
    else
        cursor = -1;

    QColor bgCol = blockFormat.backgroundColor();
    if (bgCol.isValid()) {
        QRect r = bl.layout()->rect();
        r.moveBy(offset);
        painter->fillRect(r , bgCol);
    }

    QTextLayout::Selection s;
    int nSel = 0;
    if (context.cursor.hasSelection()) {
        int selStart = context.cursor.selectionStart();
        int selEnd = context.cursor.selectionEnd();
        s.setRange(selStart - bl.position(), selEnd - selStart);
        s.setType(QTextLayout::Highlight);
        ++nSel;
    }

    QTextObject *object = q->document()->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(offset, painter, context, bl, s);

    const_cast<QTextLayout *>(tl)->setPalette(context.palette,
                                              context.textColorFromPalette ? QTextLayout::UseTextColor : QTextLayout::None);

    tl->draw(painter, offset, cursor, &s, nSel, painter->clipRegion().boundingRect());
}


void QTextDocumentLayoutPrivate::drawListItem(const QPoint &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlock bl, const QTextLayout::Selection &selection) const
{
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
    QPoint pos = offset 
                 + bl.layout()->rect().topLeft()
                 + QPoint(firstLine.x(), firstLine.y());

    {
        Qt::Alignment a = blockFormat.alignment();
        if (a == Qt::AlignRight)
            pos.rx() += firstLine.width() - firstLine.textWidth();
        else if (a == Qt::AlignHCenter)
            pos.rx() += (firstLine.width() - firstLine.textWidth()) / 2;
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

    // ### RTL

    QRect r(pos, size);

    r.moveBy( - size.width(),
             (fontMetrics.height() / 2 - size.height() / 2));

    r.moveBy(-fontMetrics.width(" "), 0);

    painter->save();

    if (selection.type() == QTextLayout::Highlight
        && (selection.from() + selection.length() > 0)
        && (selection.from() < 1)) {
        painter->setPen(context.palette.highlightedText());

        painter->fillRect(r, context.palette.highlight());
    } else {
        QColor col = charFormat.textColor();
        painter->setPen(col);
    }

    QBrush brush = context.palette.brush(QPalette::Text);

    switch (style) {
        case QTextListFormat::ListDecimal:
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
            painter->setFont(font);
            painter->drawText(r.left(), pos.y() + fontMetrics.ascent(), itemText);
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

LayoutStruct QTextDocumentLayoutPrivate::layoutCell(QTextTable *t, const QTextTableCell &cell, int width)
{
    QTextTableData *td = static_cast<QTextTableData *>(data(t));

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
    foreach (QTextFrame *frame, t->childFrames())
        if (isFrameInCell(cell, frame)) {
            QTextFrameData *cd = data(frame);
            // ####
            cd->dirty = true;
            layoutFrame(frame, frame->firstPosition(), frame->lastPosition(), width, -1);
            td->layoutedFrames.removeAll(frame);
            layoutStruct.minimumWidth = qMax(layoutStruct.minimumWidth, cd->boundingRect.width());

            if (cd->position != QTextFrameFormat::InFlow)
                floats.append(frame);
        }

    int floatMinWidth = layoutStruct.minimumWidth;

    // make sure layouted frames from other cells don't influence the flow of the current
    // cell
    QList<QTextFrame *> tableLayoutedFrames = td->layoutedFrames;
    td->layoutedFrames.clear();

    layoutFlow(cell.begin(), &layoutStruct);

    td->layoutedFrames += tableLayoutedFrames;

    // floats that are located inside the text (like inline images) aren't taken into account by
    // layoutFlow with regards to the cell height (layoutStruct->y), so for a safety measure we
    // do that here. For example with <td><img align="right" src="..." />blah</td>
    // when the image happens to be higher than the text
    foreach (QTextFrame *frame, floats)
        layoutStruct.y = qMax(layoutStruct.y, data(frame)->boundingRect.height());

    // constraint the maximumWidth by the minimum width of the fixed size floats, to
    // keep them visible
    layoutStruct.maximumWidth = qMax(layoutStruct.maximumWidth, floatMinWidth);

//    qDebug() << "layoutCell done";

    return layoutStruct;
}

void QTextDocumentLayoutPrivate::setCellPosition(QTextTableData *td, const QTextTableCell &cell, const QPoint &pos)
{
    foreach (QTextFrame *frame, td->layoutedFrames)
        if (isFrameInCell(cell, frame)) {
            QTextFrameData *cd = data(frame);
            cd->boundingRect.moveBy(pos);
        }

    for (QTextFrame::Iterator it = cell.begin(); !it.atEnd(); ++it) {
        if (QTextFrame *c = it.currentFrame()) {
            QTextFrameData *cd = data(c);
            cd->boundingRect.moveBy(pos);
        } else {
            QTextBlock bl = it.currentBlock();
            QTextLayout *tl = bl.layout();
            tl->setPosition(tl->position() + pos);
        }
    }
}

void QTextDocumentLayoutPrivate::layoutTable(QTextTable *table, int /*layoutFrom*/, int /*layoutTo*/)
{
    const int rows = table->rows();
    const int columns = table->columns();
    QTextTableData *td = static_cast<QTextTableData *>(data(table));

    const QTextTableFormat fmt = table->format();

    const QList<int> constraints = fmt.tableColumnConstraintTypes();
    const QList<int> constraintValues = fmt.tableColumnConstraintValues();
    Q_ASSERT(constraints.count() == columns);
    Q_ASSERT(constraintValues.count() == columns);

    const int cellSpacing = td->cellSpacing = fmt.cellSpacing();
    // ### fix padding
    const int margin = td->margin + td->border;

    int totalWidth = td->contentsWidth;
    // two (vertical) borders per cell per column
    totalWidth -= columns * 2 * td->border;
    // inter-cell spacing
    totalWidth -= (columns - 1) * cellSpacing;
    // cell spacing at the left and right hand side
    totalWidth -= 2 * cellSpacing;

    td->widths.resize(columns);
    td->widths.fill(0);

    td->minWidths.resize(columns);
    td->minWidths.fill(0);

    td->maxWidths.resize(columns);
    td->maxWidths.fill(INT_MAX);

    // set fixed values, figure out total percentages used and number of
    // variable length cells
    int totalPercentage = 0;
    int variableCols = 0;
    for (int i = 0; i < columns; ++i)
        if (constraints.at(i) == QTextTableFormat::FixedLength) {
            td->widths[i] = constraintValues.at(i);
            totalWidth -= constraintValues.at(i);
        } else if (constraints.at(i) == QTextTableFormat::PercentageLength) {
            totalPercentage += constraintValues.at(i);
        } else if (constraints.at(i) == QTextTableFormat::VariableLength) {
            variableCols++;
        }

    // set percentage values
    {
        const int totalPercentagedWidth = totalWidth * totalPercentage / 100;
        for (int i = 0; i < columns; ++i)
            if (constraints.at(i) == QTextTableFormat::PercentageLength)
                td->widths[i] = totalPercentagedWidth * constraintValues.at(i) / totalPercentage;

        totalWidth -= totalPercentagedWidth;
    }

    // for variable columsn set the minimum sizes first and distribute the remaining space
    if (variableCols > 0) {
        // minWidths array is filled with zero at this point

        int sharedWidth = totalWidth / variableCols;
        for (int i = 0; i < columns; ++i)
            if (constraints.at(i) == QTextTableFormat::VariableLength) {
                for (int row = 0; row < rows; ++row) {
                    const QTextTableCell cell = table->cellAt(row, i);
                    const int cspan = cell.columnSpan();

                    if (cspan > 1 && i != cell.column())
                        continue;

                    LayoutStruct layoutStruct = layoutCell(table, cell, sharedWidth - 2 * td->padding);

                    int widthToDistribute = layoutStruct.minimumWidth;
                    for (int n = 0; n < cspan; ++n) {
                        const int col = i + n;
                        int w = widthToDistribute / (cspan - n);
                        td->minWidths[col] = qMax(td->minWidths.at(col), w);
                        widthToDistribute -= td->minWidths.at(col);
                        if (widthToDistribute <= 0)
                            break;
                    }
                    // ### colspans
                    td->maxWidths[i] = qMin(td->maxWidths.at(i), layoutStruct.maximumWidth);
                }
                td->widths[i] = td->minWidths.at(i);
                totalWidth -= td->minWidths.at(i);
            }

        if (totalWidth > 0) {
            QVarLengthArray<int> anySizeColumns;
            QVarLengthArray<int> columnsWithProperMaxSize;
            for (int i = 0; i < columns; ++i)
                if (constraints.at(i) == QTextTableFormat::VariableLength) {
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

            if (totalWidth > 0 && !anySizeColumns.isEmpty()) {
                const int widthPerAnySizedCol = totalWidth / anySizeColumns.count();
                for (int k = 0; k < anySizeColumns.count(); ++k) {
                    const int col = anySizeColumns[k];
                    td->widths[col] += widthPerAnySizedCol;
                }
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
            LayoutStruct layoutStruct = layoutCell(table, cell, width);

            td->heights[r] = qMax(td->heights.at(r), layoutStruct.y + 2*td->padding);
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
                LayoutStruct layoutStruct = layoutCell(table, cell, width);

                // the last row gets all the remaining space
                int heightToDistribute = layoutStruct.y + 2*td->padding;
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
        const int y = td->rowPositions.at(r) + td->padding;
        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            const int rspan = cell.rowSpan();
            const int cspan = cell.columnSpan();

            if (cspan > 1 && cell.column() != c)
                continue;

            if (rspan > 1 && cell.row() != r)
                    continue;

            const int x = td->columnPositions.at(c) + td->padding;
            setCellPosition(td, cell, QPoint(x, y));
        }
    }

    // - margin to compensate the + margin in columnPositions[0]
//    td->contentsWidth = qMax(td->contentsWidth,
//                             td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin);
    td->contentsWidth = td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin;
    int height = td->contentsHeight == -1
                 ? td->rowPositions.last() + td->heights.last() + td->padding + td->border + cellSpacing + margin
                 : td->contentsHeight + 2*margin;
    td->boundingRect = QRect(0, 0, td->contentsWidth + 2*margin, height);
    td->dirty = false;
}

void QTextDocumentLayoutPrivate::positionFloat(QTextFrame *frame)
{
    QTextFrame *parent = frame->parentFrame();
    QTextFrameData *pd = data(parent);
    Q_ASSERT(pd && pd->currentLayoutStruct);

    if (pd->layoutedFrames.contains(frame))
        return;

    QTextFrameData *fd = data(frame);

    findY(pd->currentLayoutStruct, fd->boundingRect.width());

    int left, right;
    d->floatMargins(pd->currentLayoutStruct, &left, &right);

    if (fd->position == QTextFrameFormat::FloatLeft)
        fd->boundingRect.moveTopLeft(QPoint(left, pd->currentLayoutStruct->y));
    else
        fd->boundingRect.moveTopRight(QPoint(right, pd->currentLayoutStruct->y));

    pd->layoutedFrames.append(frame);
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo)
{
    Q_ASSERT(data(f)->dirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->format();

    QTextFrame *parent = f->parentFrame();
    const QTextFrameData *pd = parent ? data(parent) : 0;

    int width = fformat.width();
    if (width == -1) {
        QTextTable *t = qt_cast<QTextTable *>(parent);
        Q_ASSERT(t == 0);
        /*
        if (t) {
            const QTextTableData *td = static_cast<const QTextTableData *>(pd);
            QTextTableCell cell = t->cellAt(f->firstPosition());
            int c = cell.column();
            int cspan = cell.columnSpan();
            width = td->columnPositions.at(c + cspan - 1) + td->widths.at(c + cspan - 1)
                    - td->columnPositions.at(c) - 2*td->padding;
        } else {
        */
            width = pd ? pd->contentsWidth : pageSize.width();
        //}
    }

    int height = fformat.height();
    if (height == -1)
        height = pd ? pd->contentsHeight : -1;

    layoutFrame(f, layoutFrom, layoutTo, width, height);
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, int frameWidth, int frameHeight)
{
    Q_ASSERT(data(f)->dirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->format();
    QTextFrameData *fd = data(f);

    // breaks yet nonexistant incremental layouting but necessary to catch float deletions
    fd->layoutedFrames.clear();

    {
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

        fd->position = fformat.position();
    }

    int startPos = f->firstPosition();
    int endPos = f->lastPosition();
    if (startPos > endPos) {
        // inline image
        QTextCharFormat format = q->format(startPos - 1);
        QTextObjectInterface *iface = q->handlerForObject(format.objectType());
        if (iface)
            fd->boundingRect = QRect(QPoint(0, 0), iface->intrinsicSize(q->document(), format));
        fd->dirty = false;
        return;
    }

    bool fullLayout = false;

    if (QTextTable *table = qt_cast<QTextTable *>(f)) {
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
        if (cd->dirty) {
            QRect oldBr = cd->boundingRect;
            layoutFrame(c, layoutFrom, layoutTo);
            // removes all floats that need repositioning
            if (0 /*!cd->position == QTextFrameFormat::Absolute*/) {
                if (oldBr != cd->boundingRect)
                    fullLayout = true;
            } else {
                fd->layoutedFrames.removeAll(c);
            }
            newContentsWidth = qMax(newContentsWidth, cd->boundingRect.width());
        }
    }
    fd->contentsWidth = newContentsWidth;

    // #### for now
    fullLayout = true;

    int margin = fd->margin + fd->border;
    LayoutStruct layoutStruct;
    layoutStruct.frame = f;
    layoutStruct.x_left = margin + fd->padding;
    layoutStruct.x_right = margin + fd->contentsWidth - fd->padding;
    layoutStruct.y = margin + fd->padding; // #### fix for !fullLayout
    layoutStruct.widthUsed = 0;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = INT_MAX;

    // layout regular contents and position non absolute positioned child frames
    if (!fullLayout) {
        startPos = qMax(startPos, layoutFrom);
        endPos = qMin(endPos, layoutTo);
    }

#if 0
    QTextFrame *f_start = q->frameAt(startPos);
    // if the changed region started within a subframe, we can start layouting after the end of the subframe
    if (f_start != f) {
        while (f_start->parent() != f)
            f_start = f_start->parent();
        Q_ASSERT(f_start);
        startPos = f_start->end() + 1;
    }
    QTextFrame *f_end = q->frameAt(endPos);
    // if the changed region ends within a subframe, we can stop layouting before the start of the subframe
    if (f_end != f) {
        while (f_end->parent() != f)
            f_end = f_end->parent();
        Q_ASSERT(f_end);
        endPos = f_end->start() - 1;
    }

    Q_ASSERT(q->frameAt(startPos) == f);
    Q_ASSERT(q->frameAt(endPos) == f);
#endif

    QTextFrame::Iterator it = f->begin();

    layoutFlow(it, &layoutStruct);

    fd->contentsWidth = qMax(fd->contentsWidth, layoutStruct.widthUsed);
    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    int height = fd->contentsHeight == -1
                 ? layoutStruct.y + margin + fd->padding
                 : fd->contentsHeight + 2*margin;
    fd->boundingRect = QRect(0, 0, fd->contentsWidth + 2*margin, height);
    fd->dirty = false;
}

void QTextDocumentLayoutPrivate::layoutFlow(QTextFrame::Iterator it, LayoutStruct *layoutStruct)
{
    QTextFrameData *fd = data(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    for ( ; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();

        if (c && !fd->layoutedFrames.contains(c)) {
            QTextFrameData *cd = data(c);
            if (cd->position == QTextFrameFormat::InFlow) {
                cd->boundingRect.moveTopLeft(QPoint(layoutStruct->x_left, layoutStruct->y));
                layoutStruct->y += cd->boundingRect.height();
            } else {
                positionFloat(c);
            }
        } else {
            layoutBlock(it.currentBlock(), layoutStruct);
        }
    }

    fd->currentLayoutStruct = 0;
}

void QTextDocumentLayoutPrivate::layoutBlock(QTextBlock bl, LayoutStruct *layoutStruct)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    int flags = d->blockTextFlags | blockFormat.alignment();
    if (blockFormat.nonBreakableLines())
        // QTextLine::layout still obeys QChar::LineSeparator in that mode
        flags |= Qt::TextSingleLine;
    tl->setTextFlags(flags);

//    qDebug() << "layoutBlock; width" << layoutStruct->x_right - layoutStruct->x_left << "(maxWidth is btw" << tl->maximumWidth() << ")";
//    tl->useDesignMetrics(true);
//     tl->enableKerning(true);
    tl->clearLines();

    layoutStruct->y += blockFormat.topMargin();
    const int cy = layoutStruct->y;

    //QTextFrameData *fd = data(layoutStruct->frame);

    const int indent = this->indent(bl);
    const int l = layoutStruct->x_left + blockFormat.leftMargin() + indent;
    const int r = layoutStruct->x_right - blockFormat.rightMargin();

    tl->setPosition(QPoint(layoutStruct->x_left, cy));

    while (1) {
        QTextLine line = tl->createLine();
        if (!line.isValid())
            break;

        int left, right;
        floatMargins(layoutStruct, &left, &right);

        left = qMax(left, l);
        right = qMin(right, r);
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;

        if (d->fixedColumnWidth != -1)
            line.layout(d->fixedColumnWidth, QTextLine::UnitIsGlyphs);
        else
            line.layout(right - left);

//        qDebug() << "layoutBlock; layouting line with width" << right - left << "->textWidth" << line.textWidth();
        floatMargins(layoutStruct, &left, &right);
        left = qMax(left, l);
        right = qMin(right, r);

        if (d->fixedColumnWidth == -1 && line.textWidth() > right-left) {
            // float has been added in the meantime, redo
            line.layout(right-left);
//            qDebug() << "    redo: left=" << left << " right=" << right;
            if (line.textWidth() > right-left) {
                // lines min width more than what we have
                findY(layoutStruct, line.textWidth());
                floatMargins(layoutStruct, &left, &right);
                line.layout(line.textWidth());
            }
        }

        line.setPosition(QPoint(left - layoutStruct->x_left, layoutStruct->y - cy));
        layoutStruct->y += line.ascent() + line.descent() + 1;
        layoutStruct->widthUsed = qMax(layoutStruct->widthUsed, left - layoutStruct->x_left + line.textWidth());
    }
    layoutStruct->y += blockFormat.bottomMargin();
    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, tl->minimumWidth() + blockFormat.leftMargin() + indent);

    const int maxW = tl->maximumWidth() + blockFormat.leftMargin() + indent;
    if (maxW > 0)
        layoutStruct->maximumWidth = qMin(layoutStruct->maximumWidth, maxW);
}

void QTextDocumentLayoutPrivate::floatMargins(LayoutStruct *layoutStruct, int *left, int *right)
{
    QTextFrameData *fd = data(layoutStruct->frame);
    *left = layoutStruct->x_left;
    *right = layoutStruct->x_right;
    foreach (QTextFrame *f, fd->layoutedFrames) {
        QTextFrameData *fd = data(f);
        QRect r = fd->boundingRect;
        if (r.y() <= layoutStruct->y && r.bottom() > layoutStruct->y) {
            QTextFrameFormat::Position pos = fd->position;
            if (pos == QTextFrameFormat::FloatLeft)
                *left = qMax(*left, r.right());
            else
                *right = qMin(*right, r.left());
        }
    }
}


void QTextDocumentLayoutPrivate::findY(LayoutStruct *layoutStruct, int requiredWidth)
{
    int right, left;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

//     qDebug() << "findY:";
    while (1) {
        floatMargins(layoutStruct, &left, &right);
//         qDebug() << "    right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        int newY = INT_MAX;
        foreach (QTextFrame *f, data(layoutStruct->frame)->layoutedFrames) {
            QTextFrameData *fd = data(f);
            QRect r = fd->boundingRect;
            if (r.y() <= layoutStruct->y && r.bottom() > layoutStruct->y)
                newY = qMin(newY, r.bottom());
        }
        if (newY == INT_MAX)
            break;
        layoutStruct->y = newY;
    }
}


QTextDocumentLayout::QTextDocumentLayout(QTextDocument *doc)
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate, doc)
{
    d->blockTextFlags = Qt::TextIncludeTrailingSpaces|Qt::TextWordWrap;

    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QTextFrame *frame = document()->rootFrame();
    d->drawFrame(QPoint(), painter, context, frame);
}

static void markFrames(QTextFrame *current, int start, int end)
{
    if (current->firstPosition() > end || current->lastPosition() < start)
        return;
    data(current)->dirty = true;
//     qDebug("    marking frame (%d--%d) as dirty", current->firstPosition(), current->lastPosition());
    QList<QTextFrame *> children = current->childFrames();
    for (int i = 0; i < children.size(); ++i)
        markFrames(children.at(i), start, end);
}

void QTextDocumentLayout::documentChange(int from, int oldLength, int length)
{
    Q_UNUSED(oldLength);

    const QSize oldSize = sizeUsed();

//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(document()->rootFrame(), from, from + length);

    d->layoutFrame(document()->rootFrame(), from, from + length);

    const QSize newSize = sizeUsed();
    if (newSize != oldSize)
        emit usedSizeChanged();

    emit update();
}

int QTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextFrame *f = document()->rootFrame();
    int position = 0;
    QTextDocumentLayoutPrivate::HitPoint p = d->hitTest(f, point, &position);
    if (accuracy == QText::ExactHit && p < QTextDocumentLayoutPrivate::PointExact)
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
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;
    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qt_cast<QTextFrame *>(document()->objectForFormat(f));
    if (frame)
        pos = frame->format().position();

    item.setDescent(0);
    QSize inlineSize = (pos == QTextFrameFormat::InFlow ? handler.iface->intrinsicSize(document(), format) : QSize(0, 0));
    item.setWidth(inlineSize.width());
    item.setAscent(inlineSize.height());
}

void QTextDocumentLayout::layoutObject(QTextInlineObject item, const QTextFormat &format)
{
    if (item.width())
        return;

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QTextFrame *frame = qt_cast<QTextFrame *>(document()->objectForFormat(f));
    if (!frame)
        return;

    d->positionFloat(frame);
}

void QTextDocumentLayout::drawObject(QPainter *p, const QRect &rect, QTextInlineObject item,
                                     const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrame *frame = qt_cast<QTextFrame *>(document()->objectForFormat(f));
    QRect r = rect;
    if (frame) {
        QTextFrameData *fd = data(frame);
        if (fd->position != QTextFrameFormat::InFlow) {
            r = fd->boundingRect;
            r.moveBy(data(frame->parentFrame())->boundingRect.topLeft());
        }
    }
//    qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawObject(p, r, item, format, selType);
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

QSize QTextDocumentLayout::sizeUsed() const
{
    return data(q->document()->rootFrame())->boundingRect.size();
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
    d->blockTextFlags = flags;
}

int QTextDocumentLayout::blockTextFlags() const
{
    return d->blockTextFlags;
}

void QTextDocumentLayout::setFixedColumnWidth(int width)
{
    d->fixedColumnWidth = width;
}

QRect QTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    return data(frame)->boundingRect;
}

