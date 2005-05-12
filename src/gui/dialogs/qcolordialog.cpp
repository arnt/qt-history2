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

#include "qcolordialog.h"

#ifndef QT_NO_COLORDIALOG

#include "qdialog_p.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qimage.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qsettings.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qvalidator.h"
#include "qmime.h"
#include "qspinbox.h"

#ifdef Q_WS_MAC
QRgb macGetRgba(QRgb initial, bool *ok, QWidget *parent);
QColor macGetColor(const QColor& initial, QWidget *parent);
#endif

//////////// QWellArray BEGIN

struct QWellArrayData;

class QWellArray : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int selectedColumn READ selectedColumn)
    Q_PROPERTY(int selectedRow READ selectedRow)

public:
    QWellArray(int rows, int cols, QWidget* parent=0);
    ~QWellArray() {}
    QString cellContent(int row, int col) const;

    int selectedColumn() const { return selCol; }
    int selectedRow() const { return selRow; }

    virtual void setCurrent(int row, int col);
    virtual void setSelected(int row, int col);

    QSize sizeHint() const;

    virtual void setCellBrush(int row, int col, const QBrush &);
    QBrush cellBrush(int row, int col);

    inline int cellWidth() const
        { return cellw; }

    inline int cellHeight() const
        { return cellh; }

    inline int rowAt(int y) const
        { return y / cellh; }

    inline int columnAt(int x) const
        { if (isRightToLeft()) return ncols - (x / cellw) - 1; return x / cellw; }

    inline int rowY(int row) const
        { return cellh * row; }

    inline int columnX(int column) const
        { if (isRightToLeft()) return cellw * (ncols - column - 1); return cellw * column; }

    inline int numRows() const
        { return nrows; }

    inline int numCols() const
        {return ncols; }

    inline QRect cellRect() const
        { return QRect(0, 0, cellw, cellh); }

    inline QSize gridSize() const
        { return QSize(ncols * cellw, nrows * cellh); }

    QRect cellGeometry(int row, int column)
        {
            QRect r;
            if (row >= 0 && row < nrows && column >= 0 && column < ncols)
                r.setRect(columnX(column), rowY(row), cellw, cellh);
            return r;
        }


    inline void updateCell(int row, int column) { update(cellGeometry(row, column)); }


signals:
    void selected(int row, int col);

protected:
    virtual void paintCell(QPainter *, int row, int col);
    virtual void paintCellContents(QPainter *, int row, int col, const QRect&);

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void paintEvent(QPaintEvent *);

private:
    Q_DISABLE_COPY(QWellArray)

    int nrows;
    int ncols;
    int cellw;
    int cellh;
    int curRow;
    int curCol;
    int selRow;
    int selCol;
    QWellArrayData *d;
};

void QWellArray::paintEvent(QPaintEvent *e)
{
    QRect r = e->rect();
    int cx = r.x();
    int cy = r.y();
    int ch = r.height();
    int cw = r.width();
    int colfirst = columnAt(cx);
    int collast = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast = rowAt(cy + ch);

    if (isRightToLeft()) {
        int t = colfirst;
        colfirst = collast;
        collast = t;
    }

    QPainter painter(this);
    QPainter *p = &painter;


    if (collast < 0 || collast >= ncols)
        collast = ncols-1;
    if (rowlast < 0 || rowlast >= nrows)
        rowlast = nrows-1;

    // Go through the rows
    for (int r = rowfirst; r <= rowlast; ++r) {
        // get row position and height
        int rowp = rowY(r);

        // Go through the columns in the row r
        // if we know from where to where, go through [colfirst, collast],
        // else go through all of them
        for (int c = colfirst; c <= collast; ++c) {
            // get position and width of column c
            int colp = columnX(c);
            // Translate painter and draw the cell
            p->translate(colp, rowp);
            paintCell(p, r, c);
            p->translate(-colp, -rowp);
        }
    }

}

struct QWellArrayData {
    QBrush *brush;
};

QWellArray::QWellArray(int rows, int cols, QWidget *parent)
    : QWidget(parent)
        ,nrows(rows), ncols(cols)
{
    d = 0;
    setFocusPolicy(Qt::StrongFocus);
    cellw = 28;
    cellh = 24;
    curCol = 0;
    curRow = 0;
    selCol = -1;
    selRow = -1;

}


QSize QWellArray::sizeHint() const
{
    ensurePolished();
    return gridSize().boundedTo(QSize(640, 480));
}


