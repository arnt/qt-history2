/****************************************************************************
**
** Implementation of Q3GroupBox widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3groupbox.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qradiobutton.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qevent.h"
#include "qstyle.h"
#include "qcheckbox.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#include <private/qwidget_p.h>

class Q3GroupBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(Q3GroupBox)
public:

    Q3GroupBoxPrivate():
        spacer(0),
        checkbox(0),
        topMargin(0){}
    void skip();
    void init();
    void calculateFrame();
    void insertWid(QWidget*);
    void setTextSpacer();
#ifndef QT_NO_CHECKBOX
    void updateCheckBoxGeometry();
#endif
    QString str;
    int align;
    int lenvisible;
#ifndef QT_NO_ACCEL
    QAccel * accel;
#endif

    void fixFocus();
    void setChildrenEnabled(bool b);
    QVBoxLayout *vbox;
    QGridLayout *grid;
    int row;
    int col : 30;
    uint bFlat : 1;
    int nRows, nCols;
    Orientation dir;
    int spac, marg;

    QSpacerItem *spacer;
    QCheckBox *checkbox;
    int topMargin;
};

#define d d_func()
#define q q_func()


/*!
    \class Q3GroupBox qgroupbox.h
    \brief The Q3GroupBox widget provides a group box frame with a title.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A group box provides a frame, a title and a keyboard shortcut, and
    displays various other widgets inside itself. The title is on top,
    the keyboard shortcut moves keyboard focus to one of the group
    box's child widgets, and the child widgets are usually laid out
    horizontally (or vertically) inside the frame.

    The simplest way to use it is to create a group box with the
    desired number of columns (or rows) and orientation, and then just
    create widgets with the group box as parent.

    It is also possible to change the orientation() and number of
    columns() after construction, or to ignore all the automatic
    layout support and manage the layout yourself. You can add 'empty'
    spaces to the group box with addSpace().

    Q3GroupBox also lets you set the title() (normally set in the
    constructor) and the title's alignment().

    You can change the spacing used by the group box with
    setInsideMargin() and setInsideSpacing(). To minimize space
    consumption, you can remove the right, left and bottom edges of
    the frame with setFlat().

    <img src=qgrpbox-w.png>

    \sa Q3ButtonGroup
*/



/*!
    Constructs a group box widget with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(QWidget *parent, const char *name)
    : QWidget(*new Q3GroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
}

/*!
    Constructs a group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(const QString &title, QWidget *parent, const char *name)
    : QWidget(*new Q3GroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
    setTitle(title);
}

/*!
    Constructs a group box with no title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Orientation orientation,
                    QWidget *parent, const char *name)
    : QWidget(*new Q3GroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
    setColumnLayout(strips, orientation);
}

/*!
    Constructs a group box titled \a title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Orientation orientation,
                    const QString &title, QWidget *parent,
                    const char *name)
    : QWidget(*new Q3GroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
    setTitle(title);
    setColumnLayout(strips, orientation);
}

/*!
    Destroys the group box.
*/
Q3GroupBox::~Q3GroupBox()
{
}

void Q3GroupBoxPrivate::init()
{
    align = AlignAuto;
#ifndef QT_NO_ACCEL
    accel = 0;
#endif
    vbox = 0;
    grid = 0;
    lenvisible = 0;
    nCols = nRows = 0;
    dir = Horizontal;
    marg = 11;
    spac = 5;
    bFlat = false;
}

