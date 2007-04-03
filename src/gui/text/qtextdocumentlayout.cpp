/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextdocumentlayout_p.h"
#include "qtextdocument_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include "qtextengine_p.h"
#include "private/qcssutil_p.h"

#include "qabstracttextdocumentlayout_p.h"
#include "qcssparser_p.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qdebug.h>
#include <qvarlengtharray.h>
#include <limits.h>
#include <qstyle.h>
#include <qbasictimer.h>

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

struct QLayoutStruct;

class QTextFrameData : public QTextFrameLayoutData
{
public:
    QTextFrameData();

    // relative to parent frame
    QFixedPoint position;
    QFixedSize size;

    // contents starts at (margin+border/margin+border)
    QFixed topMargin;
    QFixed bottomMargin;
    QFixed leftMargin;
    QFixed rightMargin;
    QFixed border;
    QFixed padding;
    // contents width includes padding (as we need to treat this on a per cell basis for tables)
    QFixed contentsWidth;
    QFixed contentsHeight;

    QFixed minimumWidth;
    QFixed maximumWidth;

    QLayoutStruct *currentLayoutStruct;

    bool sizeDirty;
    bool layoutDirty;

    QList<QPointer<QTextFrame> > floats;
};

QTextFrameData::QTextFrameData()
    : maximumWidth(QFIXED_MAX),
      currentLayoutStruct(0), sizeDirty(true), layoutDirty(true)
{
}

struct QLayoutStruct {
    QLayoutStruct() : maximumWidth(QFIXED_MAX), fullLayout(false)
    {}
    QTextFrame *frame;
    QFixed x_left;
    QFixed x_right;
    QFixed y;
    QFixed contentsWidth;
    QFixed minimumWidth;
    QFixed maximumWidth;
    bool fullLayout;
    QList<QTextFrame *> pendingFloats;
    QFixed pageHeight;
    QFixed pageBottom;
    QFixed pageTopMargin;
    QFixed pageBottomMargin;
    QRectF updateRect;

    inline void newPage()
    { if (pageHeight == QFIXED_MAX) return; pageBottom += pageHeight; y = pageBottom - pageHeight + pageBottomMargin + pageTopMargin; }
};

class QTextTableData : public QTextFrameData
{
public:
    QFixed cellSpacing, cellPadding;
    QVector<QFixed> minWidths;
    QVector<QFixed> maxWidths;
    QVector<QFixed> widths;
    QVector<QFixed> heights;
    QVector<QFixed> columnPositions;
    QVector<QFixed> rowPositions;

    QVector<QFixed> cellVerticalOffsets;

    // maps from cell index (row + col * rowCount) to child frames belonging to
    // the specific cell
    QMultiHash<int, QTextFrame *> childFrameMap;

    inline QFixed cellWidth(int column, int colspan) const
    { return columnPositions.at(column + colspan - 1) + widths.at(column + colspan - 1)
             - columnPositions.at(column) - 2 * cellPadding; }

    inline void calcRowPosition(int row)
    {
        if (row > 0)
            rowPositions[row] = rowPositions.at(row - 1) + heights.at(row - 1) + border + cellSpacing + border;
    }

    QRectF cellRect(const QTextTableCell &cell) const;

    inline QFixedPoint cellPosition(int row, int col) const
    { return QFixedPoint(columnPositions.at(col) + cellPadding, rowPositions.at(row) + cellPadding + cellVerticalOffsets.at(col + row * widths.size())); }
    inline QFixedPoint cellPosition(const QTextTableCell &cell) const
    { return cellPosition(cell.row(), cell.column()); }

    void updateTableSize();
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

static bool isFrameFromInlineObject(QTextFrame *f)
{
    return f->firstPosition() > f->lastPosition();
}

void QTextTableData::updateTableSize()
{
    const QFixed effectiveTopMargin = this->topMargin + border + padding;
    const QFixed effectiveBottomMargin = this->bottomMargin + border + padding;
    const QFixed effectiveLeftMargin = this->leftMargin + border + padding;
    const QFixed effectiveRightMargin = this->rightMargin + border + padding;
    size.height = contentsHeight == -1
                   ? rowPositions.last() + heights.last() + padding + border + cellSpacing + effectiveBottomMargin
                   : effectiveTopMargin + contentsHeight + effectiveBottomMargin;
    size.width = effectiveLeftMargin + contentsWidth + effectiveRightMargin;
}

QRectF QTextTableData::cellRect(const QTextTableCell &cell) const
{
    const int row = cell.row();
    const int rowSpan = cell.rowSpan();
    const int column = cell.column();
    const int colSpan = cell.columnSpan();

    return QRectF(columnPositions.at(column).toReal(),
                  rowPositions.at(row).toReal(),
                  (columnPositions.at(column + colSpan - 1) + widths.at(column + colSpan - 1) - columnPositions.at(column)).toReal(),
                  (rowPositions.at(row + rowSpan - 1) + heights.at(row + rowSpan - 1) - rowPositions.at(row)).toReal());
}

static inline bool isEmptyBlockBeforeTable(const QTextBlock &block, const QTextFrame::Iterator &nextIt)
{
    QTextBlockFormat format = block.blockFormat();
    return !nextIt.atEnd()
           && qobject_cast<QTextTable *>(nextIt.currentFrame())
           && block.isValid()
           && block.length() == 1
           && !format.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)
           && !format.hasProperty(QTextFormat::BackgroundBrush)
           && nextIt.currentFrame()->firstPosition() == block.position() + 1
           ;
}

static inline bool isEmptyBlockBeforeTable(QTextFrame::Iterator it)
{
    QTextFrame::Iterator next = it; ++next;
    return it.currentFrame() == 0
           && isEmptyBlockBeforeTable(it.currentBlock(), next);
}

static inline bool isEmptyBlockAfterTable(const QTextBlock &block, const QTextFrame *lastFrame)
{
    return qobject_cast<const QTextTable *>(lastFrame)
           && block.isValid()
           && block.length() == 1
           && lastFrame->lastPosition() == block.position() - 1
           ;
}

static inline bool isLineSeparatorBlockAfterTable(const QTextBlock &block, const QTextFrame *lastFrame)
{
    return qobject_cast<const QTextTable *>(lastFrame)
           && block.isValid()
           && block.length() > 1
           && block.text().at(0) == QChar::LineSeparator
           && lastFrame->lastPosition() == block.position() - 1
           ;
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

struct QCheckPoint
{
    QFixed y;
    int positionInFrame;
    QFixed minimumWidth;
    QFixed maximumWidth;
    QFixed contentsWidth;
};
Q_DECLARE_TYPEINFO(QCheckPoint, Q_PRIMITIVE_TYPE);

static bool operator<(const QCheckPoint &checkPoint, QFixed y)
{
    return checkPoint.y < y;
}

static bool operator<(const QCheckPoint &checkPoint, int pos)
{
    return checkPoint.positionInFrame < pos;
}

static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, QRectF gradientRect = QRectF())
{
    p->save();
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (gradientRect.isNull())
            gradientRect = rect;
        QTransform m;
        m.translate(gradientRect.left(), gradientRect.top());
        m.scale(gradientRect.width(), gradientRect.height());
        brush.setTransform(m);
    } else {
        p->setBrushOrigin(rect.topLeft());
    }
    p->fillRect(rect, brush);
    p->restore();
}

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout)
public:
    QTextDocumentLayoutPrivate();

    QTextOption::WrapMode wordWrapMode;
#ifdef LAYOUT_DEBUG
    mutable QString debug_indent;
#endif

    int fixedColumnWidth;
    int cursorWidth;

    QSizeF lastReportedSize;

    mutable int currentLazyLayoutPosition;
    mutable int lazyLayoutStepSize;
    QBasicTimer layoutTimer;
    mutable QBasicTimer sizeChangedTimer;
    uint showLayoutProgress : 1;
    uint insideDocumentChange : 1;

    int lastPageCount;
    qreal idealWidth;

    qreal indent(QTextBlock bl) const;

    void drawFrame(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextFrame *f) const;
    void drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                  QTextFrame::Iterator it, const QList<QTextFrame *> &floats, QTextBlock *cursorBlockNeedingRepaint) const;
    void drawBlock(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextBlock bl) const;
    void drawListItem(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                      QTextBlock bl, const QTextCharFormat *selectionFormat) const;
    void drawTableCell(const QRectF &cellRect, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &cell_context,
                       QTextTable *table, QTextTableData *td, int r, int c,
                       QTextBlock *cursorBlockNeedingRepaint, QPointF *cursorBlockOffset) const;
    void drawBorder(QPainter *painter, const QRectF &rect, qreal border, const QBrush &brush, QTextFrameFormat::BorderStyle style) const;
    void drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect) const;

    enum HitPoint {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextFrame *frame, const QFixedPoint &point, int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const;
    HitPoint hitTest(QTextFrame::Iterator it, HitPoint hit, const QFixedPoint &p,
                     int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const;
    HitPoint hitTest(QTextTable *table, const QFixedPoint &point, int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const;
    HitPoint hitTest(QTextBlock bl, const QFixedPoint &point, int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const;

    QLayoutStruct layoutCell(QTextTable *t, const QTextTableCell &cell, QFixed width,
                            int layoutFrom, int layoutTo, QTextTableData *tableData, bool withPageBreaks = false);
    void setCellPosition(QTextTable *t, const QTextTableCell &cell, const QPointF &pos);
    QRectF layoutTable(QTextTable *t, int layoutFrom, int layoutTo);

    void positionFloat(QTextFrame *frame, QTextLine *currentLine = 0);

    // calls the next one
    QRectF layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, QFixed parentY = 0);
    QRectF layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, QFixed frameWidth, QFixed frameHeight, QFixed parentY = 0);

    void layoutBlock(const QTextBlock &bl, QLayoutStruct *layoutStruct, int layoutFrom, int layoutTo,
                     const QTextBlock &previousBlock);
    void layoutFlow(QTextFrame::Iterator it, QLayoutStruct *layoutStruct, int layoutFrom, int layoutTo, QFixed parentY, QFixed width = 0);
    void pageBreakInsideTable(QTextTable *table, QLayoutStruct *layoutStruct);


    void floatMargins(QFixed y, const QLayoutStruct *layoutStruct, QFixed *left, QFixed *right) const;
    QFixed findY(QFixed yFrom, const QLayoutStruct *layoutStruct, QFixed requiredWidth) const;

    QVector<QCheckPoint> checkPoints;

    QTextFrame::Iterator frameIteratorForYPosition(QFixed y) const;
    QTextFrame::Iterator frameIteratorForTextPosition(int position) const;

    void ensureLayouted(QFixed y) const;
    void ensureLayoutedByPosition(int position) const;
    inline void ensureLayoutFinished() const
    { ensureLayoutedByPosition(INT_MAX); }
    void layoutStep() const;

    QRectF frameBoundingRectInternal(QTextFrame *frame) const;

    qreal scaleToDevice(qreal value) const;
    QFixed scaleToDevice(QFixed value) const;
};

QTextDocumentLayoutPrivate::QTextDocumentLayoutPrivate()
    : fixedColumnWidth(-1),
      cursorWidth(1),
      currentLazyLayoutPosition(-1),
      lazyLayoutStepSize(1000),
      lastPageCount(-1)
{
    showLayoutProgress = true;
    insideDocumentChange = false;
    idealWidth = 0;
}

QTextFrame::Iterator QTextDocumentLayoutPrivate::frameIteratorForYPosition(QFixed y) const
{
    Q_Q(const QTextDocumentLayout);

    const QTextDocumentPrivate *doc = q->document()->docHandle();
    QTextFrame *rootFrame = doc->rootFrame();

    if (checkPoints.isEmpty()
        || y < 0 || y > data(rootFrame)->size.height)
        return rootFrame->begin();

    QVector<QCheckPoint>::ConstIterator checkPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), y);
    if (checkPoint == checkPoints.end())
        return rootFrame->begin();

    if (checkPoint != checkPoints.begin())
        --checkPoint;

    const int position = rootFrame->firstPosition() + checkPoint->positionInFrame;
    return frameIteratorForTextPosition(position);
}