void QWellArray::paintCell(QPainter* p, int row, int col)
{
    int w = cellWidth();                        // width of cell in pixels
    int h = cellHeight();                        // height of cell in pixels
    int b = 3;

    const QPalette & g = palette();
    QStyleOptionFrame opt;
    int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    opt.lineWidth = dfw;
    opt.midLineWidth = 1;
    opt.rect.setRect(b, b, w - 2 * b, h - 2 * b);
    opt.palette = g;
    opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
    b += dfw;

    if ((row == curRow) && (col == curCol)) {
        if (hasFocus()) {
            QStyleOptionFocusRect opt;
            opt.palette = g;
            opt.rect.setRect(0, 0, w, h);
            opt.state = QStyle::State_None;
            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
        }
    }
    QRect cr(b, b, w - 2*b, h - 2*b);
    paintCellContents(p, row, col, cr);
}

/*!
  Reimplement this function to change the contents of the well array.
 */
void QWellArray::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{

    if (d) {
        p->fillRect(r, d->brush[row*numCols()+col]);
    } else {
        p->fillRect(r, Qt::white);
        p->setPen(Qt::black);
        p->drawLine(r.topLeft(), r.bottomRight());
        p->drawLine(r.topRight(), r.bottomLeft());
    }
}


/*\reimp
*/
void QWellArray::mousePressEvent(QMouseEvent* e)
{
    // The current cell marker is set to the cell the mouse is pressed
    // in.
    QPoint pos = e->pos();
    setCurrent(rowAt(pos.y()), columnAt(pos.x()));
}

/*\reimp
*/
void QWellArray::mouseReleaseEvent(QMouseEvent*)
{
    // The current cell marker is set to the cell the mouse is clicked
    // in.
    setSelected(curRow, curCol);
}


/*
  Sets the cell currently having the focus. This is not necessarily
  the same as the currently selected cell.
*/

void QWellArray::setCurrent(int row, int col)
{

    if ((curRow == row) && (curCol == col))
        return;

    if (row < 0 || col < 0)
        row = col = -1;

    int oldRow = curRow;
    int oldCol = curCol;

    curRow = row;
    curCol = col;

    updateCell(oldRow, oldCol);
    updateCell(curRow, curCol);
}


/*
  Sets the currently selected cell to \a row, \a column. If \a row or
  \a column are less than zero, the current cell is unselected.

  Does not set the position of the focus indicator.
*/

void QWellArray::setSelected(int row, int col)
{
    if ((selRow == row) && (selCol == col))
        return;

    int oldRow = selRow;
    int oldCol = selCol;

    if (row < 0 || col < 0)
        row = col = -1;

    selCol = col;
    selRow = row;

    updateCell(oldRow, oldCol);
    updateCell(selRow, selCol);
    if (row >= 0)
        emit selected(row, col);

    if (isVisible() && qobject_cast<QMenu*>(parentWidget()))
        parentWidget()->close();
}



/*!\reimp
*/
void QWellArray::focusInEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}


void QWellArray::setCellBrush(int row, int col, const QBrush &b)
{
    if (!d) {
        d = new QWellArrayData;
        int i = numRows()*numCols();
        d->brush = new QBrush[i];
    }
    if (row >= 0 && row < numRows() && col >= 0 && col < numCols())
        d->brush[row*numCols()+col] = b;
}



/*
  Returns the brush set for the cell at \a row, \a column. If no brush is
  set, \c Qt::NoBrush is returned.
*/

QBrush QWellArray::cellBrush(int row, int col)
{
    if (d && row >= 0 && row < numRows() && col >= 0 && col < numCols())
        return d->brush[row*numCols()+col];
    return Qt::NoBrush;
}



/*!\reimp
*/

void QWellArray::focusOutEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}

/*\reimp
*/
void QWellArray::keyPressEvent(QKeyEvent* e)
{
    switch(e->key()) {                        // Look at the key code
    case Qt::Key_Left:                                // If 'left arrow'-key,
        if(curCol > 0)                        // and cr't not in leftmost col
            setCurrent(curRow, curCol - 1);        // set cr't to next left column
        break;
    case Qt::Key_Right:                                // Correspondingly...
        if(curCol < numCols()-1)
            setCurrent(curRow, curCol + 1);
        break;
    case Qt::Key_Up:
        if(curRow > 0)
            setCurrent(curRow - 1, curCol);
        break;
    case Qt::Key_Down:
        if(curRow < numRows()-1)
            setCurrent(curRow + 1, curCol);
        break;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        setSelected(curRow, curCol);
        break;
    default:                                // If not an interesting key,
        e->ignore();                        // we don't accept the event
        return;
    }

}

//////////// QWellArray END

static bool initrgb = false;
static QRgb stdrgb[6*8];
static QRgb cusrgb[2*8];
static bool customSet = false;


static void initRGB()
{
    if (initrgb)
        return;
    initrgb = true;
    int i = 0;
    for (int g = 0; g < 4; g++)
        for (int r = 0;  r < 4; r++)
            for (int b = 0; b < 3; b++)
                stdrgb[i++] = qRgb(r*255/3, g*255/3, b*255/2);

    for (i = 0; i < 2*8; i++)
        cusrgb[i] = 0xffffffff;
}

