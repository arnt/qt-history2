/****************************************************************************
**
** Implementation of layout classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlayout.h"

#ifndef QT_NO_LAYOUT

#include "qapplication.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsizepolicy.h"
#include "qvector.h"

#include "qlayoutengine_p.h"

/*
  Three internal classes related to QGridLayout: (1) QGridBox is a
  QLayoutItem with (row, column) information; (2) QGridMultiBox is a
  QGridBox with (torow, tocolumn) information; (3) QGridLayoutData is
  the internal representation of a QGridLayout.
*/

class QGridBox
{
public:
    QGridBox(QLayoutItem *lit) { item_ = lit; }

    QGridBox(QWidget *wid) { item_ = new QWidgetItem(wid); }
    QGridBox(int w, int h, QSizePolicy::SizeType hData = QSizePolicy::Minimum,
              QSizePolicy::SizeType vData = QSizePolicy::Minimum)
    { item_ = new QSpacerItem(w, h, hData, vData); }
    ~QGridBox() { delete item_; }

    QSize sizeHint() const { return item_->sizeHint(); }
    QSize minimumSize() const { return item_->minimumSize(); }
    QSize maximumSize() const { return item_->maximumSize(); }
    QSizePolicy::ExpandData expanding() const { return item_->expanding(); }
    bool isEmpty() const { return item_->isEmpty(); }

    bool hasHeightForWidth() const { return item_->hasHeightForWidth(); }
    int heightForWidth(int w) const { return item_->heightForWidth(w); }

    void setAlignment(Qt::Alignment a) { item_->setAlignment(a); }
    void setGeometry(const QRect &r) { item_->setGeometry(r); }
    Qt::Alignment alignment() const { return item_->alignment(); }
    QLayoutItem *item() { return item_; }
    QLayoutItem *takeItem() { QLayoutItem *i = item_; item_ = 0; return i; }

    int hStretch() { return item_->widget() ?
                         item_->widget()->sizePolicy().horStretch() : 0; }
    int vStretch() { return item_->widget() ?
                         item_->widget()->sizePolicy().verStretch() : 0; }

private:
    friend class QGridLayoutData;
    friend class QGridLayoutDataIterator;

    QLayoutItem *item_;
    int row, col;
};

class QGridMultiBox
{
public:
    QGridMultiBox(QGridBox *box, int toRow, int toCol)
        : box_(box), torow(toRow), tocol(toCol) { }
    ~QGridMultiBox() { delete box_; }
    QGridBox *box() { return box_; }
    QLayoutItem *takeItem() { return box_->takeItem(); }

private:
    friend class QGridLayoutData;
    friend class QGridLayoutDataIterator;

    QGridBox *box_;
    int torow, tocol;
};

class QGridLayoutData
{
public:
    QGridLayoutData();
    QGridLayoutData(int nRows, int nCols);
    ~QGridLayoutData();

    void add(QGridBox*, int row, int col);
    void add(QGridBox*, int row1, int row2, int col1, int col2);
    QSize sizeHint(int) const;
    QSize minimumSize(int) const;
    QSize maximumSize(int) const;

    QSizePolicy::ExpandData expanding(int spacing);

    void distribute(QRect, int);
    inline int numRows() const { return rr; }
    inline int numCols() const { return cc; }
    inline void expand(int rows, int cols)
        { setSize(qMax(rows, rr), qMax(cols, cc)); }
    inline void setRowStretch(int r, int s)
        { expand(r + 1, 0); rStretch[r] = s; setDirty(); }
    inline void setColStretch(int c, int s)
        { expand(0, c + 1); cStretch[c] = s; setDirty(); }
    inline int rowStretch(int r) const { return rStretch[r]; }
    inline int colStretch(int c) const { return cStretch[c]; }
    inline void setRowSpacing(int r, int s)
        { expand(r + 1, 0); rSpacing[r] = s; setDirty(); }
    inline void setColSpacing(int c, int s)
        { expand(0, c + 1); cSpacing[c] = s; setDirty(); }
    inline int rowSpacing(int r) const { return rSpacing[r]; }
    inline int colSpacing(int c) const { return cSpacing[c]; }

    inline void setReversed(bool r, bool c) { hReversed = c; vReversed = r; }
    inline bool horReversed() const { return hReversed; }
    inline bool verReversed() const { return vReversed; }
    inline void setDirty() { needRecalc = true; hfw_width = -1; }
    inline bool isDirty() const { return needRecalc; }
    bool hasHeightForWidth(int space);
    int heightForWidth(int, int, int);
    int minimumHeightForWidth(int, int, int);

    bool findWidget(QWidget* w, int *row, int *col);

    inline void getNextPos(int &row, int &col) { row = nextR; col = nextC; }
    inline int count() const { return things.count() + multi.count(); }
    QRect cellGeometry(int row, int col) const;

    inline QLayoutItem *itemAt(int idx) {
        if (idx < things.count())
            return things.at(idx)->item();
        else if (idx - things.count() < multi.count())
            return multi.at(idx - things.count())->box()->item();
        else
            return 0;
    }
    
    inline QLayoutItem *takeAt(int idx) {
        QLayoutItem *item = 0;
        if (idx < things.count()) {
            QGridBox *b = things.takeAt(idx);
            if (b) {
                item = b->takeItem();
                delete b;
            }
        } else if (idx - things.count() < multi.count()) {
            QGridMultiBox *b = multi.takeAt(idx -things.count());
            if (b) {
                item = b->takeItem();
                delete b;
            }
        } 
        return item;
    }
    
private:
    void setNextPosAfter(int r, int c);
    void recalcHFW(int w, int s);
    void addHfwData (QGridBox *box, int width);
    void init();
    QSize findSize(QCOORD QLayoutStruct::*, int) const;
    void addData(QGridBox *b, bool r = true, bool c = true);
    void setSize(int rows, int cols);
    void setupLayoutData(int space);
    void setupHfwLayoutData(int space);

    int rr;
    int cc;
    QVector<QLayoutStruct> rowData;
    QVector<QLayoutStruct> colData;
    QVector<QLayoutStruct> *hfwData;
    QVector<int> rStretch;
    QVector<int> cStretch;
    QVector<int> rSpacing;
    QVector<int> cSpacing;
    QList<QGridBox *> things;
    QList<QGridMultiBox *> multi;

    int hfw_width;
    int hfw_height;
    int hfw_minheight;
    int nextR;
    int nextC;

    uint hReversed        : 1;
    uint vReversed        : 1;
    uint needRecalc        : 1;
    uint has_hfw        : 1;
    uint addVertical        : 1;

    friend class QGridLayoutDataIterator;
};

QGridLayoutData::QGridLayoutData()
{
    init();
}

QGridLayoutData::QGridLayoutData(int nRows, int nCols)
    : rowData(0), colData(0)
{
    init();
    if (nRows  < 0) {
        nRows = 1;
        addVertical = false;
    }
    if (nCols  < 0) {
        nCols = 1;
        addVertical = true;
    }
    setSize(nRows, nCols);
}

QGridLayoutData::~QGridLayoutData()
{
    while (!things.isEmpty())
        delete things.takeFirst();
    while (!multi.isEmpty())
        delete multi.takeFirst();
    delete hfwData;
}

void QGridLayoutData::init()
{
    addVertical = false;
    setDirty();
    rr = cc = 0;
    nextR = nextC = 0;
    hfwData = 0;
    hReversed = false;
    vReversed = false;
}

bool QGridLayoutData::hasHeightForWidth(int spacing)
{
    setupLayoutData(spacing);
    return has_hfw;
}

/*
  Assumes that setupLayoutData() has been called, and that
  qGeomCalc() has filled in colData with appropriate values.
*/
void QGridLayoutData::recalcHFW(int w, int spacing)
{
    /*
      Go through all children, using colData and heightForWidth()
      and put the results in hfw_rowData.
    */
    if (!hfwData)
        hfwData = new QVector<QLayoutStruct>(rr);
    setupHfwLayoutData(spacing);
    QVector<QLayoutStruct> &rData = *hfwData;

    int h = 0;
    int mh = 0;
    int n = 0;
    for (int r = 0; r < rr; r++) {
        h += rData[r].sizeHint;
        mh += rData[r].minimumSize;
        if (!rData[r].empty)
            n++;
    }
    if (n) {
        h += (n - 1) * spacing;
        mh += (n - 1) * spacing;
    }

    hfw_width = w;
    hfw_height = qMin(QLAYOUTSIZE_MAX, h);
    hfw_minheight = qMin(QLAYOUTSIZE_MAX, h);
}