QTextFrame::Iterator QTextDocumentLayoutPrivate::frameIteratorForTextPosition(int position) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextDocumentPrivate *doc = q->document()->docHandle();
    QTextFrame *rootFrame = doc->rootFrame();

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
QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QFixedPoint &point, int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    // #########
    if (fd->layoutDirty)
        return PointAfter;
    Q_ASSERT(!fd->layoutDirty);
    Q_ASSERT(!fd->sizeDirty);
    const QFixedPoint relativePoint(point.x - fd->position.x, point.y - fd->position.y);

    QTextFrame *rootFrame = q->document()->rootFrame();

//     LDEBUG << "checking frame" << frame->firstPosition() << "point=" << point
//            << "position" << fd->position << "size" << fd->size;
    if (frame != rootFrame) {
        if (relativePoint.y < 0 || relativePoint.x < 0) {
            *position = frame->firstPosition() - 1;
//             LDEBUG << "before pos=" << *position;
            return PointBefore;
        } else if (relativePoint.y > fd->size.height || relativePoint.x > fd->size.width) {
            *position = frame->lastPosition() + 1;
//             LDEBUG << "after pos=" << *position;
            return PointAfter;
        }
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(frame))
        return hitTest(table, relativePoint, position, l, accuracy);

    QTextFrame::Iterator it = frame->begin();

    if (frame == rootFrame) {
        it = frameIteratorForYPosition(relativePoint.y);

        Q_ASSERT(it.parentFrame() == frame);
    }

    if (it.currentFrame())
        *position = it.currentFrame()->firstPosition();
    else
        *position = it.currentBlock().position();

    return hitTest(it, PointBefore, relativePoint, position, l, accuracy);
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame::Iterator it, HitPoint hit, const QFixedPoint &p,
                                    int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const
{
    INC_INDENT;

    for (; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();
        HitPoint hp;
        int pos = -1;
        if (c) {
            hp = hitTest(c, p, &pos, l, accuracy);
        } else {
            hp = hitTest(it.currentBlock(), p, &pos, l, accuracy);
        }
        if (hp >= PointInside) {
            if (isEmptyBlockBeforeTable(it))
                continue;
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
QTextDocumentLayoutPrivate::hitTest(QTextTable *table, const QFixedPoint &point,
                                    int *position, QTextLayout **l, Qt::HitTestAccuracy accuracy) const
{
    QTextTableData *td = static_cast<QTextTableData *>(data(table));

    QVector<QFixed>::ConstIterator rowIt = qLowerBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), point.y);
    if (rowIt == td->rowPositions.constEnd()) {
        rowIt = td->rowPositions.constEnd() - 1;
    } else if (rowIt != td->rowPositions.constBegin()) {
        --rowIt;
    }

    QVector<QFixed>::ConstIterator colIt = qLowerBound(td->columnPositions.constBegin(), td->columnPositions.constEnd(), point.x);
    if (colIt == td->columnPositions.constEnd()) {
        colIt = td->columnPositions.constEnd() - 1;
    } else if (colIt != td->columnPositions.constBegin()) {
        --colIt;
    }

    QTextTableCell cell = table->cellAt(rowIt - td->rowPositions.constBegin(),
                                        colIt - td->columnPositions.constBegin());
    if (!cell.isValid())
        return PointBefore;

    *position = cell.firstPosition();

    HitPoint hp = hitTest(cell.begin(), PointInside, point - td->cellPosition(cell), position, l, accuracy);

    if (hp == PointExact)
        return hp;
    if (hp == PointAfter)
        *position = cell.lastPosition();
    return PointInside;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextBlock bl, const QFixedPoint &point, int *position, QTextLayout **l,
                                    Qt::HitTestAccuracy accuracy) const
{
    QTextLayout *tl = bl.layout();
    QRectF textrect = tl->boundingRect();
    textrect.translate(tl->position());
//     LDEBUG << "    checking block" << bl.position() << "point=" << point
//            << "    tlrect" << textrect;
    *position = bl.position();
    if (point.y.toReal() < textrect.top()) {
//             LDEBUG << "    before pos=" << *position;
        return PointBefore;
    } else if (point.y.toReal() > textrect.bottom()) {
        *position += bl.length();
//             LDEBUG << "    after pos=" << *position;
        return PointAfter;
    }

    QPointF pos = point.toPointF() - tl->position();

    // ### rtl?

    HitPoint hit = PointInside;
    *l = tl;
    int off = 0;
    for (int i = 0; i < tl->lineCount(); ++i) {
        QTextLine line = tl->lineAt(i);
        const QRectF lr = line.naturalTextRect();
        if (lr.top() > pos.y()) {
            off = qMin(off, line.textStart());
        } else if (lr.bottom() <= pos.y()) {
            off = qMax(off, line.textStart() + line.textLength());
        } else {
            if (lr.left() <= pos.x() && lr.right() >= pos.x())
                hit = PointExact;
            // when trying to hit an anchor we want it to hit not only in the left
            // half
            if (accuracy == Qt::ExactHit)
                off = line.xToCursor(pos.x(), QTextLine::CursorOnCharacter);
            else
                off = line.xToCursor(pos.x(), QTextLine::CursorBetweenCharacters);
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
        scale = qreal(q->paintDevice()->logicalDpiY()) / qreal(qt_defaultDpi());
    }

    return indent * TextIndentValue * scale;
}

void QTextDocumentLayoutPrivate::drawBorder(QPainter *painter, const QRectF &rect, qreal border, const QBrush &brush, QTextFrameFormat::BorderStyle style) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(q->document()->rootFrame());

    const qreal pageHeight = q->document()->pageSize().height();
    const int topPage = static_cast<int>(rect.top() / pageHeight);
    const int bottomPage = static_cast<int>((rect.bottom() + border) / pageHeight);

    if (topPage != bottomPage) {
        const qreal pageWidth = q->document()->pageSize().width();
        const qreal pageTopMargin = fd->topMargin.toReal();
        const qreal pageBottomMargin = fd->bottomMargin.toReal();
        const qreal clipHeight = pageHeight - pageTopMargin - pageBottomMargin;

        QRegion clipRegion;
        for (int i = topPage; i <= bottomPage; ++i) {
            const qreal clipTop = i * pageHeight + pageTopMargin;
            clipRegion |= QRectF(0, clipTop, pageWidth, clipHeight).toRect();
        }
        painter->save();
        painter->setClipRegion(clipRegion, Qt::IntersectClip);
    }

    QCss::BorderStyle cssStyle = static_cast<QCss::BorderStyle>(style + 1);

    qDrawEdge(painter, rect.left(), rect.top(), rect.left() + border, rect.bottom() + border, 0, 0, QCss::LeftEdge, cssStyle, brush);
    qDrawEdge(painter, rect.left() + border, rect.top(), rect.right() + border, rect.top() + border, 0, 0, QCss::TopEdge, cssStyle, brush);
    qDrawEdge(painter, rect.right(), rect.top() + border, rect.right() + border, rect.bottom(), 0, 0, QCss::RightEdge, cssStyle, brush);
    qDrawEdge(painter, rect.left() + border, rect.bottom(), rect.right() + border, rect.bottom() + border, 0, 0, QCss::BottomEdge, cssStyle, brush);

    if (topPage != bottomPage)
        painter->restore();
}

void QTextDocumentLayoutPrivate::drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect) const
{
    if (fd->border != 0) {
        painter->save();
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);

        const qreal leftEdge = rect.left() + fd->leftMargin.toReal();
        const qreal border = fd->border.toReal();
        const qreal topMargin = fd->topMargin.toReal();
        const qreal leftMargin = fd->leftMargin.toReal();
        const qreal bottomMargin = fd->bottomMargin.toReal();
        const qreal rightMargin = fd->rightMargin.toReal();
        const qreal w = rect.width() - 2 * border - leftMargin - rightMargin;
        const qreal h = rect.height() - 2 * border - topMargin - bottomMargin;

        drawBorder(painter, QRectF(leftEdge, rect.top() + topMargin, w + border, h + border), border, frame->frameFormat().borderBrush(), frame->frameFormat().borderStyle());

        painter->restore();
    }

    const QBrush bg = frame->frameFormat().background();
    if (bg != Qt::NoBrush) {
        QRectF bgRect = rect;
        bgRect.adjust((fd->leftMargin + fd->border).toReal(),
                      (fd->topMargin + fd->border).toReal(),
                      - (fd->rightMargin + fd->border).toReal(),
                      - (fd->bottomMargin + fd->border).toReal());

        QRectF gradientRect; // invalid makes it default to bgRect
        if (!frame->parentFrame()) {
            bgRect = clip;
            gradientRect.setWidth(painter->device()->width());
            gradientRect.setHeight(painter->device()->height());
        }
        fillBackground(painter, bgRect, bg, gradientRect);
    }
}

void QTextDocumentLayoutPrivate::drawFrame(const QPointF &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextFrame *frame) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    // #######
    if (fd->layoutDirty)
        return;
    Q_ASSERT(!fd->sizeDirty);
    Q_ASSERT(!fd->layoutDirty);

    const QPointF off = offset + fd->position.toPointF();
    if (context.clip.isValid()
        && (off.y() > context.clip.bottom() || off.y() + fd->size.height.toReal() < context.clip.top()
            || off.x() > context.clip.right() || off.x() + fd->size.width.toReal() < context.clip.left()))
        return;