/*!
    Returns the number of custom colors supported by QColorDialog. All
    color dialogs share the same custom colors.
*/
int QColorDialog::customCount()
{
    return 2*8;
}

/*!
    Returns custom color number \a i as a QRgb value.
*/
QRgb QColorDialog::customColor(int i)
{
    initRGB();
    Q_ASSERT(i >= 0 && i < customCount());
    return cusrgb[i];
}

/*!
    \fn void QColorDialog::setCustomColor(int number, QRgb color)

    Sets the custom color \a number to the QRgb \a color value.
*/
void QColorDialog::setCustomColor(int i, QRgb c)
{
    initRGB();
    Q_ASSERT(i >= 0 && i < customCount());
    customSet = true;
    cusrgb[i] = c;
}

/*!
    \fn void QColorDialog::setStandardColor(int number, QRgb color)

    Sets the standard color \a number to the QRgb \a color value given.
*/

void QColorDialog::setStandardColor(int i, QRgb c)
{
    initRGB();
    Q_ASSERT(i >= 0 && i < 6*8);
    stdrgb[i] = c;
}

static inline void rgb2hsv(QRgb rgb, int&h, int&s, int&v)
{
    QColor c;
    c.setRgb(rgb);
    c.getHsv(&h, &s, &v);
}

class QColorWell : public QWellArray
{
public:
    QColorWell(QWidget *parent, int r, int c, QRgb *vals)
        :QWellArray(r, c, parent), values(vals), mousePressed(false), oldCurrent(-1, -1)
    { setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum)); }

protected:
    void paintCellContents(QPainter *, int row, int col, const QRect&);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
#endif

private:
    QRgb *values;
    bool mousePressed;
    QPoint pressPos;
    QPoint oldCurrent;

};

void QColorWell::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
    int i = row + col*numRows();
    p->fillRect(r, QColor(values[i]));
}

void QColorWell::mousePressEvent(QMouseEvent *e)
{
    oldCurrent = QPoint(selectedRow(), selectedColumn());
    QWellArray::mousePressEvent(e);
    mousePressed = true;
    pressPos = e->pos();
}

void QColorWell::mouseMoveEvent(QMouseEvent *e)
{
    QWellArray::mouseMoveEvent(e);
#ifndef QT_NO_DRAGANDDROP
    if (!mousePressed)
        return;
    if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        setCurrent(oldCurrent.x(), oldCurrent.y());
        int i = rowAt(pressPos.y()) + columnAt(pressPos.x()) * numRows();
        QColor col(values[i]);
        QMimeData *mime = new QMimeData;
        mime->setColorData(col);
        QPixmap pix(cellWidth(), cellHeight());
        pix.fill(col);
        QPainter p(&pix);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.end();
        QDrag *drg = new QDrag(this);
        drg->setMimeData(mime);
        drg->setPixmap(pix);
        mousePressed = false;
        drg->start();
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorWell::dragEnterEvent(QDragEnterEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid())
        e->accept();
    else
        e->ignore();
}

void QColorWell::dragLeaveEvent(QDragLeaveEvent *)
{
    if (hasFocus())
        parentWidget()->setFocus();
}

void QColorWell::dragMoveEvent(QDragMoveEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid()) {
        setCurrent(rowAt(e->pos().y()), columnAt(e->pos().x()));
        e->accept();
    } else {
        e->ignore();
    }
}

void QColorWell::dropEvent(QDropEvent *e)
{
    QColor col = qvariant_cast<QColor>(e->mimeData()->colorData());
    if (col.isValid()) {
        int i = rowAt(e->pos().y()) + columnAt(e->pos().x()) * numRows();
        values[i] = col.rgb();
        update();
        e->accept();
    } else {
        e->ignore();
    }
}

#endif // QT_NO_DRAGANDDROP

void QColorWell::mouseReleaseEvent(QMouseEvent *e)
{
    if (!mousePressed)
        return;
    QWellArray::mouseReleaseEvent(e);
    mousePressed = false;
}

class QColorPicker : public QFrame
{
    Q_OBJECT
public:
    QColorPicker(QWidget* parent);
    ~QColorPicker();

public slots:
    void setCol(int h, int s);

signals:
    void newCol(int h, int s);

protected:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    int hue;
    int sat;

    QPoint colPt();
    int huePt(const QPoint &pt);
    int satPt(const QPoint &pt);
    void setCol(const QPoint &pt);

    QPixmap *pix;
};

static int pWidth = 220;
static int pHeight = 200;

class QColorLuminancePicker : public QWidget
{
    Q_OBJECT
public:
    QColorLuminancePicker(QWidget* parent=0);
    ~QColorLuminancePicker();

public slots:
    void setCol(int h, int s, int v);
    void setCol(int h, int s);

signals:
    void newHsv(int h, int s, int v);

protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    enum { foff = 3, coff = 4 }; //frame and contents offset
    int val;
    int hue;
    int sat;

