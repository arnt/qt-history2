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

class QTextFrameData : public QTextFrameLayoutData
{
public:
    // relative to parent frame
    QRect boundingRect;

    // contents starts at (margin+border+padding/margin+border+padding)
    int margin;
    int border;
    int padding;
    int contentsWidth;
    int contentsHeight;

    QTextFrameFormat::Position position;
    QList<QTextFrame *> layoutedFrames;

    bool dirty;
};

struct LayoutStruct {
    QTextFrame *frame;
    int y;
};

inline QTextFrameData *data(QTextFrame *f)
{
    QTextFrameData *data = static_cast<QTextFrameData *>(f->layoutData());
    if (!data) {
        data = new QTextFrameData;
        f->setLayoutData(data);
    }
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

enum {
    TextIndentValue = 40
};

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout)
public:

#if 0
    struct Page {
        QTextBlockIterator first;
        QTextBlockIterator last;
    };
    QList<Page> pages;
#endif

    QTextDocumentLayoutPrivate()
        : widthUsed(0)
    { }

    QSize pageSize;
    bool pagedLayout;
    mutable QTextBlockIterator currentBlock;

    int widthUsed;
    int blockTextFlags;
#ifdef LAYOUT_DEBUG
    mutable QString debug_indent;
#endif

    int indent(QTextBlockIterator bl) const;

    void drawFrame(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextFrame *f) const;
    void drawBlock(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextBlockIterator bl) const;
    void drawListItem(const QPoint &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                      QTextBlockIterator bl, const QTextLayout::Selection &selection) const;

    int hitTest(QTextFrame *frame, const QPoint &point, QText::HitTestAccuracy accuracy) const;
    int hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const;

    void relayoutDocument();
    void layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo);
    void layoutFlow(LayoutStruct *layoutStruct, int from, int to);
    void layoutBlock(const QTextBlockIterator block, LayoutStruct *layoutStruct);


    void floatMargins(LayoutStruct *layoutStruct, int *left, int *right);
    void findY(LayoutStruct *layoutStruct, int requiredWidth);
};

#define d d_func()
#define q q_func()

int QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextFrameData *fd = data(frame);

    LDEBUG << "checking frame" << frame->startPosition() << "point=" << point;
    if (!fd->boundingRect.contains(point) && frame != q->rootFrame()) {
        LDEBUG << "outside";
        return -1;
    }
    INC_INDENT;

    QPoint p = point - fd->boundingRect.topLeft();

    QTextBlockIterator it = q->findBlock(frame->startPosition());
    QTextBlockIterator end = q->findBlock(frame->endPosition()+1);

    QList<QTextFrame *> children = frame->children();
    int pos = -1;
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextBlockIterator s = q->findBlock(c->startPosition());
        while (it != s) {
            pos = hitTest(it, p, accuracy);
            if (pos != -1)
                goto end;
            ++it;
        }
        pos = hitTest(c, p, accuracy);
        if (pos != -1)
            goto end;
        it = q->findBlock(c->endPosition()+1);
    }
    while (it != end) {
        pos = hitTest(it, p, accuracy);
        if (pos != -1)
            goto end;
        ++it;
    }
 end:
    DEC_INDENT;
    if (pos == -1 && accuracy == QText::FuzzyHit) {
        int p = frame->endPosition();
        QTextBlockIterator it = q->findBlock(frame->endPosition());
        if (it == q->end())
            --it;
        QRect r = it.layout()->rect();
        QPoint relative(point.x(), r.bottom() - 1);

        pos = d->hitTest(it, relative, accuracy);

        if (pos == -1)
            pos = p;
    }

    return pos;
}

int QTextDocumentLayoutPrivate::hitTest(QTextBlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    const QTextLayout *tl = bl.layout();
    QRect textrect = tl->rect();
    if (!textrect.contains(point))
        return -1;

    LDEBUG << "    block" << bl.position() << "point=" << point;
    QPoint pos = point - textrect.topLeft();

    QTextBlockFormat blockFormat = bl.blockFormat();

    // ### rtl?

    int textStartPos = blockFormat.leftMargin() + indent(bl);

    LDEBUG << "    x=" << pos.x() << "textStartPos=" << textStartPos;
    // ###### Use per line testing
    if (pos.x() < textStartPos) {
        if (accuracy == QText::ExactHit)
            return -1;

        return bl.position();
    }

    for (int i = 0; i < tl->numLines(); ++i) {
        QTextLine line = tl->lineAt(i);

        if (line.rect().contains(pos))
            return bl.position() + line.xToCursor(point.x());
    }

    return -1;
}