//     LDEBUG << debug_indent << "drawFrame" << frame->firstPosition() << "--" << frame->lastPosition() << "at" << offset;
//     INC_INDENT;

    // if the cursor is /on/ a table border we may need to repaint it
    // afterwards, as we usually draw the decoration first
    QTextBlock cursorBlockNeedingRepaint;
    QPointF offsetOfRepaintedCursorBlock = off;

    QTextTable *table = qobject_cast<QTextTable *>(frame);
    const QRectF frameRect(off, fd->size.toSizeF());

    if (table) {
        const int rows = table->rows();
        const int columns = table->columns();
        QTextTableData *td = static_cast<QTextTableData *>(data(table));

        QVarLengthArray<int> selectedTableCells(context.selections.size() * 4);
        for (int i = 0; i < context.selections.size(); ++i) {
            const QAbstractTextDocumentLayout::Selection &s = context.selections.at(i);
            int row_start = -1, col_start = -1, num_rows = -1, num_cols = -1;

            if (s.cursor.currentTable() == table)
                s.cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

            selectedTableCells[i * 4] = row_start;
            selectedTableCells[i * 4 + 1] = col_start;
            selectedTableCells[i * 4 + 2] = num_rows;
            selectedTableCells[i * 4 + 3] = num_cols;
        }

        qreal border = td->border.toReal();
        drawFrameDecoration(painter, frame, fd, context.clip, frameRect);

        int firstRow = 0;
        int lastRow = rows;

        if (context.clip.isValid()) {
            QVector<QFixed>::ConstIterator rowIt = qLowerBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), QFixed::fromReal(context.clip.top() - off.y()));
            if (rowIt != td->rowPositions.constEnd() && rowIt != td->rowPositions.constBegin()) {
                --rowIt;
                firstRow = rowIt - td->rowPositions.constBegin();
            }

            rowIt = qUpperBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), QFixed::fromReal(context.clip.bottom() - off.y()));
            if (rowIt != td->rowPositions.constEnd()) {
                ++rowIt;
                lastRow = rowIt - td->rowPositions.constBegin();
            }
        }

        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(firstRow, c);
            firstRow = qMin(firstRow, cell.row());
        }

        for (int r = firstRow; r < lastRow; ++r) {
            for (int c = 0; c < columns; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                QAbstractTextDocumentLayout::PaintContext cell_context = context;
                for (int i = 0; i < context.selections.size(); ++i) {
                    int row_start = selectedTableCells[i * 4];
                    int col_start = selectedTableCells[i * 4 + 1];
                    int num_rows = selectedTableCells[i * 4 + 2];
                    int num_cols = selectedTableCells[i * 4 + 3];

                    if (row_start != -1) {
                        if (r >= row_start && r < row_start + num_rows
                            && c >= col_start && c < col_start + num_cols)
                        {
                            int firstPosition = cell.firstPosition();
                            int lastPosition = cell.lastPosition();

                            // make sure empty cells are still selected
                            if (firstPosition == lastPosition)
                                ++lastPosition;

                            cell_context.selections[i].cursor.setPosition(firstPosition);
                            cell_context.selections[i].cursor.setPosition(lastPosition, QTextCursor::KeepAnchor);
                        } else {
                            cell_context.selections[i].cursor.clearSelection();
                        }
                    }
                }
                QRectF cellRect = td->cellRect(cell);

                cellRect.translate(off);
                // we need to account for the cell border in the clipping test
                if (cell_context.clip.isValid() && !cellRect.adjusted(1 - border, 1 - border, border, border).intersects(cell_context.clip))
                    continue;

                drawTableCell(cellRect, painter, cell_context, table, td, r, c, &cursorBlockNeedingRepaint,
                              &offsetOfRepaintedCursorBlock);
            }
        }

    } else {
        drawFrameDecoration(painter, frame, fd, context.clip, frameRect);

        QTextFrame::Iterator it = frame->begin();

        if (frame == q->document()->rootFrame())
            it = frameIteratorForYPosition(QFixed::fromReal(context.clip.top()));

        QList<QTextFrame *> floats;
        for (int i = 0; i < fd->floats.count(); ++i)
            floats.append(fd->floats.at(i));

        drawFlow(off, painter, context, it, floats, &cursorBlockNeedingRepaint);
    }

    if (cursorBlockNeedingRepaint.isValid()) {
        const QPen oldPen = painter->pen();
        painter->setPen(context.palette.color(QPalette::Text));
        const int cursorPos = context.cursorPosition - cursorBlockNeedingRepaint.position();
        cursorBlockNeedingRepaint.layout()->drawCursor(painter, offsetOfRepaintedCursorBlock,
                                                       cursorPos, cursorWidth);
        painter->setPen(oldPen);
    }

//     DEC_INDENT;

    return;
}

void QTextDocumentLayoutPrivate::drawTableCell(const QRectF &cellRect, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &cell_context,
                                               QTextTable *table, QTextTableData *td, int r, int c,
                                               QTextBlock *cursorBlockNeedingRepaint, QPointF *cursorBlockOffset) const
{
    QTextTableCell cell = table->cellAt(r, c);
    int rspan = cell.rowSpan();
    int cspan = cell.columnSpan();
    if (rspan != 1) {
        int cr = cell.row();
        if (cr != r)
            return;
    }
    if (cspan != 1) {
        int cc = cell.column();
        if (cc != c)
            return;
    }

    if (td->border != 0) {
        const QBrush oldBrush = painter->brush();
        const QPen oldPen = painter->pen();

        const qreal border = td->border.toReal();

        QRectF borderRect(cellRect.left() - border, cellRect.top() - border, cellRect.width() + border, cellRect.height() + border);

        // invert the border style for cells
        QTextFrameFormat::BorderStyle cellBorder = table->format().borderStyle();
        switch (cellBorder) {
        case QTextFrameFormat::BorderStyle_Inset:
            cellBorder = QTextFrameFormat::BorderStyle_Outset;
            break;
        case QTextFrameFormat::BorderStyle_Outset:
            cellBorder = QTextFrameFormat::BorderStyle_Inset;
            break;
        case QTextFrameFormat::BorderStyle_Groove:
            cellBorder = QTextFrameFormat::BorderStyle_Ridge;
            break;
        case QTextFrameFormat::BorderStyle_Ridge:
            cellBorder = QTextFrameFormat::BorderStyle_Groove;
            break;
        default:
            break;
        }

        drawBorder(painter, borderRect, border, table->format().borderBrush(), cellBorder);

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }

    {
        const QBrush bg = cell.format().background();
        if (bg != Qt::NoBrush)
            fillBackground(painter, cellRect, bg);
    }

    const QFixed verticalOffset = td->cellVerticalOffsets.at(c + r * table->columns());

    const QPointF cellPos = QPointF(cellRect.left() + td->cellPadding.toReal(),
                                    cellRect.top() + (td->cellPadding + verticalOffset).toReal());

    QTextBlock repaintBlock;
    drawFlow(cellPos, painter, cell_context, cell.begin(),
             td->childFrameMap.values(r + c * table->rows()),
             &repaintBlock);
    if (repaintBlock.isValid()) {
        *cursorBlockNeedingRepaint = repaintBlock;
        *cursorBlockOffset = cellPos;
    }
}

void QTextDocumentLayoutPrivate::drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                                          QTextFrame::Iterator it, const QList<QTextFrame *> &floats, QTextBlock *cursorBlockNeedingRepaint) const
{
    Q_Q(const QTextDocumentLayout);
    const bool inRootFrame = (!it.atEnd() && it.parentFrame() && it.parentFrame()->parentFrame() == 0);

    QVector<QCheckPoint>::ConstIterator lastVisibleCheckPoint = checkPoints.end();
    if (inRootFrame && context.clip.isValid()) {
        lastVisibleCheckPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), QFixed::fromReal(context.clip.bottom()));
    }

    QTextBlock lastBlock;

    for (; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame && !checkPoints.isEmpty()) {
            int currentPosInDoc;
            if (c)
                currentPosInDoc = c->firstPosition();
            else
                currentPosInDoc = it.currentBlock().position();

            // if we're past what is already laid out then we're better off
            // not trying to draw things that may not be positioned correctly yet
            if (currentPosInDoc >= checkPoints.last().positionInFrame)
                break;

            if (lastVisibleCheckPoint != checkPoints.end()
                && context.clip.isValid()
                && currentPosInDoc >= lastVisibleCheckPoint->positionInFrame
               )
                break;
        }

        if (c)
            drawFrame(offset, painter, context, c);
        else
            drawBlock(offset, painter, context, it.currentBlock());

        // when entering a table and the previous block is empty
        // then layoutFlow 'hides' the block that just causes a
        // new line by positioning it /on/ the table border. as we
        // draw that block before the table itself the decoration
        // 'overpaints' the cursor and we need to paint it afterwards
        // again
        if (isEmptyBlockBeforeTable(lastBlock, it)
            && lastBlock.contains(context.cursorPosition)
           ) {
            *cursorBlockNeedingRepaint = lastBlock;
        }

        lastBlock = it.currentBlock();
    }

    QTextDocument *document = q->document();
    for (int i = 0; i < floats.count(); ++i) {
        QTextFrame *frame = floats.at(i);
        if (!isFrameFromInlineObject(frame)
            || frame->frameFormat().position() == QTextFrameFormat::InFlow)
            continue;

        const int pos = frame->firstPosition() - 1;
        QTextCharFormat format = const_cast<QTextDocumentLayout *>(q)->format(pos);
        QTextObjectInterface *handler = q->handlerForObject(format.objectType());
        if (handler) {
            QRectF rect = frameBoundingRectInternal(frame);
            handler->drawObject(painter, rect, document, pos, format);
        }
    }
}