int QGridLayoutData::heightForWidth(int w, int margin, int spacing)
{
    setupLayoutData(spacing);
    if (!has_hfw)
        return -1;
    if (w + 2*margin != hfw_width) {
        qGeomCalc(colData, 0, cc, 0, w+2*margin, spacing);
        recalcHFW(w+2*margin, spacing);
    }
    return hfw_height + 2*margin;
}

int QGridLayoutData::minimumHeightForWidth(int w, int margin, int spacing)
{
    (void) heightForWidth(w, margin, spacing);
    return has_hfw ? (hfw_minheight + 2*margin) : -1;
}

bool QGridLayoutData::findWidget(QWidget* w, int *row, int *col)
{
    int i;

    for (i = 0; i < things.size(); ++i) {
        QGridBox * box = things.at(i);
        if (box->item()->widget() == w) {
            if (row)
                *row = box->row;
            if (col)
                *col = box->col;
            return true;
        }
    }
    for (i = 0; i < multi.size(); ++i) {
        QGridMultiBox * mbox = multi.at(i);
        QGridBox *box = mbox->box();
        if (box->item()->widget() == w) {
            if (row)
                *row = box->row;
            if (col)
                *col = box->col;
            return true;
        }
    }
    return false;
}

QSize QGridLayoutData::findSize(QCOORD QLayoutStruct::*size, int spacer) const
{
    QGridLayoutData *that = (QGridLayoutData *)this;
    that->setupLayoutData(spacer);

    int w = 0;
    int h = 0;
    int n = 0;
    for (int r = 0; r < rr; r++) {
        h = h + rowData[r].*size;
        if (!rowData[r].empty)
            n++;
    }
    if (n)
        h += (n - 1) * spacer;
    n = 0;
    for (int c = 0; c < cc; c++) {
        w = w + colData[c].*size;
        if (!colData[c].empty)
            n++;
    }
    if (n)
        w += (n - 1) * spacer;
    w = qMin(QLAYOUTSIZE_MAX, w);
    h = qMin(QLAYOUTSIZE_MAX, h);

    return QSize(w, h);
}

QSizePolicy::ExpandData QGridLayoutData::expanding(int spacing)
{
    setupLayoutData(spacing);
    int ret = 0;

    for (int r = 0; r < rr; r++) {
        if (rowData[r].expansive) {
            ret |= (int) QSizePolicy::Vertically;
            break;
        }
    }
    for (int c = 0; c < cc; c++) {
        if (colData[c].expansive) {
            ret |= (int) QSizePolicy::Horizontally;
            break;
        }
    }
    return (QSizePolicy::ExpandData) ret;
}

QSize QGridLayoutData::sizeHint(int spacer) const
{
    return findSize(&QLayoutStruct::sizeHint, spacer);
}

QSize QGridLayoutData::maximumSize(int spacer) const
{
    return findSize(&QLayoutStruct::maximumSize, spacer);
}

QSize QGridLayoutData::minimumSize(int spacer) const
{
    return findSize(&QLayoutStruct::minimumSize, spacer);
}

void QGridLayoutData::setSize(int r, int c)
{
    if ((int)rowData.size() < r) {
        int newR = qMax(r, rr * 2);
        rowData.resize(newR);
        rStretch.resize(newR);
        rSpacing.resize(newR);
        for (int i = rr; i < newR; i++) {
            rowData[i].init();
            rStretch[i] = 0;
            rSpacing[i] = 0;
        }
    }
    if ((int)colData.size() < c) {
        int newC = qMax(c, cc * 2);
        colData.resize(newC);
        cStretch.resize(newC);
        cSpacing.resize(newC);
        for (int i = cc; i < newC; i++) {
            colData[i].init();
            cStretch[i] = 0;
            cSpacing[i] = 0;
        }
    }

    if (hfwData && (int)hfwData->size() < r) {
        delete hfwData;
        hfwData = 0;
        hfw_width = -1;
    }
    rr = r;
    cc = c;
}

void QGridLayoutData::setNextPosAfter(int row, int col)
{
    if (addVertical) {
        if (col > nextC || col == nextC && row >= nextR) {
            nextR = row + 1;
            nextC = col;
            if (nextR >= rr) {
                nextR = 0;
                nextC++;
            }
        }
    } else {
        if (row > nextR || row == nextR && col >= nextC) {
            nextR = row;
            nextC = col + 1;
            if (nextC >= cc) {
                nextC = 0;
                nextR++;
            }
        }
    }
}

void QGridLayoutData::add(QGridBox *box, int row, int col)
{
    expand(row+1, col+1);
    box->row = row;
    box->col = col;
    things.append(box);
    setDirty();
    setNextPosAfter(row, col);
}

void QGridLayoutData::add(QGridBox *box, int row1, int row2, int col1,
                           int col2 )
{
    if (row2 >= 0 && row2 < row1)
        qWarning("QGridLayout: Multi-cell fromRow greater than toRow");
    if (col2 >= 0 && col2 < col1)
        qWarning("QGridLayout: Multi-cell fromCol greater than toCol");
    if (row1 == row2 && col1 == col2) {
        add(box, row1, col1);
        return;
    }
    expand(row2+1, col2+1);
    box->row = row1;
    box->col = col1;
    QGridMultiBox *mbox = new QGridMultiBox(box, row2, col2);
    multi.append(mbox);
    setDirty();
    if (col2 < 0)
        col2 = cc - 1;

    setNextPosAfter(row2, col2);
}

void QGridLayoutData::addData(QGridBox *box, bool r, bool c)
{
    QSize hint = box->sizeHint();
    QSize minS = box->minimumSize();
    QSize maxS = box->maximumSize();

    if (c) {
        if (!cStretch[box->col])
            colData[box->col].stretch = qMax(colData[box->col].stretch,
                                              box->hStretch());
        colData[box->col].sizeHint = qMax(hint.width(),
                                           colData[box->col].sizeHint);
        colData[box->col].minimumSize = qMax(minS.width(),
                                              colData[box->col].minimumSize);

        qMaxExpCalc(colData[box->col].maximumSize, colData[box->col].expansive,
                     maxS.width(),
                     box->expanding() & QSizePolicy::Horizontally);
    }
    if (r) {
        if (!rStretch[box->row])
            rowData[box->row].stretch = qMax(rowData[box->row].stretch,
                                              box->vStretch());
        rowData[box->row].sizeHint = qMax(hint.height(),
                                           rowData[box->row].sizeHint);
        rowData[box->row].minimumSize = qMax(minS.height(),
                                              rowData[box->row].minimumSize);

        qMaxExpCalc(rowData[box->row].maximumSize, rowData[box->row].expansive,
                     maxS.height(),
                     box->expanding() & QSizePolicy::Vertically);
    }
    if (box->isEmpty()) {
        if (box->item()->widget() != 0) {
            /*
              Hidden widgets should behave exactly the same as if
              there were no widgets at all in the cell. We could have
              QWidgetItem::maximumSize() to return
              QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX). However, that
              value is not suitable for QBoxLayouts. So let's fix it
              here.
            */
            if (c)
                colData[box->col].maximumSize = QLAYOUTSIZE_MAX;
            if (r)
                rowData[box->row].maximumSize = QLAYOUTSIZE_MAX;
        }
    } else {
        // Empty boxes (i.e. spacers) do not get borders. This is
        // hacky, but compatible.
        if (c)
            colData[box->col].empty = false;
        if (r)
            rowData[box->row].empty = false;
    }
}

static void distributeMultiBox(QVector<QLayoutStruct> &chain, int spacing,
                                int start, int end,
                                int minSize, int sizeHint,
                                QVector<int> &stretchArray, int stretch)
{
    int i;
    int w = 0;
    int wh = 0;
    int max = 0;
    for (i = start; i <= end; i++) {
        w += chain[i].minimumSize;
        wh += chain[i].sizeHint;
        max += chain[i].maximumSize;
        chain[i].empty = false;
        if (stretchArray[i] == 0)
            chain[i].stretch = qMax(chain[i].stretch,stretch);
    }
    w += spacing * (end - start);
    wh += spacing * (end - start);
    max += spacing * (end - start);

    if (max < minSize) { // implies w < minSize
        /*
          We must increase the maximum size of at least one of the
          items. qGeomCalc() will put the extra space in between the
          items. We must recover that extra space and put it
          somewhere. It does not really matter where, since the user
          can always specify stretch factors and avoid this code.
        */
        qGeomCalc(chain, start, end - start + 1, 0, minSize, spacing);
        int pos = 0;
        for (i = start; i <= end; i++) {
            int nextPos = (i == end) ? minSize - 1 : chain[i + 1].pos;
            int realSize = nextPos - pos;
            if (i != end)
                realSize -= spacing;
            if (chain[i].minimumSize < realSize)
                chain[i].minimumSize = realSize;
            if (chain[i].maximumSize < chain[i].minimumSize)
                chain[i].maximumSize = chain[i].minimumSize;
            pos = nextPos;
        }
    } else if (w < minSize) {
        qGeomCalc(chain, start, end - start + 1, 0, minSize, spacing);
        for (i = start; i <= end; i++) {
            if (chain[i].minimumSize < chain[i].size)
                chain[i].minimumSize = chain[i].size;
        }
    }

    if (wh < sizeHint) {
        qGeomCalc(chain, start, end - start + 1, 0, sizeHint, spacing);
        for (i = start; i <= end; i++) {
            if (chain[i].sizeHint < chain[i].size)
                chain[i].sizeHint = chain[i].size;
        }
    }
}

