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

#include "q3groupbox.h"

#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "q3accel.h"
#include "qradiobutton.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qcheckbox.h"
#include "qaccessible.h"
#include "qstyleoption.h"
using namespace Qt;

/*!
    \class Q3GroupBox qgroupbox.h
    \brief The Q3GroupBox widget provides a group box frame with a title.

    \compat

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

    \sa QButtonGroup
*/

class QCheckBox;

class Q3GroupBoxPrivate
{
public:
    Q3GroupBoxPrivate():
        spacer(0),
        checkbox(0) {}

    QSpacerItem *spacer;
    QCheckBox *checkbox;
};




/*!
    Constructs a group box widget with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(QWidget *parent, const char *name)
    : Q3Frame(parent, name)
{
    init();
}

/*!
    Constructs a group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(const QString &title, QWidget *parent, const char *name)
    : Q3Frame(parent, name)
{
    init();
    setTitle(title);
}

/*!
    Constructs a group box with no title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Qt::Orientation orientation,
                    QWidget *parent, const char *name)
    : Q3Frame(parent, name)
{
    init();
    setColumnLayout(strips, orientation);
}

/*!
    Constructs a group box titled \a title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Qt::Orientation orientation,
                    const QString &title, QWidget *parent,
                    const char *name)
    : Q3Frame(parent, name)
{
    init();
    setTitle(title);
    setColumnLayout(strips, orientation);
}

/*!
    Destroys the group box.
*/
Q3GroupBox::~Q3GroupBox()
{
    delete d;
}

void Q3GroupBox::init()
{
    align = Qt::AlignAuto;
    setFrameStyle(Q3Frame::GroupBoxPanel | Q3Frame::Sunken);
#ifndef QT_NO_ACCEL
    accel = 0;
#endif
    vbox = 0;
    grid = 0;
    d = new Q3GroupBoxPrivate();
    lenvisible = 0;
    nCols = nRows = 0;
    dir = Horizontal;
    marg = 11;
    spac = 5;
    bFlat = false;
}

void Q3GroupBox::setTextSpacer()
{
    if (!d->spacer)
        return;
    int h = 0;
    int w = 0;
    if (isCheckable() || lenvisible) {
        QFontMetrics fm = fontMetrics();
        int fh = fm.height();
        if (isCheckable()) {
#ifndef QT_NO_CHECKBOX
            fh = d->checkbox->sizeHint().height() + 2;
            w = d->checkbox->sizeHint().width() + 2*fm.width("xx");
#endif
        } else {
            fh = fm.height();
            w = fm.width(str, lenvisible) + 2*fm.width("xx");
        }
        h = frameRect().y();
        if (layout()) {
            int m = layout()->margin();
            int sp = layout()->spacing();
            // do we have a child layout?
            for (QLayoutIterator it = layout()->iterator(); it.current(); ++it) {
                if (it.current()->layout()) {
                    m += it.current()->layout()->margin();
                    sp = QMAX(sp, it.current()->layout()->spacing());
                    break;
                }
            }
            h = QMAX(fh-m, h);
            h += QMAX(sp - (h+m - fh), 0);
        }
    }
    d->spacer->changeSize(w, h, QSizePolicy::Minimum, QSizePolicy::Fixed);
}