void QTextDocumentLayoutPrivate::drawBlock(const QPointF &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextLayout *tl = bl.layout();
    QRectF r = tl->boundingRect();
    r.translate(offset + tl->position());
    if (context.clip.isValid() && (r.bottom() < context.clip.y() || r.top() > context.clip.bottom()))
        return;
//      LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset << "br" << tl->boundingRect();

    QTextBlockFormat blockFormat = bl.blockFormat();

    QBrush bg = blockFormat.background();
    if (bg != Qt::NoBrush) {
        // don't paint into the left margin. Right margin is already excluded by the line breaking
        QRectF contentsRect = r;
        contentsRect.setLeft(contentsRect.left() + bl.blockFormat().leftMargin());
        fillBackground(painter, contentsRect, bg);
    }

    QVector<QTextLayout::FormatRange> selections;
    int blpos = bl.position();
    int bllen = bl.length();
    const QTextCharFormat *selFormat = 0;
    for (int i = 0; i < context.selections.size(); ++i) {
        const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
        const int selStart = range.cursor.selectionStart() - blpos;
        const int selEnd = range.cursor.selectionEnd() - blpos;
        if (selStart < bllen && selEnd > 0
             && selEnd > selStart) {
            QTextLayout::FormatRange o;
            o.start = selStart;
            o.length = selEnd - selStart;
            o.format = range.format;
            selections.append(o);
        } else if (range.format.hasProperty(QTextFormat::FullWidthSelection)
                   && bl.contains(range.cursor.position())) {
            // for full width selections we don't require an actual selection, just
            // a position to specify the line. that's more convenience in usage.
            QTextLayout::FormatRange o;
            QTextLine l = tl->lineForTextPosition(range.cursor.position() - blpos);
            o.start = l.textStart();
            o.length = qMax(1, l.textLength());
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

    if ((context.cursorPosition >= blpos && context.cursorPosition < blpos + bllen)
        || (context.cursorPosition < -1 && !tl->preeditAreaText().isEmpty())) {
        int cpos = context.cursorPosition;
        if (cpos < -1)
            cpos = tl->preeditAreaPosition() - (cpos + 2);
        else
            cpos -= blpos;
        tl->drawCursor(painter, offset, cpos, cursorWidth);
    }

    if (blockFormat.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
        const qreal width = blockFormat.lengthProperty(QTextFormat::BlockTrailingHorizontalRulerWidth).value(r.width());
        painter->setPen(context.palette.color(QPalette::Dark));
        qreal y = r.bottom();
        if (bl.length() == 1)
            y = r.top() + r.height() / 2;

        const qreal middleX = r.left() + r.width() / 2;
        painter->drawLine(QLineF(middleX - width / 2, y, middleX + width / 2, y));
    }

    painter->setPen(oldPen);
}


void QTextDocumentLayoutPrivate::drawListItem(const QPointF &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlock bl, const QTextCharFormat *selectionFormat) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextBlockFormat blockFormat = bl.blockFormat();
    const QTextCharFormat charFormat = QTextCursor(bl).charFormat();
    QFont font(charFormat.font());
    if (q->paintDevice())
        font = QFont(font, q->paintDevice());

    const QFontMetrics fontMetrics(font);
    QTextObject * const object = q->document()->objectForFormat(blockFormat);
    const QTextListFormat lf = object->format().toListFormat();
    int style = lf.style();
    QString itemText;
    QSizeF size;

    if (blockFormat.hasProperty(QTextFormat::ListStyle))
        style = QTextListFormat::Style(blockFormat.intProperty(QTextFormat::ListStyle));

    QTextLayout *layout = bl.layout();
    if (layout->lineCount() == 0)
        return;
    QTextLine firstLine = layout->lineAt(0);
    Q_ASSERT(firstLine.isValid());
    QPointF pos = (offset + layout->position()).toPoint();
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
    case QTextListFormat::ListUpperAlpha: {
        QTextLayout layout(itemText, font, q->paintDevice());
        layout.setCacheEnabled(true);
        QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
        option.setTextDirection(dir);
        layout.setTextOption(option);
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        layout.draw(painter, QPointF(r.left(), pos.y()));
        break;
    }
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

static QFixed flowPosition(const QTextFrame::iterator it)
{
    if (it.atEnd())
        return 0;

    if (it.currentFrame()) {
        return data(it.currentFrame())->position.y;
    } else {
        QTextBlock block = it.currentBlock();
        QTextLayout *layout = block.layout();
        if (layout->lineCount() == 0)
            return QFixed::fromReal(layout->position().y());
        else
            return QFixed::fromReal(layout->position().y() + layout->lineAt(0).y());
    }
}

static QFixed firstChildPos(const QTextFrame *f)
{
    return flowPosition(f->begin());
}

QLayoutStruct QTextDocumentLayoutPrivate::layoutCell(QTextTable *t, const QTextTableCell &cell, QFixed width,
                                                    int layoutFrom, int layoutTo, QTextTableData *td,
                                                    bool withPageBreaks)
{
    Q_Q(QTextDocumentLayout);
    QFixed cellY = withPageBreaks ? td->position.y + td->rowPositions.at(cell.row()) + td->cellPadding : 0;

    LDEBUG << "layoutCell";
    QLayoutStruct layoutStruct;
    layoutStruct.frame = t;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = QFIXED_MAX;
    layoutStruct.y = cellY;
    layoutStruct.x_left = 0;
    layoutStruct.x_right = width;
    // we get called with different widths all the time (for example for figuring
    // out the min/max widths), so we always have to do the full layout ;(
    // also when for example in a table layoutFrom/layoutTo affect only one cell,
    // making that one cell grow the available width of the other cells may change
    // (shrink) and therefore when layoutCell gets called for them they have to
    // be re-laid out, even if layoutFrom/layoutTo is not in their range. Hence
    // this line:

    layoutStruct.pageHeight = QFixed::fromReal(q->document()->pageSize().height());
    if (layoutStruct.pageHeight < 0 || !withPageBreaks)
        layoutStruct.pageHeight = QFIXED_MAX;
    const int currentPage = layoutStruct.pageHeight == 0 ? 0 : (layoutStruct.y / layoutStruct.pageHeight).truncate();
    layoutStruct.pageTopMargin = data(q->document()->rootFrame())->topMargin;
    layoutStruct.pageBottomMargin = data(q->document()->rootFrame())->bottomMargin;
    layoutStruct.pageBottom = (currentPage + 1) * layoutStruct.pageHeight - layoutStruct.pageBottomMargin;

    layoutStruct.fullLayout = true;

    QFixed pageTop = currentPage * layoutStruct.pageHeight + layoutStruct.pageTopMargin;
    layoutStruct.y = qMax(layoutStruct.y, pageTop);

    layoutFlow(cell.begin(), &layoutStruct, layoutFrom, layoutTo, cellY, width);

    QFixed floatMinWidth;

    // floats that are located inside the text (like inline images) aren't taken into account by
    // layoutFlow with regards to the cell height (layoutStruct->y), so for a safety measure we
    // do that here. For example with <td><img align="right" src="..." />blah</td>
    // when the image happens to be higher than the text
    const QList<QTextFrame *> childFrames = td->childFrameMap.values(cell.row() + cell.column() * t->rows());
    for (int i = 0; i < childFrames.size(); ++i) {
        QTextFrame *frame = childFrames.at(i);
        QTextFrameData *cd = data(frame);

        if (frame->frameFormat().position() != QTextFrameFormat::InFlow)
            layoutStruct.y = qMax(layoutStruct.y, cd->position.y + cd->size.height);

        floatMinWidth = qMax(floatMinWidth, cd->minimumWidth);
    }

    // constraint the maximumWidth by the minimum width of the fixed size floats, to
    // keep them visible
    layoutStruct.maximumWidth = qMax(layoutStruct.maximumWidth, floatMinWidth);

    // as floats in cells get added to the table's float list but must not affect
    // floats in other cells we must clear the list here.
    data(t)->floats.clear();

    if (layoutStruct.y + td->cellPadding + td->border > layoutStruct.pageBottom)
        layoutStruct.newPage();

    layoutStruct.y -= cellY;

//    qDebug() << "layoutCell done";

    return layoutStruct;
}

QRectF QTextDocumentLayoutPrivate::layoutTable(QTextTable *table, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutTable";
    QTextTableData *td = static_cast<QTextTableData *>(data(table));
    Q_ASSERT(td->sizeDirty);
    const int rows = table->rows();
    const int columns = table->columns();

    const QTextTableFormat fmt = table->format();

    td->childFrameMap.clear();
    {
        const QList<QTextFrame *> children = table->childFrames();
        for (int i = 0; i < children.count(); ++i) {
            QTextFrame *frame = children.at(i);
            QTextTableCell cell = table->cellAt(frame->firstPosition());
            td->childFrameMap.insertMulti(cell.row() + cell.column() * rows, frame);
        }
    }

    QVector<QTextLength> columnWidthConstraints = fmt.columnWidthConstraints();
    if (columnWidthConstraints.size() != columns)
        columnWidthConstraints.resize(columns);
    Q_ASSERT(columnWidthConstraints.count() == columns);

    const QFixed cellSpacing = td->cellSpacing = QFixed::fromReal(scaleToDevice(fmt.cellSpacing()));
    td->cellPadding = QFixed::fromReal(scaleToDevice(fmt.cellPadding()));
    const QFixed leftMargin = td->leftMargin + td->border + td->padding;
    const QFixed rightMargin = td->rightMargin + td->border + td->padding;
    const QFixed topMargin = td->topMargin + td->border + td->padding;

    QFixed totalWidth = td->contentsWidth;
    // two (vertical) borders per cell per column
    totalWidth -= columns * 2 * td->border;
    // inter-cell spacing
    totalWidth -= (columns - 1) * cellSpacing;
    // cell spacing at the left and right hand side
    totalWidth -= 2 * cellSpacing;
    // remember the width used to distribute to percentaged columns
    QFixed initialTotalWidth = totalWidth;

    td->widths.resize(columns);
    td->widths.fill(0);

    td->minWidths.resize(columns);
    // start with a minimum width of 0. totally empty
    // cells of default created tables are invisible otherwise
    // and therefore hardly editable
    td->minWidths.fill(1);

    td->maxWidths.resize(columns);
    td->maxWidths.fill(QFIXED_MAX);

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
            QLayoutStruct layoutStruct = layoutCell(table, cell, QFIXED_MAX, layoutFrom, layoutTo, td);

            // distribute the minimum width over all columns the cell spans
            QFixed widthToDistribute = layoutStruct.minimumWidth + 2 * td->cellPadding;
            for (int n = 0; n < cspan; ++n) {
                const int col = i + n;
                QFixed w = widthToDistribute / (cspan - n);
                td->minWidths[col] = qMax(td->minWidths.at(col), w);
                widthToDistribute -= td->minWidths.at(col);
                if (widthToDistribute <= 0)
                    break;
            }

            QFixed maxW = td->maxWidths.at(i);
            if (layoutStruct.maximumWidth != QFIXED_MAX) {
                if (maxW == QFIXED_MAX)
                    maxW = layoutStruct.maximumWidth + 2 * td->cellPadding;
                else
                    maxW = qMax(maxW, layoutStruct.maximumWidth + 2 * td->cellPadding);
            }
            if (maxW == QFIXED_MAX)
                continue;

            widthToDistribute = maxW;
            for (int n = 0; n < cspan; ++n) {
                const int col = i + n;
                QFixed w = widthToDistribute / (cspan - n);
                td->maxWidths[col] = qMax(td->minWidths.at(col), w);
                widthToDistribute -= td->maxWidths.at(col);
                if (widthToDistribute <= 0)
                    break;
            }
        }
    }

    // set fixed values, figure out total percentages used and number of
    // variable length cells. Also assign the minimum width for variable columns.
    QFixed totalPercentage;
    int variableCols = 0;
    for (int i = 0; i < columns; ++i) {
        const QTextLength &length = columnWidthConstraints.at(i);
        if (length.type() == QTextLength::FixedLength) {
            td->minWidths[i] = td->widths[i] = qMax(QFixed::fromReal(length.rawValue()), td->minWidths.at(i));
            totalWidth -= td->widths.at(i);
        } else if (length.type() == QTextLength::PercentageLength) {
            totalPercentage += QFixed::fromReal(length.rawValue());
        } else if (length.type() == QTextLength::VariableLength) {
            variableCols++;

            td->widths[i] = td->minWidths.at(i);
            totalWidth -= td->minWidths.at(i);
        }
    }

    // set percentage values
    {
        QFixed minimumSlack = QFIXED_MAX;
        int colsWithSlack = 0;

        QFixed overhangingPercent;
        const QFixed totalPercentagedWidth = initialTotalWidth * totalPercentage / 100;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::PercentageLength) {
                const QFixed allottedPercentage = QFixed::fromReal(columnWidthConstraints.at(i).rawValue());

                const QFixed percentWidth = totalPercentagedWidth * allottedPercentage / totalPercentage;
                if (percentWidth >= td->minWidths.at(i)) {
                    td->widths[i] = percentWidth;

                    if (td->widths.at(i) > td->minWidths.at(i)) {
                        ++colsWithSlack;
                        minimumSlack = qMin(minimumSlack, td->widths.at(i) - td->minWidths.at(i));
                    }
                } else {
                    td->widths[i] = td->minWidths.at(i);
                    QFixed effectivePercentage = td->widths.at(i) * 100 / totalPercentagedWidth;
                    overhangingPercent += effectivePercentage - allottedPercentage;
                }
                totalWidth -= td->widths.at(i);
            }

        // subtract the overhanging percent from columns with slack
        while (colsWithSlack > 0 && overhangingPercent > 0) {
            const QFixed slackToSubtract = qMin(minimumSlack, totalPercentagedWidth * overhangingPercent / (colsWithSlack * totalPercentage));
            overhangingPercent -= (slackToSubtract * colsWithSlack * 100) / totalPercentagedWidth;

            minimumSlack = QFIXED_MAX;
            colsWithSlack = 0;

            for (int i = 0; i < columns; ++i)
                if (columnWidthConstraints.at(i).type() == QTextLength::PercentageLength) {
                    if (td->widths.at(i) > td->minWidths.at(i)) {
                        td->widths[i] -= slackToSubtract;

                        if (td->widths.at(i) > td->minWidths.at(i)) {
                            ++colsWithSlack;
                            minimumSlack = qMin(minimumSlack, td->widths.at(i) - td->minWidths.at(i));
                        }
                    }
                }
        }
    }

    // for variable columns distribute the remaining space
    if (variableCols > 0 && totalWidth > 0) {
        QVarLengthArray<int> columnsWithProperMaxSize;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::VariableLength
                && td->maxWidths.at(i) != QFIXED_MAX)
                columnsWithProperMaxSize.append(i);

        QFixed lastTotalWidth = totalWidth;
        while (totalWidth > 0) {
            for (int k = 0; k < columnsWithProperMaxSize.count(); ++k) {
                const int col = columnsWithProperMaxSize[k];
                const int colsLeft = columnsWithProperMaxSize.count() - k;
                const QFixed w = qMin(td->maxWidths.at(col) - td->widths.at(col), totalWidth / colsLeft);
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
            const QFixed widthPerAnySizedCol = totalWidth / variableCols;
            for (int col = 0; col < columns; ++col) {
                if (columnWidthConstraints.at(col).type() == QTextLength::VariableLength)
                    td->widths[col] += widthPerAnySizedCol;
            }
        }
    }


    td->columnPositions.resize(columns);
    td->columnPositions[0] = leftMargin /*includes table border*/ + cellSpacing + td->border;

    for (int i = 1; i < columns; ++i)
        td->columnPositions[i] = td->columnPositions.at(i-1) + td->widths.at(i-1) + 2 * td->border + cellSpacing;

    td->heights.resize(rows);
    td->heights.fill(0);

    td->rowPositions.resize(rows);
    td->rowPositions[0] = topMargin /*includes table border*/ + cellSpacing + td->border;

    bool haveRowSpannedCells = false;

    // need to keep track of cell heights for vertical alignment
    QVector<QFixed> cellHeights;
    cellHeights.reserve(rows * columns);

    Q_Q(QTextDocumentLayout);
    QFixed pageHeight = QFixed::fromReal(q->document()->pageSize().height());
    if (pageHeight <= 0)
        pageHeight = QFIXED_MAX;
    QFixed pageTopMargin = data(q->document()->rootFrame())->topMargin;
    QFixed pageBottomMargin = data(q->document()->rootFrame())->bottomMargin;

    QVector<QFixed> heightToDistribute;
    heightToDistribute.resize(columns);

    // now that we have the column widths we can lay out all cells with the right width.
    // spanning cells are only allowed to grow the last row spanned by the cell.
    //
    // ### this could be made faster by iterating over the cells array of QTextTable
    for (int r = 0; r < rows; ++r) {
        td->calcRowPosition(r);

        const int currentPage = ((td->rowPositions[r] + td->position.y) / pageHeight).truncate();
        QFixed pageBottom = (currentPage + 1) * pageHeight - pageBottomMargin - td->position.y;
        QFixed pageTop = currentPage * pageHeight + pageTopMargin - td->position.y + td->border;
        //qDebug() << "Row" << r << "pos:" << td->position.y + td->rowPositions.at(r) << "page bottom:" << td->position.y + pageBottom << "page top:" << td->position.y + pageBottom - pageHeight + pageTopMargin;

        if (td->rowPositions[r] > pageBottom)
            td->rowPositions[r] = pageBottom + pageTopMargin + pageBottomMargin + td->border;
        else if (td->rowPositions[r] < pageTop)
            td->rowPositions[r] = pageTop;

        bool dropRowToNextPage = true;
        int cellCountBeforeRow = cellHeights.size();

        // if we drop the row to the next page we need to subtract the drop
        // distance from any row spanning cells
        QFixed dropDistance = 0;

relayout:
        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            const int rspan = cell.rowSpan();
            const int cspan = cell.columnSpan();

            if (cspan > 1 && cell.column() != c)
                continue;

            if (rspan > 1) {
                haveRowSpannedCells = true;

                const int cellRow = cell.row();
                if (cellRow != r) {
                    // the last row gets all the remaining space
                    if (cellRow + rspan - 1 == r)
                        td->heights[r] = qMax(td->heights.at(r), heightToDistribute.at(c) - dropDistance);
                    continue;
                }
            }

            const QFixed width = td->cellWidth(c, cspan);
            QLayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo, td, true);

            const QFixed height = layoutStruct.y + 2 * td->cellPadding;

            if (rspan > 1)
                heightToDistribute[c] = height + dropDistance;
            else
                td->heights[r] = qMax(td->heights.at(r), height);

            cellHeights.append(layoutStruct.y);

            QFixed childPos = td->rowPositions.at(r) + td->cellPadding + flowPosition(cell.begin());
            if (childPos < pageBottom)
                dropRowToNextPage = false;
        }

        if (dropRowToNextPage) {
            QFixed droppedPosition = pageBottom + pageTopMargin + pageBottomMargin + td->border;
            dropDistance = droppedPosition - td->rowPositions[r];
            //qDebug() << "Dropping row" << r << "of table" << table << "to next page, drop distance:" << dropDistance;
            td->rowPositions[r] = droppedPosition;
            td->heights[r] = 0;
            dropRowToNextPage = false;
            cellHeights.resize(cellCountBeforeRow);
            goto relayout;
        }

        if (haveRowSpannedCells) {
            const QFixed effectiveHeight = td->heights.at(r) + td->border + cellSpacing + td->border;
            for (int c = 0; c < columns; ++c)
                heightToDistribute[c] = qMax(heightToDistribute.at(c) - effectiveHeight - dropDistance, QFixed(0));
        }
    }

    // now that all cells have been properly laid out, we can compute the
    // vertical offsets for vertical alignment
    td->cellVerticalOffsets.resize(rows * columns);
    int cellIndex = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            if (cell.row() != r || cell.column() != c)
                continue;

            const int rowSpan = cell.rowSpan();
            const QFixed availableHeight = td->rowPositions.at(r + rowSpan - 1) + td->heights.at(r + rowSpan - 1) - td->rowPositions.at(r);

            const QFixed cellHeight = cellHeights.at(cellIndex++) + 2 * td->cellPadding;
            QTextCharFormat cellFormat = cell.format();

            QFixed offset = 0;
            switch (cellFormat.verticalAlignment()) {
            case QTextCharFormat::AlignMiddle:
                offset = (availableHeight - cellHeight) / 2;
                break;
            case QTextCharFormat::AlignBottom:
                offset = availableHeight - cellHeight;
                break;
            default:
                break;
            };

            for (int rd = 0; rd < cell.rowSpan(); ++rd) {
                for (int cd = 0; cd < cell.columnSpan(); ++cd) {
                    const int index = (c + cd) + (r + rd) * columns;
                    td->cellVerticalOffsets[index] = offset;
                }
            }
        }
    }

    // - margin to compensate the + margin in columnPositions[0]
    td->contentsWidth = td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - leftMargin;

    td->minimumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i) {
        td->minimumWidth += td->minWidths.at(i) + 2 * td->border + cellSpacing;
    }
    td->minimumWidth += rightMargin - td->border;

    td->maximumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i)
        if (td->maxWidths.at(i) != QFIXED_MAX)
            td->maximumWidth += td->maxWidths.at(i) + 2 * td->border + cellSpacing;
    td->maximumWidth += rightMargin - td->border;

    td->updateTableSize();
    td->sizeDirty = false;
    return QRectF(); // invalid rect -> update everything
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
    QFixed y = pd->currentLayoutStruct->y;
    if (currentLine) {
        QFixed left, right;
        floatMargins(y, pd->currentLayoutStruct, &left, &right);
//         qDebug() << "have line: right=" << right << "left=" << left << "textWidth=" << currentLine->textWidth();
        if (right - left < QFixed::fromReal(currentLine->naturalTextWidth()) + fd->size.width) {
            pd->currentLayoutStruct->pendingFloats.append(frame);
//             qDebug() << "    adding to pending list";
            return;
        }
    }

    if (!parent->parentFrame() /* float in root frame */
        && y + fd->size.height > pd->currentLayoutStruct->pageBottom) {
        y = pd->currentLayoutStruct->pageBottom;
    }

    y = findY(y, pd->currentLayoutStruct, fd->size.width);

    QFixed left, right;
    floatMargins(y, pd->currentLayoutStruct, &left, &right);

    if (frame->frameFormat().position() == QTextFrameFormat::FloatLeft) {
        fd->position.x = left;
        fd->position.y = y;
    } else {
        fd->position.x = right - fd->size.width;
        fd->position.y = y;
    }