//#define QT_LAYOUT_DISABLE_CACHING

void QGridLayoutData::setupLayoutData(int spacing)
{
#ifndef QT_LAYOUT_DISABLE_CACHING
    if (!needRecalc)
        return;
#endif
    has_hfw = false;
    int i;

    for (i = 0; i < rr; i++)
        rowData[i].init(rStretch[i], rSpacing[i]);
    for (i = 0; i < cc; i++)
        colData[i].init(cStretch[i], cSpacing[i]);

    for (i = 0; i < things.size(); ++i) {
        QGridBox * box = things.at(i);
        addData(box);
        has_hfw = has_hfw || box->item()->hasHeightForWidth();
    }

    for (i = 0; i < multi.size(); ++i) {
        QGridMultiBox *mbox = multi.at(i);
        QGridBox *box = mbox->box();
        int r1 = box->row;
        int c1 = box->col;
        int r2 = mbox->torow;
        int c2 = mbox->tocol;
        if (r2 < 0)
            r2 = rr - 1;
        if (c2 < 0)
            c2 = cc - 1;

        QSize hint = box->sizeHint();
        QSize min = box->minimumSize();
        if (box->hasHeightForWidth())
            has_hfw = true;

        if (r1 == r2) {
            addData(box, true, false);
        } else {
            distributeMultiBox(rowData, spacing, r1, r2,
                                min.height(), hint.height(),
                                rStretch, box->vStretch());
        }
        if (c1 == c2) {
            addData(box, false, true);
        } else {
            distributeMultiBox(colData, spacing, c1, c2,
                                min.width(), hint.width(),
                                cStretch, box->hStretch());
        }
    }
    for (i = 0; i < rr; i++)
        rowData[i].expansive = rowData[i].expansive || rowData[i].stretch > 0;
    for (i = 0; i < cc; i++)
        colData[i].expansive = colData[i].expansive || colData[i].stretch > 0;

    needRecalc = false;
}

void QGridLayoutData::addHfwData(QGridBox *box, int width)
{
    QVector<QLayoutStruct> &rData = *hfwData;
    if (box->hasHeightForWidth()) {
        int hint = box->heightForWidth(width);
        rData[box->row].sizeHint = qMax(hint, rData[box->row].sizeHint);
        rData[box->row].minimumSize = qMax(hint, rData[box->row].minimumSize);
    } else {
        QSize hint = box->sizeHint();
        QSize minS = box->minimumSize();
        rData[box->row].sizeHint = qMax(hint.height(),
                                         rData[box->row].sizeHint);
        rData[box->row].minimumSize = qMax(minS.height(),
                                            rData[box->row].minimumSize);
    }
}

/*
  Similar to setupLayoutData(), but uses heightForWidth(colData)
  instead of sizeHint(). Assumes that setupLayoutData() and
  qGeomCalc(colData) has been called.
*/
void QGridLayoutData::setupHfwLayoutData(int spacing)
{
    QVector<QLayoutStruct> &rData = *hfwData;
    int i;
    for (i = 0; i < rr; i++) {
        rData[i] = rowData[i];
        rData[i].minimumSize = rData[i].sizeHint = 0;
    }
    for (i = 0; i < things.size(); ++i) {
        QGridBox * box = things.at(i);
        addHfwData(box, colData[box->col].size);
    }
    for (i = 0; i < multi.size(); ++i) {
        QGridMultiBox *mbox = multi.at(i);
        QGridBox *box = mbox->box();
        int r1 = box->row;
        int c1 = box->col;
        int r2 = mbox->torow;
        int c2 = mbox->tocol;
        if (r2 < 0)
            r2 = rr-1;
        if (c2 < 0)
            c2 = cc-1;
        int w = colData[c2].pos + colData[c2].size - colData[c1].pos;
        if (r1 == r2) {
            addHfwData(box, w);
        } else {
            QSize hint = box->sizeHint();
            QSize min = box->minimumSize();
            if (box->hasHeightForWidth()) {
                int hfwh = box->heightForWidth(w);
                if (hfwh > hint.height())
                    hint.setHeight(hfwh);
                if (hfwh > min.height())
                    min.setHeight(hfwh);
            }
            distributeMultiBox(rData, spacing, r1, r2,
                                min.height(), hint.height(),
                                rStretch, box->vStretch());
        }
    }
    for (i = 0; i < rr; i++)
        rData[i].expansive = rData[i].expansive || rData[i].stretch > 0;
}

void QGridLayoutData::distribute(QRect r, int spacing)
{
    bool visualHReversed = hReversed;
    if (QApplication::reverseLayout())
        visualHReversed = !visualHReversed;

    setupLayoutData(spacing);

    qGeomCalc(colData, 0, cc, r.x(), r.width(), spacing);
    QVector<QLayoutStruct> *rDataPtr;
    if (has_hfw) {
        recalcHFW(r.width(), spacing);
        qGeomCalc(*hfwData, 0, rr, r.y(), r.height(), spacing);
        rDataPtr = hfwData;
    } else {
        qGeomCalc(rowData, 0, rr, r.y(), r.height(), spacing);
        rDataPtr = &rowData;
    }
    QVector<QLayoutStruct> &rData = *rDataPtr;
    int i;

    for (i = 0; i < things.size(); ++i) {
        QGridBox * box = things.at(i);
        int x = colData[box->col].pos;
        int y = rData[box->row].pos;
        int w = colData[box->col].size;
        int h = rData[box->row].size;
        if (visualHReversed)
            x = r.left() + r.right() - x - w + 1;
        if (vReversed)
            y = r.top() + r.bottom() - y - h + 1;
        box->setGeometry(QRect(x, y, w, h));
    }
    for (i = 0; i < multi.size(); ++i) {
        QGridMultiBox *mbox = multi.at(i);
        QGridBox *box = mbox->box();
        int r2 = mbox->torow;
        int c2 = mbox->tocol;
        if (r2 < 0)
            r2 = rr-1;
        if (c2 < 0)
            c2 = cc-1;

        int x = colData[box->col].pos;
        int y = rData[box->row].pos;
        int x2p = colData[c2].pos + colData[c2].size; // x2+1
        int y2p = rData[r2].pos + rData[r2].size;    // y2+1
        int w = x2p - x;
        int h = y2p - y;
        // this code is copied from above:
        if (visualHReversed)
            x = r.left() + r.right() - x - w + 1;
        if (vReversed)
            y = r.top() + r.bottom() - y - h + 1;
        box->setGeometry(QRect(x, y, w, h));
    }
}

QRect QGridLayoutData::cellGeometry(int row, int col) const
{
    if (row < 0 || row >= rr || col < 0 || col >= cc)
        return QRect();

    const QVector<QLayoutStruct> *rDataPtr;
    if (has_hfw)
        rDataPtr = hfwData;
    else
        rDataPtr = &rowData;
    return QRect(colData[col].pos, (*rDataPtr)[row].pos, colData[col].size, (*rDataPtr)[row].size);
}