    int y2val(int y);
    int val2y(int val);
    void setVal(int v);

    QPixmap *pix;
};


int QColorLuminancePicker::y2val(int y)
{
    int d = height() - 2*coff - 1;
    return 255 - (y - coff)*255/d;
}

int QColorLuminancePicker::val2y(int v)
{
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent)
    :QWidget(parent)
{
    hue = 100; val = 100; sat = 100;
    pix = 0;
    //    setAttribute(WA_NoErase, true);
}

QColorLuminancePicker::~QColorLuminancePicker()
{
    delete pix;
}

void QColorLuminancePicker::mouseMoveEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}
void QColorLuminancePicker::mousePressEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}

void QColorLuminancePicker::setVal(int v)
{
    if (val == v)
        return;
    val = qMax(0, qMin(v,255));
    delete pix; pix=0;
    repaint();
    emit newHsv(hue, sat, val);
}

//receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol(int h, int s)
{
    setCol(h, s, val);
    emit newHsv(h, s, val);
}

void QColorLuminancePicker::paintEvent(QPaintEvent *)
{
    int w = width() - 5;

    QRect r(0, foff, w, height() - 2*foff);
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if (!pix || pix->height() != hi || pix->width() != wi) {
        delete pix;
        QImage img(wi, hi, QImage::Format_RGB32);
        int y;
        for (y = 0; y < hi; y++) {
            QColor c;
            c.setHsv(hue, sat, y2val(y+coff));
            QRgb r = c.rgb();
            int x;
            for (x = 0; x < wi; x++)
                img.setPixel(x, y, r);
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }
    QPainter p(this);
    p.drawPixmap(1, coff, *pix);
    const QPalette &g = palette();
    qDrawShadePanel(&p, r, g, true);
    p.setPen(g.foreground().color());
    p.setBrush(g.foreground());
    QPolygon a;
    int y = val2y(val);
    a.setPoints(3, w, y, w+5, y+5, w+5, y-5);
    p.eraseRect(w, 0, 5, height());
    p.drawPolygon(a);
}

void QColorLuminancePicker::setCol(int h, int s , int v)
{
    val = v;
    hue = h;
    sat = s;
    delete pix; pix=0;
    repaint();
}

QPoint QColorPicker::colPt()
{ return QPoint((360-hue)*(pWidth-1)/360, (255-sat)*(pHeight-1)/255); }
int QColorPicker::huePt(const QPoint &pt)
{ return 360 - pt.x()*360/(pWidth-1); }
int QColorPicker::satPt(const QPoint &pt)
{ return 255 - pt.y()*255/(pHeight-1) ; }
void QColorPicker::setCol(const QPoint &pt)
{ setCol(huePt(pt), satPt(pt)); }

QColorPicker::QColorPicker(QWidget* parent)
    : QFrame(parent)
{
    hue = 0; sat = 0;
    setCol(150, 255);

    QImage img(pWidth, pHeight, QImage::Format_RGB32);
    int x,y;
    for (y = 0; y < pHeight; y++)
        for (x = 0; x < pWidth; x++) {
            QPoint p(x, y);
            QColor c;
            c.setHsv(huePt(p), satPt(p), 200);
            img.setPixel(x, y, c.rgb());
        }
    pix = new QPixmap(QPixmap::fromImage(img));
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
}

QColorPicker::~QColorPicker()
{
    delete pix;
}

QSize QColorPicker::sizeHint() const
{
    return QSize(pWidth + 2*frameWidth(), pHeight + 2*frameWidth());
}

void QColorPicker::setCol(int h, int s)
{
    int nhue = qMin(qMax(0,h), 359);
    int nsat = qMin(qMax(0,s), 255);
    if (nhue == hue && nsat == sat)
        return;
    QRect r(colPt(), QSize(20,20));
    hue = nhue; sat = nsat;
    r = r.unite(QRect(colPt(), QSize(20,20)));
    r.translate(contentsRect().x()-9, contentsRect().y()-9);
    //    update(r);
    repaint(r);
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent* )
{
    QPainter p(this);
    drawFrame(&p);
    QRect r = contentsRect();

    p.drawPixmap(r.topLeft(), *pix);
    QPoint pt = colPt() + r.topLeft();
    p.setPen(Qt::black);

    p.fillRect(pt.x()-9, pt.y(), 20, 2, Qt::black);
    p.fillRect(pt.x(), pt.y()-9, 2, 20, Qt::black);

}

class QColSpinBox : public QSpinBox
{
public:
    QColSpinBox(QWidget *parent)
        : QSpinBox(parent) { setRange(0, 255); }
    void setValue(int i) {
        bool block = signalsBlocked();
        blockSignals(true);
        QSpinBox::setValue(i);
        blockSignals(block);
    }
};

class QColorShowLabel;

class QColorShower : public QWidget
{
    Q_OBJECT
public:
    QColorShower(QWidget *parent);

    //things that don't emit signals
    void setHsv(int h, int s, int v);

    int currentAlpha() const { return alphaEd->value(); }
    void setCurrentAlpha(int a) { alphaEd->setValue(a); }
    void showAlpha(bool b);


    QRgb currentColor() const { return curCol; }

public slots:
    void setRgb(QRgb rgb);

signals:
    void newCol(QRgb rgb);
private slots:
    void rgbEd();
    void hsvEd();
private:
    void showCurrentColor();
    int hue, sat, val;
    QRgb curCol;
    QColSpinBox *hEd;
    QColSpinBox *sEd;
    QColSpinBox *vEd;
    QColSpinBox *rEd;
    QColSpinBox *gEd;
    QColSpinBox *bEd;
    QColSpinBox *alphaEd;
    QLabel *alphaLab;
    QColorShowLabel *lab;
    bool rgbOriginal;
};

class QColorShowLabel : public QFrame
{
    Q_OBJECT

public:
    QColorShowLabel(QWidget *parent) : QFrame(parent) {
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
        setAcceptDrops(true);
        mousePressed = false;
    }
    void setColor(QColor c) { col = c; }

signals:
    void colorDropped(QRgb);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
#endif

private:
    QColor col;
    bool mousePressed;
    QPoint pressPos;

};

void QColorShowLabel::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    drawFrame(&p);
    p.fillRect(contentsRect()&e->rect(), col);
}