// ### could be moved to QTextBlockIterator
int QTextDocumentLayoutPrivate::indent(QTextBlockIterator bl) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    int indent = blockFormat.indent();

    QTextObject *object = q->objectForFormat(blockFormat);
    if (object)
        indent += object->format().toListFormat().indent();

    return indent * TextIndentValue;
}

void QTextDocumentLayoutPrivate::drawFrame(const QPoint &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextFrame *frame) const
{
    QTextFrameData *fd = data(frame);
    if (!fd->boundingRect.intersects(painter->clipRegion().boundingRect()))
        return;
//     LDEBUG << debug_indent << "drawFrame" << frame->startPosition() << "--" << frame->endPosition() << "at" << offset;
//     INC_INDENT;

    QPoint off = offset + fd->boundingRect.topLeft();

    // draw frame decoration
    if (fd->border) {
        painter->save();
        painter->setBrush(black);
        painter->setPen(NoPen);
        int w = fd->contentsWidth + 2*fd->padding + fd->border;
        int h = fd->contentsHeight + 2*fd->padding + fd->border;
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin, fd->border, h);
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin, w, fd->border);
        painter->drawRect(off.x() + fd->margin + w, off.y() + fd->margin, fd->border, h);
        painter->drawRect(off.x() + fd->margin, off.y() + fd->margin + h, w + fd->border, fd->border);
        painter->restore();
    }

    QTextBlockIterator it = q->findBlock(frame->startPosition());
    QTextBlockIterator end = q->findBlock(frame->endPosition()+1);

    QList<QTextFrame *> children = frame->children();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextBlockIterator s = q->findBlock(c->startPosition());
        while (it != s) {
            drawBlock(off, painter, context, it);
            ++it;
        }
        drawFrame(off, painter, context, children.at(i));
        it = q->findBlock(c->endPosition()+1);
    }
    while (it != end) {
        drawBlock(off, painter, context, it);
        ++it;
    }

//     DEC_INDENT;
}

void QTextDocumentLayoutPrivate::drawBlock(const QPoint &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextBlockIterator bl) const
{
//     LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset;
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

    QTextObject *object = q->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(offset, painter, context, bl, s);

    if (tl->numLines() == 0) {

        QPoint pos = tl->rect().topLeft() + offset;

        pos.rx() += blockFormat.leftMargin() + indent(bl);
        pos.ry() += blockFormat.topMargin();

        if (cursor != -1)
            painter->drawLine(pos.x(), pos.y(),
                              pos.x(), pos.y() + QFontMetrics(bl.charFormat().font()).height());
        return;
    }

    const_cast<QTextLayout *>(tl)->setPalette(context.palette,
                                              context.textColorFromPalette ? QTextLayout::UseTextColor : QTextLayout::None);

    tl->draw(painter, offset, cursor, &s, nSel, painter->clipRegion().boundingRect());
}


void QTextDocumentLayoutPrivate::drawListItem(const QPoint &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlockIterator bl, const QTextLayout::Selection &selection) const
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextCharFormat charFormat = bl.charFormat();
    QFontMetrics fontMetrics(charFormat.font());
    QTextObject *object = q->objectForFormat(blockFormat);
    QTextListFormat lf = object->format().toListFormat();
    const int style = lf.style();
    QString itemText;
    QPoint pos = bl.layout()->rect().topLeft() + offset;
    QSize size;

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


void QTextDocumentLayoutPrivate::relayoutDocument()
{
    widthUsed = 0;
    q->documentChange(0, 0, q->end().position() + q->end().length());
}

void QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo)
{
    Q_ASSERT(data(f)->dirty);
//     qDebug("layouting frame (%d--%d)", f->startPosition(), f->endPosition());

    QTextFrameData *fd = data(f);
    QTextFrameFormat fformat = f->format();

    {
        // set sizes of this frame from the format
        QTextFrame *parent = f->parent();
        QTextFrameData *pd = parent ? data(parent) : 0;
        fd->margin = fformat.margin();
        fd->border = fformat.border();
        fd->padding = fformat.padding();

        int width = fformat.width();
        if (width == -1)
            width = pd ? pd->contentsWidth : pageSize.width();
        fd->contentsWidth = width - 2*(fd->margin + fd->border + fd->padding);

        int height = fformat.height();
        if (height == -1)
            height = pd ? pd->contentsHeight : -1;
        if (height != -1) {
            fd->contentsHeight = height - 2*(fd->margin + fd->border + fd->padding);
        } else {
            fd->contentsHeight = height;
        }

        fd->position = fformat.position();
    }

    int startPos = f->startPosition();
    int endPos = f->endPosition();
    if (startPos > endPos) {
        // inline image
        data(f)->dirty = false;
        return;
    }

    bool fullLayout = false;

    // layout child frames
    QList<QTextFrame *> children = f->children();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = data(c);
        QRect oldBr = cd->boundingRect;
        if (cd->dirty) {
            layoutFrame(c, layoutFrom, layoutTo);
            // removes all floats that need repositioning
            if (0 /*!cd->position == QTextFrameFormat::Absolute*/) {
                if (oldBr != cd->boundingRect)
                    fullLayout = true;
            } else {
                fd->layoutedFrames.removeAll(c);
            }
        }
    }
    // #### for now
    fullLayout = true;

    int margin = fd->margin + fd->border + fd->padding;
    LayoutStruct layoutStruct;
    layoutStruct.frame = f;
    layoutStruct.y = margin; // #### fix for !fullLayout

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

    int pos = startPos;

    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        int c_start = c->startPosition();
        int c_end = c->endPosition();

        if (c_start > pos)
            layoutFlow(&layoutStruct, pos, c_start);

        if (!fd->layoutedFrames.contains(c)) {
            QTextFrameData *cd = data(c);
            if (cd->position == QTextFrameFormat::InFlow) {
                cd->boundingRect.moveTopLeft(QPoint(margin, layoutStruct.y));
                layoutStruct.y += cd->boundingRect.height();
            } else {
                int left, right;
                floatMargins(&layoutStruct, &left, &right);
                if (cd->position == QTextFrameFormat::FloatLeft)
                    cd->boundingRect.moveTopLeft(QPoint(left, layoutStruct.y));
                else
                    cd->boundingRect.moveTopRight(QPoint(right, layoutStruct.y));
                fd->layoutedFrames.append(c);
            }
        }

        pos = c_end + 1;
    }
    if (endPos > pos)
        layoutFlow(&layoutStruct, pos, endPos);

    int height = fd->contentsHeight == -1 ? layoutStruct.y + margin : fd->contentsHeight + 2*margin;
    fd->boundingRect = QRect(0, 0, fd->contentsWidth + 2*margin, height);
    fd->dirty = false;
}

void QTextDocumentLayoutPrivate::layoutFlow(LayoutStruct *layoutStruct, int from, int to)
{
//     qDebug("layoutFlow (%d--%d)", from, to);
    QTextBlockIterator it = q->findBlock(from);
    QTextBlockIterator end = q->findBlock(to+1);

    while (it != end) {
//         qDebug("layouting block at pos %d", it.position());
        layoutBlock(it, layoutStruct);
        ++it;
    }
}

void QTextDocumentLayoutPrivate::layoutBlock(QTextBlockIterator bl, LayoutStruct *layoutStruct)
{
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();
    currentBlock = bl;

    tl->setTextFlags(blockFormat.alignment()|d->blockTextFlags);
    tl->useDesignMetrics(true);
//     tl->enableKerning(true);
    tl->clearLines();

    int cy = layoutStruct->y;
    layoutStruct->y += blockFormat.topMargin();

    QTextFrameData *fd = data(layoutStruct->frame);

    int frameMargin = fd->margin + fd->border + fd->padding;
    int l = blockFormat.leftMargin() + indent(bl);
    int r = fd->contentsWidth + 2*frameMargin - blockFormat.rightMargin();

    tl->setPosition(QPoint(0, cy));

    while (1) {
        QTextLine line = tl->createLine();
        if (!line.isValid())
            break;

        int left, right;
        floatMargins(layoutStruct, &left, &right);

        left = qMax(left, l);
        right = qMin(right, r);
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;
        line.layout(right-left);

        floatMargins(layoutStruct, &left, &right);
        left = qMax(left, l);
        right = qMax(right, r);

        if (line.width() > right-left) {
            // float has been added in the meantime, redo
            line.layout(right-left);
//             qDebug() << "    redo: left=" << left << " right=" << right;
            if (line.textWidth() > right-left) {
                // lines min width more than what we have
                findY(layoutStruct, line.textWidth());
                floatMargins(layoutStruct, &left, &right);
                line.layout(right-left);
            }
        }

        line.setPosition(QPoint(left, layoutStruct->y - cy));
        layoutStruct->y += line.ascent() + line.descent() + 1;
        widthUsed = qMax(widthUsed, line.textWidth());
    }

    layoutStruct->y += blockFormat.bottomMargin();
}