//     qDebug()<< "float positioned at " << fd->position;
    fd->layoutDirty = false;
}

QRectF QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, QFixed parentY)
{
    LDEBUG << "layoutFrame (pre)";
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->frameFormat();

    QTextFrame *parent = f->parentFrame();
    const QTextFrameData *pd = parent ? data(parent) : 0;

    const qreal maximumWidth = qMax(qreal(0), pd ? pd->contentsWidth.toReal() : q_func()->document()->pageSize().width());

    const QFixed width = QFixed::fromReal(fformat.width().value(maximumWidth));

    QTextLength height = fformat.height();
    qreal h = height.value(pd ? pd->contentsHeight.toReal() : -1);

    return layoutFrame(f, layoutFrom, layoutTo, width, QFixed::fromReal(h), parentY);
}

QRectF QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, QFixed frameWidth, QFixed frameHeight, QFixed parentY)
{
    LDEBUG << "layoutFrame from=" << layoutFrom << "to=" << layoutTo;
    Q_Q(QTextDocumentLayout);
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameData *fd = data(f);
    const QFixed oldContentsWidth = fd->contentsWidth;
    QFixed newContentsWidth;

    {
        QTextFrameFormat fformat = f->frameFormat();
        // set sizes of this frame from the format
        fd->topMargin = QFixed::fromReal(fformat.topMargin());
        fd->bottomMargin = QFixed::fromReal(fformat.bottomMargin());
        fd->leftMargin = QFixed::fromReal(fformat.leftMargin());
        fd->rightMargin = QFixed::fromReal(fformat.rightMargin());
        fd->border = QFixed::fromReal(fformat.border());
        fd->padding = QFixed::fromReal(fformat.padding());

        newContentsWidth = frameWidth - 2*(fd->border + fd->padding)
                           - fd->leftMargin - fd->rightMargin;

        if (frameHeight != -1) {
            fd->contentsHeight = frameHeight - 2*(fd->border + fd->padding)
                                 - fd->topMargin - fd->bottomMargin;
        } else {
            fd->contentsHeight = frameHeight;
        }
    }

    if (isFrameFromInlineObject(f)) {
        int startPos = f->firstPosition();
        fd->contentsWidth = newContentsWidth;
        // inline image
        QTextCharFormat format = q->format(startPos - 1);
        QTextObjectInterface *iface = q->handlerForObject(format.objectType());
        if (iface) {
            fd->size = QFixedSize::fromSizeF(iface->intrinsicSize(q->document(), startPos - 1, format));
            fd->minimumWidth = fd->maximumWidth = fd->size.width;
        }
        fd->sizeDirty = false;
        return QRectF();
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
        fd->contentsWidth = newContentsWidth;
        return layoutTable(table, layoutFrom, layoutTo);
    }

    // set fd->contentsWidth temporarily, so that layoutFrame for the children
    // picks the right width. We'll initialize it properly at the end of this
    // function.
    fd->contentsWidth = newContentsWidth;

    QLayoutStruct layoutStruct;
    layoutStruct.frame = f;
    layoutStruct.x_left = fd->leftMargin + fd->border + fd->padding;
    layoutStruct.x_right = layoutStruct.x_left + newContentsWidth;
    layoutStruct.y = parentY + fd->topMargin + fd->border + fd->padding;
    layoutStruct.contentsWidth = 0;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = QFIXED_MAX;
    layoutStruct.fullLayout = oldContentsWidth != newContentsWidth;
    layoutStruct.updateRect = QRectF(QPointF(0, 0), QSizeF(INT_MAX, INT_MAX));
    LDEBUG << "layoutStruct: x_left" << layoutStruct.x_left << "x_right" << layoutStruct.x_right
           << "fullLayout" << layoutStruct.fullLayout;

    layoutStruct.pageHeight = QFixed::fromReal(q->document()->pageSize().height());
    if (layoutStruct.pageHeight < 0)
        layoutStruct.pageHeight = QFIXED_MAX;

    const int currentPage = layoutStruct.pageHeight == 0 ? 0 : (parentY / layoutStruct.pageHeight).truncate();
    layoutStruct.pageTopMargin = data(q->document()->rootFrame())->topMargin;
    layoutStruct.pageBottomMargin = data(q->document()->rootFrame())->bottomMargin;
    layoutStruct.pageBottom = (currentPage + 1) * layoutStruct.pageHeight - layoutStruct.pageBottomMargin;

    if (!f->parentFrame())
        idealWidth = 0; // reset

    QTextFrame::Iterator it = f->begin();
    layoutFlow(it, &layoutStruct, layoutFrom, layoutTo, parentY);

    QFixed maxChildFrameWidth = 0;
    QList<QTextFrame *> children = f->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = data(c);
        maxChildFrameWidth = qMax(maxChildFrameWidth, cd->size.width);
    }

    if (!f->parentFrame())
        idealWidth = qMax(maxChildFrameWidth, layoutStruct.contentsWidth).toReal();

    QFixed actualWidth = qMax(newContentsWidth, qMax(maxChildFrameWidth, layoutStruct.contentsWidth));
    fd->contentsWidth = actualWidth;
    if (newContentsWidth <= 0) { // nowrap layout?
        fd->contentsWidth = newContentsWidth;
    }

    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    fd->size.height = fd->contentsHeight == -1
                 ? layoutStruct.y + fd->border + fd->padding + fd->bottomMargin - parentY
                 : fd->contentsHeight + 2*(fd->border + fd->padding) + fd->topMargin + fd->bottomMargin;
    fd->size.width = actualWidth + 2*(fd->border + fd->padding) + fd->leftMargin + fd->rightMargin;
    fd->sizeDirty = false;
    return layoutStruct.updateRect;
}