void Q3GroupBox::setTitle(const QString &title)
{
    if (str == title)                           // no change
        return;
    str = title;
#ifndef QT_NO_ACCEL
    if (accel)
        delete accel;
    accel = 0;
    int s = Q3Accel::shortcutKey(title);
    if (s) {
        accel = new Q3Accel(this, "automatic focus-change accelerator");
        accel->connectItem(accel->insertItem(s, 0),
                            this, SLOT(fixFocus()));
    }
#endif
#ifndef QT_NO_CHECKBOX
    if (d->checkbox) {
        d->checkbox->setText(str);
        updateCheckBoxGeometry();
    }
#endif
    calculateFrame();
    setTextSpacer();

    update();
    updateGeometry();
#if defined(QT_ACCESSIBILITY_SUPPORT)
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

/*!
    \property Q3GroupBox::alignment
    \brief the alignment of the group box title.

    The title is always placed on the upper frame line. The horizontal
    alignment can be specified by the alignment parameter.

    The alignment is one of the following flags:
    \list
    \i \c Qt::AlignAuto aligns the title according to the language,
    usually to the left.
    \i \c Qt::AlignLeft aligns the title text to the left.
    \i \c Qt::AlignRight aligns the title text to the right.
    \i \c Qt::AlignHCenter aligns the title text centered.
    \endlist

    The default alignment is \c Qt::AlignAuto.

    \sa Qt::AlignmentFlags
*/

void Q3GroupBox::setAlignment(int alignment)
{
    align = alignment;
#ifndef QT_NO_CHECKBOX
    updateCheckBoxGeometry();
#endif
    update();
}

/*! \reimp
*/
void Q3GroupBox::resizeEvent(QResizeEvent *e)
{
    Q3Frame::resizeEvent(e);
#ifndef QT_NO_CHECKBOX
    if (align & Qt::AlignRight || align & Qt::AlignCenter ||
         (QApplication::reverseLayout() && !(align & Qt::AlignLeft)))
        updateCheckBoxGeometry();
#endif
    calculateFrame();
}

/*! \reimp

  \internal
  overrides Q3Frame::paintEvent
*/

void Q3GroupBox::paintEvent(QPaintEvent *event)
{
    QPainter paint(this);

    QRect frameRect = QFrame::frameRect();

    QStyleOptionFrame opt;
    opt.rect = frameRect;
    opt.palette = palette();
    opt.lineWidth = 1;
    opt.midLineWidth = 0;
    opt.state = QStyle::Style_None | QStyle::Style_Sunken;
    if (hasFocus())
        opt.state |= QStyle::Style_HasFocus;
    if (testAttribute(Qt::WA_UnderMouse))
        opt.state |= QStyle::Style_MouseOver;
    if (lenvisible && !isCheckable()) {        // draw title
        QFontMetrics fm = paint.fontMetrics();
        int h = fm.height();
        int tw = fm.width(str, lenvisible) + fm.width(QChar(' '));
        int x;
        int marg = bFlat ? 0 : 8;
        if (align & Qt::AlignHCenter)                // center alignment
            x = frameRect.width()/2 - tw/2;
        else if (align & Qt::AlignRight)        // right alignment
            x = frameRect.width() - tw - marg;
        else if (align & Qt::AlignLeft)                 // left alignment
            x = marg;
        else { // auto align
            if(QApplication::reverseLayout())
                x = frameRect.width() - tw - marg;
            else
                x = marg;
        }
        QRect r(x, 0, tw, h);
        int va = style()->styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, &opt, this);
        if(va & Qt::AlignTop)
            r.moveBy(0, fm.descent());
        QColor pen((QRgb) style()->styleHint(QStyle::SH_GroupBox_TextLabelColor, &opt, this));
        if (!style()->styleHint(QStyle::SH_UnderlineAccelerator, &opt, this))
            va |= Qt::NoAccel;
        style()->drawItem(&paint, r, Qt::ShowPrefix | Qt::AlignHCenter | va, palette(),
                          isEnabled(), QPixmap(), str,
                          testAttribute(Qt::WA_SetPalette) ? 0 : &pen);
        paint.setClipRegion(event->region().subtract(r)); // clip everything but title
    } else if (d->checkbox) {
        QRect cbClip = d->checkbox->geometry();
        QFontMetrics fm = paint.fontMetrics();
        cbClip.setX(cbClip.x() - fm.width(QChar(' ')));
       cbClip.setWidth(cbClip.width() + fm.width(QChar(' ')));
        paint.setClipRegion(event->region().subtract(cbClip));
    }
    if (bFlat) {
       QRect fr = frameRect;
        QPoint p1(fr.x(), fr.y() + 1);
        QPoint p2(fr.x() + fr.width(), p1.y());
        // ### This should probably be a style primitive.
        qDrawShadeLine(&paint, p1, p2, palette(), true, 1, 0);
    } else {
        style()->drawPrimitive(QStyle::PE_FrameGroupBox, &opt, &paint, this);
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

    if (nCols <= 0 || nRows <= 0)
        return;

    if (row >= nRows || col >= nCols)
        grid->expand(row+1, col+1);

    if (size > 0) {
        QSpacerItem *spacer
            = new QSpacerItem((dir == Horizontal) ? 0 : size,
                               (dir == Vertical) ? 0 : size,
                               QSizePolicy::Fixed, QSizePolicy::Fixed);
        grid->addItem(spacer, row, col);
    }

    skip();
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
    if (dir == Horizontal)
        return nCols;
    return nRows;
}

void Q3GroupBox::setColumns(int c)
{
    setColumnLayout(c, dir);
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
    return marg;
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
    return spac;
}

/*!
    Sets the the width of the inside margin to \a m pixels.

    \sa insideMargin()
*/
void Q3GroupBox::setInsideMargin(int m)
{
    marg = m;
    setColumnLayout(columns(), dir);
}

/*!
    Sets the width of the empty space between each of the items in
    the group to \a s pixels.

    \sa insideSpacing()
*/
void Q3GroupBox::setInsideSpacing(int s)
{
    spac = s;
    setColumnLayout(columns(), dir);
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
void Q3GroupBox::setColumnLayout(int strips, Qt::Orientation direction)
{
    if (layout())
        delete layout();

    vbox = 0;
    grid = 0;

    if (strips < 0) // if 0, we create the vbox but not the grid. See below.
        return;

    vbox = new QVBoxLayout(this, marg, 0);

    d->spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                 QSizePolicy::Fixed);

    setTextSpacer();
    vbox->addItem(d->spacer);

    nCols = 0;
    nRows = 0;
    dir = direction;

    // Send all child events and ignore them. Otherwise we will end up
    // with doubled insertion. This won't do anything because nCols ==
    // nRows == 0.
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    // if 0 or smaller , create a vbox-layout but no grid. This allows
    // the designer to handle its own grid layout in a group box.
    if (strips <= 0)
        return;

    dir = direction;
    if (dir == Horizontal) {
        nCols = strips;
        nRows = 1;
    } else {
        nCols = 1;
        nRows = strips;
    }
    grid = new QGridLayout(nRows, nCols, spac);
    row = col = 0;
    grid->setAlignment(Qt::AlignTop);
    vbox->addLayout(grid);

    // Add all children
    QObjectList childs = children();
    if (!childs.isEmpty()) {
        for (int i = 0; i < childs.size(); ++i) {
            QObject *o = childs.at(i);
            if (o->isWidgetType() && o != d->checkbox)
                insertWid(static_cast<QWidget *>(o));
        }
    }
}


/*! \reimp  */
bool Q3GroupBox::event(QEvent * e)
{
    if (e->type() == QEvent::LayoutHint && layout())
        setTextSpacer();
    return Q3Frame::event(e);
}

/*!\reimp */
void Q3GroupBox::childEvent(QChildEvent *c)
{
    if (!c->inserted() || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
#ifndef QT_NO_CHECKBOX
    if (d->checkbox) {
        if (w == d->checkbox)
            return;
        if (d->checkbox->isChecked()) {
            if (!w->testAttribute(Qt::WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                ((Q3GroupBox*)w)->setAttribute(Qt::WA_ForceDisabled, false);
            }
        }
    }
#endif
    if (!grid)
        return;
    insertWid(w);
}

void Q3GroupBox::insertWid(QWidget* w)
{
    if (row >= nRows || col >= nCols)
        grid->expand(row+1, col+1);
    grid->addWidget(w, row, col);
    skip();
    QApplication::postEvent(this, new QEvent(QEvent::LayoutHint));
}


void Q3GroupBox::skip()
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

void Q3GroupBox::fixFocus()
{
    QWidget *fw = focusWidget();
    if (!fw) {
        QWidget * best = 0;
        QWidget * candidate = 0;
        QWidget * w = this;
        while ((w = w->nextInFocusChain()) != this) {
            if (isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus && w->isVisibleTo(this)) {
                if (!best && qt_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
                    // we prefer a checked radio button or a widget that
                    // already has focus, if there is one
                    best = w;
                else
                    if (!candidate)
                        // but we'll accept anything that takes focus
                        candidate = w;
            }
        }
        if (best)
            fw = best;
        else
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
void Q3GroupBox::calculateFrame()
{
    lenvisible = str.length();

    if (lenvisible && !isCheckable()) { // do we have a label?
        QFontMetrics fm = fontMetrics();
        while (lenvisible) {
            int tw = fm.width(str, lenvisible) + 4*fm.width(QChar(' '));
            if (tw < width())
                break;
            lenvisible--;
        }
        if (lenvisible) { // but do we also have a visible label?
            QRect r = rect();
            int va = style()->styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, 0, this);
            if(va & Qt::AlignVCenter)
                r.setTop(fm.height()/2);                                // frame rect should be
            else if(va & Qt::AlignTop)
                r.setTop(fm.ascent());
            setFrameRect(r);                    //   smaller than client rect
            return;
        }
    } else if (isCheckable()) {
#ifndef QT_NO_CHECKBOX
        QRect r = rect();
        int va = style()->styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, 0, this);
        if(va & Qt::AlignVCenter)
            r.setTop(d->checkbox->rect().height()/2);
        else if(va & Qt::AlignTop)
            r.setTop(fontMetrics().ascent());
        setFrameRect(r);
        return;
#endif
    }

    // no visible label
    setFrameRect(QRect(0,0,0,0));               //  then use client rect
}



/*! \reimp
 */
void Q3GroupBox::focusInEvent(QFocusEvent *)
{ // note no call to super
    fixFocus();
}


/*!\reimp
 */
void Q3GroupBox::fontChange(const QFont & oldFont)
{
    QWidget::fontChange(oldFont);
#ifndef QT_NO_CHECKBOX
    updateCheckBoxGeometry();
#endif
    calculateFrame();
    setTextSpacer();
}

/*!
  \reimp
*/

QSize Q3GroupBox::sizeHint() const
{
    QFontMetrics fm(font());
    int tw, th;
    if (isCheckable()) {
#ifndef QT_NO_CHECKBOX
        tw = d->checkbox->sizeHint().width() + 2*fm.width("xx");
        th = d->checkbox->sizeHint().height() + fm.width(QChar(' '));
#endif
    } else {
        tw = fm.width(title()) + 2 * fm.width("xx");
        th = fm.height() + fm.width(QChar(' '));
    }

    QSize s;
    if (layout()) {
        s = Q3Frame::sizeHint();
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
    return bFlat;
}

void Q3GroupBox::setFlat(bool b)
{
    if ((bool)bFlat == b)
        return;
    bFlat = b;
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
#ifndef QT_NO_CHECKBOX
void Q3GroupBox::setCheckable(bool b)
{
    if ((d->checkbox != 0) == b)
        return;

    if (b) {
        if (!d->checkbox) {
            d->checkbox = new QCheckBox(title(), this, "qt_groupbox_checkbox");
            setChecked(true);
            setChildrenEnabled(true);
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SLOT(setChildrenEnabled(bool)));
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SIGNAL(toggled(bool)));
            updateCheckBoxGeometry();
        }
        d->checkbox->show();
    } else {
        setChildrenEnabled(true);
        delete d->checkbox;
        d->checkbox = 0;
    }
    calculateFrame();
    setTextSpacer();
    update();
}
#endif //QT_NO_CHECKBOX

bool Q3GroupBox::isCheckable() const
{
#ifndef QT_NO_CHECKBOX
    return (d->checkbox != 0);
#else
    return false;
#endif
}


bool Q3GroupBox::isChecked() const
{
#ifndef QT_NO_CHECKBOX
    return d->checkbox && d->checkbox->isChecked();
#else
    return false;
#endif
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
#ifndef QT_NO_CHECKBOX
void Q3GroupBox::setChecked(bool b)
{
    if (d->checkbox)
        d->checkbox->setChecked(b);
}
#endif

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void Q3GroupBox::setChildrenEnabled(bool b)
{
    QObjectList childs = children();
    if (childs.isEmpty())
        return;
    for (int i = 0; i < childs.size(); ++i) {
        QObject *o = childs.at(i);
        if (o->isWidgetType() && o != d->checkbox) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(Qt::WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(Qt::WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \internal */
void Q3GroupBox::setEnabled(bool on)
{
    // ### setEnabled() is no longer virtual; we should use a changeEvent() instead
    Q3Frame::setEnabled(on);
    if (!d->checkbox || !on)
        return;

#ifndef QT_NO_CHECKBOX
    // we are being enabled - disable children
    if (!d->checkbox->isChecked())
        setChildrenEnabled(false);
#endif
}

/*
  recalculates and sets the checkbox setGeometry
*/
void Q3GroupBox::updateCheckBoxGeometry()
{
    if (d->checkbox) {
        QSize cbSize = d->checkbox->sizeHint();
        QRect cbRect(0, 0, cbSize.width(), cbSize.height());

        int marg = bFlat ? 2 : 8;
        marg += fontMetrics().width(QChar(' '));

        if (align & Qt::AlignHCenter) {
            cbRect.moveCenter(frameRect().center());
            cbRect.moveTop(0);
        } else if (align & Qt::AlignRight) {
            cbRect.moveRight(frameRect().right() - marg);
        } else if (align & Qt::AlignLeft) {
            cbRect.moveLeft(frameRect().left() + marg);
        } else { // auto align
            if(QApplication::reverseLayout())
                cbRect.moveRight(frameRect().right() - marg);
            else
                cbRect.moveLeft(frameRect().left() + marg);
        }

        d->checkbox->setGeometry(cbRect);
    }
}