void QColorShower::showAlpha(bool b)
{
    if (b) {
        alphaLab->show();
        alphaEd->show();
    } else {
        alphaLab->hide();
        alphaEd->hide();
    }
}

void QColorShowLabel::mousePressEvent(QMouseEvent *e)
{
    mousePressed = true;
    pressPos = e->pos();
}

void QColorShowLabel::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_DRAGANDDROP
    if (!mousePressed)
        return;
    if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData *mime = new QMimeData;
        mime->setColorData(col);
        QPixmap pix(30, 20);
        pix.fill(col);
        QPainter p(&pix);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.end();
        QDrag *drg = new QDrag(this);
        drg->setMimeData(mime);
        drg->setPixmap(pix);
        mousePressed = false;
        drg->start();
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorShowLabel::dragEnterEvent(QDragEnterEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid())
        e->accept();
    else
        e->ignore();
}

void QColorShowLabel::dragLeaveEvent(QDragLeaveEvent *)
{
}

void QColorShowLabel::dropEvent(QDropEvent *e)
{
    QColor color = qvariant_cast<QColor>(e->mimeData()->colorData());
    if (color.isValid()) {
        col = color;
        repaint();
        emit colorDropped(col.rgb());
        e->accept();
    } else {
        e->ignore();
    }
}
#endif // QT_NO_DRAGANDDROP

void QColorShowLabel::mouseReleaseEvent(QMouseEvent *)
{
    if (!mousePressed)
        return;
    mousePressed = false;
}