void QTextDocumentLayoutPrivate::layoutFlow(QTextFrame::Iterator it, QLayoutStruct *layoutStruct,
                                            int layoutFrom, int layoutTo,
                                            QFixed parentY,
                                            QFixed width)
{
    Q_Q(QTextDocumentLayout);
    LDEBUG << "layoutFlow from=" << layoutFrom << "to=" << layoutTo;
    QTextFrameData *fd = data(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    QTextFrame::Iterator previousIt;

    const bool inRootFrame = (it.parentFrame() == q->document()->rootFrame());
    if (inRootFrame) {
        bool redoCheckPoints = layoutStruct->fullLayout || checkPoints.isEmpty();

        if (!redoCheckPoints) {
            QVector<QCheckPoint>::Iterator checkPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), layoutFrom);
            if (checkPoint != checkPoints.end()) {
                if (checkPoint != checkPoints.begin())
                    --checkPoint;

                layoutStruct->y = checkPoint->y;
                layoutStruct->minimumWidth = checkPoint->minimumWidth;
                layoutStruct->maximumWidth = checkPoint->maximumWidth;
                layoutStruct->contentsWidth = checkPoint->contentsWidth;

                if (layoutStruct->pageHeight > 0) {
                    int page = (layoutStruct->y / layoutStruct->pageHeight).truncate();
                    layoutStruct->pageBottom = (page + 1) * layoutStruct->pageHeight - layoutStruct->pageBottomMargin;
                }

                it = frameIteratorForTextPosition(checkPoint->positionInFrame);
                checkPoints.resize(checkPoint - checkPoints.begin() + 1);

                if (checkPoint != checkPoints.begin()) {
                    previousIt = it;
                    --previousIt;
                }
            } else {
                redoCheckPoints = true;
            }
        }

        if (redoCheckPoints) {
            checkPoints.clear();
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.positionInFrame = 0;
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
        }
    }

    QFixed maximumBlockWidth = 0;
    while (!it.atEnd()) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame) {
            int docPos;
            if (it.currentFrame())
                docPos = it.currentFrame()->firstPosition();
            else
                docPos = it.currentBlock().position();

            if (qAbs(layoutStruct->y - checkPoints.last().y) > 2000) {
                QFixed left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                if (left == layoutStruct->x_left && right == layoutStruct->x_right) {
                    QCheckPoint p;
                    p.y = layoutStruct->y;
                    p.positionInFrame = docPos;
                    p.minimumWidth = layoutStruct->minimumWidth;
                    p.maximumWidth = layoutStruct->maximumWidth;
                    p.contentsWidth = layoutStruct->contentsWidth;
                    checkPoints.append(p);

                    if (currentLazyLayoutPosition != -1
                        && docPos > currentLazyLayoutPosition + lazyLayoutStepSize)
                        break;

                    if (layoutTo != -1
                        && docPos > layoutTo)
                        break;
                }
            }
        }

        if (c) {
            // position child frame
            QTextFrameData *cd = data(c);

            QTextFrameFormat fformat = c->frameFormat();

            if (fformat.position() == QTextFrameFormat::InFlow) {
                if (fformat.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
                    layoutStruct->newPage();

                QFixed left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, layoutStruct->x_left);
                right = qMin(right, layoutStruct->x_right);

                if (right - left < cd->size.width) {
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, cd->size.width);
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                }

                QFixedPoint pos(left, layoutStruct->y);

                Qt::Alignment align = Qt::AlignLeft;

                QTextTable *table = qobject_cast<QTextTable *>(c);

                if (table)
                    align = table->format().alignment() & Qt::AlignHorizontal_Mask;

                cd->position = pos;

                if (q->document()->pageSize().height() > 0.0f)
                    cd->sizeDirty = true;

                if (cd->sizeDirty) {
                    if (width != 0)
                        layoutFrame(c, layoutFrom, layoutTo, width, -1, pos.y);
                    else
                        layoutFrame(c, layoutFrom, layoutTo, pos.y);

                    QFixed childPos = table ? pos.y + static_cast<QTextTableData *>(data(table))->rowPositions.at(0) : pos.y + firstChildPos(c);

                    // drop entire frame to next page if first child of frame is on next page
                    if (childPos > layoutStruct->pageBottom) {
                        layoutStruct->newPage();
                        pos.y = layoutStruct->y;

                        cd->position = pos;
                        cd->sizeDirty = true;

                        if (width != 0)
                            layoutFrame(c, layoutFrom, layoutTo, width, -1, pos.y);
                        else
                            layoutFrame(c, layoutFrom, layoutTo, pos.y);
                    }
                }

                // align only if there is space for alignment
                if (right - left > cd->size.width) {
                    if (align & Qt::AlignRight)
                        pos.x += layoutStruct->x_right - cd->size.width;
                    else if (align & Qt::AlignHCenter)
                        pos.x += (layoutStruct->x_right - cd->size.width) / 2;
                }

                cd->position = pos;

                layoutStruct->y += cd->size.height;
                const int page = (layoutStruct->y / layoutStruct->pageHeight).truncate();
                layoutStruct->pageBottom = (page + 1) * layoutStruct->pageHeight - layoutStruct->pageBottomMargin;

                cd->layoutDirty = false;

                if (c->frameFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
                    layoutStruct->newPage();

                // positions should be relative
                cd->position.y -= parentY;
            } else {
                if (cd->sizeDirty)
                    layoutFrame(c, layoutFrom, layoutTo);

                positionFloat(c);
            }

            layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, cd->minimumWidth);
            layoutStruct->maximumWidth = qMin(layoutStruct->maximumWidth, cd->maximumWidth);

            previousIt = it;
            ++it;
        } else {
            QTextFrame::Iterator lastIt;
            if (!previousIt.atEnd())
                lastIt = previousIt;
            previousIt = it;
            QTextBlock block = it.currentBlock();
            ++it;

            if (block.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
                layoutStruct->newPage();

            const QFixed origY = layoutStruct->y;
            const QFixed origPageBottom = layoutStruct->pageBottom;
            const QFixed origMaximumWidth = layoutStruct->maximumWidth;
            layoutStruct->maximumWidth = 0;

            // layout and position child block
            layoutBlock(block, layoutStruct, layoutFrom, layoutTo, lastIt.currentBlock());

            // if the block right before a table is empty 'hide' it by
            // positioning it into the table border
            if (isEmptyBlockBeforeTable(block, it)) {
                const QTextBlock lastBlock = lastIt.currentBlock();
                const qreal lastBlockBottomMargin = lastBlock.isValid() ? lastBlock.blockFormat().bottomMargin() : 0.0f;
                layoutStruct->y = origY + QFixed::fromReal(qMax(lastBlockBottomMargin, block.blockFormat().topMargin()));
                layoutStruct->pageBottom = origPageBottom;
            } else {
                // if the block right after a table is empty then 'hide' it, too
                if (isEmptyBlockAfterTable(block, lastIt.currentFrame())) {
                    QTextTableData *td = static_cast<QTextTableData *>(data(lastIt.currentFrame()));
                    QTextLayout *layout = block.layout();

                    QPointF pos((td->position.x + td->size.width).toReal(),
                                (td->position.y + td->size.height).toReal() - layout->boundingRect().height());

                    layout->setPosition(pos);
                    layoutStruct->y = origY;
                    layoutStruct->pageBottom = origPageBottom;
                }

                // if the block right after a table starts with a line separator, shift it up by one line
                if (isLineSeparatorBlockAfterTable(block, lastIt.currentFrame())) {
                    QTextTableData *td = static_cast<QTextTableData *>(data(lastIt.currentFrame()));
                    QTextLayout *layout = block.layout();

                    QFixed height = QFixed::fromReal(layout->lineAt(0).height());

                    if (layoutStruct->pageBottom == origPageBottom) {
                        layoutStruct->y -= height;
                        layout->setPosition(layout->position() - QPointF(0, height.toReal()));
                    } else {
                        // relayout block to correctly handle page breaks
                        layoutStruct->y = origY - height;
                        layoutStruct->pageBottom = origPageBottom;
                        layoutBlock(block, layoutStruct, layoutFrom, layoutTo, lastIt.currentBlock());
                    }

                    QPointF linePos((td->position.x + td->size.width).toReal(),
                                    (td->position.y + td->size.height - height).toReal());

                    layout->lineAt(0).setPosition(linePos - layout->position());
                }

                if (block.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
                    layoutStruct->newPage();
            }

            // positions are relative
            QTextLayout *layout = block.layout();
            QPointF pos = layout->position();
            layout->setPosition(QPointF(pos.x(), pos.y() - parentY.toReal()));

            maximumBlockWidth = qMax(maximumBlockWidth, layoutStruct->maximumWidth);
            layoutStruct->maximumWidth = origMaximumWidth;
        }
    }
    if (layoutStruct->maximumWidth == QFIXED_MAX && maximumBlockWidth > 0)
        layoutStruct->maximumWidth = maximumBlockWidth;
    else
        layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maximumBlockWidth);

    // a float at the bottom of a frame may make it taller, hence the qMax() for layoutStruct->y.
    // we don't need to do it for tables though because floats in tables are per table
    // and not per cell and layoutCell already takes care of doing the same as we do here
    if (!qobject_cast<QTextTable *>(layoutStruct->frame)) {
        QList<QTextFrame *> children = layoutStruct->frame->childFrames();
        for (int i = 0; i < children.count(); ++i) {
            QTextFrameData *fd = data(children.at(i));
            if (!fd->layoutDirty && children.at(i)->frameFormat().position() != QTextFrameFormat::InFlow)
                layoutStruct->y = qMax(layoutStruct->y, fd->position.y + fd->size.height);
        }
    }

    if (inRootFrame) {
        if (it.atEnd()) {
            //qDebug() << "layout done!";
            currentLazyLayoutPosition = -1;
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.positionInFrame = q->document()->docHandle()->length();
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
        } else {
            currentLazyLayoutPosition = checkPoints.last().positionInFrame;
            // #######
            //checkPoints.last().positionInFrame = q->document()->docHandle()->length();
        }
    }


    fd->currentLayoutStruct = 0;
}