/*!
    \class QGridLayout

    \brief The QGridLayout class lays out widgets in a grid.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QGridLayout takes the space made available to it (by its parent
    layout or by the parentWidget()), divides it up into rows and
    columns, and puts each widget it manages into the correct cell.

    Columns and rows behave identically; we will discuss columns, but
    there are equivalent functions for rows.

    Each column has a minimum width and a stretch factor. The minimum
    width is the greatest of that set using addColSpacing() and the
    minimum width of each widget in that column. The stretch factor is
    set using setColStretch() and determines how much of the available
    space the column will get over and above its necessary minimum.

    Normally, each managed widget or layout is put into a cell of its
    own using addWidget(). It is also possible for a widget to occupy
    multiple cells using the row and column spanning overloads of
    addItem() and addWidget(). If you do this, QGridLayout will guess
    how to distribute the size over the columns/rows (based on the
    stretch factors).

    To remove a widget from a layout, call remove(). Calling
    QWidget::hide() on a widget also effectively removes the widget
    from the layout until QWidget::show() is called.

    This illustration shows a fragment of a dialog with a five-column,
    three-row grid (the grid is shown overlaid in magenta):

    \img gridlayout.png

    Columns 0, 2 and 4 in this dialog fragment are made up of a
    QLabel, a QLineEdit, and a QListBox. Columns 1 and 3 are
    placeholders made with addColSpacing(). Row 0 consists of three
    QLabel objects, row 1 of three QLineEdit objects and row 2 of
    three QListBox objects. We used placeholder columns (1 and 3) to
    get the right amount of space between the columns.

    Note that the columns and rows are not equally wide or tall. If
    you want two columns to have the same width, you must set their
    minimum widths and stretch factors to be the same yourself. You do
    this using addColSpacing() and setColStretch().

    If the QGridLayout is not the top-level layout (i.e. does not
    manage all of the widget's area and children), you must add it to
    its parent layout when you create it, but before you do anything
    with it. The normal way to add a layout is by calling
    parentLayout-\>addLayout().

    Once you have added your layout you can start putting widgets and
    other layouts into the cells of your grid layout using
    addWidget(), addItem(), and addLayout().

    QGridLayout also includes two margin widths: the border and the
    spacing. The border is the width of the reserved space along each
    of the QGridLayout's four sides. The spacing is the width of the
    automatically allocated spacing between neighboring boxes.

    Both the border and the spacing are parameters of the constructor
    and default to 0.

    \sa QGrid, \link layout.html Layout Overview \endlink
*/


/*!
    Constructs a new QGridLayout with \a nRows rows, \a nCols columns
    and parent widget, \a  parent. \a parent may not be 0. The grid
    layout is called \a name.

    \a margin is the number of pixels between the edge of the widget
    and its managed children. \a space is the default number of pixels
    between cells. If \a space is -1, the value of \a margin is used.
*/
QGridLayout::QGridLayout(QWidget *parent, int nRows, int nCols, int margin,
                          int space, const char *name)
    : QLayout(parent, margin, space, name)
{
    init(nRows, nCols);
}

/*!
    Constructs a new grid that is placed inside \a parentLayout with
    \a nRows rows and \a nCols columns. If \a spacing is -1, this
    QGridLayout inherits its parent's spacing(); otherwise \a spacing
    is used. The grid layout is called \a name.

    This grid is placed according to \a parentLayout's default
    placement rules.
*/
QGridLayout::QGridLayout(QLayout *parentLayout, int nRows, int nCols,
                          int spacing, const char *name)
    : QLayout(parentLayout, spacing, name)
{
    init(nRows, nCols);
}

/*!
    Constructs a new grid with \a nRows rows and \a nCols columns. If
    \a spacing is -1, this QGridLayout inherits its parent's
    spacing(); otherwise \a spacing is used. The grid layout is called
    \a name.

    You must insert this grid into another layout. You can insert
    widgets and layouts into this layout at any time, but laying out
    will not be performed before this is inserted into another layout.
*/
QGridLayout::QGridLayout(int nRows, int nCols,
                          int spacing, const char *name)
     : QLayout(spacing, name)
{
    init(nRows, nCols);
}

/*!
    Destroys the grid layout. Geometry management is terminated if
    this is a top-level grid.

    The layout's widgets aren't destroyed.
*/
QGridLayout::~QGridLayout()
{
    delete data;
}

/*!
    Returns the number of rows in this grid.
*/
int QGridLayout::numRows() const
{
    return data->numRows();
}

/*!
    Returns the number of columns in this grid.
*/
int QGridLayout::numCols() const
{
    return data->numCols();
}

/*!
    Returns the preferred size of this grid.
*/
QSize QGridLayout::sizeHint() const
{
    return data->sizeHint(spacing()) + QSize(2 * margin(), 2 * margin());
}

/*!
    Returns the minimum size needed by this grid.
*/
QSize QGridLayout::minimumSize() const
{
    return data->minimumSize(spacing()) + QSize(2 * margin(), 2 * margin());
}

/*!
    Returns the maximum size needed by this grid.
*/
QSize QGridLayout::maximumSize() const
{
    QSize s = data->maximumSize(spacing()) +
              QSize(2 * margin(), 2 * margin());
    s = s.boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));
    if (alignment() & Qt::AlignHorizontal_Mask)
        s.setWidth(QLAYOUTSIZE_MAX);
    if (alignment() & Qt::AlignVertical_Mask)
        s.setHeight(QLAYOUTSIZE_MAX);
    return s;
}

/*!
    Returns true if this layout's preferred height depends on its
    width; otherwise returns false.
*/
bool QGridLayout::hasHeightForWidth() const
{
    return ((QGridLayout*)this)->data->hasHeightForWidth(spacing());
}

/*!
    Returns the layout's preferred height when it is \a w pixels wide.
*/
int QGridLayout::heightForWidth(int w) const
{
    QGridLayout *that = (QGridLayout*)this;
    return that->data->heightForWidth(w, margin(), spacing());
}

/*! \internal */
int QGridLayout::minimumHeightForWidth(int w) const
{
    QGridLayout *that = (QGridLayout*)this;
    return that->data->minimumHeightForWidth(w, margin(), spacing());
}

/*!
    Searches for widget \a w in this layout (not including child
    layouts). If \a w is found, it sets \c *\a row and \c *\a col to
    the row and column and returns true; otherwise returns false.

    Note: if a widget spans multiple rows/columns, the top-left cell
    is returned.
*/
bool QGridLayout::findWidget(QWidget* w, int *row, int *col)
{
    return data->findWidget(w, row, col);
}

/*!
    \reimp
*/
QLayoutItem *QGridLayout::itemAt(int idx)
{
    return data->itemAt(idx);
}


/*!
    \reimp
*/
QLayoutItem *QGridLayout::takeAt(int idx)
{
    return data->takeAt(idx);
}


/*!
    Resizes managed widgets within the rectangle \a r.
*/
void QGridLayout::setGeometry(const QRect &r)
{
    if (data->isDirty() || r != geometry()) {
        QLayout::setGeometry(r);
        QRect cr = alignment() ? alignmentRect(r) : r;
        QRect s(cr.x() + margin(), cr.y() + margin(),
                 cr.width() - 2 * margin(), cr.height() - 2 * margin());
        data->distribute(s, spacing());
    }
}

/*!
    Returns the geometry of the cell with row \a row and column \a col
    in the grid. Returns an invalid rectangle if \a row or \a col is
    outside the grid.

    \warning in the current version of Qt this function does not
    return valid results until setGeometry() has been called, i.e.
    after the parentWidget() is visible.
*/
QRect QGridLayout::cellGeometry(int row, int col) const
{
    return data->cellGeometry(row, col);
}
#ifdef QT_COMPAT
/*!
    Expands this grid so that it will have \a nRows rows and \a nCols
    columns. Will not shrink the grid. You should not need to call
    this function because QGridLayout expands automatically as new
    items are inserted.
*/
void QGridLayout::expand(int nRows, int nCols)
{
    data->expand(nRows, nCols);
}
#endif
/*!
    Sets up the grid.
*/
void QGridLayout::init(int nRows, int nCols)
{
    setSupportsMargin(true);
    data = new QGridLayoutData(nRows, nCols);
}

/*!
    \overload

    Adds \a item to the next free position of this layout.
*/
void QGridLayout::addItem(QLayoutItem *item)
{
    int r, c;
    data->getNextPos(r, c);
    addItem(item, r, c);
}

/*!
    Adds \a item at position \a row, \a col, spanning \a rowSpan rows
    and \a colSpan columns, and aligns it according to \a alignment.
    The layout takes ownership of the \a item.

    \warning Do not use this function to add child layouts or child
    widget items. Use addLayout() or addWidget() instead.
*/
void QGridLayout::addItem(QLayoutItem *item, int row, int col, int rowSpan, int colSpan, Alignment alignment)
{
    QGridBox *b = new QGridBox(item);
    b->setAlignment(alignment);
    data->add(b, row, row + rowSpan - 1, col, col + colSpan - 1);
    invalidate();
}

/*
  Returns true if the widget \a w can be added to the layout \a l;
  otherwise returns false.
*/
static bool checkWidget(QLayout *l, QWidget *w)
{
    if (!w) {
        qWarning("QLayout: Cannot add null widget to %s/%s", l->className(),
                  l->objectName());
        return false;
    }
    return true;
}