void QTextDocumentLayoutPrivate::floatMargins(LayoutStruct *layoutStruct, int *left, int *right)
{
    QTextFrameData *fd = data(layoutStruct->frame);
    *left = fd->margin + fd->border + fd->padding;
    *right = *left + fd->contentsWidth;;
    QList<QTextFrame *> frames = fd->layoutedFrames;
    for (int i = 0; i < frames.size(); ++i) {
        QTextFrame *f = frames.at(i);
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
    requiredWidth = qMin(requiredWidth, data(layoutStruct->frame)->contentsWidth);

//     qDebug() << "findY:";
    while (1) {
        floatMargins(layoutStruct, &left, &right);
//         qDebug() << "    right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        int newY = INT_MAX;
        QList<QTextFrame *> frames = data(layoutStruct->frame)->layoutedFrames;
        for (int i = 0; i < frames.size(); ++i) {
            QTextFrame *f = frames.at(i);
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


QTextDocumentLayout::QTextDocumentLayout()
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate)
{
    d->blockTextFlags = Qt::IncludeTrailingSpaces|Qt::WordBreak;

    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QTextFrame *frame = rootFrame();
    d->drawFrame(QPoint(), painter, context, frame);
}

static void markFrames(QTextFrame *current, int start, int end)
{
    if (current->startPosition() > end || current->endPosition() < start)
        return;
    data(current)->dirty = true;
//     qDebug("    marking frame (%d--%d) as dirty", current->startPosition(), current->endPosition());
    QList<QTextFrame *> children = current->children();
    for (int i = 0; i < children.size(); ++i)
        markFrames(children.at(i), start, end);
}

void QTextDocumentLayout::documentChange(int from, int oldLength, int length)
{
    Q_UNUSED(oldLength);
//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(rootFrame(), from, from + length);

    d->layoutFrame(rootFrame(), from, from + length);
}

int QTextDocumentLayout::hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const
{
    QTextFrame *f = rootFrame();
    return d->hitTest(f, point, accuracy);
}

void QTextDocumentLayout::setSize(QTextInlineObject item, const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;
    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(f));
    if (frame)
        pos = frame->format().position();

    item.setDescent(0);
    QSize inlineSize = (pos == QTextFrameFormat::InFlow ? handler.iface->intrinsicSize(format) : QSize(0, 0));
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
    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(f));
    if (frame)
        pos = frame->format().position();
    if (pos == QTextFrameFormat::InFlow)
        return;
#if 0
    if (d->floats.contains(obj))
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

    d->floats[obj] = fl;
#endif
}

void QTextDocumentLayout::drawObject(QPainter *p, const QRect &rect, QTextInlineObject item,
                                     const QTextFormat &format, QTextLayout::SelectionType selType)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(f));
    QRect r = rect;
    if (frame) {
#if 0
        pos = frame->format().position();
        if (frame != QTextFrameFormat::None) {
            const QTextDocumentLayoutPrivate::Float &f = d->floats.value(obj);
            r = f.rect;
            r.moveBy(f.block.layout()->position());
        }
#endif
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


int QTextDocumentLayout::widthUsed() const
{
    return d->widthUsed;
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
    if (widthUsed() != 0) {
        w = qt_int_sqrt(5 * totalHeight() * widthUsed() / 3);
        setPageSize(QSize(qMin(w, mw), INT_MAX));

        if (w*3 < 5*totalHeight()) {
            w = qt_int_sqrt(2 * totalHeight() * widthUsed());
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