void Q3GroupBoxPrivate::setTextSpacer()
{
    if (!spacer)
        return;
    int h = 0;
    int w = 0;
    if (checkbox || lenvisible) {
        QFontMetrics fm = q->fontMetrics();
        int fh = fm.height();
        if (checkbox) {
            fh = checkbox->sizeHint().height() + 2;
            w = checkbox->sizeHint().width() + 2*fm.width("xx");
        } else {
            fh = fm.height();
            w = fm.width(str, lenvisible) + 2*fm.width("xx");
        }
        h = topMargin;
        QLayout *layout = q->layout();
        if (layout) {
            int m = layout->margin();
            int sp = layout->spacing();
            // do we have a child layout?
            int i = 0;
            while (QLayoutItem *child = layout->itemAt(i++)) {
                if (QLayout *childLayout = child->layout()) {
                    m += childLayout->margin();
                    sp = qMax(sp, childLayout->spacing());
                    break;
                }
            }
            h = qMax(fh-m, h);
            h += qMax(sp - (h+m - fh), 0);
        }
    }
    spacer->changeSize(w, h, QSizePolicy::Minimum, QSizePolicy::Fixed);
}


void Q3GroupBox::setTitle(const QString &title)
{
    if (d->str == title)                                // no change
        return;
    d->str = title;
#ifndef QT_NO_ACCEL
    if (d->accel)
        delete d->accel;
    d->accel = 0;
    int s = QAccel::shortcutKey(title);
    if (s) {
        d->accel = new QAccel(this, "automatic focus-change accelerator");
        d->accel->connectItem(d->accel->insertItem(s, 0),
                            this, SLOT(fixFocus()));
    }
#endif
    if (d->checkbox) {
        d->checkbox->setText(d->str);
        d->updateCheckBoxGeometry();
    }
    d->calculateFrame();
    d->setTextSpacer();

    update();
    updateGeometry();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
#endif
}

/*!
    \property Q3GroupBox::title
    \brief the group box title text.

    The group box title text will have a focus-change keyboard
    accelerator if the title contains \&, followed by a letter.

    \code
        g->setTitle("&User information");
    \endcode
    This produces "<u>U</u>ser information"; Alt+U moves the keyboard
    focus to the group box.

    There is no default title text.
*/

QString Q3GroupBox::title() const
{
    return d->str;
}

/*!
    \property Q3GroupBox::alignment
    \brief the alignment of the group box title.

    The title is always placed on the upper frame line. The horizontal
    alignment can be specified by the alignment parameter.

    The alignment is one of the following flags:
    \list
    \i \c AlignAuto aligns the title according to the language,
    usually to the left.
    \i \c AlignLeft aligns the title text to the left.
    \i \c AlignRight aligns the title text to the right.
    \i \c AlignHCenter aligns the title text centered.
    \endlist

    The default alignment is \c AlignAuto.

    \sa Qt::AlignmentFlags
*/
int Q3GroupBox::alignment() const
{
    return d->align;
}

void Q3GroupBox::setAlignment(int alignment)
{
    d->align = alignment;
    d->updateCheckBoxGeometry();
    update();
}

/*! \reimp
*/
void Q3GroupBox::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (d->align & AlignRight || d->align & AlignCenter ||
         (QApplication::reverseLayout() && !(d->align & AlignLeft)))
        d->updateCheckBoxGeometry();
    d->calculateFrame();
}

/*! \reimp
*/