/*!
    Adds the widget \a w to the cell grid at \a row, \a col. The
    top-left position is (0, 0) by default.

    Alignment is specified by \a alignment, which is a bitwise OR of
    \l Qt::AlignmentFlags values. The default alignment is 0, which
    means that the widget fills the entire cell.

*/
void QGridLayout::addWidget(QWidget *w, int row, int col, Alignment alignment)
{
    if (!checkWidget(this, w))
        return;
    if (row < 0 || col < 0) {
        qWarning("QGridLayout: Cannot add %s/%s to %s/%s at row %d col %d",
                  w->className(), w->objectName(), className(), objectName(), row, col);
        return;
    }
    addChildWidget(w);
    QWidgetItem *b = new QWidgetItem(w);
    b->setAlignment(alignment);
    addItem(b, row, col);
}

/*!
  \overload
    This version adds the widget \a w to the cell grid, spanning multiple
    rows/columns. The cell will start at \a fromRow, \a fromCol spanning \a
    rowSpan rows and \a colSpan columns.

*/
void QGridLayout::addWidget(QWidget *w, int fromRow, int fromCol,
                             int rowSpan, int colSpan, Alignment alignment)
{
    if (!checkWidget(this, w))
        return;
    int toRow = fromRow + rowSpan - 1;
    int toCol = fromCol + colSpan - 1;
    addChildWidget(w);
    QGridBox *b = new QGridBox(w);
    b->setAlignment(alignment);
    data->add(b, fromRow, toRow, fromCol, toCol);
}

/*!
    \fn void QGridLayout::addWidget(QWidget *w)

    \internal
*/

/*!
    Places the \a layout at position (\a row, \a col) in the grid. The
    top-left position is (0, 0).

    Alignment is specified by \a alignment, which is a bitwise OR of
    \l Qt::AlignmentFlags values. The default alignment is 0, which
    means that the widget fills the entire cell.

    A non-zero alignment indicates that the layout should not grow to
    fill the available space but should be sized according to
    sizeHint().


    \a layout becomes a child of the grid layout.
*/
void QGridLayout::addLayout(QLayout *layout, int row, int col, Alignment alignment)
{
    addChildLayout(layout);
    QGridBox *b = new QGridBox(layout);
    b->setAlignment(alignment);
    data->add(b, row, col);
}

/*!
  \overload
    This version adds the layout \a layout to the cell grid, spanning multiple
    rows/columns. The cell will start at \a row, \a col spanning \a
    rowSpan rows and \a colSpan columns.
*/
void QGridLayout::addLayout(QLayout *layout, int row, int col,
                                      int rowSpan, int colSpan, Alignment alignment)
{
    addChildLayout(layout);
    QGridBox *b = new QGridBox(layout);
    b->setAlignment(alignment);
    data->add(b, row, row + rowSpan - 1, col, col + colSpan - 1);
}

/*!
    Sets the stretch factor of row \a row to \a stretch. The first row
    is number 0.

    The stretch factor is relative to the other rows in this grid.
    Rows with a higher stretch factor take more of the available
    space.

    The default stretch factor is 0. If the stretch factor is 0 and no
    other row in this table can grow at all, the row may still grow.

    \sa rowStretch(), setRowSpacing(), setColStretch()
*/
void QGridLayout::setRowStretch(int row, int stretch)
{
    data->setRowStretch(row, stretch);
}

/*!
    Returns the stretch factor for row \a row.

    \sa setRowStretch()
*/
int QGridLayout::rowStretch(int row) const
{
    return data->rowStretch(row);
}

/*!
    Returns the stretch factor for column \a col.

    \sa setColStretch()
*/
int QGridLayout::colStretch(int col) const
{
    return data->colStretch(col);
}

/*!
    Sets the stretch factor of column \a col to \a stretch. The first
    column is number 0.

    The stretch factor is relative to the other columns in this grid.
    Columns with a higher stretch factor take more of the available
    space.

    The default stretch factor is 0. If the stretch factor is 0 and no
    other column in this table can grow at all, the column may still
    grow.

    An alternative approach is to add spacing using addItem() with a
    QSpacerItem.

    \sa colStretch(), setRowStretch()
*/
void QGridLayout::setColStretch(int col, int stretch)
{
    data->setColStretch(col, stretch);
}

/*!
    Sets the minimum height of row \a row to \a minSize pixels.

    \sa rowSpacing(), setColSpacing()
*/
void QGridLayout::setRowSpacing(int row, int minSize)
{
    data->setRowSpacing(row, minSize);
}

/*!
    Returns the row spacing for row \a row.

    \sa setRowSpacing()
*/
int QGridLayout::rowSpacing(int row) const
{
    return data->rowSpacing(row);
}

/*!
    Sets the minimum width of column \a col to \a minSize pixels.

    \sa colSpacing(), setRowSpacing()
*/
void QGridLayout::setColSpacing(int col, int minSize)
{
    data->setColSpacing(col, minSize);
}

/*!
    Returns the column spacing for column \a col.

    \sa setColSpacing()
*/
int QGridLayout::colSpacing(int col) const
{
    return data->colSpacing(col);
}

/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.
*/
QSizePolicy::ExpandData QGridLayout::expanding() const
{
    return data->expanding(spacing());
}

/*!
    Sets the grid's origin corner, i.e. position (0, 0), to \a c.
*/
void QGridLayout::setOrigin(Corner c)
{
    data->setReversed(c == BottomLeft || c == BottomRight,
                       c == TopRight || c == BottomRight);
}

/*!
    Returns the corner that's used for the grid's origin, i.e. for
    position (0, 0).
*/
QGridLayout::Corner QGridLayout::origin() const
{
    if (data->horReversed()) {
        return data->verReversed() ? BottomRight : TopRight;
    } else {
        return data->verReversed() ? BottomLeft : TopLeft;
    }
}

/*!
    Resets cached information.
*/
void QGridLayout::invalidate()
{
    data->setDirty();
    QLayout::invalidate();
}

struct QBoxLayoutItem
{
    QBoxLayoutItem(QLayoutItem *it, int stretch_ = 0)
        : item(it), stretch(stretch_), magic(false) { }
    ~QBoxLayoutItem() { delete item; }

    int hfw(int w) {
        if (item->hasHeightForWidth()) {
            return item->heightForWidth(w);
        } else {
            return item->sizeHint().height();
        }
    }
    int mhfw(int w) {
        if (item->hasHeightForWidth()) {
            return item->heightForWidth(w);
        } else {
            return item->minimumSize().height();
        }
    }
    int hStretch() {
        if (stretch == 0 && item->widget()) {
            return item->widget()->sizePolicy().horStretch();
        } else {
            return stretch;
        }
    }
    int vStretch() {
        if (stretch == 0 && item->widget()) {
            return item->widget()->sizePolicy().verStretch();
        } else {
            return stretch;
        }
    }

    QLayoutItem *item;
    int stretch;
    bool magic;
};

class QBoxLayoutData
{
public:
    QBoxLayoutData() : hfwWidth(-1), dirty(true) { }
    ~QBoxLayoutData();

    void setDirty() {
        geomArray.clear();
        hfwWidth = -1;
        hfwHeight = -1;
        dirty = true;
    }

    QList<QBoxLayoutItem *> list;
    QVector<QLayoutStruct> geomArray;
    int hfwWidth;
    int hfwHeight;
    int hfwMinHeight;
    QSize sizeHint;
    QSize minSize;
    QSize maxSize;
    QSizePolicy::ExpandData expanding;
    uint hasHfw : 1;
    uint dirty : 1;
};

QBoxLayoutData::~QBoxLayoutData()
{
    while (!list.isEmpty())
        delete list.takeFirst();
}