void QTextDocumentLayoutPrivate::layoutBlock(const QTextBlock &bl, QLayoutStruct *layoutStruct,
                                             int layoutFrom, int layoutTo, const QTextBlock &previousBlock)
{
    Q_Q(QTextDocumentLayout);

    const QTextDocument *doc = q->document();
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    LDEBUG << "layoutBlock from=" << layoutFrom << "to=" << layoutTo;

    QTextOption option = doc->defaultTextOption();
    if (blockFormat.hasProperty(QTextFormat::LayoutDirection))
        option.setTextDirection(blockFormat.layoutDirection());
    const Qt::LayoutDirection dir = option.textDirection();

    if (blockFormat.hasProperty(QTextFormat::BlockAlignment)) {
        Qt::Alignment align = QStyle::visualAlignment(dir, blockFormat.alignment());
        option.setAlignment(align);
    }

    if (blockFormat.nonBreakableLines() || doc->pageSize().width() < 0) {
        option.setWrapMode(QTextOption::ManualWrap);
    }

    tl->setTextOption(option);

    const bool haveWordOrAnyWrapMode = (option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere);

//    qDebug() << "layoutBlock; width" << layoutStruct->x_right - layoutStruct->x_left << "(maxWidth is btw" << tl->maximumWidth() << ")";

    if (previousBlock.isValid()) {
        qreal margin = qMax(blockFormat.topMargin(), previousBlock.blockFormat().bottomMargin());
        if (margin > 0 && q->paintDevice()) {
            extern int qt_defaultDpi();
            margin *= qreal(q->paintDevice()->logicalDpiY()) / qreal(qt_defaultDpi());
        }
        layoutStruct->y += QFixed::fromReal(margin);
    }

    //QTextFrameData *fd = data(layoutStruct->frame);

    const QFixed indent = QFixed::fromReal(this->indent(bl));
    const QFixed totalLeftMargin = QFixed::fromReal(blockFormat.leftMargin()) + (dir == Qt::RightToLeft ? 0 : indent);
    const QFixed totalRightMargin = QFixed::fromReal(blockFormat.rightMargin()) + (dir == Qt::RightToLeft ? indent : 0);

    const QPointF oldPosition = tl->position();
    tl->setPosition(QPointF(layoutStruct->x_left.toReal(), layoutStruct->y.toReal()));

    if (layoutStruct->fullLayout
        || (bl.position() + bl.length() > layoutFrom && bl.position() <= layoutTo)
        // force relayout if we cross a page boundary
        || (layoutStruct->pageHeight > 0 && layoutStruct->y + QFixed::fromReal(tl->boundingRect().height()) > layoutStruct->pageBottom)) {

//         qDebug() << "    layouting block at" << bl.position();
        const QFixed cy = layoutStruct->y;
        const QFixed l = layoutStruct->x_left  + totalLeftMargin;
        const QFixed r = layoutStruct->x_right - totalRightMargin;

        tl->beginLayout();
        bool firstLine = true;
        while (1) {
            QTextLine line = tl->createLine();
            if (!line.isValid())
                break;

            QFixed left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            QFixed text_indent;
            if (firstLine) {
                text_indent = QFixed::fromReal(blockFormat.textIndent());
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
                firstLine = false;
            }
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;

            if (fixedColumnWidth != -1)
                line.setNumColumns(fixedColumnWidth, (right - left).toReal());
            else
                line.setLineWidth((right - left).toReal());

//        qDebug() << "layoutBlock; layouting line with width" << right - left << "->textWidth" << line.textWidth();
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            if (dir == Qt::LeftToRight)
                left += text_indent;
            else
                right -= text_indent;

            if (fixedColumnWidth == -1 && QFixed::fromReal(line.naturalTextWidth()) > right-left) {
                // float has been added in the meantime, redo
                layoutStruct->pendingFloats.clear();

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WrapAnywhere);
                    tl->setTextOption(option);
                }

                line.setLineWidth((right-left).toReal());
                if (QFixed::fromReal(line.naturalTextWidth()) > right-left) {
                    layoutStruct->pendingFloats.clear();
                    // lines min width more than what we have
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, QFixed::fromReal(line.naturalTextWidth()));
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                    left = qMax(left, l);
                    right = qMin(right, r);
                    if (dir == Qt::LeftToRight)
                        left += text_indent;
                    else
                        right -= text_indent;
                    line.setLineWidth(qMax<qreal>(line.naturalTextWidth(), (right-left).toReal()));
                }

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WordWrap);
                    tl->setTextOption(option);
                }
            }

            QFixed lineHeight = QFixed::fromReal(line.height());
            if (layoutStruct->pageHeight > 0 && layoutStruct->y + lineHeight > layoutStruct->pageBottom) {
                layoutStruct->newPage();

                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, l);
                right = qMin(right, r);
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
            }

            line.setPosition(QPointF((left - layoutStruct->x_left).toReal(), (layoutStruct->y - cy).toReal()));
            layoutStruct->y += lineHeight;
            layoutStruct->contentsWidth
                = qMax<QFixed>(layoutStruct->contentsWidth, QFixed::fromReal(line.x() + line.naturalTextWidth()) + totalRightMargin);

            // position floats
            for (int i = 0; i < layoutStruct->pendingFloats.size(); ++i) {
                QTextFrame *f = layoutStruct->pendingFloats.at(i);
                positionFloat(f);
            }
            layoutStruct->pendingFloats.clear();
        }
        tl->endLayout();
    } else {
        const int cnt = tl->lineCount();
        for (int i = 0; i < cnt; ++i) {
            QTextLine line = tl->lineAt(i);
            layoutStruct->contentsWidth
                = qMax(layoutStruct->contentsWidth, QFixed::fromReal(line.x() + tl->lineAt(i).naturalTextWidth()) + totalRightMargin);
            const QFixed lineHeight = QFixed::fromReal(line.height());
            if (layoutStruct->pageHeight > 0 && layoutStruct->y + lineHeight > layoutStruct->pageBottom)
                layoutStruct->newPage();
            line.setPosition(QPointF(line.position().x(), layoutStruct->y.toReal() - tl->position().y()));
            layoutStruct->y += lineHeight;
        }
        if (layoutStruct->updateRect.isValid()
            && bl.length() > 1) {
            if (layoutFrom >= bl.position() + bl.length()) {
                // if our height didn't change and the change in the document is
                // in one of the later paragraphs, then we don't need to repaint
                // this one
                layoutStruct->updateRect.setTop(qMax(layoutStruct->updateRect.top(), layoutStruct->y.toReal()));
            } else if (layoutTo < bl.position()
                       && oldPosition == tl->position()) {
                // if the change in the document happened earlier in the document
                // and our position did /not/ change because none of the earlier paragraphs
                // or frames changed their height, then we don't need to repaint
                // this one
                layoutStruct->updateRect.setBottom(qMin(layoutStruct->updateRect.bottom(), tl->position().y()));
            }
        }
    }

    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, QFixed::fromReal(tl->minimumWidth() + blockFormat.leftMargin()) + indent);

    const QFixed maxW = QFixed::fromReal(tl->maximumWidth() + blockFormat.leftMargin()) + indent;
    if (maxW > 0) {
        if (layoutStruct->maximumWidth == QFIXED_MAX)
            layoutStruct->maximumWidth = maxW;
        else
            layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maxW);
    }
}

void QTextDocumentLayoutPrivate::floatMargins(QFixed y, const QLayoutStruct *layoutStruct,
                                              QFixed *left, QFixed *right) const
{
//     qDebug() << "floatMargins y=" << y;
    *left = layoutStruct->x_left;
    *right = layoutStruct->x_right;
    QTextFrameData *lfd = data(layoutStruct->frame);
    for (int i = 0; i < lfd->floats.size(); ++i) {
        QTextFrameData *fd = data(lfd->floats.at(i));
        if (!fd->layoutDirty) {
            if (fd->position.y <= y && fd->position.y + fd->size.height > y) {
//                 qDebug() << "adjusting with float" << f << fd->position.x()<< fd->size.width();
                if (lfd->floats.at(i)->frameFormat().position() == QTextFrameFormat::FloatLeft)
                    *left = qMax(*left, fd->position.x + fd->size.width);
                else
                    *right = qMin(*right, fd->position.x);
            }
        }
    }
//     qDebug() << "floatMargins: left="<<*left<<"right="<<*right<<"y="<<y;
}