void Q3GroupBox::paintEvent(QPaintEvent *event)
{
    QPainter paint(this);

    QRect frameRect = rect();
    frameRect.setTop(d->topMargin);

    if (d->lenvisible && !isCheckable()) {        // draw title
        QFontMetrics fm = paint.fontMetrics();
        int h = fm.height();
        int tw = fm.width(d->str, d->lenvisible) + fm.width(QChar(' '));
        int x;
        int marg = d->bFlat ? 0 : 8;
        if (d->align & AlignHCenter)                // center alignment
            x = frameRect.width()/2 - tw/2;
        else if (d->align & AlignRight)        // right alignment
            x = frameRect.width() - tw - marg;
        else if (d->align & AlignLeft)                 // left alignment
            x = marg;
        else { // auto align
            if(QApplication::reverseLayout())
                x = frameRect.width() - tw - marg;
            else
                x = marg;
        }
        QRect r(x, 0, tw, h);
        int va = style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, this);
        if(va & AlignTop)
            r.moveBy(0, fm.descent());
        QColor pen((QRgb) style().styleHint(QStyle::SH_GroupBox_TextLabelColor, this));
        if (!style().styleHint(QStyle::SH_UnderlineAccelerator, this))
            va |= NoAccel;
        style().drawItem(&paint, r, ShowPrefix | AlignHCenter | va, palette(),
                          isEnabled(), QPixmap(), d->str, -1,
                          testAttribute(WA_SetPalette) ? 0 : &pen);
        paint.setClipRegion(event->region().subtract(r)); // clip everything but title
    } else if (d->checkbox) {
        QRect cbClip = d->checkbox->geometry();
        QFontMetrics fm = paint.fontMetrics();
        cbClip.setX(cbClip.x() - fm.width(QChar(' ')));
        cbClip.setWidth(cbClip.width() + fm.width(QChar(' ')));
        paint.setClipRegion(event->region().subtract(cbClip));
    }
    if (d->bFlat) {
            QRect fr = frameRect;
            QPoint p1(fr.x(), fr.y() + 1);
            QPoint p2(fr.x() + fr.width(), p1.y());
            // ### This should probably be a style primitive.
            qDrawShadeLine(&paint, p1, p2, palette(), true, 1, 0);
    } else {
        QStyleOption opt(1, 0);
        QStyle::SFlags flags = QStyle::Style_Default | QStyle::Style_Sunken;
        if (hasFocus())
            flags |= QStyle::Style_HasFocus;
        if (testAttribute(WA_UnderMouse))
            flags |= QStyle::Style_MouseOver;
        style().drawPrimitive(QStyle::PE_PanelGroupBox, &paint, frameRect, palette(), flags, opt);
    }
}


/*!
    Adds an empty cell at the next free position. If \a size is
    greater than 0, the empty cell takes \a size to be its fixed width
    (if orientation() is \c Horizontal) or height (if orientation() is
    \c Vertical).

    Use this method to separate the widgets in the group box or to
    skip the next free cell. For performance reasons, call this method
    after calling setColumnLayout() or by changing the \l
    Q3GroupBox::columns or \l Q3GroupBox::orientation properties. It is
    generally a good idea to call these methods first (if needed at
    all), and insert the widgets and spaces afterwards.
*/
void Q3GroupBox::addSpace(int size)
{
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    if (d->nCols <= 0 || d->nRows <= 0)
        return;

    if (size > 0) {
        QSpacerItem *spacer
            = new QSpacerItem((d->dir == Horizontal) ? 0 : size,
                               (d->dir == Vertical) ? 0 : size,
                               (d->dir == Horizontal) ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                               (d->dir == Vertical) ? QSizePolicy::Fixed : QSizePolicy::Expanding);
        d->grid->addItem(spacer, d->row, d->col);
    }

    d->skip();
}

/*!
    \property Q3GroupBox::columns
    \brief the number of columns or rows (depending on \l Q3GroupBox::orientation) in the group box

    Usually it is not a good idea to set this property because it is
    slow (it does a complete layout). It is best to set the number
    of columns directly in the constructor.
*/
int Q3GroupBox::columns() const
{
    if (d->dir == Horizontal)
        return d->nCols;
    return d->nRows;
}

void Q3GroupBox::setColumns(int c)
{
    setColumnLayout(c, d->dir);
}

/*!
    Returns the width of the empty space between the items in the
    group and the frame of the group.

    Only applies if the group box has a defined orientation.

    The default is usually 11, by may vary depending on the platform
    and style.

    \sa setInsideMargin(), orientation
*/
int Q3GroupBox::insideMargin() const
{
    return d->marg;
}

/*!
    Returns the width of the empty space between each of the items
    in the group.

    Only applies if the group box has a defined orientation.

    The default is usually 5, by may vary depending on the platform
    and style.

    \sa setInsideSpacing(), orientation
*/
int Q3GroupBox::insideSpacing() const
{
    return d->spac;
}