/*!
    \class QBoxLayout

    \brief The QBoxLayout class lines up child widgets horizontally or
    vertically.

    \ingroup geomanagement
    \ingroup appearance

    QBoxLayout takes the space it gets (from its parent layout or from
    the parentWidget()), divides it up into a row of boxes, and makes
    each managed widget fill one box.

    \img qhbox-m.png Horizontal box with five child widgets

    If the QBoxLayout's orientation is \c Horizontal the boxes are
    placed in a row, with suitable sizes. Each widget (or other box)
    will get at least its minimum size and at most its maximum size.
    Any excess space is shared according to the stretch factors (more
    about that below).

    \img qvbox-m.png Vertical box with five child widgets

    If the QBoxLayout's orientation is \c Vertical, the boxes are
    placed in a column, again with suitable sizes.

    The easiest way to create a QBoxLayout is to use one of the
    convenience classes, e.g. QHBoxLayout (for \c Horizontal boxes) or
    QVBoxLayout (for \c Vertical boxes). You can also use the
    QBoxLayout constructor directly, specifying its direction as \c
    LeftToRight, \c Down, \c RightToLeft or \c Up.

    If the QBoxLayout is not the top-level layout (i.e. it is not
    managing all of the widget's area and children), you must add it
    to its parent layout before you can do anything with it. The
    normal way to add a layout is by calling
    parentLayout-\>addLayout().

    Once you have done this, you can add boxes to the QBoxLayout using
    one of four functions:

    \list
    \i addWidget() to add a widget to the QBoxLayout and set the
    widget's stretch factor. (The stretch factor is along the row of
    boxes.)

    \i addSpacing() to create an empty box; this is one of the
    functions you use to create nice and spacious dialogs. See below
    for ways to set margins.

    \i addStretch() to create an empty, stretchable box.

    \i addLayout() to add a box containing another QLayout to the row
    and set that layout's stretch factor.
    \endlist

    Use insertWidget(), insertSpacing(), insertStretch() or
    insertLayout() to insert a box at a specified position in the
    layout.

    QBoxLayout also includes two margin widths:

    \list
    \i setMargin() sets the width of the outer border. This is the width
       of the reserved space along each of the QBoxLayout's four sides.
    \i setSpacing() sets the width between neighboring boxes. (You
       can use addSpacing() to get more space at a particular spot.)
    \endlist

    The margin defaults to 0. The spacing defaults to the same as the
    margin width for a top-level layout, or to the same as the parent
    layout. Both are parameters to the constructor.

    To remove a widget from a layout, call remove(). Calling
    QWidget::hide() on a widget also effectively removes the widget
    from the layout until QWidget::show() is called.

    You will almost always want to use QVBoxLayout and QHBoxLayout
    rather than QBoxLayout because of their convenient constructors.

    \sa QGrid \link layout.html Layout Overview \endlink
*/

/*!
    \enum QBoxLayout::Direction

    This type is used to determine the direction of a box layout.

    \value LeftToRight  Horizontal, from left to right
    \value RightToLeft  Horizontal, from right to left
    \value TopToBottom  Vertical, from top to bottom
    \value Down  The same as \c TopToBottom
    \value BottomToTop  Vertical, from bottom to top
    \value Up  The same as \c BottomToTop
*/

static inline bool horz(QBoxLayout::Direction dir)
{
    return dir == QBoxLayout::RightToLeft || dir == QBoxLayout::LeftToRight;
}

/*!
    Constructs a new QBoxLayout with direction \a d and main widget \a
    parent. \a parent may not be 0.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.

    \a name is the internal object name.

    \sa direction()
*/
QBoxLayout::QBoxLayout(QWidget *parent, Direction d,
                        int margin, int spacing, const char *name)
    : QLayout(parent, margin, spacing, name)
{
    data = new QBoxLayoutData;
    dir = d;
    setSupportsMargin(true);
}

/*!
    Constructs a new QBoxLayout called \a name, with direction \a d,
    and inserts it into \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, the layout will inherit its
    parent's spacing().
*/
QBoxLayout::QBoxLayout(QLayout *parentLayout, Direction d, int spacing,
                        const char *name)
    : QLayout(parentLayout, spacing, name)
{
    data = new QBoxLayoutData;
    dir = d;
    setSupportsMargin(true);
}

/*!
    Constructs a new QBoxLayout called \a name, with direction \a d.

    If \a spacing is -1, the layout will inherit its parent's
    spacing(); otherwise \a spacing is used.

    You must insert this box into another layout.
*/
QBoxLayout::QBoxLayout(Direction d, int spacing, const char *name)
    : QLayout(spacing, name)
{
    data = new QBoxLayoutData;
    dir = d;
    setSupportsMargin(true);
}

/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QBoxLayout::~QBoxLayout()
{
    delete data;
}

/*!
    Returns the preferred size of this box layout.
*/
QSize QBoxLayout::sizeHint() const
{
    if (data->dirty) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->setupGeom();
    }
    return data->sizeHint + QSize(2 * margin(), 2 * margin());
}

/*!
    Returns the minimum size needed by this box layout.
*/
QSize QBoxLayout::minimumSize() const
{
    if (data->dirty) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->setupGeom();
    }
    return data->minSize + QSize(2 * margin(), 2 * margin());
}

/*!
    Returns the maximum size needed by this box layout.
*/
QSize QBoxLayout::maximumSize() const
{
    if (data->dirty) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->setupGeom();
    }
    QSize s = (data->maxSize + QSize(2 * margin(), 2 * margin()))
              .boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));
    if (alignment() & Qt::AlignHorizontal_Mask)
        s.setWidth(QLAYOUTSIZE_MAX);
    if (alignment() & Qt::AlignVertical_Mask)
        s.setHeight(QLAYOUTSIZE_MAX);
    return s;
}

/*!
  Returns true if this layout's preferred height depends on its width;
  otherwise returns false.
*/
bool QBoxLayout::hasHeightForWidth() const
{
    if (data->dirty) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->setupGeom();
    }
    return data->hasHfw;
}

/*!
    Returns the layout's preferred height when it is \a w pixels wide.
*/
int QBoxLayout::heightForWidth(int w) const
{
    if (!hasHeightForWidth())
        return -1;
    w -= 2 * margin();
    if (w != data->hfwWidth) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->calcHfw(w);
    }
    return data->hfwHeight + 2 * margin();
}

/*! \internal */
int QBoxLayout::minimumHeightForWidth(int w) const
{
    (void) heightForWidth(w);
    return data->hasHfw ? (data->hfwMinHeight + 2 * margin()) : -1;
}

/*!
    Resets cached information.
*/
void QBoxLayout::invalidate()
{
    data->setDirty();
    QLayout::invalidate();
}

/*!
    \reimp
*/
QLayoutItem *QBoxLayout::itemAt(int idx)
{
    return idx >= 0 && idx < data->list.count() ? data->list.at(idx)->item : 0;
}


/*!
    \reimp
*/
QLayoutItem *QBoxLayout::takeAt(int idx)
{
    if (idx < 0 || idx >= data->list.count())
        return 0;
    QBoxLayoutItem *b =  data->list.takeAt(idx);
    QLayoutItem *item = b->item;
    b->item = 0;
    delete b;
    return item;
}


/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.
*/
QSizePolicy::ExpandData QBoxLayout::expanding() const
{
    if (data->dirty) {
        QBoxLayout *that = (QBoxLayout*)this;
        that->setupGeom();
    }
    return data->expanding;
}

/*!
    Resizes managed widgets within the rectangle \a r.
*/
void QBoxLayout::setGeometry(const QRect &r)
{
    if (data->dirty || r != geometry()) {
        QLayout::setGeometry(r);
        if (data->dirty)
            setupGeom();
        QRect cr = alignment() ? alignmentRect(r) : r;
        QRect s(cr.x() + margin(), cr.y() + margin(),
                 cr.width() - 2 * margin(), cr.height() - 2 * margin());

        QVector<QLayoutStruct> a = data->geomArray;
        int pos = horz(dir) ? s.x() : s.y();
        int space = horz(dir) ? s.width() : s.height();
        int n = a.count();
        if (data->hasHfw && !horz(dir)) {
            for (int i = 0; i < n; i++) {
                QBoxLayoutItem *box = data->list.at(i);
                if (box->item->hasHeightForWidth())
                    a[i].sizeHint = a[i].minimumSize =
                                    box->item->heightForWidth(s.width());
            }
        }

        Direction visualDir = dir;
        if (QApplication::reverseLayout()) {
            if (dir == LeftToRight)
                visualDir = RightToLeft;
            else if (dir == RightToLeft)
                visualDir = LeftToRight;
        }

        qGeomCalc(a, 0, n, pos, space, spacing());
        for (int i = 0; i < n; i++) {
            QBoxLayoutItem *box = data->list.at(i);

            switch (visualDir) {
            case LeftToRight:
                box->item->setGeometry(QRect(a[i].pos, s.y(),
                                              a[i].size, s.height()));
                break;
            case RightToLeft:
                box->item->setGeometry(QRect(s.left() + s.right()
                                              - a[i].pos - a[i].size + 1, s.y(),
                                              a[i].size, s.height()));
                break;
            case TopToBottom:
                box->item->setGeometry(QRect(s.x(), a[i].pos,
                                              s.width(), a[i].size));
                break;
            case BottomToTop:
                box->item->setGeometry(QRect(s.x(), s.top() + s.bottom()
                                              - a[i].pos - a[i].size + 1,
                                              s.width(), a[i].size));
            }
        }
    }
}

/*!
    Adds \a item to the end of this box layout.
*/
void QBoxLayout::addItem(QLayoutItem *item)
{
    QBoxLayoutItem *it = new QBoxLayoutItem(item);
    data->list.append(it);
    invalidate();
}

/*!
    Inserts \a item into this box layout at position \a index. If \a
    index is negative, the item is added at the end.

    \warning Does not call QLayout::insertChildLayout() if \a item is
    a QLayout.

    \sa addItem(), findWidget()
*/
void QBoxLayout::insertItem(int index, QLayoutItem *item)
{
    if (index < 0)                                // append
        index = data->list.count();

    QBoxLayoutItem *it = new QBoxLayoutItem(item);
    data->list.insert(index, it);
    invalidate();
}