QColorShower::QColorShower(QWidget *parent)
    :QWidget(parent)
{
    curCol = qRgb(-1, -1, -1);

    QGridLayout *gl = new QGridLayout(this);
    gl->setMargin(6);
    lab = new QColorShowLabel(this);
    lab->setMinimumWidth(60);
    gl->addWidget(lab, 0, 0, -1, 1);
    connect(lab, SIGNAL(colorDropped(QRgb)),
             this, SIGNAL(newCol(QRgb)));
    connect(lab, SIGNAL(colorDropped(QRgb)),
             this, SLOT(setRgb(QRgb)));

    hEd = new QColSpinBox(this);
    hEd->setRange(0, 359);
    QLabel *l = new QLabel(QColorDialog::tr("Hu&e:"), this);
    l->setBuddy(hEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 0, 1);
    gl->addWidget(hEd, 0, 2);

    sEd = new QColSpinBox(this);
    l = new QLabel(QColorDialog::tr("&Sat:"), this);
    l->setBuddy(sEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 1, 1);
    gl->addWidget(sEd, 1, 2);

    vEd = new QColSpinBox(this);
    l = new QLabel(QColorDialog::tr("&Val:"), this);
    l->setBuddy(vEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 2, 1);
    gl->addWidget(vEd, 2, 2);

    rEd = new QColSpinBox(this);
    l = new QLabel(QColorDialog::tr("&Red:"), this);
    l->setBuddy(rEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 0, 3);
    gl->addWidget(rEd, 0, 4);

    gEd = new QColSpinBox(this);
    l = new QLabel(QColorDialog::tr("&Green:"), this);
    l->setBuddy(gEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 1, 3);
    gl->addWidget(gEd, 1, 4);

    bEd = new QColSpinBox(this);
    l = new QLabel(QColorDialog::tr("Bl&ue:"), this);
    l->setBuddy(bEd);
    l->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(l, 2, 3);
    gl->addWidget(bEd, 2, 4);

    alphaEd = new QColSpinBox(this);
    alphaLab = new QLabel(QColorDialog::tr("A&lpha channel:"), this);
    l->setBuddy(alphaEd);
    alphaLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(alphaLab, 3, 1, 1, 3);
    gl->addWidget(alphaEd, 3, 4);
    alphaEd->hide();
    alphaLab->hide();

    connect(hEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
    connect(sEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
    connect(vEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));

    connect(rEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(gEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(bEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(alphaEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
}

void QColorShower::showCurrentColor()
{
    lab->setColor(currentColor());
    lab->repaint();
}

void QColorShower::rgbEd()
{
    rgbOriginal = true;
    if (alphaEd->isVisible())
        curCol = qRgba(rEd->value(), gEd->value(), bEd->value(), currentAlpha());
    else
        curCol = qRgb(rEd->value(), gEd->value(), bEd->value());

    rgb2hsv(currentColor(), hue, sat, val);

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    showCurrentColor();
    emit newCol(currentColor());
}

void QColorShower::hsvEd()
{
    rgbOriginal = false;
    hue = hEd->value();
    sat = sEd->value();
    val = vEd->value();

    QColor c;
    c.setHsv(hue, sat, val);
    curCol = c.rgb();

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
    emit newCol(currentColor());
}

void QColorShower::setRgb(QRgb rgb)
{
    rgbOriginal = true;
    curCol = rgb;

    rgb2hsv(currentColor(), hue, sat, val);

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
}

void QColorShower::setHsv(int h, int s, int v)
{
    if (h < -1 || (uint)s > 255 || (uint)v > 255)
	return;

    rgbOriginal = false;
    hue = h; val = v; sat = s;
    QColor c;
    c.setHsv(hue, sat, val);
    curCol = c.rgb();

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
}

class QColorDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QColorDialog)
public:
    void init();
    QRgb currentColor() const { return cs->currentColor(); }
    void setCurrentColor(QRgb rgb);

    int currentAlpha() const { return cs->currentAlpha(); }
    void setCurrentAlpha(int a) { cs->setCurrentAlpha(a); }
    void showAlpha(bool b) { cs->showAlpha(b); }

    void addCustom();

    void newHsv(int h, int s, int v);
    void newColorTypedIn(QRgb rgb);
    void newCustom(int, int);
    void newStandard(int, int);

    QWellArray *custom;
    QWellArray *standard;

    QColorPicker *cp;
    QColorLuminancePicker *lp;
    QColorShower *cs;
    int nextCust;
    bool compact;
};

//sets all widgets to display h,s,v
void QColorDialogPrivate::newHsv(int h, int s, int v)
{
    cs->setHsv(h, s, v);
    cp->setCol(h, s);
    lp->setCol(h, s, v);
}

//sets all widgets to display rgb
void QColorDialogPrivate::setCurrentColor(QRgb rgb)
{
    cs->setRgb(rgb);
    newColorTypedIn(rgb);
}

//sets all widgets exept cs to display rgb
void QColorDialogPrivate::newColorTypedIn(QRgb rgb)
{
    int h, s, v;
    rgb2hsv(rgb, h, s, v);
    cp->setCol(h, s);
    lp->setCol(h, s, v);
}

void QColorDialogPrivate::newCustom(int r, int c)
{
    int i = r+2*c;
    setCurrentColor(cusrgb[i]);
    nextCust = i;
    if (standard)
        standard->setSelected(-1,-1);
}

void QColorDialogPrivate::newStandard(int r, int c)
{
    setCurrentColor(stdrgb[r+c*6]);
    if (custom)
        custom->setSelected(-1,-1);
}

void QColorDialogPrivate::init()
{
    Q_Q(QColorDialog);
    compact = false;
    // small displays (e.g. PDAs cannot fit the full color dialog,
    // so just use the color picker.
    if (qApp->desktop()->width() < 480 || qApp->desktop()->height() < 350)
        compact = true;

    nextCust = 0;
    const int lumSpace = 3;
    int border = 12;
    if (compact)
        border = 6;
    QHBoxLayout *topLay = new QHBoxLayout(q);
    topLay->setMargin(border);
    topLay->setSpacing(6);
    QVBoxLayout *leftLay = 0;

    if (!compact) {
        leftLay = new QVBoxLayout;
        topLay->addLayout(leftLay);
    }

    initRGB();

    if (!compact) {
        standard = new QColorWell(q, 6, 8, stdrgb);
        QLabel *lab = new QLabel(QColorDialog::tr("&Basic colors"), q);
        lab->setBuddy(standard);
        q->connect(standard, SIGNAL(selected(int,int)), SLOT(newStandard(int,int)));
        leftLay->addWidget(lab);
        leftLay->addWidget(standard);


        leftLay->addStretch();

        custom = new QColorWell(q, 2, 8, cusrgb);
        custom->setAcceptDrops(true);

        q->connect(custom, SIGNAL(selected(int,int)), SLOT(newCustom(int,int)));
        lab = new QLabel(QColorDialog::tr("&Custom colors") , q);
        lab->setBuddy(custom);
        leftLay->addWidget(lab);
        leftLay->addWidget(custom);

        QPushButton *custbut = new QPushButton(QColorDialog::tr("&Define Custom Colors >>"), q);
        custbut->setEnabled(false);
        leftLay->addWidget(custbut);
    } else {
        // better color picker size for small displays
        pWidth = 150;
        pHeight = 100;
        custom = 0;
        standard = 0;
    }

    QVBoxLayout *rightLay = new QVBoxLayout;
    topLay->addLayout(rightLay);

    QHBoxLayout *pickLay = new QHBoxLayout;
    rightLay->addLayout(pickLay);


    QVBoxLayout *cLay = new QVBoxLayout;
    pickLay->addLayout(cLay);
    cp = new QColorPicker(q);
    cp->setFrameStyle(QFrame::Panel + QFrame::Sunken);
    cLay->addSpacing(lumSpace);
    cLay->addWidget(cp);
    cLay->addSpacing(lumSpace);

    lp = new QColorLuminancePicker(q);
    lp->setFixedWidth(20);
    pickLay->addWidget(lp);

    QObject::connect(cp, SIGNAL(newCol(int,int)), lp, SLOT(setCol(int,int)));
    QObject::connect(lp, SIGNAL(newHsv(int,int,int)), q, SLOT(newHsv(int,int,int)));

    rightLay->addStretch();

    cs = new QColorShower(q);
    QObject::connect(cs, SIGNAL(newCol(QRgb)), q, SLOT(newColorTypedIn(QRgb)));
    rightLay->addWidget(cs);

    QHBoxLayout *buttons;
    if (compact) {
        buttons = new QHBoxLayout;
        rightLay->addLayout(buttons);
    } else {
        buttons = new QHBoxLayout;
        leftLay->addLayout(buttons);
    }

    QPushButton *ok, *cancel;
    ok = new QPushButton(QColorDialog::tr("OK"), q);
    QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
    ok->setDefault(true);
    cancel = new QPushButton(QColorDialog::tr("Cancel"), q);
    QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));
    buttons->addWidget(ok);
    buttons->addWidget(cancel);
    buttons->addStretch();

    if (!compact) {
        QPushButton *addCusBt = new QPushButton(QColorDialog::tr("&Add to Custom Colors"), q);
        rightLay->addWidget(addCusBt);
        QObject::connect(addCusBt, SIGNAL(clicked()), q, SLOT(addCustom()));
    }
}

void QColorDialogPrivate::addCustom()
{
    cusrgb[nextCust] = cs->currentColor();
    if (custom)
        custom->update();
    nextCust = (nextCust+1) % 16;
}


/*!
    \class QColorDialog qcolordialog.h
    \brief The QColorDialog class provides a dialog widget for specifying colors.

    \mainclass
    \ingroup dialogs
    \ingroup multimedia

    The color dialog's function is to allow users to choose colors.
    For example, you might use this in a drawing program to allow the
    user to set the brush color.

    The static functions provide modal color dialogs.
    \omit
    If you require a modeless dialog, use the QColorDialog constructor.
    \endomit

    The static getColor() function shows the dialog, and allows the
    user to specify a color. The getRgba() function does the same, but
    also allows the user to specify a color with an alpha channel
    (transparency) value.

    The user can store customCount() different custom colors. The
    custom colors are shared by all color dialogs, and remembered
    during the execution of the program. Use setCustomColor() to set
    the custom colors, and use customColor() to get them.

    Additional widgets that allow users to pick colors are available
    as \link
    http://www.trolltech.com/products/solutions/index.html Qt
    Solutions\endlink.

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QColorDialog as well as other built-in Qt dialogs.

    \img qcolordlg-w.png
*/

/*!
    Constructs a default color dialog called \a name with the given \a parent.
    If \a modal is true the dialog will be modal. Use setColor() to set an
    initial value.

    \sa getColor()
*/

QColorDialog::QColorDialog(QWidget* parent, bool modal) :
    QDialog(*new QColorDialogPrivate, parent, (Qt::Dialog | Qt::WindowTitleHint |
                     Qt::MSWindowsFixedSizeDialogHint | Qt::WindowSystemMenuHint))
{
    Q_D(QColorDialog);
    setModal(modal);
    setSizeGripEnabled(false);
    d->init();

#ifndef QT_NO_SETTINGS
    if (!customSet) {
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("Qt"));
        for (int i = 0; i < 2*8; ++i) {
            QVariant v = settings.value(QLatin1String("customColors/") + QString::number(i));
            if (v.isValid()) {
                QRgb rgb = v.toUInt();
                cusrgb[i] = rgb;
            }
        }
        settings.endGroup(); // Qt
    }
#endif
}