/*!
    Sets the the width of the inside margin to \a m pixels.

    \sa insideMargin()
*/
void Q3GroupBox::setInsideMargin(int m)
{
    d->marg = m;
    setColumnLayout(columns(), d->dir);
}

/*!
    Sets the width of the empty space between each of the items in
    the group to \a s pixels.

    \sa insideSpacing()
*/
void Q3GroupBox::setInsideSpacing(int s)
{
    d->spac = s;
    setColumnLayout(columns(), d->dir);
}

/*!
    \property Q3GroupBox::orientation
    \brief the group box's orientation

    A horizontal group box arranges it's children in columns, while a
    vertical group box arranges them in rows.

    Usually it is not a good idea to set this property because it is
    slow (it does a complete layout). It is better to set the
    orientation directly in the constructor.
*/
Q3GroupBox::Orientation Q3GroupBox::orientation() const
{
    return d->dir;
}

void Q3GroupBox::setOrientation(Qt::Orientation o)
{
    setColumnLayout(columns(), o);
}

/*!
    Changes the layout of the group box. This function is only useful
    in combination with the default constructor that does not take any
    layout information. This function will put all existing children
    in the new layout. It is not good Qt programming style to call
    this function after children have been inserted. Sets the number
    of columns or rows to be \a strips, depending on \a direction.

    \sa orientation columns
*/
void Q3GroupBox::setColumnLayout(int strips, Orientation direction)
{
    if (layout())
        delete layout();

    d->vbox = 0;
    d->grid = 0;

    if (strips < 0) // if 0, we create the vbox but not the grid. See below.
        return;

    d->vbox = new QVBoxLayout(this, d->marg, 0);

    d->spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                 QSizePolicy::Fixed);

    d->setTextSpacer();
    d->vbox->addItem(d->spacer);

    d->nCols = 0;
    d->nRows = 0;
    d->dir = direction;

    // Send all child events and ignore them. Otherwise we will end up
    // with doubled insertion. This won't do anything because nCols ==
    // nRows == 0.
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    // if 0 or smaller , create a vbox-layout but no grid. This allows
    // the designer to handle its own grid layout in a group box.
    if (strips <= 0)
        return;

    d->dir = direction;
    if (d->dir == Horizontal) {
        d->nCols = strips;
        d->nRows = 1;
    } else {
        d->nCols = 1;
        d->nRows = strips;
    }
    d->grid = new QGridLayout(d->nRows, d->nCols, d->spac);
    d->row = d->col = 0;
    d->grid->setAlignment(AlignTop);
    d->vbox->addLayout(d->grid);

    // Add all children
    QObjectList childs = children();
    if (!childs.isEmpty()) {
        for (int i = 0; i < childs.size(); ++i) {
            QObject *o = childs.at(i);
            if (o->isWidgetType()
                 && o != d->checkbox
               )
                d->insertWid(static_cast<QWidget *>(o));
        }
    }
}


/*! \reimp  */
bool Q3GroupBox::event(QEvent * e)
{
    if (e->type() == QEvent::LayoutHint && layout())
        d->setTextSpacer();
    return QWidget::event(e);
}