QFixed QTextDocumentLayoutPrivate::findY(QFixed yFrom, const QLayoutStruct *layoutStruct, QFixed requiredWidth) const
{
    QFixed right, left;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

//     qDebug() << "findY:" << yFrom;
    while (1) {
        floatMargins(yFrom, layoutStruct, &left, &right);
//         qDebug() << "    yFrom=" << yFrom<<"right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        QFixed newY = QFIXED_MAX;
        QTextFrameData *lfd = data(layoutStruct->frame);
        for (int i = 0; i < lfd->floats.size(); ++i) {
            QTextFrameData *fd = data(lfd->floats.at(i));
            if (!fd->layoutDirty) {
                if (fd->position.y <= yFrom && fd->position.y + fd->size.height > yFrom)
                    newY = qMin(newY, fd->position.y + fd->size.height);
            }
        }
        if (newY == QFIXED_MAX)
            break;
        yFrom = newY;
    }
    return yFrom;
}

QTextDocumentLayout::QTextDocumentLayout(QTextDocument *doc)
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate, doc)
{
    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    Q_D(QTextDocumentLayout);
    QTextFrame *frame = document()->rootFrame();
    if(data(frame)->sizeDirty)
        return;
    if (context.clip.isValid()) {
        d->ensureLayouted(QFixed::fromReal(context.clip.bottom()));
    } else {
        d->ensureLayoutFinished();
    }
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
        if (!f) {
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

    const QSizeF pageSize = document()->pageSize();
    if (pageSize.isNull())
        return;

    QRectF updateRect;

    d->lazyLayoutStepSize = 1000;
    d->sizeChangedTimer.stop();
    d->insideDocumentChange = true;

    const int documentLength = document()->docHandle()->length();
    const bool fullLayout = (oldLength == 0 && length == documentLength);
    const bool smallChange = documentLength > 0
                             && (qMax(length, oldLength) * 100 / documentLength) < 5;

    // don't show incremental layout progress (avoid scroll bar flicker)
    // if we see only a small change in the document and we're either starting
    // a layout run or we're already in progress for that and we haven't seen
    // any bigger change previously (showLayoutProgress already false)
    if (smallChange
        && (d->currentLazyLayoutPosition == -1 || d->showLayoutProgress == false))
        d->showLayoutProgress = false;
    else
        d->showLayoutProgress = true;

    if (fullLayout) {
        d->currentLazyLayoutPosition = 0;
        d->checkPoints.clear();
        d->layoutStep();
    } else {
        d->ensureLayoutedByPosition(from);
        updateRect = doLayout(from, oldLength, length);
    }

    if (!d->layoutTimer.isActive() && d->currentLazyLayoutPosition != -1)
        d->layoutTimer.start(10, this);

    d->insideDocumentChange = false;

    if (d->showLayoutProgress) {
        const QSizeF newSize = dynamicDocumentSize();
        if (newSize != d->lastReportedSize) {
            d->lastReportedSize = newSize;
            emit documentSizeChanged(newSize);
        }
    }

    if (!updateRect.isValid()) {
        // don't use the frame size, it might have shrunken
        updateRect = QRectF(QPointF(0, 0), QSizeF(INT_MAX, INT_MAX));
    }

    emit update(updateRect);
}

QRectF QTextDocumentLayout::doLayout(int from, int oldLength, int length)
{
    Q_D(QTextDocumentLayout);

//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(document()->rootFrame(), from, oldLength, length);

    QRectF updateRect;

    QTextFrame *root = document()->rootFrame();
    if(data(root)->sizeDirty)
        updateRect = d->layoutFrame(root, from, from + length);
    data(root)->layoutDirty = false;

    if (d->currentLazyLayoutPosition == -1)
        layoutFinished();
    else if (d->showLayoutProgress)
        d->sizeChangedTimer.start(0, this);

    return updateRect;
}

int QTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayouted(QFixed::fromReal(point.y()));
    QTextFrame *f = document()->rootFrame();
    int position = 0;
    QTextLayout *l = 0;
    QFixedPoint pointf;
    pointf.x = QFixed::fromReal(point.x());
    pointf.y = QFixed::fromReal(point.y());
    QTextDocumentLayoutPrivate::HitPoint p = d->hitTest(f, pointf, &position, &l, accuracy);
    if (accuracy == Qt::ExactHit && p < QTextDocumentLayoutPrivate::PointExact)
        return -1;

    // ensure we stay within document bounds
    int lastPos = f->lastPosition();
    if (l && !l->preeditAreaText().isEmpty())
        lastPos += l->preeditAreaText().length();
    if (position > lastPos)
        position = lastPos;
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
        data(frame)->size = QFixedSize::fromSizeF(intrinsic);
    }

    QSizeF inlineSize = (pos == QTextFrameFormat::InFlow ? intrinsic : QSizeF(0, 0));
    item.setWidth(inlineSize.width());
    if (f.verticalAlignment() == QTextCharFormat::AlignMiddle) {
        item.setDescent(inlineSize.height() / 2);
        item.setAscent(inlineSize.height() / 2);
    } else {
        item.setDescent(0);
        item.setAscent(inlineSize.height());
    }
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
    if (frame && frame->frameFormat().position() != QTextFrameFormat::InFlow)
        return; // don't draw floating frames from inline objects here but in drawFlow instead

//    qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawInlineObject(p, rect, item, posInDocument, format);
}

int QTextDocumentLayout::dynamicPageCount() const
{
    const QSizeF pgSize = document()->pageSize();
    if (pgSize.height() < 0)
        return 1;
    return (int)(dynamicDocumentSize().height()
                 / pgSize.height()) + 1;
}

QSizeF QTextDocumentLayout::dynamicDocumentSize() const
{
    return data(document()->rootFrame())->size.toSizeF();
}

int QTextDocumentLayout::pageCount() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return dynamicPageCount();
}

QSizeF QTextDocumentLayout::documentSize() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return dynamicDocumentSize();
}

void QTextDocumentLayoutPrivate::ensureLayouted(QFixed y) const
{
    Q_Q(const QTextDocumentLayout);
    if (currentLazyLayoutPosition == -1)
        return;
    const QSizeF oldSize = q->dynamicDocumentSize();

    if (checkPoints.isEmpty())
        layoutStep();

    while (currentLazyLayoutPosition != -1
           && checkPoints.last().y < y)
        layoutStep();
}

void QTextDocumentLayoutPrivate::ensureLayoutedByPosition(int position) const
{
    if (currentLazyLayoutPosition == -1)
        return;
    if (position < currentLazyLayoutPosition)
        return;
    while (currentLazyLayoutPosition != -1
           && currentLazyLayoutPosition < position) {
        const_cast<QTextDocumentLayout *>(q_func())->doLayout(currentLazyLayoutPosition, 0, INT_MAX - currentLazyLayoutPosition);
    }
}

void QTextDocumentLayoutPrivate::layoutStep() const
{
    ensureLayoutedByPosition(currentLazyLayoutPosition + lazyLayoutStepSize);
    lazyLayoutStepSize = qMin(200000, lazyLayoutStepSize * 2);
}

void QTextDocumentLayout::setCursorWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->cursorWidth = width;
}

int QTextDocumentLayout::cursorWidth() const
{
    Q_D(const QTextDocumentLayout);
    return d->cursorWidth;
}

void QTextDocumentLayout::setFixedColumnWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->fixedColumnWidth = width;
}

QRectF QTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return d->frameBoundingRectInternal(frame);
}

QRectF QTextDocumentLayoutPrivate::frameBoundingRectInternal(QTextFrame *frame) const
{
    QPointF pos;
    const int framePos = frame->firstPosition();
    QTextFrame *f = frame;
    while (f) {
        QTextFrameData *fd = data(f);
        pos += fd->position.toPointF();

        if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
            QTextTableCell cell = table->cellAt(framePos);
            if (cell.isValid())
                pos += static_cast<QTextTableData *>(fd)->cellPosition(cell.row(), cell.column()).toPointF();
        }

        f = f->parentFrame();
    }
    return QRectF(pos, data(frame)->size.toSizeF());
}

QRectF QTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    d_func()->ensureLayoutedByPosition(block.position() + block.length());
    QTextFrame *frame = document()->frameAt(block.position());
    QPointF offset;
    const int blockPos = block.position();

    while (frame) {
        QTextFrameData *fd = data(frame);
        offset += fd->position.toPointF();

        if (QTextTable *table = qobject_cast<QTextTable *>(frame)) {
            QTextTableCell cell = table->cellAt(blockPos);
            if (cell.isValid())
                offset += static_cast<QTextTableData *>(fd)->cellPosition(cell.row(), cell.column()).toPointF();
        }

        frame = frame->parentFrame();
    }

    const QTextLayout *layout = block.layout();
    QRectF rect = layout->boundingRect();
    rect.moveTopLeft(layout->position() + offset);
    return rect;
}

int QTextDocumentLayout::layoutStatus() const
{
    int pos = d_func()->currentLazyLayoutPosition;
    if (pos == -1)
        return 100;
    return pos * 100 / document()->docHandle()->length();
}

void QTextDocumentLayout::timerEvent(QTimerEvent *e)
{
    Q_D(QTextDocumentLayout);
    if (e->timerId() == d->layoutTimer.timerId()) {
        if (d->currentLazyLayoutPosition != -1)
            d->layoutStep();
    } else if (e->timerId() == d->sizeChangedTimer.timerId()) {
        d->lastReportedSize = dynamicDocumentSize();
        emit documentSizeChanged(d->lastReportedSize);
        d->sizeChangedTimer.stop();

        if (d->currentLazyLayoutPosition == -1) {
            const int newCount = dynamicPageCount();
            if (newCount != d->lastPageCount) {
                d->lastPageCount = newCount;
                emit pageCountChanged(newCount);
            }
        }
    } else {
        QAbstractTextDocumentLayout::timerEvent(e);
    }
}

void QTextDocumentLayout::layoutFinished()
{
    Q_D(QTextDocumentLayout);
    d->layoutTimer.stop();
    if (!d->insideDocumentChange)
        d->sizeChangedTimer.start(0, this);
    // reset
    d->showLayoutProgress = true;
}

void QTextDocumentLayout::ensureLayouted(qreal y)
{
    d_func()->ensureLayouted(QFixed::fromReal(y));
}

qreal QTextDocumentLayout::idealWidth() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return d->idealWidth;
}

qreal QTextDocumentLayoutPrivate::scaleToDevice(qreal value) const
{
    QPaintDevice *dev = q_func()->paintDevice();
    if (!dev)
        return value;
    extern int qt_defaultDpi();
    return value * dev->logicalDpiY() / qreal(qt_defaultDpi());
}

QFixed QTextDocumentLayoutPrivate::scaleToDevice(QFixed value) const
{
    QPaintDevice *dev = q_func()->paintDevice();
    if (!dev)
        return value;
    extern int qt_defaultDpi();
    return value * QFixed(dev->logicalDpiY()) / QFixed(qt_defaultDpi());
}

#include "moc_qtextdocumentlayout_p.cpp"