/*!
    Pops up a modal color dialog, lets the user choose a color, and
    returns that color. The color is initially set to \a initial. The
    dialog is a child of \a parent. It returns an invalid (see
    QColor::isValid()) color if the user cancels the dialog. All
    colors allocated by the dialog will be deallocated before this
    function returns.
*/

QColor QColorDialog::getColor(const QColor& initial, QWidget *parent)
{
#if defined(Q_WS_MAC)
    return macGetColor(initial, parent);
#endif

    QColorDialog *dlg = new QColorDialog(parent, true);  //modal
    dlg->setWindowTitle(QColorDialog::tr("Select color"));
    dlg->setColor(initial);
    dlg->selectColor(initial);
    int resultCode = dlg->exec();
    QColor result;
    if (resultCode == QDialog::Accepted)
        result = dlg->color();
    delete dlg;
    return result;
}


/*!
    Pops up a modal color dialog to allow the user to choose a color
    and an alpha channel (transparency) value. The color+alpha is
    initially set to \a initial. The dialog is a child of \a parent.

    If \a ok is non-null, \e *\a ok is set to true if the user clicked
    OK, and to false if the user clicked Cancel.

    If the user clicks Cancel, the \a initial value is returned.
*/

QRgb QColorDialog::getRgba(QRgb initial, bool *ok, QWidget *parent)
{
#if defined(Q_WS_MAC)
    return macGetRgba(initial, ok, parent);
#endif

    QColorDialog *dlg = new QColorDialog(parent, true);  //modal

    dlg->setWindowTitle(QColorDialog::tr("Select color"));
    dlg->setColor(initial);
    dlg->selectColor(initial);
    dlg->setSelectedAlpha(qAlpha(initial));
    int resultCode = dlg->exec();
    QRgb result = initial;
    if (resultCode == QDialog::Accepted) {
        QRgb c = dlg->color().rgb();
        int alpha = dlg->selectedAlpha();
        result = qRgba(qRed(c), qGreen(c), qBlue(c), alpha);
    }
    if (ok)
        *ok = resultCode == QDialog::Accepted;

    delete dlg;
    return result;
}