/*!\reimp */
void Q3GroupBox::childEvent(QChildEvent *c)
{
    if (c->type() != QEvent::ChildInserted || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
    if (d->checkbox) {
        if (w == d->checkbox)
            return;
        if (d->checkbox->isChecked()) {
            if (!w->testAttribute(WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(WA_ForceDisabled, false);
            }
        }
    }
    if (!d->grid)
        return;
    d->insertWid(w);
}

void Q3GroupBoxPrivate::insertWid(QWidget* w)
{
    if (row >= nRows || col >= nCols)
        grid->expand(row+1, col+1);
    grid->addWidget(w, row, col);
    skip();
    QApplication::postEvent(q, new QEvent(QEvent::LayoutHint));
}


void Q3GroupBoxPrivate::skip()
{
    // Same as QGrid::skip()
    if (dir == Horizontal) {
        if (col+1 < nCols) {
            col++;
        } else {
            col = 0;
            row++;
        }
    } else { //Vertical
        if (row+1 < nRows) {
            row++;
        } else {
            row = 0;
            col++;
        }
    }
}


/*!
    \internal

    This private slot finds a widget in this group box that can accept
    focus, and gives the focus to that widget.
*/

void Q3GroupBoxPrivate::fixFocus()
{
    QWidget *fw = q->focusWidget();
    if (!fw) {
#ifndef QT_NO_RADIOBUTTON
        QWidget * best = 0;
#endif
        QWidget * candidate = 0;
        QWidget * w = q;
        while ((w = w->nextInFocusChain()) != q) {
            if (q->isAncestorOf(w) && (w->focusPolicy() & TabFocus) == TabFocus && w->isVisibleTo(q)) {
#ifndef QT_NO_RADIOBUTTON
                if (!best && qt_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
                    // we prefer a checked radio button or a widget that
                    // already has focus, if there is one
                    best = w;
                else
#endif
                    if (!candidate)
                        // but we'll accept anything that takes focus
                        candidate = w;
            }
        }
#ifndef QT_NO_RADIOBUTTON
        if (best)
            fw = best;
        else
#endif
            if (candidate)
                fw = candidate;
    }
    if (fw)
        fw->setFocus();
}


/*
    Sets the right frame rect depending on the title. Also calculates
    the visible part of the title.
*/
void Q3GroupBoxPrivate::calculateFrame()
{
    lenvisible = str.length();

    d->topMargin = 0;
    QFontMetrics fm = q->fontMetrics();
    if (lenvisible && !checkbox) { // do we have a label?
        while (lenvisible) {
            int tw = fm.width(str, lenvisible) + 4*fm.width(QChar(' '));
            if (tw < q->width())
                break;
            lenvisible--;
        }
        if (lenvisible) { // but do we also have a visible label?
            int va = q->style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, q);
            if(va & AlignVCenter)
                d->topMargin = fm.height()/2;
            else if(va & AlignTop)
                d->topMargin = fm.ascent();
        }
    }
    else if (checkbox) {
        int va = q->style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, q);
        if(va & AlignVCenter)
            topMargin = checkbox->height()/2;
        else if(va & AlignTop)
            topMargin = fm.ascent();
    }
}



/*! \reimp
 */
void Q3GroupBox::focusInEvent(QFocusEvent *)
{ // note no call to super
    d->fixFocus();
}


/*!
  \reimp
*/

QSize Q3GroupBox::sizeHint() const
{
    QFontMetrics fm(font());
    int tw, th;
    if (isCheckable()) {
        tw = d->checkbox->sizeHint().width() + 2*fm.width("xx");
        th = d->checkbox->sizeHint().height() + fm.width(QChar(' '));
    } else {
        tw = fm.size(ShowPrefix, title() + "xxxx").width();
        th = fm.height() + fm.width(QChar(' '));
    }

    QSize s;
    if (layout()) {
        s = QWidget::sizeHint();
        return s.expandedTo(QSize(tw, 0));
    } else {
        QRect r = childrenRect();
        QSize s(100, 50);
        s = s.expandedTo(QSize(tw, th));
        if (r.isNull())
            return s;

        return s.expandedTo(QSize(r.width() + 2 * r.x(), r.height()+ 2 * r.y()));
    }
}

/*!
    \property Q3GroupBox::flat
    \brief whether the group box is painted flat or has a frame

    By default a group box has a surrounding frame, with the title
    being placed on the upper frame line. In flat mode the right, left
    and bottom frame lines are omitted, and only the thin line at the
    top is drawn.

    \sa title
*/
bool Q3GroupBox::isFlat() const
{
    return d->bFlat;
}

void Q3GroupBox::setFlat(bool b)
{
    if ((bool)d->bFlat == b)
        return;
    d->bFlat = b;
    update();
}


/*!
    \property Q3GroupBox::checkable
    \brief Whether the group box has a checkbox in its title.

    If this property is true, the group box has a checkbox. If the
    checkbox is checked (which is the default), the group box's
    children are enabled.

    setCheckable() controls whether or not the group box has a
    checkbox, and isCheckable() controls whether the checkbox is
    checked or not.
*/
void Q3GroupBox::setCheckable(bool b)
{
    if ((d->checkbox != 0) == b)
        return;

    if (b) {
        if (!d->checkbox) {
            d->checkbox = new QCheckBox(title(), this, "qt_groupbox_checkbox");
            setChecked(true);
            d->setChildrenEnabled(true);
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SLOT(setChildrenEnabled(bool)));
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SIGNAL(toggled(bool)));
            d->updateCheckBoxGeometry();
        }
        d->checkbox->show();
    } else {
        d->setChildrenEnabled(true);
        delete d->checkbox;
        d->checkbox = 0;
    }
    d->calculateFrame();
    d->setTextSpacer();
    update();
}