/*!
    Inserts a non-stretchable space at position \a index, with size \a
    size. If \a index is negative the space is added at the end.

    The box layout has default margin and spacing. This function adds
    additional space.

    \sa insertStretch()
*/
void QBoxLayout::insertSpacing(int index, int size)
{
    if (index < 0)                                // append
        index = data->list.count();

    // hack in QGridLayoutData: spacers do not get insideSpacing
    QLayoutItem *b;
    if (horz(dir))
        b = new QSpacerItem(size, 0, QSizePolicy::Fixed,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(0, size, QSizePolicy::Minimum,
                             QSizePolicy::Fixed);

    QBoxLayoutItem *it = new QBoxLayoutItem(b);
    it->magic = true;
    data->list.insert(index, it);
    invalidate();
}

/*!
    Inserts a stretchable space at position \a index, with zero
    minimum size and stretch factor \a stretch. If \a index is
    negative the space is added at the end.

    \sa insertSpacing()
*/
void QBoxLayout::insertStretch(int index, int stretch)
{
    if (index < 0)                                // append
        index = data->list.count();

    // hack in QGridLayoutData: spacers do not get insideSpacing
    QLayoutItem *b;
    if (horz(dir))
        b = new QSpacerItem(0, 0, QSizePolicy::Expanding,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(0, 0,  QSizePolicy::Minimum,
                             QSizePolicy::Expanding);

    QBoxLayoutItem *it = new QBoxLayoutItem(b, stretch);
    it->magic = true;
    data->list.insert(index, it);
    invalidate();
}

/*!
    Inserts \a layout at position \a index, with stretch factor \a
    stretch. If \a index is negative, the layout is added at the end.

    \a layout becomes a child of the box layout.

    \sa insertWidget(), insertSpacing()
*/
void QBoxLayout::insertLayout(int index, QLayout *layout, int stretch)
{
    addChildLayout(layout);
    if (index < 0)                                // append
        index = data->list.count();
    QBoxLayoutItem *it = new QBoxLayoutItem(layout, stretch);
    data->list.insert(index, it);
    invalidate();
}

/*!
    Inserts \a widget at position \a index, with stretch factor \a
    stretch and alignment \a alignment. If \a index is negative, the
    widget is added at the end.

    The stretch factor applies only in the \link direction() direction
    \endlink of the QBoxLayout, and is relative to the other boxes and
    widgets in this QBoxLayout. Widgets and boxes with higher stretch
    factors grow more.

    If the stretch factor is 0 and nothing else in the QBoxLayout has
    a stretch factor greater than zero, the space is distributed
    according to the QWidget:sizePolicy() of each widget that's
    involved.

    Alignment is specified by \a alignment, which is a bitwise OR of
    \l Qt::AlignmentFlags values. The default alignment is 0, which
    means that the widget fills the entire cell.

    From Qt 3.0, the \a alignment parameter is interpreted more
    aggressively than in previous versions of Qt. A non-default
    alignment now indicates that the widget should not grow to fill
    the available space, but should be sized according to sizeHint().

    \sa insertLayout(), insertSpacing()
*/
void QBoxLayout::insertWidget(int index, QWidget *widget, int stretch,
                               Alignment alignment)
{
    if (!checkWidget(this, widget))
         return;
    addChildWidget(widget);
    if (index < 0)                                // append
        index = data->list.count();
    QWidgetItem *b = new QWidgetItem(widget);
    b->setAlignment(alignment);
    QBoxLayoutItem *it = new QBoxLayoutItem(b, stretch);
    data->list.insert(index, it);
    invalidate();
}

/*!
    Adds a non-stretchable space with size \a size to the end of this
    box layout. QBoxLayout provides default margin and spacing. This
    function adds additional space.

    \sa insertSpacing(), addStretch()
*/
void QBoxLayout::addSpacing(int size)
{
    insertSpacing(-1, size);
}

/*!
    Adds a stretchable space with zero minimum size and stretch factor
    \a stretch to the end of this box layout.

    \sa addSpacing()
*/
void QBoxLayout::addStretch(int stretch)
{
    insertStretch(-1, stretch);
}

/*!
    Adds \a widget to the end of this box layout, with a stretch
    factor of \a stretch and alignment \a alignment.

    The stretch factor applies only in the \link direction() direction
    \endlink of the QBoxLayout, and is relative to the other boxes and
    widgets in this QBoxLayout. Widgets and boxes with higher stretch
    factors grow more.

    If the stretch factor is 0 and nothing else in the QBoxLayout has
    a stretch factor greater than zero, the space is distributed
    according to the QWidget:sizePolicy() of each widget that's
    involved.

    Alignment is specified by \a alignment which is a bitwise OR of \l
    Qt::AlignmentFlags values. The default alignment is 0, which means
    that the widget fills the entire cell.

    From Qt 3.0, the \a alignment parameter is interpreted more
    aggressively than in previous versions of Qt. A non-default
    alignment now indicates that the widget should not grow to fill
    the available space, but should be sized according to sizeHint().

    \sa insertWidget(), addLayout(), addSpacing()
*/
void QBoxLayout::addWidget(QWidget *widget, int stretch,
                            Alignment alignment)
{
    insertWidget(-1, widget, stretch, alignment);
}

/*!
    Adds \a layout to the end of the box, with serial stretch factor
    \a stretch.

    \sa insertLayout(), addWidget(), addSpacing()
*/
void QBoxLayout::addLayout(QLayout *layout, int stretch)
{
    insertLayout(-1, layout, stretch);
}

/*!
    Limits the perpendicular dimension of the box (e.g. height if the
    box is LeftToRight) to a minimum of \a size. Other constraints may
    increase the limit.
*/
void QBoxLayout::addStrut(int size)
{
    QLayoutItem *b;
    if (horz(dir))
        b = new QSpacerItem(0, size, QSizePolicy::Fixed,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(size, 0, QSizePolicy::Minimum,
                             QSizePolicy::Fixed);

    QBoxLayoutItem *it = new QBoxLayoutItem(b);
    it->magic = true;
    data->list.append(it);
    invalidate();
}

/*!
    Searches for widget \a w in this layout (not including child
    layouts).

    Returns the index of \a w, or -1 if \a w is not found.
*/
int QBoxLayout::findWidget(QWidget* w)
{
    const int n = data->list.count();
    for (int i = 0; i < n; i++) {
        if (data->list.at(i)->item->widget() == w)
            return i;
    }
    return -1;
}

/*!
    Sets the stretch factor for widget \a w to \a stretch and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.

    \sa setAlignment()
*/
bool QBoxLayout::setStretchFactor(QWidget *w, int stretch)
{
    for (int i = 0; i < data->list.size(); ++i) {
        QBoxLayoutItem *box = data->list.at(i);
        if (box->item->widget() == w) {
            box->stretch = stretch;
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
  \overload

  Sets the stretch factor for the layout \a l to \a stretch and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QBoxLayout::setStretchFactor(QLayout *l, int stretch)
{
    for (int i = 0; i < data->list.size(); ++i) {
        QBoxLayoutItem *box = data->list.at(i);
        if (box->item->layout() == l) {
            box->stretch = stretch;
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
    Sets the alignment for widget \a w to \a alignment and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.

    \sa setStretchFactor()
*/
bool QBoxLayout::setAlignment(QWidget *w, Alignment alignment)
{
    for (int i = 0; i < data->list.size(); ++i) {
        QBoxLayoutItem *box = data->list.at(i);
        if (box->item->widget() == w) {
            box->item->setAlignment(alignment);
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
  \overload

  Sets the alignment for the layout \a l to \a alignment and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QBoxLayout::setAlignment(QLayout *l, Alignment alignment)
{
    for (int i = 0; i < data->list.size(); ++i) {
        QBoxLayoutItem *box = data->list.at(i);
        if (box->item->layout() == l) {
            box->item->setAlignment(alignment);
            invalidate();
            return true;
        }
    }
    return false;
}

/*! \reimp */
void QBoxLayout::setAlignment(Alignment alignment)
{
    QLayout::setAlignment(alignment);
}

/*!
    Sets the direction of this layout to \a direction.
*/
void QBoxLayout::setDirection(Direction direction)
{
    if (dir == direction)
        return;
    if (horz(dir) != horz(direction)) {
        //swap around the spacers (the "magic" bits)
        //#### a bit yucky, knows too much.
        //#### probably best to add access functions to spacerItem
        //#### or even a QSpacerItem::flip()
        for (int i = 0; i < data->list.size(); ++i) {
            QBoxLayoutItem *box = data->list.at(i);
            if (box->magic) {
                QSpacerItem *sp = box->item->spacerItem();
                if (sp) {
                    if (sp->expanding() == QSizePolicy::NoDirection) {
                        //spacing or strut
                        QSize s = sp->sizeHint();
                        sp->changeSize(s.height(), s.width(),
                            horz(direction) ? QSizePolicy::Fixed:QSizePolicy::Minimum,
                            horz(direction) ? QSizePolicy::Minimum:QSizePolicy::Fixed);

                    } else {
                        //stretch
                        if (horz(direction))
                            sp->changeSize(0, 0, QSizePolicy::Expanding,
                                            QSizePolicy::Minimum);
                        else
                            sp->changeSize(0, 0, QSizePolicy::Minimum,
                                            QSizePolicy::Expanding);
                    }
                }
            }
        }
    }
    dir = direction;
    invalidate();
}

/*
    Initializes the data structure needed by qGeomCalc and
    recalculates max/min and size hint.
*/
void QBoxLayout::setupGeom()
{
    if (!data->dirty)
        return;

    int maxw = horz(dir) ? 0 : QLAYOUTSIZE_MAX;
    int maxh = horz(dir) ? QLAYOUTSIZE_MAX : 0;
    int minw = 0;
    int minh = 0;
    int hintw = 0;
    int hinth = 0;

    bool horexp = false;
    bool verexp = false;

    data->hasHfw = false;

    int n = data->list.count();
    data->geomArray.clear();
    data->geomArray.resize(n);
    QVector<QLayoutStruct> &a = data->geomArray;

    bool first = true;
    for (int i = 0; i < n; i++) {
        QBoxLayoutItem *box = data->list.at(i);
        QSize max = box->item->maximumSize();
        QSize min = box->item->minimumSize();
        QSize hint = box->item->sizeHint();
        QSizePolicy::ExpandData exp = box->item->expanding();
        bool empty = box->item->isEmpty();
        // space before non-empties, except the first:
        int space = (empty || first) ? 0 : spacing();
        bool ignore = empty && box->item->widget(); // ignore hidden widgets

        if (horz(dir)) {
            bool expand = exp & QSizePolicy::Horizontally || box->stretch > 0;
            horexp = horexp || expand;
            maxw += max.width() + space;
            minw += min.width() + space;
            hintw += hint.width() + space;
            if (!ignore)
                qMaxExpCalc(maxh, verexp,
                             max.height(), exp & QSizePolicy::Vertically);
            minh = qMax(minh, min.height());
            hinth = qMax(hinth, hint.height());

            a[i].sizeHint = hint.width();
            a[i].maximumSize = max.width();
            a[i].minimumSize = min.width();
            a[i].expansive = expand;
            a[i].stretch = box->stretch ? box->stretch : box->hStretch();
        } else {
            bool expand = (exp & QSizePolicy::Vertically || box->stretch > 0);
            verexp = verexp || expand;
            maxh += max.height() + space;
            minh += min.height() + space;
            hinth += hint.height() + space;
            if (!ignore)
                qMaxExpCalc(maxw, horexp,
                             max.width(), exp & QSizePolicy::Horizontally);
            minw = qMax(minw, min.width());
            hintw = qMax(hintw, hint.width());

            a[i].sizeHint = hint.height();
            a[i].maximumSize = max.height();
            a[i].minimumSize = min.height();
            a[i].expansive = expand;
            a[i].stretch = box->stretch ? box->stretch : box->vStretch();
        }

        a[i].empty = empty;
        data->hasHfw = data->hasHfw || box->item->hasHeightForWidth();
        first = first && empty;
    }

    data->expanding = (QSizePolicy::ExpandData)
                       ((horexp ? QSizePolicy::Horizontally : 0)
                         | (verexp ? QSizePolicy::Vertically : 0));

    data->minSize = QSize(minw, minh);
    data->maxSize = QSize(maxw, maxh).expandedTo(data->minSize);
    data->sizeHint = QSize(hintw, hinth)
                     .expandedTo(data->minSize)
                     .boundedTo(data->maxSize);

    data->dirty = false;
}

/*
  Calculates and stores the preferred height given the width \a w.
*/
void QBoxLayout::calcHfw(int w)
{
    int h = 0;
    int mh = 0;

    if (horz(dir)) {
        QVector<QLayoutStruct> &a = data->geomArray;
        int n = a.count();
        qGeomCalc(a, 0, n, 0, w, spacing());
        for (int i = 0; i < n; i++) {
            QBoxLayoutItem *box = data->list.at(i);
            h = qMax(h, box->hfw(a[i].size));
            mh = qMax(mh, box->mhfw(a[i].size));
        }
    } else {
        bool first = true;
        for (int i = 0; i < data->list.size(); ++i) {
            QBoxLayoutItem *box = data->list.at(i);
            bool empty = box->item->isEmpty();
            h += box->hfw(w);
            mh += box->mhfw(w);
            if (!first && !empty) {
                h += spacing();
                mh += spacing();
            }
            first = first && empty;
        }
    }
    data->hfwWidth = w;
    data->hfwHeight = h;
    data->hfwMinHeight = mh;
}

/*!
    \fn QBoxLayout::Direction QBoxLayout::direction() const

    Returns the direction of the box. addWidget() and addSpacing()
    work in this direction; the stretch stretches in this direction.

    \sa QBoxLayout::Direction addWidget() addSpacing()
*/

/*!
    \class QHBoxLayout
    \brief The QHBoxLayout class lines up widgets horizontally.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    This class is used to construct horizontal box layout objects. See
    \l QBoxLayout for more details.

    The simplest use of the class is like this:
    \code
        QBoxLayout * l = new QHBoxLayout(widget);
        l->addWidget(existingChildOfWidget);
        l->addWidget(anotherChildOfWidget);
    \endcode

    \img qhboxlayout.png QHBox

    \sa QVBoxLayout QGridLayout
        \link layout.html the Layout overview \endlink
*/

/*!
    Constructs a new top-level horizontal box called \a name, with
    parent \a parent.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.
*/
QHBoxLayout::QHBoxLayout(QWidget *parent, int margin,
                          int spacing, const char *name)
    : QBoxLayout(parent, LeftToRight, margin, spacing, name)
{
}

/*!
    Constructs a new horizontal box called name \a name and adds it to
    \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QHBoxLayout will inherit its
    parent's spacing().
*/
QHBoxLayout::QHBoxLayout(QLayout *parentLayout, int spacing,
                          const char *name)
    : QBoxLayout(parentLayout, LeftToRight, spacing, name)
{
}

/*!
    Constructs a new horizontal box called name \a name. You must add
    it to another layout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QHBoxLayout will inherit its
    parent's spacing().
*/
QHBoxLayout::QHBoxLayout(int spacing, const char *name)
    : QBoxLayout(LeftToRight, spacing, name)
{
}

/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QHBoxLayout::~QHBoxLayout()
{
}

/*!
    \class QVBoxLayout

    \brief The QVBoxLayout class lines up widgets vertically.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    This class is used to construct vertical box layout objects. See
    QBoxLayout for more details.

    The simplest use of the class is like this:
    \code
        QBoxLayout * l = new QVBoxLayout(widget);
        l->addWidget(aWidget);
        l->addWidget(anotherWidget);
    \endcode

    \img qvboxlayout.png QVBox

    \sa QHBoxLayout QGridLayout \link layout.html the Layout overview \endlink
*/

/*!
    Constructs a new top-level vertical box called \a name, with
    parent \a parent.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.
*/
QVBoxLayout::QVBoxLayout(QWidget *parent, int margin, int spacing,
                          const char *name)
    : QBoxLayout(parent, TopToBottom, margin, spacing, name)
{

}

/*!
    Constructs a new vertical box called name \a name and adds it to
    \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QVBoxLayout will inherit its
    parent's spacing().
*/
QVBoxLayout::QVBoxLayout(QLayout *parentLayout, int spacing,
                          const char *name)
    : QBoxLayout(parentLayout, TopToBottom, spacing, name)
{
}

/*!
    Constructs a new vertical box called name \a name. You must add
    it to another layout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QVBoxLayout will inherit its
    parent's spacing().
*/
QVBoxLayout::QVBoxLayout(int spacing, const char *name)
    : QBoxLayout(TopToBottom, spacing, name)
{
}

/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QVBoxLayout::~QVBoxLayout()
{
}

QBoxLayout *QBoxLayout::createTmpCopy()
{
    QBoxLayout *bl = new QBoxLayout(direction());
    delete bl->data;
    bl->data = data;
    return bl;
}

#endif // QT_NO_LAYOUT