/*!
    Returns the currently selected color in the dialog.

    \sa setColor()
*/

QColor QColorDialog::color() const
{
    Q_D(const QColorDialog);
    return QColor(d->currentColor());
}


/*!
    Destroys the color dialog.
*/

QColorDialog::~QColorDialog()
{
#ifndef QT_NO_SETTINGS
    if (!customSet) {
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("Qt"));
        for (int i = 0; i < 2*8; ++i)
            settings.setValue(QLatin1String("customColors/") + QString::number(i), cusrgb[i]);
        settings.endGroup();
    }
#endif
}


/*!
    \fn void QColorDialog::setColor(const QColor &color)

    Sets the color shown in the dialog to the \a color given.

    \sa color()
*/

void QColorDialog::setColor(const QColor& c)
{
    Q_D(QColorDialog);
    d->setCurrentColor(c.rgb());
}




/*!
    \fn void QColorDialog::setSelectedAlpha(int alpha)

    Sets the initial alpha channel value to the \a alpha value given, and
    shows the alpha channel entry box.
*/

void QColorDialog::setSelectedAlpha(int a)
{
    Q_D(QColorDialog);
    d->showAlpha(true);
    d->setCurrentAlpha(a);
}


/*!
    Returns the value selected for the alpha channel.
*/

int QColorDialog::selectedAlpha() const
{
    Q_D(const QColorDialog);
    return d->currentAlpha();
}

/*!
    Sets focus to the corresponding button, if any.
*/
bool QColorDialog::selectColor(const QColor& col)
{
    Q_D(QColorDialog);
    QRgb color = col.rgb();
    int i = 0, j = 0;
    // Check standard colors
    if (d->standard) {
        for (i = 0; i < 6; i++) {
            for (j = 0; j < 8; j++) {
                if (color == stdrgb[i + j*6]) {
                    d->newStandard(i, j);
                    d->standard->setCurrent(i, j);
                    d->standard->setSelected(i, j);
                    d->standard->setFocus();
                    return true;
                }
            }
        }
    }
    // Check custom colors
    if (d->custom) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 8; j++) {
                if (color == cusrgb[i + j*2]) {
                    d->newCustom(i, j);
                    d->custom->setCurrent(i, j);
                    d->custom->setSelected(i, j);
                    d->custom->setFocus();
                    return true;
                }
            }
        }
    }
    return false;
}

#include "qcolordialog.moc"
#include "moc_qcolordialog.cpp"

#endif

/*!
    \fn QColor QColorDialog::getColor(const QColor &init, QWidget *parent, const char *name)
    \compat
*/

/*!
    \fn QRgb QColorDialog::getRgba(QRgb rgba, bool *ok, QWidget *parent, const char *name)
    \compat
*/