bool Q3GroupBox::isCheckable() const
{
    return (d->checkbox != 0);
}


bool Q3GroupBox::isChecked() const
{
    return d->checkbox && d->checkbox->isChecked();
}


/*!
    \fn void Q3GroupBox::toggled(bool on)

    If the group box has a check box (see \l isCheckable()) this signal
    is emitted when the check box is toggled. \a on is true if the check
    box is checked; otherwise it is false.
*/

/*!
    \property Q3GroupBox::checked
    \brief Whether the group box's checkbox is checked.

    If the group box has a check box (see \l isCheckable()), and the
    check box is checked (see \l isChecked()), the group box's children
    are enabled. If the checkbox is unchecked the children are
    disabled.
*/
void Q3GroupBox::setChecked(bool b)
{
    if (d->checkbox)
        d->checkbox->setChecked(b);
}

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void Q3GroupBoxPrivate::setChildrenEnabled(bool b)
{
    QObjectList childs = q->children();
    if (childs.isEmpty())
        return;
    for (int i = 0; i < childs.size(); ++i) {
        QObject *o = childs.at(i);
        if (o->isWidgetType()
             && o != d->checkbox
           ) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \reimp */
void Q3GroupBox::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::EnabledChange) {
        if (!d->checkbox || !isEnabled())
            return;

        // we are being enabled - disable children
        if (!d->checkbox->isChecked())
            d->setChildrenEnabled(false);
    } else if(ev->type() == QEvent::FontChange) {
        d->updateCheckBoxGeometry();
        d->calculateFrame();
        d->setTextSpacer();
    }
    QWidget::changeEvent(ev);
}

/*
  recalculates and sets the checkbox setGeometry
*/
void Q3GroupBoxPrivate::updateCheckBoxGeometry()
{
    if (d->checkbox) {
        QSize cbSize = d->checkbox->sizeHint();
        QRect cbRect(0, 0, cbSize.width(), cbSize.height());
        QRect frameRect = q->rect();
        frameRect.setTop(topMargin);

        int marg = bFlat ? 2 : 8;
        marg += q->fontMetrics().width(QChar(' '));

        if (align & AlignHCenter) {
            cbRect.moveCenter(frameRect.center());
            cbRect.moveTop(0);
        } else if (align & AlignRight) {
            cbRect.moveRight(frameRect.right() - marg);
        } else if (align & AlignLeft) {
            cbRect.moveLeft(frameRect.left() + marg);
        } else { // auto align
            if(QApplication::reverseLayout())
                cbRect.moveRight(frameRect.right() - marg);
            else
                cbRect.moveLeft(frameRect.left() + marg);
        }

        d->checkbox->setGeometry(cbRect);
    }
}


#include "moc_q3groupbox.cpp"

