/****************************************************************************
**
** Implementation of QIconView widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the iconview module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qglobal.h"
#if defined(Q_CC_BOR)
// needed for qsort() because of a std namespace problem on Borland
#include "qplatformdefs.h"
#endif

#include "qiconview.h"

#ifndef QT_NO_ICONVIEW

#include "qfontmetrics.h"
#include "qpainter.h"
#include "qevent.h"
#include "qpalette.h"
#include "qmime.h"
#include "qimage.h"
#include "qpen.h"
#include "qbrush.h"
#include "qtimer.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qtextedit.h"
#include "qvbox.h"
#include "qtooltip.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qhash.h"
#include "qstringlist.h"
#include "qcleanuphandler.h"
#include "private/qrichtext_p.h"
#include "qstyle.h"

#include <limits.h>
#include <stdlib.h>

#define RECT_EXTENSION 300

static const char * const unknown_xpm[] = {
    "32 32 11 1",
    "c c #ffffff",
    "g c #c0c0c0",
    "a c #c0ffc0",
    "h c #a0a0a4",
    "d c #585858",
    "f c #303030",
    "i c #400000",
    "b c #00c000",
    "e c #000000",
    "# c #000000",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebaaaaaaa#.........",
    ".#cccccgcgghhebbbbaaaaa#........",
    ".#ccccccgcgggdebbbbbbaa#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "....##ddhhhghhhhhhhhhhd#b#......",
    "......##ddhhhhhhhhhhhhd#b#......",
    "........##ddhhhhhhhhhhd#b#......",
    "..........##ddhhhhhhhhd#b#......",
    "............##ddhhhhhhd#b###....",
    "..............##ddhhhhd#b#####..",
    "................##ddhhd#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};

static QPixmap *unknown_icon = 0;
static QPixmap *qiv_buffer_pixmap = 0;
#if !defined(Q_WS_X11)
static QPixmap *qiv_selection = 0;
#endif
static bool optimize_layout = false;

static QCleanupHandler<QPixmap> qiv_cleanup_pixmap;

#if !defined(Q_WS_X11)
static void createSelectionPixmap(const QPalette &cg)
{
    QBitmap m(2, 2);
    m.fill(Qt::color1);
    QPainter p(&m);
    p.setPen(Qt::color0);
    for (int j = 0; j < 2; ++j) {
        p.drawPoint(j % 2, j);
    }
    p.end();

    qiv_selection = new QPixmap(2, 2);
    qiv_cleanup_pixmap.add(&qiv_selection);
    qiv_selection->fill(Qt::color0);
    qiv_selection->setMask(m);
    qiv_selection->fill(cg.highlight());
}
#endif

static QPixmap *get_qiv_buffer_pixmap(const QSize &s)
{
    if (!qiv_buffer_pixmap) {
        qiv_buffer_pixmap = new QPixmap(s);
        qiv_cleanup_pixmap.add(&qiv_buffer_pixmap);
        return qiv_buffer_pixmap;
    }

    qiv_buffer_pixmap->resize(s);
    return qiv_buffer_pixmap;
}

#ifndef QT_NO_DRAGANDDROP

class Q_COMPAT_EXPORT QIconDragData
{
public:
    QIconDragData();
    QIconDragData(const QRect &ir, const QRect &tr);

    QRect pixmapRect() const;
    QRect textRect() const;

    void setPixmapRect(const QRect &r);
    void setTextRect(const QRect &r);

    QRect iconRect_, textRect_;
    QString key_;

    bool operator==(const QIconDragData &i) const;
};

class Q_COMPAT_EXPORT QIconDragDataItem
{
public:
    QIconDragDataItem() {}
    QIconDragDataItem(const QIconDragItem &i1, const QIconDragData &i2) : data(i1), item(i2) {}
    QIconDragItem data;
    QIconDragData item;
    bool operator== (const QIconDragDataItem&) const;
};

class QIconDragPrivate
{
public:
    QLinkedList<QIconDragDataItem> items;
    static bool decode(QMimeSource* e, QLinkedList<QIconDragDataItem> &lst);
};

#endif

class QIconViewToolTip;

class QIconViewPrivate
{
public:
    QIconViewItem *firstItem, *lastItem;
    uint count;
    QIconView::SelectionMode selectionMode;
    QIconViewItem *currentItem, *tmpCurrentItem, *highlightedItem,
        *startDragItem, *pressedItem, *selectAnchor, *renamingItem;
    QRect *rubber;
    QTimer *scrollTimer, *adjustTimer, *updateTimer, *inputTimer,
        *fullRedrawTimer;
    int rastX, rastY, spacing;
    int dragItems;
    QPoint oldDragPos;
    QIconView::Arrangement arrangement;
    QIconView::ResizeMode resizeMode;
    QSize oldSize;
#ifndef QT_NO_DRAGANDDROP
    QLinkedList<QIconDragDataItem> iconDragData;
#endif
    int numDragItems, cachedW, cachedH;
    int maxItemWidth, maxItemTextLength;
    QPoint dragStart;
    QString currInputString;
    QIconView::ItemTextPos itemTextPos;
#ifndef QT_NO_CURSOR
    QCursor oldCursor;
#endif
    int cachedContentsX, cachedContentsY;
    QBrush itemTextBrush;
    QRegion clipRegion;
    QPoint dragStartPos;
    QFontMetrics *fm;
    int minLeftBearing, minRightBearing;

    uint mousePressed : 1;
    uint cleared : 1;
    uint dropped : 1;
    uint clearing : 1;
    uint oldDragAcceptAction : 1;
    uint isIconDrag : 1;
    uint drawDragShapes : 1;
    uint dirty : 1;
    uint rearrangeEnabled : 1;
    uint reorderItemsWhenInsert : 1;
    uint drawAllBack : 1;
    uint resortItemsWhenInsert : 1;
    uint sortDirection : 1;
    uint wordWrapIconText : 1;
    uint containerUpdateLocked : 1;
    uint firstSizeHint : 1;
    uint showTips : 1;
    uint pressedSelected : 1;
    uint dragging : 1;
    uint drawActiveSelection : 1;
    uint inMenuMode : 1;

    QIconViewToolTip *toolTip;
    QPixmapCache maskCache;
    QHash<QIconViewItem *, QIconViewItem *> selectedItems;

    struct ItemContainer {
        ItemContainer(ItemContainer *pr, ItemContainer *nx, const QRect &r)
            : p(pr), n(nx), rect(r)
        {
            if (p)
                p->n = this;
            if (n)
                n->p = this;
        }
        ItemContainer *p, *n;
        QRect rect;
        QList<QIconViewItem*> items;
    } *firstContainer, *lastContainer;

    struct SortableItem {
        QIconViewItem *item;
    };

public:

    /* finds the containers that intersect with \a searchRect in the direction \a dir relative to \a relativeTo */
    QList<ItemContainer* >* findContainers(
        QIconView:: Direction dir,
        const QPoint &relativeTo,
        const QRect &searchRect) const;
    //    friend int cmpIconViewItems(const void *n1, const void *n2);
};


QList<QIconViewPrivate::ItemContainer *>* QIconViewPrivate::findContainers(
        QIconView:: Direction dir,
        const QPoint &relativeTo,
        const QRect &searchRect) const
{

    QList<QIconViewPrivate::ItemContainer *>* list =
        new QList<QIconViewPrivate::ItemContainer*>();

    if (arrangement == QIconView::LeftToRight) {
        if (dir == QIconView::DirLeft || dir == QIconView::DirRight) {
            ItemContainer *c = firstContainer;
            for (; c; c = c->n)
                if (c->rect.intersects(searchRect))
                    list->append(c);
        } else {
            if (dir == QIconView::DirDown) {
                ItemContainer *c = firstContainer;
                for (; c; c = c->n)
                    if (c->rect.intersects(searchRect) &&
                         c->rect.bottom() >= relativeTo.y())
                        list->append(c);
            } else {
                ItemContainer *c = lastContainer;
                for (; c; c = c->p)
                    if (c->rect.intersects(searchRect) &&
                         c->rect.top() <= relativeTo.y())
                        list->append(c);
            }
        }
    } else {
        if (dir == QIconView::DirUp || dir == QIconView::DirDown) {
            ItemContainer *c = firstContainer;
            for (; c; c = c->n)
                if (c->rect.intersects(searchRect))
                    list->append(c);
        } else {
            if (dir == QIconView::DirRight) {
                ItemContainer *c = firstContainer;
                for (; c; c = c->n)
                    if (c->rect.intersects(searchRect) &&
                         c->rect.right() >= relativeTo.x())
                        list->append(c);
            } else {
                ItemContainer *c = lastContainer;
                for (; c; c = c->p)
                    if (c->rect.intersects(searchRect) &&
                         c->rect.left() <= relativeTo.x())
                        list->append(c);
            }
        }
    }
    return list;
}


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int _cdecl cmpIconViewItems(const void *n1, const void *n2)
#else
static int cmpIconViewItems(const void *n1, const void *n2)
#endif
{
    if (!n1 || !n2)
        return 0;

    QIconViewPrivate::SortableItem *i1 = (QIconViewPrivate::SortableItem *)n1;
    QIconViewPrivate::SortableItem *i2 = (QIconViewPrivate::SortableItem *)n2;

    return i1->item->compare(i2->item);
}

#if defined(Q_C_CALLBACKS)
}
#endif


#ifndef QT_NO_TOOLTIP
class QIconViewToolTip : public QToolTip
{
public:
    QIconViewToolTip(QWidget *parent, QIconView *iv);

    void maybeTip(const QPoint &pos);

private:
    QIconView *view;
};

QIconViewToolTip::QIconViewToolTip(QWidget *parent, QIconView *iv)
    : QToolTip(parent), view(iv)
{
}

void QIconViewToolTip::maybeTip(const QPoint &pos)
{
    if (!parentWidget() || !view || view->wordWrapIconText() || !view->showToolTips())
        return;

    QIconViewItem *item = view->findItem(view->viewportToContents(pos));
    if (!item || item->tmpText == item->itemText)
        return;

    QRect r(item->textRect(false));
    QRect r2 = item->pixmapRect(false);
    /* this probably should be | r, but QToolTip does not handle that
     * well */

    // At this point the rectangle is too small (it is the width of the icon)
    // since we need it to be bigger than that, extend it here.
    r.setWidth(view->d->fm->boundingRect(item->itemText).width() + 4);
    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())), QSize(r.width(), r.height()));

    r2 = QRect(view->contentsToViewport(QPoint(r2.x(), r2.y())), QSize(r2.width(), r2.height()));
    tip(r2, item->itemText, r);
}
#endif


class QIconViewItemPrivate
{
public:
    QIconViewPrivate::ItemContainer *container1, *container2;
};

#ifndef QT_NO_TEXTEDIT

class QIconViewItemLineEdit : public QTextEdit
{
    friend class QIconViewItem;

public:
    QIconViewItemLineEdit(const QString &text, QWidget *parent, QIconViewItem *theItem, const char* name=0);

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusOutEvent(QFocusEvent *e);

protected:
    QIconViewItem *item;
    QString startText;
private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QIconViewItemLineEdit(const QIconViewItemLineEdit &);
    QIconViewItemLineEdit &operator=(const QIconViewItemLineEdit &);
#endif

};

QIconViewItemLineEdit::QIconViewItemLineEdit(const QString &text, QWidget *parent,
                                              QIconViewItem *theItem, const char *name)
    : QTextEdit(parent, name), item(theItem), startText(text)
{
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);

    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(AlwaysOff);

    setWordWrap(WidgetWidth);
    setWrapColumnOrWidth(item->iconView()->maxItemWidth() -
                          (item->iconView()->itemTextPos() == QIconView::Bottom ?
                            0 : item->pixmapRect().width()));
    document()->formatter()->setAllowBreakInWords(true);
    resize(200, 200); // ### some size, there should be a forceReformat()
    setTextFormat(PlainText);
    setText(text);
    setAlignment(Qt::AlignCenter);

    resize(wrapColumnOrWidth() + 2, heightForWidth(wrapColumnOrWidth()) + 2);
}

void QIconViewItemLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key()  == Key_Escape) {
        item->QIconViewItem::setText(startText);
        item->cancelRenameItem();
    } else if (e->key() == Key_Enter ||
                e->key() == Key_Return) {
        item->renameItem();
    } else {
        QTextEdit::keyPressEvent(e);
        sync();
        resize(width(), document()->height() + 2);

    }
}

void QIconViewItemLineEdit::focusOutEvent(QFocusEvent *e)
{
    Q_UNUSED(e) // I need this to get rid of a Borland warning
    if (e->reason() != QFocusEvent::Popup)
        item->cancelRenameItem();
}
#endif

#ifndef QT_NO_DRAGANDDROP


/*!
    \class QIconDragItem qiconview.h
    \ingroup draganddrop

    \brief The QIconDragItem class encapsulates a drag item.

    \module iconview

    The QIconDrag class uses a list of QIconDragItems to support drag
    and drop operations.

    In practice a QIconDragItem object (or an object of a class derived
    from QIconDragItem) is created for each icon view item which is
    dragged. Each of these QIconDragItems is stored in a QIconDrag
    object.

    See QIconView::dragObject() for more information.

    See the \l fileiconview/qfileiconview.cpp and
    \l iconview/simple_dd/main.cpp examples.
*/

/*!
    Constructs a QIconDragItem with no data.
*/

QIconDragItem::QIconDragItem()
{
    ba = "no data";
}

/*!
    Destructor.
*/

QIconDragItem::~QIconDragItem()
{
}

/*!
    Returns the data contained in the QIconDragItem.
*/

QByteArray QIconDragItem::data() const
{
    return ba;
}

/*!
    Sets the data for the QIconDragItem to the data stored in the
    QByteArray \a d.
*/

void QIconDragItem::setData(const QByteArray &d)
{
    ba = d;
}

/*!
    \reimp
*/

bool QIconDragItem::operator==(const QIconDragItem &i) const
{
    return ba == i.ba;
}

/*!
    \reimp
*/

bool QIconDragDataItem::operator==(const QIconDragDataItem &i) const
{
    return (i.item == item &&
             i.data == data);
}

/*!
    \reimp
*/

bool QIconDragData::operator==(const QIconDragData &i) const
{
    return key_ == i.key_;
}


/*!
    \class QIconDrag qiconview.h

    \brief The QIconDrag class supports drag and drop operations
    within a QIconView.

    \ingroup draganddrop
    \module iconview

    A QIconDrag object is used to maintain information about the
    positions of dragged items and the data associated with the
    dragged items. QIconViews are able to use this information to
    paint the dragged items in the correct positions. Internally
    QIconDrag stores the data associated with drag items in
    QIconDragItem objects.

    If you want to use the extended drag-and-drop functionality of
    QIconView, create a QIconDrag object in a reimplementation of
    QIconView::dragObject(). Then create a QIconDragItem for each item
    which should be dragged, set the data it represents with
    QIconDragItem::setData(), and add each QIconDragItem to the drag
    object using append().

    The data in QIconDragItems is stored in a QByteArray and is
    mime-typed (see QMimeSource and the
    \link http://doc.trolltech.com/dnd.html Drag and Drop\endlink
    overview). If you want to use your own mime-types derive a class
    from QIconDrag and reimplement format(), encodedData() and
    canDecode().

    The fileiconview example program demonstrates the use of the
    QIconDrag class including subclassing and reimplementing
    dragObject(), format(), encodedData() and canDecode(). See the files
    \c qt/examples/fileiconview/qfileiconview.h and
    \c qt/examples/fileiconview/qfileiconview.cpp.

    \sa QMimeSource::format()
*/
// ### consider using \dontinclude and friends there
// ### Not here in the module overview instead...

/*!
    Constructs a drag object called \a name, which is a child of \a
    dragSource.

    Note that the drag object will be deleted when \a dragSource is deleted.
*/

QIconDrag::QIconDrag(QWidget * dragSource, const char* name)
    : QDragObject(dragSource, name)
{
    d = new QIconDragPrivate;
}

/*!
    Destructor.
*/

QIconDrag::~QIconDrag()
{
    delete d;
}

/*!
    Append the QIconDragItem, \a i, to the QIconDrag object's list of
    items. You must also supply the geometry of the pixmap, \a pr, and
    the textual caption, \a tr.

    \sa QIconDragItem
*/

void QIconDrag::append(const QIconDragItem &i, const QRect &pr, const QRect &tr)
{
    d->items.append(QIconDragDataItem(i, QIconDragData(pr, tr)));
}

/*!
    \reimp
*/

const char* QIconDrag::format(int i) const
{
    if (i == 0)
        return "application/x-qiconlist";
    return 0;
}

/*!
    Returns the encoded data of the drag object if \a mime is
    application/x-qiconlist.
*/

QByteArray QIconDrag::encodedData(const char* mime) const
{
    if (d->items.size() <= 0 || QString(mime) !=
         "application/x-qiconlist")
        return QByteArray();

    QLinkedList<QIconDragDataItem>::ConstIterator it = d->items.begin();
    QString s;
    for (; it != d->items.end(); ++it) {
        QString k("%1$@@$%2$@@$%3$@@$%4$@@$%5$@@$%6$@@$%7$@@$%8$@@$");
        k = k.arg((*it).item.pixmapRect().x()).arg(
            (*it).item.pixmapRect().y()).arg((*it).item.pixmapRect().width()).
            arg((*it).item.pixmapRect().height()).arg(
                (*it).item.textRect().x()).arg((*it).item.textRect().y()).
            arg((*it).item.textRect().width()).arg(
                (*it).item.textRect().height());
        k += QString((*it).data.data()) + "$@@$";
        s += k;
    }

    QByteArray a;
    a.resize(s.length() + 1);
    memcpy(a.data(), s.latin1(), a.size());
    return a;
}

/*!
    Returns true if \a e can be decoded by the QIconDrag, otherwise
    return false.
*/

bool QIconDrag::canDecode(QMimeSource* e)
{
    if (e->provides("application/x-qiconlist"))
        return true;
    return false;
}

/*!
    Decodes the data which is stored (encoded) in \a e and, if
    successful, fills the \a list of icon drag items with the decoded
    data. Returns true if there was some data, false otherwise.
*/

bool QIconDragPrivate::decode(QMimeSource* e, QLinkedList<QIconDragDataItem> &lst)
{
    QByteArray ba = e->encodedData("application/x-qiconlist");
    if (ba.size()) {
        lst.clear();
        // #### unicode clean????
        QString s = ba;
        QIconDragDataItem item;
        QRect ir, tr;
        QStringList l = QStringList::split("$@@$", s);

        int i = 0;
        QStringList::Iterator it = l.begin();
        for (; it != l.end(); ++it) {
            if (i == 0) {
                ir.setX((*it).toInt());
            } else if (i == 1) {
                ir.setY((*it).toInt());
            } else if (i == 2) {
                ir.setWidth((*it).toInt());
            } else if (i == 3) {
                ir.setHeight((*it).toInt());
            } else if (i == 4) {
                tr.setX((*it).toInt());
            } else if (i == 5) {
                tr.setY((*it).toInt());
            } else if (i == 6) {
                tr.setWidth((*it).toInt());
            } else if (i == 7) {
                tr.setHeight((*it).toInt());
            } else if (i == 8) {
                QByteArray d;
                d.resize((*it).length());
                memcpy(d.data(), (*it).latin1(), (*it).length());
                item.item.setPixmapRect(ir);
                item.item.setTextRect(tr);
                item.data.setData(d);
                lst.append(item);
            }
            ++i;
            if (i > 8)
                i = 0;
        }
        return true;
    }

    return false;
}

QIconDragData::QIconDragData()
    : iconRect_(), textRect_()
{
}

QIconDragData::QIconDragData(const QRect &ir, const QRect &tr)
    : iconRect_(ir), textRect_(tr)
{
}

QRect QIconDragData::textRect() const
{
    return textRect_;
}

QRect QIconDragData::pixmapRect() const
{
    return iconRect_;
}

void QIconDragData::setPixmapRect(const QRect &r)
{
    iconRect_ = r;
}

void QIconDragData::setTextRect(const QRect &r)
{
    textRect_ = r;
}

#endif


/*!
    \class QIconViewItem qiconview.h
    \brief The QIconViewItem class provides a single item in a QIconView.

    \ingroup advanced
    \module iconview

    A QIconViewItem contains an icon, a string and optionally a sort
    key, and can display itself in a QIconView.

    The simplest way to create a QIconViewItem and insert it into a
    QIconView is to construct the item passing the constructor a
    pointer to the icon view, a string and an icon:

    \code
    (void) new QIconViewItem(
                    iconView,        // A pointer to a QIconView
                    "This is the text of the item",
                    aPixmap);
    \endcode

    By default the text of an icon view item may not be edited by the
    user but calling setRenameEnabled(true) will allow the user to
    perform in-place editing of the item's text.

    When the icon view is deleted all items in it are deleted
    automatically.

    The QIconView::firstItem() and QIconViewItem::nextItem() functions
    provide a means of iterating over all the items in a QIconView:

    \code
    QIconViewItem *item;
    for (item = iconView->firstItem(); item; item = item->nextItem())
        do_something_with(item);
    \endcode

    The item's icon view is available from iconView(), and its
    position in the icon view from index().

    The item's selection status is available from isSelected() and is
    set and controlled by setSelected() and isSelectable().

    The text and icon can be set with setText() and setPixmap() and
    retrieved with text() and pixmap(). The item's sort key defaults
    to text() but may be set with setKey() and retrieved with key().
    The comparison function, compare() uses key().

    Items may be repositioned with move() and moveBy(). An item's
    geometry is available from rect(), x(), y(), width(), height(),
    size(), pos(), textRect() and pixmapRect(). You can also test
    against the position of a point with contains() and intersects().

    To remove an item from an icon view, just delete the item. The
    QIconViewItem destructor removes it cleanly from its icon view.

    Because the icon view is designed to use drag-and-drop, the icon
    view item also has functions for drag-and-drop which may be
    reimplemented.

    The class is designed to be very similar to QListView and QListBox
    in use, both via instantiation and subclassing.
*/

/*!
    Constructs a QIconViewItem and inserts it into icon view \a parent
    with no text and a default icon.
*/

QIconViewItem::QIconViewItem(QIconView *parent)
    : view(parent), itemText(), itemIcon(unknown_icon)
{
    init();
}

/*!
    Constructs a QIconViewItem and inserts it into the icon view \a
    parent with no text and a default icon, after the icon view item
    \a after.
*/

QIconViewItem::QIconViewItem(QIconView *parent, QIconViewItem *after)
    : view(parent), itemText(), itemIcon(unknown_icon),
      prev(0), next(0)
{
    init(after);
}

/*!
    Constructs an icon view item  and inserts it into the icon view \a
    parent using \a text as the text and a default icon.
*/

QIconViewItem::QIconViewItem(QIconView *parent, const QString &text)
    : view(parent), itemText(text), itemIcon(unknown_icon)
{
    init(0);
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and a default icon, after the
    icon view item \a after.
*/

QIconViewItem::QIconViewItem(QIconView *parent, QIconViewItem *after,
                              const QString &text)
    : view(parent), itemText(text), itemIcon(unknown_icon)
{
    init(after);
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a icon as the icon.
*/

QIconViewItem::QIconViewItem(QIconView *parent, const QString &text,
                              const QPixmap &icon)
    : view(parent),
      itemText(text), itemIcon(new QPixmap(icon))
{
    init(0);
}


/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a icon as the icon, after
    the icon view item \a after.
*/

QIconViewItem::QIconViewItem(QIconView *parent, QIconViewItem *after,
                              const QString &text, const QPixmap &icon)
    : view(parent), itemText(text), itemIcon(new QPixmap(icon))
{
    init(after);
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a picture as the icon.
*/

#ifndef QT_NO_PICTURE
QIconViewItem::QIconViewItem(QIconView *parent, const QString &text,
                              const QPicture &picture)
    : view(parent), itemText(text), itemIcon(0)
{
    init(0, new QPicture(picture));
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a picture as the icon, after
    the icon view item \a after.
*/

QIconViewItem::QIconViewItem(QIconView *parent, QIconViewItem *after,
                              const QString &text, const QPicture &picture)
    : view(parent), itemText(text), itemIcon(0)
{
    init(after, new QPicture(picture));
}
#endif

/*!
  This private function initializes the icon view item and inserts it
  into the icon view.
*/

void QIconViewItem::init(QIconViewItem *after
#ifndef QT_NO_PICTURE
                          , QPicture *pic
#endif
                         )
{
    d = new QIconViewItemPrivate;
    d->container1 = 0;
    d->container2 = 0;
    prev = next = 0;
    allow_rename = false;
    allow_drag = true;
    allow_drop = true;
    selected = false;
    selectable = true;
#ifndef QT_NO_TEXTEDIT
    renameBox = 0;
#endif
#ifndef QT_NO_PICTURE
    itemPic = pic;
#endif
    if (view) {
        itemKey = itemText;
        dirty = true;
        wordWrapDirty = true;
        itemRect = QRect(-1, -1, 0, 0);
        calcRect();
        view->insertItem(this, after);
    }
}

/*!
    Destroys the icon view item and tells the parent icon view that
    the item has been destroyed.
*/

QIconViewItem::~QIconViewItem()
{
#ifndef QT_NO_TEXTEDIT
    removeRenameBox();
#endif
    if (view && !view->d->clearing)
        view->takeItem(this);
    view = 0;
    if (itemIcon && itemIcon->serialNumber() != unknown_icon->serialNumber())
        delete itemIcon;
#ifndef QT_NO_PICTURE
    delete itemPic;
#endif
    delete d;
}

int QIconViewItem::RTTI = 0;

/*!
    Returns 0.

    Make your derived classes return their own values for rtti(), so
    that you can distinguish between icon view item types. You should
    use values greater than 1000, preferably a large random number, to
    allow for extensions to this class.
*/

int QIconViewItem::rtti() const
{
    return RTTI;
}


/*!
    Sets \a text as the text of the icon view item. This function
    might be a no-op if you reimplement text().

    \sa text()
*/

void QIconViewItem::setText(const QString &text)
{
    if (text == itemText)
        return;

    wordWrapDirty = true;
    itemText = text;
    if (itemKey.isEmpty())
        itemKey = itemText;

    QRect oR = rect();
    calcRect();
    oR = oR.unite(rect());

    if (view) {
        if (QRect(view->contentsX(), view->contentsY(),
                    view->visibleWidth(), view->visibleHeight()).
             intersects(oR))
            view->repaintContents(oR.x() - 1, oR.y() - 1, oR.width() + 2, oR.height() + 2);
    }
}

/*!
    Sets \a k as the sort key of the icon view item. By default
    text() is used for sorting.

    \sa compare()
*/

void QIconViewItem::setKey(const QString &k)
{
    if (k == itemKey)
        return;

    itemKey = k;
}

/*!
    Sets \a icon as the item's icon in the icon view. This function
    might be a no-op if you reimplement pixmap().

    \sa pixmap()
*/

void QIconViewItem::setPixmap(const QPixmap &icon)
{
    if (itemIcon && itemIcon == unknown_icon)
        itemIcon = 0;

    if (itemIcon)
        *itemIcon = icon;
    else
        itemIcon = new QPixmap(icon);
    QRect oR = rect();
    calcRect();
    oR = oR.unite(rect());

    if (view) {
        if (QRect(view->contentsX(), view->contentsY(),
                    view->visibleWidth(), view->visibleHeight()).
             intersects(oR))
            view->repaintContents(oR.x() - 1, oR.y() - 1, oR.width() + 2, oR.height() + 2);
    }
}

/*!
    Sets \a icon as the item's icon in the icon view. This function
    might be a no-op if you reimplement picture().

    \sa picture()
*/

#ifndef QT_NO_PICTURE
void QIconViewItem::setPicture(const QPicture &icon)
{
    // clear assigned pixmap if any
    if (itemIcon) {
        if (itemIcon == unknown_icon) {
            itemIcon = 0;
        } else {
            delete itemIcon;
            itemIcon = 0;
        }
    }
    if (itemPic)
        delete itemPic;
    itemPic = new QPicture(icon);

    QRect oR = rect();
    calcRect();
    oR = oR.unite(rect());

    if (view) {
        if (QRect(view->contentsX(), view->contentsY(),
                    view->visibleWidth(), view->visibleHeight()).
             intersects(oR))
            view->repaintContents(oR.x() - 1, oR.y() - 1, oR.width() + 2, oR.height() + 2);
    }
}
#endif

/*!
    \overload

    Sets \a text as the text of the icon view item. If \a recalc is
    true, the icon view's layout is recalculated. If \a redraw is true
    (the default), the icon view is repainted.

    \sa text()
*/

void QIconViewItem::setText(const QString &text, bool recalc, bool redraw)
{
    if (text == itemText)
        return;

    wordWrapDirty = true;
    itemText = text;

    if (recalc)
        calcRect();
    if (redraw)
        repaint();
}

/*!
    \overload

    Sets \a icon as the item's icon in the icon view. If \a recalc is
    true, the icon view's layout is recalculated. If \a redraw is true
    (the default), the icon view is repainted.

    \sa pixmap()
*/

void QIconViewItem::setPixmap(const QPixmap &icon, bool recalc, bool redraw)
{
    if (itemIcon && itemIcon == unknown_icon)
        itemIcon = 0;

    if (itemIcon)
        *itemIcon = icon;
    else
        itemIcon = new QPixmap(icon);

    if (redraw) {
        if (recalc) {
            QRect oR = rect();
            calcRect();
            oR = oR.unite(rect());

            if (view) {
                if (QRect(view->contentsX(), view->contentsY(),
                            view->visibleWidth(), view->visibleHeight()).
                     intersects(oR))
                    view->repaintContents(oR.x() - 1, oR.y() - 1, oR.width() + 2, oR.height() + 2);
            }
        } else {
            repaint();
        }
    } else if (recalc) {
        calcRect();
    }
}

/*!
    If \a allow is true, the user can rename the icon view item by
    clicking on the text (or pressing F2) while the item is selected
    (in-place renaming). If \a allow is false, in-place renaming is
    not possible.
*/

void QIconViewItem::setRenameEnabled(bool allow)
{
    allow_rename = (uint)allow;
}

/*!
    If \a allow is true, the icon view permits the user to drag the
    icon view item either to another position within the icon view or
    to somewhere outside of it. If \a allow is false, the item cannot
    be dragged.
*/

void QIconViewItem::setDragEnabled(bool allow)
{
    allow_drag = (uint)allow;
}

/*!
    If \a allow is true, the icon view lets the user drop something on
    this icon view item.
*/

void QIconViewItem::setDropEnabled(bool allow)
{
    allow_drop = (uint)allow;
}

/*!
    Returns the text of the icon view item. Normally you set the text
    of the item with setText(), but sometimes it's inconvenient to
    call setText() for every item; so you can subclass QIconViewItem,
    reimplement this function, and return the text of the item. If you
    do this, you must call calcRect() manually each time the text
    (and therefore its size) changes.

    \sa setText()
*/

QString QIconViewItem::text() const
{
    return itemText;
}

/*!
    Returns the key of the icon view item or text() if no key has been
    explicitly set.

    \sa setKey(), compare()
*/

QString QIconViewItem::key() const
{
    return itemKey;
}

/*!
    Returns the icon of the icon view item if it is a pixmap, or 0 if
    it is a picture. In the latter case use picture() instead.
    Normally you set the pixmap of the item with setPixmap(), but
    sometimes it's inconvenient to call setPixmap() for every item. So
    you can subclass QIconViewItem, reimplement this function and
    return a pointer to the item's pixmap. If you do this, you \e must
    call calcRect() manually each time the size of this pixmap
    changes.

    \sa setPixmap()
*/

QPixmap *QIconViewItem::pixmap() const
{
    return itemIcon;
}

/*!
    Returns the icon of the icon view item if it is a picture, or 0 if
    it is a pixmap. In the latter case use pixmap() instead. Normally
    you set the picture of the item with setPicture(), but sometimes
    it's inconvenient to call setPicture() for every item. So you can
    subclass QIconViewItem, reimplement this function and return a
    pointer to the item's picture. If you do this, you \e must call
    calcRect() manually each time the size of this picture changes.

    \sa setPicture()
*/

#ifndef QT_NO_PICTURE
QPicture *QIconViewItem::picture() const
{
    return itemPic;
}
#endif

/*!
    Returns true if the item can be renamed by the user with in-place
    renaming; otherwise returns false.

    \sa setRenameEnabled()
*/

bool QIconViewItem::renameEnabled() const
{
    return (bool)allow_rename;
}

/*!
    Returns true if the user is allowed to drag the icon view item;
    otherwise returns false.

    \sa setDragEnabled()
*/

bool QIconViewItem::dragEnabled() const
{
    return (bool)allow_drag;
}

/*!
    Returns true if the user is allowed to drop something onto the
    item; otherwise returns false.

    \sa setDropEnabled()
*/

bool QIconViewItem::dropEnabled() const
{
    return (bool)allow_drop;
}

/*!
    Returns a pointer to this item's icon view parent.
*/

QIconView *QIconViewItem::iconView() const
{
    return view;
}

/*!
    Returns a pointer to the previous item, or 0 if this is the first
    item in the icon view.

    \sa nextItem() QIconView::firstItem()
*/

QIconViewItem *QIconViewItem::prevItem() const
{
    return prev;
}

/*!
    Returns a pointer to the next item, or 0 if this is the last item
    in the icon view.

    To find the first item use QIconView::firstItem().

    Example:
    \code
    QIconViewItem *item;
    for (item = iconView->firstItem(); item; item = item->nextItem())
        do_something_with(item);
    \endcode

    \sa prevItem()
*/

QIconViewItem *QIconViewItem::nextItem() const
{
    return next;
}

/*!
    Returns the index of this item in the icon view, or -1 if an error
    occurred.
*/

int QIconViewItem::index() const
{
    if (view)
        return view->index(this);

    return -1;
}



/*!
    \overload

    This variant is equivalent to calling the other variant with \e cb
    set to false.
*/

void QIconViewItem::setSelected(bool s)
{
    setSelected(s, false);
}

/*!
    Selects or unselects the item, depending on \a s; it may also
    unselect other items, depending on QIconView::selectionMode() and
    \a cb.

    If \a s is false, the item is unselected.

    If \a s is true and QIconView::selectionMode() is \c Single, the
    item is selected and the item previously selected is unselected.

    If \a s is true and QIconView::selectionMode() is \c Extended, the
    item is selected. If \a cb is true, the selection state of the
    other items is left unchanged. If \a cb is false (the default) all
    other items are unselected.

    If \a s is true and QIconView::selectionMode() is \c Multi, the
    item is selected.

    Note that \a cb is used only if QIconView::selectionMode() is \c
    Extended; cb defaults to false.

    All items whose selection status changes repaint themselves.
*/

void QIconViewItem::setSelected(bool s, bool cb)
{
    if (!view)
        return;
    if (view->selectionMode() != QIconView::NoSelection &&
         selectable && s != (bool)selected) {

        if (view->d->selectionMode == QIconView::Single && this != view->d->currentItem) {
            QIconViewItem *o = view->d->currentItem;
            if (o && o->selected)
                o->selected = false;
            view->d->currentItem = this;
            if (o)
                o->repaint();
            emit view->currentChanged(this);
        }

        if (!s) {
            selected = false;
        } else {
            if (view->d->selectionMode == QIconView::Single && view->d->currentItem) {
                view->d->currentItem->selected = false;
            }
            if ((view->d->selectionMode == QIconView::Extended && !cb) ||
                 view->d->selectionMode == QIconView::Single) {
                bool b = view->signalsBlocked();
                view->blockSignals(true);
                view->selectAll(false);
                view->blockSignals(b);
            }
            selected = s;
        }

        repaint();
        if (!view->signalsBlocked()) {
            bool emitIt = view->d->selectionMode == QIconView::Single && s;
            QIconView *v = view;
            emit v->selectionChanged();
            if (emitIt)
                emit v->selectionChanged(this);
        }
    }
}

/*!
    Sets this item to be selectable if \a enable is true (the default)
    or unselectable if \a enable is false.

    The user is unable to select a non-selectable item using either
    the keyboard or the mouse. (The application programmer can select
    an item in code regardless of this setting.)

    \sa isSelectable()
*/

void QIconViewItem::setSelectable(bool enable)
{
    selectable = (uint)enable;
}

/*!
    Returns true if the item is selected; otherwise returns false.

    \sa setSelected()
*/

bool QIconViewItem::isSelected() const
{
    return (bool)selected;
}

/*!
    Returns true if the item is selectable; otherwise returns false.

    \sa setSelectable()
*/

bool QIconViewItem::isSelectable() const
{
    return (bool)selectable;
}

/*!
    Repaints the item.
*/

void QIconViewItem::repaint()
{
    if (view)
        view->repaintItem(this);
}

/*!
    Moves the item to position (\a x, \a y) in the icon view (these
    are contents coordinates).
*/

bool QIconViewItem::move(int x, int y)
{
    if (x == this->x() && y == this->y())
        return false;
    itemRect.setRect(x, y, itemRect.width(), itemRect.height());
    checkRect();
    if (view)
        view->updateItemContainer(this);
    return true;
}

/*!
    Moves the item \a dx pixels in the x-direction and \a dy pixels in
    the y-direction.
*/

void QIconViewItem::moveBy(int dx, int dy)
{
    itemRect.moveBy(dx, dy);
    checkRect();
    if (view)
        view->updateItemContainer(this);
}

/*!
    \overload

    Moves the item to the point \a pnt.
*/

bool QIconViewItem::move(const QPoint &pnt)
{
    return move(pnt.x(), pnt.y());
}

/*!
    \overload

    Moves the item by the x, y values in point \a pnt.
*/

void QIconViewItem::moveBy(const QPoint &pnt)
{
    moveBy(pnt.x(), pnt.y());
}

/*!
    Returns the bounding rectangle of the item (in contents
    coordinates).
*/

QRect QIconViewItem::rect() const
{
    return itemRect;
}

/*!
    Returns the x-coordinate of the item (in contents coordinates).
*/

int QIconViewItem::x() const
{
    return itemRect.x();
}

/*!
    Returns the y-coordinate of the item (in contents coordinates).
*/

int QIconViewItem::y() const
{
    return itemRect.y();
}

/*!
    Returns the width of the item.
*/

int QIconViewItem::width() const
{
    return qMax(itemRect.width(), QApplication::globalStrut().width());
}

/*!
    Returns the height of the item.
*/

int QIconViewItem::height() const
{
    return qMax(itemRect.height(), QApplication::globalStrut().height());
}

/*!
    Returns the size of the item.
*/

QSize QIconViewItem::size() const
{
    return QSize(itemRect.width(), itemRect.height());
}

/*!
    Returns the position of the item (in contents coordinates).
*/

QPoint QIconViewItem::pos() const
{
    return QPoint(itemRect.x(), itemRect.y());
}

/*!
    Returns the bounding rectangle of the item's text.

    If \a relative is true, (the default), the returned rectangle is
    relative to the origin of the item's rectangle. If \a relative is
    false, the returned rectangle is relative to the origin of the
    icon view's contents coordinate system.
*/

QRect QIconViewItem::textRect(bool relative) const
{
    if (relative)
        return itemTextRect;
    else
        return QRect(x() + itemTextRect.x(), y() + itemTextRect.y(), itemTextRect.width(), itemTextRect.height());
}

/*!
    Returns the bounding rectangle of the item's icon.

    If \a relative is true, (the default), the rectangle is relative to
    the origin of the item's rectangle. If \a relative is false, the
    returned rectangle is relative to the origin of the icon view's
    contents coordinate system.
*/

QRect QIconViewItem::pixmapRect(bool relative) const
{
    if (relative)
        return itemIconRect;
    else
        return QRect(x() + itemIconRect.x(), y() + itemIconRect.y(), itemIconRect.width(), itemIconRect.height());
}

/*!
    Returns true if the item contains the point \a pnt (in contents
    coordinates); otherwise returns false.
*/

bool QIconViewItem::contains(const QPoint& pnt) const
{
    QRect textArea = textRect(false);
    QRect pixmapArea = pixmapRect(false);
    if (iconView()->itemTextPos() == QIconView::Bottom)
        textArea.setTop(pixmapArea.bottom());
    else
        textArea.setLeft(pixmapArea.right());
    return textArea.contains(pnt) || pixmapArea.contains(pnt);
}

/*!
    Returns true if the item intersects the rectangle \a r (in
    contents coordinates); otherwise returns false.
*/

bool QIconViewItem::intersects(const QRect& r) const
{
    return (textRect(false).intersects(r) ||
             pixmapRect(false).intersects(r));
}

/*!
    \fn bool QIconViewItem::acceptDrop(const QMimeSource *mime) const

    Returns true if you can drop things with a QMimeSource of \a mime
    onto this item; otherwise returns false.

    The default implementation always returns false. You must subclass
    QIconViewItem and reimplement acceptDrop() to accept drops.
*/

bool QIconViewItem::acceptDrop(const QMimeSource *) const
{
    return false;
}

#ifndef QT_NO_TEXTEDIT
/*!
    Starts in-place renaming of an icon, if allowed.

    This function sets up the icon view so that the user can edit the
    item text, and then returns. When the user is done, setText() will
    be called and QIconView::itemRenamed() will be emitted (unless the
    user canceled, e.g. by pressing the Escape key).

    \sa setRenameEnabled()
*/

void QIconViewItem::rename()
{
    if (!view)
        return;
    if (renameBox)
        removeRenameBox();
    oldRect = rect();
    renameBox = new QIconViewItemLineEdit(itemText, view->viewport(), this, "qt_renamebox");
    iconView()->ensureItemVisible(this);
    QRect tr(textRect(false));
    view->addChild(renameBox, tr.x() + (tr.width() / 2 - renameBox->width() / 2), tr.y() - 3);
    renameBox->selectAll();
    view->viewport()->setFocusProxy(renameBox);
    renameBox->setFocus();
    renameBox->show();
    Q_ASSERT(view->d->renamingItem == 0L);
    view->d->renamingItem = this;
}
#endif

/*!
    Compares this icon view item to \a i. Returns -1 if this item is
    less than \a i, 0 if they are equal, and 1 if this icon view item
    is greater than \a i.

    The default implementation compares the item keys (key()) using
    QString::localeAwareCompare(). A reimplementation may use
    different values and a different comparison function. Here is a
    reimplementation that uses plain Unicode comparison:

    \code
        int MyIconViewItem::compare(QIconViewItem *i) const
        {
            return key().compare(i->key());
        }
    \endcode

    \sa key() QString::localeAwareCompare() QString::compare()
*/

int QIconViewItem::compare(QIconViewItem *i) const
{
    return key().localeAwareCompare(i->key());
}

#ifndef QT_NO_TEXTEDIT
/*!
  This private function is called when the user pressed Return during
  in-place renaming.
*/

void QIconViewItem::renameItem()
{
    if (!renameBox || !view)
        return;

    if (!view->d->wordWrapIconText) {
        wordWrapDirty = true;
        calcRect();
    }
    QRect r = itemRect;
    setText(renameBox->text());
    view->repaintContents(oldRect.x() - 1, oldRect.y() - 1, oldRect.width() + 2, oldRect.height() + 2);
    view->repaintContents(r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2);
    removeRenameBox();

    view->emitRenamed(this);
}

/*!
    Cancels in-place renaming.
*/

void QIconViewItem::cancelRenameItem()
{
    if (!view)
        return;

    QRect r = itemRect;
    calcRect();
    view->repaintContents(oldRect.x() - 1, oldRect.y() - 1, oldRect.width() + 2, oldRect.height() + 2);
    view->repaintContents(r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2);

    if (!renameBox)
        return;

    removeRenameBox();
}

/*!
    Removes the editbox that is used for in-place renaming.
*/

void QIconViewItem::removeRenameBox()
{
    if (!renameBox || !view)
        return;

    bool resetFocus = view->viewport()->focusProxy() == renameBox;
    delete renameBox;
    renameBox = 0;
    if (resetFocus) {
        view->viewport()->setFocusProxy(view);
        view->setFocus();
    }
    Q_ASSERT(view->d->renamingItem == this);
    view->d->renamingItem = 0L;
}
#endif

/*!
    This virtual function is responsible for calculating the
    rectangles returned by rect(), textRect() and pixmapRect().
    setRect(), setTextRect() and setPixmapRect() are provided mainly
    for reimplementations of this function.

    \a text_ is an internal parameter which defaults to QString::null.
*/

void QIconViewItem::calcRect(const QString &text_)
{
    if (!view) // #####
        return;

    wordWrapDirty = true;
    int pw = 0;
    int ph = 0;

#ifndef QT_NO_PICTURE
    if (picture()) {
        QRect br = picture()->boundingRect();
        pw = br.width() + 2;
        ph = br.height() + 2;
    } else
#endif
    {
        pw = (pixmap() ? pixmap() : unknown_icon)->width() + 2;
        ph = (pixmap() ? pixmap() : unknown_icon)->height() + 2;
    }

    itemIconRect.setWidth(pw);
    itemIconRect.setHeight(ph);

    calcTmpText();

    QString t = text_;
    if (t.isEmpty()) {
        if (view->d->wordWrapIconText)
            t = itemText;
        else
            t = tmpText;
    }

    int tw = 0;
    int th = 0;
    // ##### TODO: fix font bearings!
    QRect r;
    if (view->d->wordWrapIconText) {
        r = QRect(view->d->fm->boundingRect(0, 0, iconView()->maxItemWidth() -
                                              (iconView()->itemTextPos() == QIconView::Bottom ? 0 :
                                                pixmapRect().width()),
                                              0xFFFFFFFF, AlignHCenter | WordBreak | BreakAnywhere, t));
        r.setWidth(r.width() + 4);
    } else {
        r = QRect(0, 0, view->d->fm->width(t), view->d->fm->height());
        r.setWidth(r.width() + 4);
    }

    if (r.width() > iconView()->maxItemWidth() -
         (iconView()->itemTextPos() == QIconView::Bottom ? 0 :
           pixmapRect().width()))
        r.setWidth(iconView()->maxItemWidth() - (iconView()->itemTextPos() == QIconView::Bottom ? 0 :
                                                   pixmapRect().width()));

    tw = r.width();
    th = r.height();
    if (tw < view->d->fm->width("X"))
        tw = view->d->fm->width("X");

    itemTextRect.setWidth(tw);
    itemTextRect.setHeight(th);

    int w = 0;
    int h = 0;
    if (view->itemTextPos() == QIconView::Bottom) {
        w = qMax(itemTextRect.width(), itemIconRect.width());
        h = itemTextRect.height() + itemIconRect.height() + 1;

        itemRect.setWidth(w);
        itemRect.setHeight(h);

        itemTextRect = QRect((width() - itemTextRect.width()) / 2, height() - itemTextRect.height(),
                              itemTextRect.width(), itemTextRect.height());
        itemIconRect = QRect((width() - itemIconRect.width()) / 2, 0,
                              itemIconRect.width(), itemIconRect.height());
    } else {
        h = qMax(itemTextRect.height(), itemIconRect.height());
        w = itemTextRect.width() + itemIconRect.width() + 1;

        itemRect.setWidth(w);
        itemRect.setHeight(h);

        itemTextRect = QRect(width() - itemTextRect.width(), (height() - itemTextRect.height()) / 2,
                              itemTextRect.width(), itemTextRect.height());
        itemIconRect = QRect(0, (height() - itemIconRect.height()) / 2,
                              itemIconRect.width(), itemIconRect.height());
    }
    if (view)
        view->updateItemContainer(this);
}

/*!
    Paints the item using the painter \a p and the color group \a cg.
    If you want the item to be drawn with a different font or color,
    reimplement this function, change the values of the color group or
    the painter's font, and then call the QIconViewItem::paintItem()
    with the changed values.
*/

void QIconViewItem::paintItem(QPainter *p, const QPalette &cg)
{
    if (!view)
        return;

    p->save();

    if (isSelected()) {
        p->setPen(cg.highlightedText());
    } else {
        p->setPen(cg.text());
    }

    calcTmpText();

#ifndef QT_NO_PICTURE
    if (picture()) {
        QPicture *pic = picture();
        if (isSelected()) {
            p->fillRect(pixmapRect(false), QBrush(cg.highlight(), QBrush::Dense4Pattern));
        }
        p->drawPicture(x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic);
        if (isSelected()) {
            p->fillRect(textRect(false), cg.highlight());
            p->setPen(QPen(cg.highlightedText()));
        } else if (view->d->itemTextBrush != QBrush(NoBrush))
            p->fillRect(textRect(false), view->d->itemTextBrush);

        int align = view->itemTextPos() == QIconView::Bottom ? AlignHCenter : AlignAuto;
        if (view->d->wordWrapIconText)
            align |= WordBreak | BreakAnywhere;
        p->drawText(textRect(false), align, view->d->wordWrapIconText ? itemText : tmpText);
        p->restore();
        return;
    }
#endif
    bool textOnBottom = (view->itemTextPos() == QIconView::Bottom);
    int dim;
    if (textOnBottom)
        dim = (pixmap() ? pixmap() : unknown_icon)->width();
    else
        dim = (pixmap() ? pixmap() : unknown_icon)->height();
    if (isSelected()) {
        QPixmap *pix = pixmap() ? pixmap() : unknown_icon;
        if (pix && !pix->isNull()) {
            QPixmap *buffer = get_qiv_buffer_pixmap(pix->size());
            QBitmap mask = view->mask(pix);

            QPainter p2(buffer);
            p2.fillRect(pix->rect(), white);
            p2.drawPixmap(0, 0, *pix);
            p2.end();
            buffer->setMask(mask);
            p2.begin(buffer);
#if defined(Q_WS_X11)
            p2.fillRect(pix->rect(), QBrush(cg.highlight(), QBrush::Dense4Pattern));
#else // in WIN32 Dense4Pattern doesn't work correctly (transparency problem), so work around it
            if (iconView()->d->drawActiveSelection) {
                if (!qiv_selection)
                    createSelectionPixmap(cg);
                p2.drawTiledPixmap(0, 0, pix->width(), pix->height(),
                                    *qiv_selection);
            }
#endif
            p2.end();
            QRect cr = pix->rect();
            if (textOnBottom)
                p->drawPixmap(x() + (width() - dim) / 2, y(), *buffer, 0, 0,
                               cr.width(), cr.height());
            else
                p->drawPixmap(x() , y() + (height() - dim) / 2, *buffer, 0, 0,
                               cr.width(), cr.height());
        }
    } else {
        if (textOnBottom)
            p->drawPixmap(x() + (width() - dim) / 2, y(),
                           *(pixmap() ? pixmap() : unknown_icon));
        else
            p->drawPixmap(x() , y() + (height() - dim) / 2,
                           *(pixmap() ? pixmap() : unknown_icon));
    }

    p->save();
    if (isSelected()) {
        p->fillRect(textRect(false), cg.highlight());
        p->setPen(QPen(cg.highlightedText()));
    } else if (view->d->itemTextBrush != QBrush(NoBrush))
        p->fillRect(textRect(false), view->d->itemTextBrush);

    int align = AlignHCenter;
    if (view->d->wordWrapIconText)
        align |= WordBreak | BreakAnywhere;
    p->drawText(textRect(false), align,
                 view->d->wordWrapIconText ? itemText : tmpText);

    p->restore();

    p->restore();
}

/*!
    Paints the focus rectangle of the item using the painter \a p and
    the color group \a cg.
*/

void QIconViewItem::paintFocus(QPainter *p, const QPalette &cg)
{
    if (!view)
        return;

    view->style().drawPrimitive(QStyle::PE_FocusRect, p,
                                QRect(textRect(false).x(), textRect(false).y(),
                                       textRect(false).width(),
                                       textRect(false).height()), cg,
                                (isSelected() ?
                                 QStyle::Style_FocusAtBorder :
                                 QStyle::Style_Default),
                                QStyleOption(isSelected() ? cg.highlight() : cg.base()));

    if (this != view->d->currentItem) {
        view->style().drawPrimitive(QStyle::PE_FocusRect, p,
                                    QRect(pixmapRect(false).x(),
                                           pixmapRect(false).y(),
                                           pixmapRect(false).width(),
                                           pixmapRect(false).height()),
                                    cg, QStyle::Style_Default,
                                    QStyleOption(cg.base()));
    }
}

/*!
    \fn void QIconViewItem::dropped(QDropEvent *e, const QLinkedList<QIconDragItem> &lst)

    This function is called when something is dropped on the item. \a
    e provides all the information about the drop. If the drag object
    of the drop was a QIconDrag, \a lst contains the list of the
    dropped items. You can get the data by calling
    QIconDragItem::data() on each item. If the \a lst is empty, i.e.
    the drag was not a QIconDrag, you must decode the data in \a e and
    work with that.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

#ifndef QT_NO_DRAGANDDROP
void QIconViewItem::dropped(QDropEvent *, const QLinkedList<QIconDragItem> &)
{
}
#endif

/*!
    This function is called when a drag enters the item's bounding
    rectangle.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

void QIconViewItem::dragEntered()
{
}

/*!
    This function is called when a drag leaves the item's bounding
    rectangle.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

void QIconViewItem::dragLeft()
{
}

/*!
    Sets the bounding rectangle of the whole item to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() textRect() setTextRect() pixmapRect() setPixmapRect()
*/

void QIconViewItem::setItemRect(const QRect &r)
{
    itemRect = r;
    checkRect();
    if (view)
        view->updateItemContainer(this);
}

/*!
    Sets the bounding rectangle of the item's text to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() textRect() setItemRect() setPixmapRect()
*/

void QIconViewItem::setTextRect(const QRect &r)
{
    itemTextRect = r;
    if (view)
        view->updateItemContainer(this);
}

/*!
    Sets the bounding rectangle of the item's icon to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() pixmapRect() setItemRect() setTextRect()
*/

void QIconViewItem::setPixmapRect(const QRect &r)
{
    itemIconRect = r;
    if (view)
        view->updateItemContainer(this);
}

/*!
    \internal
*/

void QIconViewItem::calcTmpText()
{
    if (!view || view->d->wordWrapIconText || !wordWrapDirty)
        return;
    wordWrapDirty = false;

    int w = iconView()->maxItemWidth() - (iconView()->itemTextPos() == QIconView::Bottom ? 0 :
                                           pixmapRect().width());
    if (view->d->fm->width(itemText) < w) {
        tmpText = itemText;
        return;
    }

    tmpText = "...";
    int i = 0;
    while (view->d->fm->width(tmpText + itemText[i]) < w)
        tmpText += itemText[i++];
    tmpText.remove((uint)0, 3);
    tmpText += "...";
}

/*! \internal */

QString QIconViewItem::tempText() const
{
    return tmpText;
}

void QIconViewItem::checkRect()
{
    int x = itemRect.x();
    int y = itemRect.y();
    int w = itemRect.width();
    int h = itemRect.height();

    bool changed = false;
    if (x < 0) {
        x = 0;
        changed = true;
    }
    if (y < 0) {
        y = 0;
        changed = true;
    }

    if (changed)
        itemRect.setRect(x, y, w, h);
}


/*! \file iconview/simple_dd/main.h */
/*! \file iconview/simple_dd/main.cpp */


/*!
    \class QIconView qiconview.h
    \brief The QIconView class provides an area with movable labelled icons.

    \module iconview
    \ingroup advanced
    \mainclass

    A QIconView can display and manage a grid or other 2D layout of
    labelled icons. Each labelled icon is a QIconViewItem. Items
    (QIconViewItems) can be added or deleted at any time; items can be
    moved within the QIconView. Single or multiple items can be
    selected. Items can be renamed in-place. QIconView also supports
    \link #draganddrop drag and drop\endlink.

    Each item contains a label string, a pixmap or picture (the icon
    itself) and optionally a sort key. The sort key is used for
    sorting the items and defaults to the label string. The label
    string can be displayed below or to the right of the icon (see \l
    ItemTextPos).

    The simplest way to create a QIconView is to create a QIconView
    object and create some QIconViewItems with the QIconView as their
    parent, set the icon view's geometry and show it.
    For example:
    \code
    QIconView *iv = new QIconView(this);
    QDir dir(path, "*.xpm");
    for (uint i = 0; i < dir.count(); i++) {
        (void) new QIconViewItem(iv, dir[i], QPixmap(path + dir[i]));
    }
    iv->resize(600, 400);
    iv->show();
    \endcode

    The QIconViewItem call passes a pointer to the QIconView we wish to
    populate, along with the label text and a QPixmap.

    When an item is inserted the QIconView allocates a position for it.
    Existing items are rearranged if autoArrange() is true. The
    default arrangement is \c LeftToRight -- the QIconView fills up
    the \e left-most column from top to bottom, then moves one column
    \e right and fills that from top to bottom and so on. The
    arrangement can be modified with any of the following approaches:
    \list
    \i Call setArrangement(), e.g. with \c TopToBottom which will fill
    the \e top-most row from left to right, then moves one row \e down
    and fills that row from left to right and so on.
    \i Construct each QIconViewItem using a constructor which allows
    you to specify which item the new one is to follow.
    \i Call setSorting() or sort() to sort the items.
    \endlist

    The spacing between items is set with setSpacing(). Items can be
    laid out using a fixed grid using setGridX() and setGridY(); by
    default the QIconView calculates a grid dynamically. The position
    of items' label text is set with setItemTextPos(). The text's
    background can be set with setItemTextBackground(). The maximum
    width of an item and of its text are set with setMaxItemWidth()
    and setMaxItemTextLength(). The label text will be word-wrapped if
    it is too long; this is controlled by setWordWrapIconText(). If
    the label text is truncated, the user can still see the entire
    text in a tool tip if they hover the mouse over the item. This is
    controlled with setShowToolTips().

    Items which are \link QIconViewItem::isSelectable()
    selectable\endlink may be selected depending on the SelectionMode;
    the default is \c Single. Because QIconView offers multiple
    selection it must display keyboard focus and selection state
    separately. Therefore there are functions to set the selection
    state of an item (setSelected()) and to select which item displays
    keyboard focus (setCurrentItem()). When multiple items may be
    selected the icon view provides a rubberband, too.

    When in-place renaming is enabled (it is disabled by default), the
    user may change the item's label. They do this by selecting the item
    (single clicking it or navigating to it with the arrow keys), then
    single clicking it (or pressing F2), and entering their text. If no
    key has been set with QIconViewItem::setKey() the new text will also
    serve as the key. (See QIconViewItem::setRenameEnabled().)

    You can control whether users can move items themselves with
    setItemsMovable().

    Because the internal structure used to store the icon view items is
    linear, no iterator class is needed to iterate over all the items.
    Instead we iterate by getting the first item from the \e{icon view}
    and then each subsequent (\l QIconViewItem::nextItem()) from each
    \e item in turn:
    \code
        for (QIconViewItem *item = iv->firstItem(); item; item = item->nextItem())
            do_something(item);
    \endcode
    QIconView also provides currentItem(). You can search for an item
    using findItem() (searching by position or for label text) and
    with findFirstVisibleItem() and findLastVisibleItem(). The number
    of items is returned by count(). An item can be removed from an
    icon view using takeItem(); to delete an item use \c delete. All
    the items can be deleted with clear().

    The QIconView emits a wide range of useful signals, including
    selectionChanged(), currentChanged(), clicked(), moved() and
    itemRenamed().

    \target draganddrop
    \section1 Drag and Drop

    QIconView supports the drag and drop of items within the QIconView
    itself. It also supports the drag and drop of items out of or into
    the QIconView and drag and drop onto items themselves. The drag and
    drop of items outside the QIconView can be achieved in a simple way
    with basic functionality, or in a more sophisticated way which
    provides more power and control.

    The simple approach to dragging items out of the icon view is to
    subclass QIconView and reimplement QIconView::dragObject().

    \code
    QDragObject *MyIconView::dragObject()
    {
        return new QTextDrag(currentItem()->text(), this);
    }
    \endcode

    In this example we create a QTextDrag object, (derived from
    QDragObject), containing the item's label and return it as the drag
    object. We could just as easily have created a QImageDrag from the
    item's pixmap and returned that instead.

    QIconViews and their QIconViewItems can also be the targets of drag
    and drops. To make the QIconView itself able to accept drops connect
    to the dropped() signal. When a drop occurs this signal will be
    emitted with a QDragEvent and a QLinkedList of QIconDragItems. To
    make a QIconViewItem into a drop target subclass QIconViewItem and
    reimplement QIconViewItem::acceptDrop() and
    QIconViewItem::dropped().

    \code
    bool MyIconViewItem::acceptDrop(const QMimeSource *mime) const
    {
        if (mime->provides("text/plain"))
            return true;
        return false;
    }

    void MyIconViewItem::dropped(QDropEvent *evt, const QLinkedList<QIconDragItem>&)
    {
        QString label;
        if (QTextDrag::decode(evt, label))
            setText(label);
    }
    \endcode

    See \l iconview/simple_dd/main.h and \l
    iconview/simple_dd/main.cpp for a simple drag and drop example
    which demonstrates drag and drop between a QIconView and a
    QListBox.

    If you want to use extended drag-and-drop or have drag shapes drawn
    you must take a more sophisticated approach.

    The first part is starting drags -- you should use a QIconDrag (or a
    class derived from it) for the drag object. In dragObject() create the
    drag object, populate it with QIconDragItems and return it. Normally
    such a drag should offer each selected item's data. So in dragObject()
    you should iterate over all the items, and create a QIconDragItem for
    each selected item, and append these items with QIconDrag::append() to
    the QIconDrag object. You can use QIconDragItem::setData() to set the
    data of each item that should be dragged. If you want to offer the
    data in additional mime-types, it's best to use a class derived from
    QIconDrag, which implements additional encoding and decoding
    functions.

    When a drag enters the icon view, there is little to do. Simply
    connect to the dropped() signal and reimplement
    QIconViewItem::acceptDrop() and QIconViewItem::dropped(). If you've
    used a QIconDrag (or a subclass of it) the second argument to the
    dropped signal contains a QLinkedList of QIconDragItems -- you can
    access their data by calling QIconDragItem::data() on each one.

    For an example implementation of complex drag-and-drop look at the
    fileiconview example (qt/examples/fileiconview).

    \sa QIconViewItem::setDragEnabled(), QIconViewItem::setDropEnabled(),
        QIconViewItem::acceptDrop(), QIconViewItem::dropped().

    <img src=qiconview-m.png> <img src=qiconview-w.png>
*/

/*! \enum QIconView::ResizeMode

    This enum type is used to tell QIconView how it should treat the
    positions of its icons when the widget is resized. The modes are:

    \value Fixed  The icons' positions are not changed.
    \value Adjust  The icons' positions are adjusted to be within the
    new geometry, if possible.
*/

/*!
    \enum QIconView::SelectionMode

    This enumerated type is used by QIconView to indicate how it
    reacts to selection by the user. It has four values:

    \value Single  When the user selects an item, any already-selected
    item becomes unselected and the user cannot unselect the selected
    item. This means that the user can never clear the selection. (The
    application programmer can, using QIconView::clearSelection().)

    \value Multi  When the user selects an item, e.g. by navigating to
    it with the keyboard arrow keys or by clicking it, the selection
    status of that item is toggled and the other items are left alone.

    \value Extended  When the user selects an item the selection is
    cleared and the new item selected. However, if the user presses
    the Ctrl key when clicking on an item, the clicked item gets
    toggled and all other items are left untouched. If the user
    presses the Shift key while clicking on an item, all items between
    the current item and the clicked item get selected or unselected,
    depending on the state of the clicked item. Also, multiple items
    can be selected by dragging the mouse while the left mouse button
    stays pressed.

    \value NoSelection  Items cannot be selected.

    To summarise: \c Single is a real single-selection icon view; \c
    Multi a real multi-selection icon view; \c Extended is an icon
    view in which users can select multiple items but usually want to
    select either just one or a range of contiguous items; and \c
    NoSelection mode is for an icon view where the user can look but
    not touch.
*/

/*!
    \enum QIconView::Arrangement

    This enum type determines in which direction the items flow when
    the view runs out of space.

    \value LeftToRight  Items which don't fit into the view go further
    down (you get a vertical scrollbar)

    \value TopToBottom  Items which don't fit into the view go further
    right (you get a horizontal scrollbar)
*/

/*!
    \enum QIconView::ItemTextPos

    This enum type specifies the position of the item text in relation
    to the icon.

    \value Bottom  The text is drawn below the icon.
    \value Right  The text is drawn to the right of the icon.
*/

/*!
    \fn void  QIconView::dropped (QDropEvent * e, const QLinkedList<QIconDragItem> &lst)

    This signal is emitted when a drop event occurs in the viewport
    (but not on any icon) which the icon view itself can't handle.

    \a e provides all the information about the drop. If the drag
    object of the drop was a QIconDrag, \a lst contains the list of
    the dropped items. You can get the data using
    QIconDragItem::data() on each item. If the \a lst is empty, i.e.
    the drag was not a QIconDrag, you have to decode the data in \a e
    and work with that.

    Note QIconViewItems may be drop targets; if a drop event occurs on
    an item the item handles the drop.
*/

/*!
    \fn void QIconView::moved()

    This signal is emitted after successfully dropping one (or more)
    items of the icon view. If the items should be removed, it's best
    to do so in a slot connected to this signal.
*/

/*!
    \fn void  QIconView::doubleClicked(QIconViewItem * item)

    This signal is emitted when the user double-clicks on \a item.
*/

/*!
    \fn void  QIconView::returnPressed (QIconViewItem * item)

    This signal is emitted if the user presses the Return or Enter
    key. \a item is the currentItem() at the time of the keypress.
*/

/*!
    \fn void  QIconView::selectionChanged()

    This signal is emitted when the selection has been changed. It's
    emitted in each selection mode.
*/

/*!
    \fn void QIconView::selectionChanged(QIconViewItem *item)
    \overload

    This signal is emitted when the selection changes. \a item is the
    newly selected item. This signal is emitted only in single
    selection mode.
*/

/*!
    \fn void QIconView::currentChanged(QIconViewItem *item)

    This signal is emitted when a new item becomes current. \a item is
    the new current item (or 0 if no item is now current).

    \sa currentItem()
*/

/*!
    \fn void  QIconView::onItem(QIconViewItem *item)

    This signal is emitted when the user moves the mouse cursor onto
    an \a item, similar to the QWidget::enterEvent() function.
*/

// ### bug here - enter/leave event aren't considered. move the mouse
// out of the window and back in, to the same item.

/*!
    \fn void QIconView::onViewport()

    This signal is emitted when the user moves the mouse cursor from
    an item to an empty part of the icon view.

    \sa onItem()
*/

/*!
    \fn void QIconView::itemRenamed (QIconViewItem * item)
    \overload

    This signal is emitted when \a item has been renamed, usually by
    in-place renaming.

    \sa QIconViewItem::setRenameEnabled() QIconViewItem::rename()
*/

/*!
    \fn void QIconView::itemRenamed (QIconViewItem * item, const QString &name)

    This signal is emitted when \a item has been renamed to \a name,
    usually by in-place renaming.

    \sa QIconViewItem::setRenameEnabled() QIconViewItem::rename()
*/

/*!
    \fn void QIconView::rightButtonClicked (QIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user clicks the right mouse
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa rightButtonPressed() mouseButtonClicked() clicked()
*/

/*!
    \fn void QIconView::contextMenuRequested(QIconViewItem *item, const QPoint & pos)

    This signal is emitted when the user invokes a context menu with
    the right mouse button or with special system keys, with \a item
    being the item under the mouse cursor or the current item,
    respectively.

    \a pos is the position for the context menu in the global
    coordinate system.
*/

/*!
    \fn void QIconView::mouseButtonPressed (int button, QIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user presses mouse button \a
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).

    \sa rightButtonClicked() mouseButtonPressed() pressed()
*/

/*!
    \fn void QIconView::mouseButtonClicked (int button, QIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user clicks mouse button \a
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa mouseButtonPressed() rightButtonClicked() clicked()
*/

/*!
    \fn void QIconView::clicked (QIconViewItem * item, const QPoint & pos)
    \overload

    This signal is emitted when the user clicks any mouse button on an
    icon view item. \a item is a pointer to the item that has been
    clicked.

    \a pos is the position of the mouse cursor in the global coordinate
    system (QMouseEvent::globalPos()). (If the click's press and release
    differ by a pixel or two, \a pos is the  position at release time.)

    \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
    \fn void QIconView::pressed (QIconViewItem * item, const QPoint & pos)
    \overload

    This signal is emitted when the user presses any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
    \fn void QIconView::clicked (QIconViewItem * item)

    This signal is emitted when the user clicks any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
    \fn void QIconView::pressed (QIconViewItem * item)

    This signal is emitted when the user presses any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
    \fn void QIconView::rightButtonPressed(QIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user presses the right mouse
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).
*/

/*!
    Constructs an empty icon view called \a name, with parent \a
    parent and using the widget flags \a f.
*/

QIconView::QIconView(QWidget *parent, const char *name, WFlags f)
    : QScrollView(parent, name, WStaticContents | WNoAutoErase  | f)
{
    if (!unknown_icon) {
        unknown_icon = new QPixmap((const char **)unknown_xpm);
        qiv_cleanup_pixmap.add(&unknown_icon);
    }

    d = new QIconViewPrivate;
    d->dragging = false;
    d->firstItem = 0;
    d->lastItem = 0;
    d->count = 0;
    d->mousePressed = false;
    d->selectionMode = Single;
    d->currentItem = 0;
    d->highlightedItem = 0;
    d->rubber = 0;
    d->scrollTimer = 0;
    d->startDragItem = 0;
    d->tmpCurrentItem = 0;
    d->rastX = d->rastY = -1;
    d->spacing = 5;
    d->cleared = false;
    d->arrangement = LeftToRight;
    d->resizeMode = Fixed;
    d->dropped = false;
    d->adjustTimer = new QTimer(this, "iconview adjust timer");
    d->isIconDrag = false;
    d->inMenuMode = false;
#ifndef QT_NO_DRAGANDDROP
    d->iconDragData.clear();
#endif
    d->numDragItems = 0;
    d->updateTimer = new QTimer(this, "iconview update timer");
    d->cachedW = d->cachedH = 0;
    d->maxItemWidth = 100;
    d->maxItemTextLength = 255;
    d->inputTimer = new QTimer(this, "iconview input timer");
    d->currInputString = QString::null;
    d->dirty = false;
    d->rearrangeEnabled = true;
    d->itemTextPos = Bottom;
    d->reorderItemsWhenInsert = true;
#ifndef QT_NO_CURSOR
    d->oldCursor = ArrowCursor;
#endif
    d->resortItemsWhenInsert = false;
    d->sortDirection = true;
    d->wordWrapIconText = true;
    d->cachedContentsX = d->cachedContentsY = -1;
    d->clearing = false;
    d->fullRedrawTimer = new QTimer(this, "iconview full redraw timer");
    d->itemTextBrush = NoBrush;
    d->drawAllBack = true;
    d->fm = new QFontMetrics(font());
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();
    d->firstContainer = d->lastContainer = 0;
    d->containerUpdateLocked = false;
    d->firstSizeHint = false;
    d->selectAnchor = 0;
    d->renamingItem = 0;
    d->drawActiveSelection = true;
    d->drawDragShapes = false;

    connect(d->adjustTimer, SIGNAL(timeout()),
             this, SLOT(adjustItems()));
    connect(d->updateTimer, SIGNAL(timeout()),
             this, SLOT(slotUpdate()));
    connect(d->fullRedrawTimer, SIGNAL(timeout()),
             this, SLOT(updateContents()));
    connect(this, SIGNAL(contentsMoving(int,int)),
             this, SLOT(movedContents(int,int)));

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    viewport()->setBackgroundRole(QPalette::Base);
    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(QWidget::WheelFocus);

#ifndef QT_NO_TOOLTIP
    d->toolTip = new QIconViewToolTip(viewport(), this);
#endif
    d->showTips = true;
}

/*!
    \reimp
*/
void QIconView::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        *d->fm = QFontMetrics(font());
        d->minLeftBearing = d->fm->minLeftBearing();
        d->minRightBearing = d->fm->minRightBearing();

        QIconViewItem *item = d->firstItem;
        for (; item; item = item->next) {
            item->wordWrapDirty = true;
            item->calcRect();
        }

#if !defined(Q_WS_X11)
        delete qiv_selection;
        qiv_selection = 0;
#endif
    } else if(ev->type() == QEvent::ActivationChange) {
        if (!isActiveWindow() && d->scrollTimer)
            d->scrollTimer->stop();
        if(isVisible() && !palette().isEqual(QPalette::Active, QPalette::Inactive))
            repaintSelectedItems();
    }
    QScrollView::changeEvent(ev);
}

/*!
    \reimp
*/

void QIconView::setFont(const QFont & f)
{
    QScrollView::setFont(f);
    *d->fm = QFontMetrics(font());
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    QIconViewItem *item = d->firstItem;
    for (; item; item = item->next) {
        item->wordWrapDirty = true;
        item->calcRect();
    }
}

/*!
    \reimp
*/

void QIconView::setPalette(const QPalette & p)
{
    QScrollView::setPalette(p);
    *d->fm = QFontMetrics(font());
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    QIconViewItem *item = d->firstItem;
    for (; item; item = item->next) {
        item->wordWrapDirty = true;
        item->calcRect();
    }
}

/*!
    Destroys the icon view and deletes all items.
*/

QIconView::~QIconView()
{
    QIconViewItem *tmp, *item = d->firstItem;
    d->clearing = true;
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while (c) {
        tmpc = c->n;
        delete c;
        c = tmpc;
    }
    while (item) {
        tmp = item->next;
        delete item;
        item = tmp;
    }
    delete d->fm;
    d->fm = 0;
#ifndef QT_NO_TOOLTIP
    delete d->toolTip;
    d->toolTip = 0;
#endif
    delete d;
}

/*!
    Inserts the icon view item \a item after \a after. If \a after is
    0, \a item is appended after the last item.

    \e{You should never need to call this function.} Instead create
    QIconViewItem's and associate them with your icon view like this:

    \code
        (void) new QIconViewItem(myIconview, "The text of the item", aPixmap);
    \endcode
*/

void QIconView::insertItem(QIconViewItem *item, QIconViewItem *after)
{
    if (!item)
        return;

    if (d->firstItem == item || item->prev || item->next)
        return;

    if (!item->view)
        item->view = this;

    if (!d->firstItem) {
        d->firstItem = d->lastItem = item;
        item->prev = 0;
        item->next = 0;
    } else {
        if (!after || after == d->lastItem) {
            d->lastItem->next = item;
            item->prev = d->lastItem;
            item->next = 0;
            d->lastItem = item;
        } else {
            QIconViewItem *i = d->firstItem;
            while (i != after)
                i = i->next;

            if (i) {
                QIconViewItem *next = i->next;
                item->next = next;
                item->prev = i;
                i->next = item;
                next->prev = item;
            }
        }
    }

    if (isVisible()) {
        if (d->reorderItemsWhenInsert) {
            if (d->updateTimer->isActive())
                d->updateTimer->stop();
            d->fullRedrawTimer->stop();
            // #### uncomment this ASA insertInGrid uses cached values and is efficient
            //insertInGrid(item);

            d->cachedW = qMax(d->cachedW, item->x() + item->width());
            d->cachedH= qMax(d->cachedH, item->y() + item->height());

            d->updateTimer->start(0, true);
        } else {
            insertInGrid(item);

            viewport()->update(item->x() - contentsX(),
                               item->y() - contentsY(),
                               item->width(), item->height());
        }
    } else if (!autoArrange()) {
        item->dirty = false;
    }

    d->count++;
    d->dirty = true;
}

/*!
    This slot is used for a slightly-delayed update.

    The icon view is not redrawn immediately after inserting a new item
    but after a very small delay using a QTimer. This means that when
    many items are inserted in a loop the icon view is probably redrawn
    only once at the end of the loop. This makes the insertions both
    flicker-free and faster.
*/

void QIconView::slotUpdate()
{
    d->updateTimer->stop();
    d->fullRedrawTimer->stop();

    if (!d->firstItem || !d->lastItem)
        return;

    // #### remove that ASA insertInGrid uses cached values and is efficient
    if (d->resortItemsWhenInsert)
        sort(d->sortDirection);
    else {
        int y = d->spacing;
        QIconViewItem *item = d->firstItem;
        int w = 0, h = 0;
        while (item) {
            bool changed;
            QIconViewItem *next = makeRowLayout(item, y, changed);
            if (!next || !next->next)
                break;

            if(!QApplication::reverseLayout())
                item = next;
            w = qMax(w, item->x() + item->width());
            h = qMax(h, item->y() + item->height());
            item = next;
            if (d->arrangement == LeftToRight)
                h = qMax(h, y);

            item = item->next;
        }

        if (d->lastItem && d->arrangement == TopToBottom) {
            item = d->lastItem;
            int x = item->x();
            while (item && item->x() >= x) {
                w = qMax(w, item->x() + item->width());
                h = qMax(h, item->y() + item->height());
                item = item->prev;
            }
        }

        w = qMax(qMax(d->cachedW, w), d->lastItem->x() + d->lastItem->width());
        h = qMax(qMax(d->cachedH, h), d->lastItem->y() + d->lastItem->height());

        if (d->arrangement == TopToBottom)
            w += d->spacing;
        else
            h += d->spacing;
        viewport()->setUpdatesEnabled(false);
        resizeContents(w, h);
        viewport()->setUpdatesEnabled(true);
        viewport()->repaint();
    }

    int cx = d->cachedContentsX == -1 ? contentsX() : d->cachedContentsX;
    int cy = d->cachedContentsY == -1 ? contentsY() : d->cachedContentsY;

    if (cx != contentsX() || cy != contentsY())
        setContentsPos(cx, cy);

    d->cachedContentsX = d->cachedContentsY = -1;
    d->cachedW = d->cachedH = 0;
}

/*!
    Takes the icon view item \a item out of the icon view and causes
    an update of the screen display. The item is not deleted. You
    should normally not need to call this function because
    QIconViewItem::~QIconViewItem() calls it. The normal way to delete
    an item is to delete it.
*/

void QIconView::takeItem(QIconViewItem *item)
{
    if (!item)
        return;

    if (item->d->container1)
        item->d->container1->items.removeAll(item);
    if (item->d->container2)
        item->d->container2->items.removeAll(item);
    item->d->container2 = 0;
    item->d->container1 = 0;

    bool block = signalsBlocked();
    blockSignals(d->clearing);

    QRect r = item->rect();

    if (d->currentItem == item) {
        if (item->prev) {
            d->currentItem = item->prev;
            emit currentChanged(d->currentItem);
            repaintItem(d->currentItem);
        } else if (item->next) {
            d->currentItem = item->next;
            emit currentChanged(d->currentItem);
            repaintItem(d->currentItem);
        } else {
            d->currentItem = 0;
            emit currentChanged(d->currentItem);
        }
    }
    if (item->isSelected()) {
        item->selected = false;
        emit selectionChanged();
    }

    if (item == d->firstItem) {
        d->firstItem = d->firstItem->next;
        if (d->firstItem)
            d->firstItem->prev = 0;
    } else if (item == d->lastItem) {
        d->lastItem = d->lastItem->prev;
        if (d->lastItem)
            d->lastItem->next = 0;
    } else {
        QIconViewItem *i = item;
        if (i) {
            if (i->prev)
                i->prev->next = i->next;
            if (i->next)
                i->next->prev = i->prev;
        }
    }

    if (d->selectAnchor == item)
        d->selectAnchor = d->currentItem;

    if (!d->clearing)
        repaintContents(r.x(), r.y(), r.width(), r.height());

    item->view = 0;
    item->prev = 0;
    item->next = 0;
    d->count--;

    blockSignals(block);
}

/*!
    Returns the index of \a item, or -1 if \a item doesn't exist in
    this icon view.
*/

int QIconView::index(const QIconViewItem *item) const
{
    if (!item)
        return -1;

    if (item == d->firstItem)
        return 0;
    else if (item == d->lastItem)
        return d->count - 1;
    else {
        QIconViewItem *i = d->firstItem;
        int j = 0;
        while (i && i != item) {
            i = i->next;
            ++j;
        }

        return i ? j : -1;
    }
}

/*!
    Returns a pointer to the first item of the icon view, or 0 if
    there are no items in the icon view.

    \sa lastItem() currentItem()
*/

QIconViewItem *QIconView::firstItem() const
{
    return d->firstItem;
}

/*!
    Returns a pointer to the last item of the icon view, or 0 if there
    are no items in the icon view.

    \sa firstItem() currentItem()
*/

QIconViewItem *QIconView::lastItem() const
{
    return d->lastItem;
}

/*!
    Returns a pointer to the current item of the icon view, or 0 if no
    item is current.

    \sa setCurrentItem() firstItem() lastItem()
*/

QIconViewItem *QIconView::currentItem() const
{
    return d->currentItem;
}

/*!
    Makes \a item the new current item of the icon view.
*/

void QIconView::setCurrentItem(QIconViewItem *item)
{
    if (!item || item == d->currentItem)
        return;

    setMicroFocusHint(item->x(), item->y(), item->width(), item->height(), false);

    QIconViewItem *old = d->currentItem;
    d->currentItem = item;
    emit currentChanged(d->currentItem);
    if (d->selectionMode == Single) {
        bool changed = false;
        if (old && old->selected) {
            old->selected = false;
            changed = true;
        }
        if (item && !item->selected && item->isSelectable() && d->selectionMode != NoSelection) {
            item->selected = true;
            changed = true;
            emit selectionChanged(item);
        }
        if (changed)
            emit selectionChanged();
    }

    if (old)
        repaintItem(old);
    repaintItem(d->currentItem);
}

/*!
    Selects or unselects \a item depending on \a s, and may also
    unselect other items, depending on QIconView::selectionMode() and
    \a cb.

    If \a s is false, \a item is unselected.

    If \a s is true and QIconView::selectionMode() is \c Single, \a
    item is selected, and the item which was selected is unselected.

    If \a s is true and QIconView::selectionMode() is \c Extended, \a
    item is selected. If \a cb is true, the selection state of the
    icon view's other items is left unchanged. If \a cb is false (the
    default) all other items are unselected.

    If \a s is true and QIconView::selectionMode() is \c Multi \a item
    is selected.

    Note that \a cb is used only if QIconView::selectionMode() is \c
    Extended. \a cb defaults to false.

    All items whose selection status is changed repaint themselves.
*/

void QIconView::setSelected(QIconViewItem *item, bool s, bool cb)
{
    if (!item)
        return;
    item->setSelected(s, cb);
}

/*!
    \property QIconView::count
    \brief the number of items in the icon view
*/

uint QIconView::count() const
{
    return d->count;
}

/*!
    Performs autoscrolling when selecting multiple icons with the
    rubber band.
*/

void QIconView::doAutoScroll()
{
    QRect oldRubber = QRect(*d->rubber);

    QPoint vp = viewport()->mapFromGlobal(QCursor::pos());
    QPoint pos = viewportToContents(vp);

    if (pos == d->rubber->bottomRight())
        return;

    d->rubber->setRight(pos.x());
    d->rubber->setBottom(pos.y());

    int minx = contentsWidth(), miny = contentsHeight();
    int maxx = 0, maxy = 0;
    bool changed = false;
    bool block = signalsBlocked();

    QRect rr;
    QRegion region(0, 0, visibleWidth(), visibleHeight());

    blockSignals(true);
    viewport()->setUpdatesEnabled(false);
    bool alreadyIntersected = false;
    QRect nr = d->rubber->normalize();
    QRect rubberUnion = nr.unite(oldRubber.normalize());
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    for (; c; c = c->n) {
        if (c->rect.intersects(rubberUnion)) {
            alreadyIntersected = true;
            for (int i = 0; i < c->items.size(); ++i) {
                QIconViewItem *item = c->items.at(i);
                if (d->selectedItems.contains(item))
                    continue;
                if (!item->intersects(nr)) {
                    if (item->isSelected()) {
                        item->setSelected(false);
                        changed = true;
                        rr = rr.unite(item->rect());
                    }
                } else if (item->intersects(nr)) {
                    if (!item->isSelected() && item->isSelectable()) {
                        item->setSelected(true, true);
                        changed = true;
                        rr = rr.unite(item->rect());
                    } else {
                        region = region.subtract(QRect(contentsToViewport(item->pos()),
                                                         item->size()));
                    }

                    minx = qMin(minx, item->x() - 1);
                    miny = qMin(miny, item->y() - 1);
                    maxx = qMax(maxx, item->x() + item->width() + 1);
                    maxy = qMax(maxy, item->y() + item->height() + 1);
                }
            }
        } else {
            if (alreadyIntersected)
                break;
        }
    }
    viewport()->setUpdatesEnabled(true);
    blockSignals(block);

    QRect r = *d->rubber;
    *d->rubber = oldRubber;

    QPainter p;
    p.begin(viewport());
    p.setRasterOp(NotROP);
    p.setPen(QPen(color0, 1));
    p.setBrush(NoBrush);
    drawRubber(&p);
    d->dragging = false;
    p.end();

    *d->rubber = r;

    if (changed) {
        d->drawAllBack = false;
        d->clipRegion = region;
        repaintContents(rr);
        d->drawAllBack = true;
    }

    ensureVisible(pos.x(), pos.y());

    p.begin(viewport());
    p.setRasterOp(NotROP);
    p.setPen(QPen(color0, 1));
    p.setBrush(NoBrush);
    drawRubber(&p);
    d->dragging = true;

    p.end();

    if (changed) {
        emit selectionChanged();
        if (d->selectionMode == Single)
            emit selectionChanged(d->currentItem);
    }

    if (!QRect(50, 50, viewport()->width()-100, viewport()->height()-100).contains(vp) &&
         !d->scrollTimer) {
        d->scrollTimer = new QTimer(this);

        connect(d->scrollTimer, SIGNAL(timeout()),
                 this, SLOT(doAutoScroll()));
        d->scrollTimer->start(100, false);
    } else if (QRect(50, 50, viewport()->width()-100, viewport()->height()-100).contains(vp) &&
                d->scrollTimer) {
        disconnect(d->scrollTimer, SIGNAL(timeout()),
                    this, SLOT(doAutoScroll()));
        d->scrollTimer->stop();
        delete d->scrollTimer;
        d->scrollTimer = 0;
    }

}

/*!
    \reimp
*/

void QIconView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    if (d->dragging && d->rubber)
        drawRubber(p);

    QRect r = QRect(cx, cy, cw, ch);

    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QRegion remaining(QRect(cx, cy, cw, ch));
    bool alreadyIntersected = false;
    while (c) {
        if (c->rect.intersects(r)) {
            p->save();
            p->resetXForm();
            QRect r2 = c->rect;
            r2 = r2.intersect(r);
            QRect r3(contentsToViewport(QPoint(r2.x(), r2.y())), QSize(r2.width(), r2.height()));
            if (d->drawAllBack) {
                p->setClipRect(r3);
            } else {
                QRegion reg = d->clipRegion.intersect(r3);
                p->setClipRegion(reg);
            }
            drawBackground(p, r3);
            remaining = remaining.subtract(r3);
            p->restore();

            QPalette pal = palette();
            d->drawActiveSelection = hasFocus() || d->inMenuMode
                || !style().styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, this);
            if (!d->drawActiveSelection)
                pal.setCurrentColorGroup(QPalette::Inactive);

            // clip items to the container rect by default... this
            // prevents icons with alpha channels from being painted
            // twice when they are in 2 containers
            //
            // NOTE: the item could override this cliprect in it's
            // paintItem() implementation, which makes this useless
            p->setClipRect(QRect(contentsToViewport(r2.topLeft()), r2.size()));
            for (int i = 0; i < c->items.size(); ++i) {
                QIconViewItem *item = c->items.at(i);
                if (item->rect().intersects(r) && !item->dirty) {
                    p->save();
                    p->setFont(font());
                    item->paintItem(p, pal);
                    p->restore();
                }
            }
            alreadyIntersected = true;
        } else {
            if (alreadyIntersected)
                break;
        }
        c = c->n;
    }

    if (!remaining.isEmpty()) {
        p->save();
        p->resetXForm();
        if (d->drawAllBack) {
            p->setClipRegion(remaining);
        } else {
            remaining = d->clipRegion.intersect(remaining);
            p->setClipRegion(remaining);
        }
        drawBackground(p, remaining.boundingRect());
        p->restore();
    }

    if ((hasFocus() || viewport()->hasFocus()) && d->currentItem &&
         d->currentItem->rect().intersects(r)) {
        d->currentItem->paintFocus(p, palette());
    }

    if (d->dragging && d->rubber)
        drawRubber(p);
}

/*!
    \overload

    Arranges all the items in the grid given by gridX() and gridY().

    Even if sorting() is enabled, the items are not sorted by this
    function. If you want to sort or rearrange the items, use
    iconview->sort(iconview->sortDirection()).

    If \a update is true (the default), the viewport is repainted as
    well.

    \sa QIconView::setGridX(), QIconView::setGridY(), QIconView::sort()
*/

void QIconView::arrangeItemsInGrid(bool update)
{
    if (!d->firstItem || !d->lastItem)
        return;

    d->containerUpdateLocked = true;

    int w = 0, h = 0, y = d->spacing;

    QIconViewItem *item = d->firstItem;
    bool changedLayout = false;
    while (item) {
        bool changed;
        QIconViewItem *next = makeRowLayout(item, y, changed);
        changedLayout = changed || changedLayout;
        if(!QApplication::reverseLayout())
            item = next;
        w = qMax(w, item->x() + item->width());
        h = qMax(h, item->y() + item->height());
        item = next;
        if (d->arrangement == LeftToRight)
            h = qMax(h, y);

        if (!item || !item->next)
            break;

        item = item->next;
    }

    if (d->lastItem && d->arrangement == TopToBottom) {
        item = d->lastItem;
        int x = item->x();
        while (item && item->x() >= x) {
            w = qMax(w, item->x() + item->width());
            h = qMax(h, item->y() + item->height());
            item = item->prev;
        }
    }
    d->containerUpdateLocked = false;

    w = qMax(qMax(d->cachedW, w), d->lastItem->x() + d->lastItem->width());
    h = qMax(qMax(d->cachedH, h), d->lastItem->y() + d->lastItem->height());

    if (d->arrangement == TopToBottom)
        w += d->spacing;
    else
        h += d->spacing;

    bool ue = isUpdatesEnabled();
    viewport()->setUpdatesEnabled(false);
    int vw = visibleWidth();
    int vh = visibleHeight();
    resizeContents(w, h);
    bool doAgain = false;
    if (d->arrangement == LeftToRight)
        doAgain = visibleWidth() != vw;
    if (d->arrangement == TopToBottom)
        doAgain = visibleHeight() != vh;
    if (doAgain) // in the case that the visibleExtend changed because of the resizeContents (scrollbar show/hide), redo layout again
        arrangeItemsInGrid(false);
    viewport()->setUpdatesEnabled(ue);
    d->dirty = !isVisible();
    rebuildContainers();
    if (update && (!optimize_layout || changedLayout))
        repaintContents(contentsX(), contentsY(), viewport()->width(), viewport()->height());
}

/*!
    This variant uses \a grid instead of (gridX(), gridY()). If \a
    grid is invalid (see QSize::isValid()), arrangeItemsInGrid()
    calculates a valid grid itself and uses that.

    If \a update is true (the default) the viewport is repainted.
*/

void QIconView::arrangeItemsInGrid(const QSize &grid, bool update)
{
    d->containerUpdateLocked = true;
    QSize grid_(grid);
    if (!grid_.isValid()) {
        int w = 0, h = 0;
        QIconViewItem *item = d->firstItem;
        for (; item; item = item->next) {
            w = qMax(w, item->width());
            h = qMax(h, item->height());
        }

        grid_ = QSize(qMax(d->rastX + d->spacing, w),
                       qMax(d->rastY + d->spacing, h));
    }

    int w = 0, h = 0;
    QIconViewItem *item = d->firstItem;
    for (; item; item = item->next) {
        int nx = item->x() / grid_.width();
        int ny = item->y() / grid_.height();
        item->move(nx * grid_.width(),
                    ny * grid_.height());
        w = qMax(w, item->x() + item->width());
        h = qMax(h, item->y() + item->height());
        item->dirty = false;
    }
    d->containerUpdateLocked = false;

    resizeContents(w, h);
    rebuildContainers();
    if (update)
        repaintContents(contentsX(), contentsY(), viewport()->width(), viewport()->height());
}

/*!
    \reimp
*/

void QIconView::setContentsPos(int x, int y)
{
    if (d->updateTimer->isActive()) {
        d->cachedContentsX = x;
        d->cachedContentsY = y;
    } else {
        d->cachedContentsY = d->cachedContentsX = -1;
        QScrollView::setContentsPos(x, y);
    }
}

/*!
    \reimp
*/

void QIconView::showEvent(QShowEvent *)
{
    if (d->dirty) {
        resizeContents(qMax(contentsWidth(), viewport()->width()),
                        qMax(contentsHeight(), viewport()->height()));
        if (d->resortItemsWhenInsert)
            sort(d->sortDirection);
        if (autoArrange())
            arrangeItemsInGrid(false);
    }
    QScrollView::show();
}

/*!
    \property QIconView::selectionMode
    \brief the selection mode of the icon view

    This can be \c Single (the default), \c Extended, \c Multi or \c
    NoSelection.
*/

void QIconView::setSelectionMode(SelectionMode m)
{
    d->selectionMode = m;
}

QIconView::SelectionMode QIconView::selectionMode() const
{
    return d->selectionMode;
}

/*!
    Returns a pointer to the item that contains point \a pos, which is
    given in contents coordinates, or 0 if no item contains point \a
    pos.
*/

QIconViewItem *QIconView::findItem(const QPoint &pos) const
{
    if (!d->firstItem)
        return 0;

    QIconViewPrivate::ItemContainer *c = d->lastContainer;
    for (; c; c = c->p) {
        if (c->rect.contains(pos))
            for (int i = c->items.size()-1; i >= 0; --i)
                if (c->items.at(i)->contains(pos))
                    return c->items.at(i);
    }

    return 0;
}

/*!
    \overload

    Returns a pointer to the first item whose text begins with \a
    text, or 0 if no such item could be found. Use the \a compare flag
    to control the comparison behaviour. (See \l
    {Qt::StringComparisonFlags}.)
*/

QIconViewItem *QIconView::findItem(const QString &text, StringComparison compare) const
{
    if (!d->firstItem)
        return 0;

    if (compare == CaseSensitive || compare == 0)
        compare |= ExactMatch;

    QString itmtxt;
    QString comtxt = text;
    if (! (compare & CaseSensitive))
        comtxt = text.toLower();

    QIconViewItem *item;
    if (d->currentItem)
        item = d->currentItem;
    else
        item = d->firstItem;

    QIconViewItem *beginsWithItem = 0;
    QIconViewItem *endsWithItem = 0;
    QIconViewItem *containsItem = 0;

    if (item) {
        for (; item; item = item->next) {
            if (! (compare & CaseSensitive))
                itmtxt = item->text().toLower();
            else
                itmtxt = item->text();

            if (compare & ExactMatch && itmtxt == comtxt)
                return item;
            if (compare & BeginsWith && !beginsWithItem && itmtxt.startsWith(comtxt))
                beginsWithItem = containsItem = item;
            if (compare & EndsWith && !endsWithItem && itmtxt.endsWith(comtxt))
                endsWithItem = containsItem = item;
            if (compare & Contains && !containsItem && itmtxt.contains(comtxt))
                containsItem = item;
        }

        if (d->currentItem && d->firstItem) {
            item = d->firstItem;
            for (; item && item != d->currentItem; item = item->next) {
                if (! (compare & CaseSensitive))
                    itmtxt = item->text().toLower();
                else
                    itmtxt = item->text();

                if (compare & ExactMatch && itmtxt == comtxt)
                    return item;
                if (compare & BeginsWith && !beginsWithItem && itmtxt.startsWith(comtxt))
                    beginsWithItem = containsItem = item;
                if (compare & EndsWith && !endsWithItem && itmtxt.endsWith(comtxt))
                    endsWithItem = containsItem = item;
                if (compare & Contains && !containsItem && itmtxt.contains(comtxt))
                    containsItem = item;
            }
        }
    }

    // Obey the priorities
    if (beginsWithItem)
        return beginsWithItem;
    else if (endsWithItem)
        return endsWithItem;
    else if (containsItem)
        return containsItem;
    return 0;
}

/*!
    Unselects all the items.
*/

void QIconView::clearSelection()
{
    selectAll(false);
}

/*!
    In Multi and Extended modes, this function sets all items to be
    selected if \a select is true, and to be unselected if \a select
    is false.

    In Single and NoSelection modes, this function only changes the
    selection status of currentItem().
*/

void QIconView::selectAll(bool select)
{
    if (d->selectionMode == NoSelection)
        return;

    if (d->selectionMode == Single) {
        if (d->currentItem)
            d->currentItem->setSelected(select);
        return;
    }

    bool b = signalsBlocked();
    blockSignals(true);
    QIconViewItem *item = d->firstItem;
    QIconViewItem *i = d->currentItem;
    bool changed = false;
    bool ue = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled(false);
    QRect rr;
    for (; item; item = item->next) {
        if (select != item->isSelected()) {
            item->setSelected(select, true);
            rr = rr.unite(item->rect());
            changed = true;
        }
    }
    viewport()->setUpdatesEnabled(ue);
    // we call updateContents not repaintContents because of possible previous updateContents
    QScrollView::updateContents(rr);
    QApplication::sendPostedEvents(viewport(), QEvent::Paint);
    if (i)
        setCurrentItem(i);
    blockSignals(b);
    if (changed) {
        emit selectionChanged();
    }
}

/*!
    Inverts the selection. Works only in Multi and Extended selection
    mode.
*/

void QIconView::invertSelection()
{
    if (d->selectionMode == Single ||
         d->selectionMode == NoSelection)
        return;

    bool b = signalsBlocked();
    blockSignals(true);
    QIconViewItem *item = d->firstItem;
    for (; item; item = item->next)
        item->setSelected(!item->isSelected(), true);
    blockSignals(b);
    emit selectionChanged();
}

/*!
    Repaints the \a item.
*/

void QIconView::repaintItem(QIconViewItem *item)
{
    if (!item || item->dirty)
        return;

    if (QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()).
         intersects(QRect(item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2)))
        repaintContents(item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2);
}

/*!
    Repaints the selected items.
*/
void QIconView::repaintSelectedItems()
{
    if (selectionMode() == NoSelection)
        return;

    if (selectionMode() == Single) {
        if (!currentItem() || !currentItem()->isSelected())
            return;
        QRect itemRect = currentItem()->rect(); //rect in contents coordinates
        itemRect.moveBy(-contentsX(), -contentsY());
        viewport()->update(itemRect);
    } else {
        // check if any selected items are visible
        QIconViewItem *item = firstItem();
        const QRect vr = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());

        while (item) {
            if (item->isSelected() && item->rect().intersects(vr))
                repaintItem(item);
            item = item->nextItem();
        }
    }
}

/*!
    Makes sure that \a item is entirely visible. If necessary,
    ensureItemVisible() scrolls the icon view.

    \sa ensureVisible()
*/

void QIconView::ensureItemVisible(QIconViewItem *item)
{
    if (!item)
        return;

    if (d->updateTimer && d->updateTimer->isActive() ||
         d->fullRedrawTimer && d->fullRedrawTimer->isActive())
        slotUpdate();

    int w = item->width();
    int h = item->height();
    ensureVisible(item->x() + w / 2, item->y() + h / 2,
                   w / 2 + 1, h / 2 + 1);
}

/*!
    Finds the first item whose bounding rectangle overlaps \a r and
    returns a pointer to that item. \a r is given in content
    coordinates. Returns 0 if no item overlaps \a r.

    If you want to find all items that touch \a r, you will need to
    use this function and nextItem() in a loop ending at
    findLastVisibleItem() and test QIconViewItem::rect() for each of
    these items.

    \sa findLastVisibleItem() QIconViewItem::rect()
*/

QIconViewItem* QIconView::findFirstVisibleItem(const QRect &r) const
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QIconViewItem *i = 0;
    bool alreadyIntersected = false;
    for (; c; c = c->n) {
        if (c->rect.intersects(r)) {
            alreadyIntersected = true;
            for (int j = 0; j < c->items.size(); ++j) {
                QIconViewItem *item = c->items.at(j);
                if (r.intersects(item->rect())) {
                    if (!i) {
                        i = item;
                    } else {
                        QRect r2 = item->rect();
                        QRect r3 = i->rect();
                        if (r2.y() < r3.y())
                            i = item;
                        else if (r2.y() == r3.y() &&
                                  r2.x() < r3.x())
                            i = item;
                    }
                }
            }
        } else {
            if (alreadyIntersected)
                break;
        }
    }

    return i;
}

/*!
    Finds the last item whose bounding rectangle overlaps \a r and
    returns a pointer to that item. \a r is given in content
    coordinates. Returns 0 if no item overlaps \a r.

    \sa findFirstVisibleItem()
*/

QIconViewItem* QIconView::findLastVisibleItem(const QRect &r) const
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QIconViewItem *i = 0;
    bool alreadyIntersected = false;
    for (; c; c = c->n) {
        if (c->rect.intersects(r)) {
            alreadyIntersected = true;
            for (int j = 0; j < c->items.size(); ++j) {
                QIconViewItem *item = c->items.at(j);
                if (r.intersects(item->rect())) {
                    if (!i) {
                        i = item;
                    } else {
                        QRect r2 = item->rect();
                        QRect r3 = i->rect();
                        if (r2.y() > r3.y())
                            i = item;
                        else if (r2.y() == r3.y() &&
                                  r2.x() > r3.x())
                            i = item;
                    }
                }
            }
        } else {
            if (alreadyIntersected)
                break;
        }
    }

    return i;
}

/*!
    Clears the icon view. All items are deleted.
*/

void QIconView::clear()
{
    setContentsPos(0, 0);
    d->clearing = true;
    bool block = signalsBlocked();
    blockSignals(true);
    clearSelection();
    blockSignals(block);
    setContentsPos(0, 0);
    d->currentItem = 0;

    if (!d->firstItem) {
        d->clearing = false;
        return;
    }

    QIconViewItem *item = d->firstItem, *tmp;
    d->firstItem = 0;
    while (item) {
        tmp = item->next;
        delete item;
        item = tmp;
    }
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while (c) {
        tmpc = c->n;
        delete c;
        c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    d->count = 0;
    d->lastItem = 0;
    setCurrentItem(0);
    d->highlightedItem = 0;
    d->tmpCurrentItem = 0;
    d->drawDragShapes = false;

    resizeContents(0, 0);
    // maybe we don´t need this update, so delay it
    d->fullRedrawTimer->start(0, true);

    d->cleared = true;
    d->clearing = false;
}

/*!
    \property QIconView::gridX
    \brief the horizontal grid  of the icon view

    If the value is -1, (the default), QIconView computes suitable
    column widths based on the icon view's contents.

    Note that setting a grid width overrides setMaxItemWidth().
*/

void QIconView::setGridX(int rx)
{
    d->rastX = rx >= 0 ? rx : -1;
}

/*!
    \property QIconView::gridY
    \brief the vertical grid  of the icon view

    If the value is -1, (the default), QIconView computes suitable
    column heights based on the icon view's contents.
*/

void QIconView::setGridY(int ry)
{
    d->rastY = ry >= 0 ? ry : -1;
}

int QIconView::gridX() const
{
    return d->rastX;
}

int QIconView::gridY() const
{
    return d->rastY;
}

/*!
    \property QIconView::spacing
    \brief the space in pixels between icon view items

    The default is 5 pixels.

    Negative values for spacing are illegal.
*/

void QIconView::setSpacing(int sp)
{
    d->spacing = sp;
}

int QIconView::spacing() const
{
    return d->spacing;
}

/*!
    \property QIconView::itemTextPos
    \brief the position where the text of each item is drawn.

    Valid values are \c Bottom or \c Right. The default is \c Bottom.
*/

void QIconView::setItemTextPos(ItemTextPos pos)
{
    if (pos == d->itemTextPos || (pos != Bottom && pos != Right))
        return;

    d->itemTextPos = pos;

    QIconViewItem *item = d->firstItem;
    for (; item; item = item->next) {
        item->wordWrapDirty = true;
        item->calcRect();
    }

    arrangeItemsInGrid(true);
}

QIconView::ItemTextPos QIconView::itemTextPos() const
{
    return d->itemTextPos;
}

/*!
    \property QIconView::itemTextBackground
    \brief the brush to use when drawing the background of an item's text.

    By default this brush is set to NoBrush, meaning that only the
    normal icon view background is used.
*/

void QIconView::setItemTextBackground(const QBrush &brush)
{
    d->itemTextBrush = brush;
}

QBrush QIconView::itemTextBackground() const
{
    return d->itemTextBrush;
}

/*!
    \property QIconView::arrangement
    \brief the arrangement mode of the icon view

    This can be \c LeftToRight or \c TopToBottom. The default is \c
    LeftToRight.
*/

void QIconView::setArrangement(Arrangement am)
{
    if (d->arrangement == am)
        return;

    d->arrangement = am;

    viewport()->setUpdatesEnabled(false);
    resizeContents(viewport()->width(), viewport()->height());
    viewport()->setUpdatesEnabled(true);
    arrangeItemsInGrid(true);
}

QIconView::Arrangement QIconView::arrangement() const
{
    return d->arrangement;
}

/*!
    \property QIconView::resizeMode
    \brief the resize mode of the icon view

    This can be \c Fixed or \c Adjust. The default is \c Fixed.
    See \l ResizeMode.
*/

void QIconView::setResizeMode(ResizeMode rm)
{
    if (d->resizeMode == rm)
        return;

    d->resizeMode = rm;
}

QIconView::ResizeMode QIconView::resizeMode() const
{
    return d->resizeMode;
}

/*!
    \property QIconView::maxItemWidth
    \brief the maximum width that an item may have.

    The default is 100 pixels.

    Note that if the gridX() value is set QIconView will ignore
    this property.
*/

void QIconView::setMaxItemWidth(int w)
{
    d->maxItemWidth = w;
}

/*!
    \property QIconView::maxItemTextLength
    \brief the maximum length (in characters) that an item's text may have.

    The default is 255 characters.
*/

void QIconView::setMaxItemTextLength(int w)
{
    d->maxItemTextLength = w;
}

int QIconView::maxItemWidth() const
{
    if (d->rastX != -1)
        return d->rastX - 2;
    else
        return d->maxItemWidth;
}

int QIconView::maxItemTextLength() const
{
    return d->maxItemTextLength;
}

/*!
    \property QIconView::itemsMovable
    \brief whether the user is allowed to move items around in the icon view

    The default is true.
*/

void QIconView::setItemsMovable(bool b)
{
    d->rearrangeEnabled = b;
}

bool QIconView::itemsMovable() const
{
    return d->rearrangeEnabled;
}

/*!
    \property QIconView::autoArrange
    \brief whether the icon view rearranges its items when a new item is inserted.

    The default is true.

    Note that if the icon view is not visible at the time of
    insertion, QIconView defers all position-related work until it is
    shown and then calls arrangeItemsInGrid().
*/

void QIconView::setAutoArrange(bool b)
{
    d->reorderItemsWhenInsert = b;
}

bool QIconView::autoArrange() const
{
    return d->reorderItemsWhenInsert;
}

/*!
    If \a sort is true, this function sets the icon view to sort items
    when a new item is inserted. If \a sort is false, the icon view
    will not be sorted.

    Note that autoArrange() must be true for sorting to take place.

    If \a ascending is true (the default), items are sorted in
    ascending order. If \a ascending is false, items are sorted in
    descending order.

    QIconViewItem::compare() is used to compare pairs of items. The
    sorting is based on the items' keys; these default to the items'
    text unless specifically set to something else.

    \sa QIconView::setAutoArrange(), QIconView::autoArrange(),
    sortDirection(), sort(), QIconViewItem::setKey()
*/

void QIconView::setSorting(bool sort, bool ascending)
{
    d->resortItemsWhenInsert = sort;
    d->sortDirection = ascending;
}

/*!
    \property QIconView::sorting
    \brief whether the icon view sorts on insertion

    The default is false, i.e. no sorting on insertion.

    To set the sorting, use setSorting().
*/

bool QIconView::sorting() const
{
    return d->resortItemsWhenInsert;
}

/*!
    \property QIconView::sortDirection
    \brief whether the sort direction for inserting new items is ascending;

    The default is true (i.e. ascending). This sort direction is only
    meaningful if both sorting() and autoArrange() are true.

    To set the sort direction, use setSorting()
*/

bool QIconView::sortDirection() const
{
    return d->sortDirection;
}

/*!
    \property QIconView::wordWrapIconText
    \brief whether the item text will be word-wrapped if it is too long

    The default is true.

    If this property is false, icon text that is too long is
    truncated, and an ellipsis (...) appended to indicate that
    truncation has occurred. The full text can still be seen by the
    user if they hover the mouse because the full text is shown in a
    tooltip; see setShowToolTips().
*/

void QIconView::setWordWrapIconText(bool b)
{
    if (d->wordWrapIconText == (uint)b)
        return;

    d->wordWrapIconText = b;
    for (QIconViewItem *item = d->firstItem; item; item = item->next) {
        item->wordWrapDirty = true;
        item->calcRect();
    }
    arrangeItemsInGrid(true);
}

bool QIconView::wordWrapIconText() const
{
    return d->wordWrapIconText;
}

/*!
    \property QIconView::showToolTips
    \brief whether the icon view will display a tool tip with the complete text for any truncated item text

    The default is true. Note that this has no effect if
    setWordWrapIconText() is true, as it is by default.
*/

void QIconView::setShowToolTips(bool b)
{
    d->showTips = b;
}

bool QIconView::showToolTips() const
{
    return d->showTips;
}

/*!
    \reimp
*/
void QIconView::contentsMousePressEvent(QMouseEvent *e)
{
    contentsMousePressEventEx(e);
}

void QIconView::contentsMousePressEventEx(QMouseEvent *e)
{
    if (d->rubber) {
        QPainter p;
        p.begin(viewport());
        p.setRasterOp(NotROP);
        p.setPen(QPen(color0, 1));
        p.setBrush(NoBrush);

        drawRubber(&p);
        d->dragging = false;
        p.end();
        delete d->rubber;
        d->rubber = 0;

        if (d->scrollTimer) {
            disconnect(d->scrollTimer, SIGNAL(timeout()), this, SLOT(doAutoScroll()));
            d->scrollTimer->stop();
            delete d->scrollTimer;
            d->scrollTimer = 0;
        }
    }

    d->dragStartPos = e->pos();
    QIconViewItem *item = findItem(e->pos());
    d->pressedItem = item;

    if (item)
        d->selectAnchor = item;

#ifndef QT_NO_TEXTEDIT
    if (d->renamingItem)
        d->renamingItem->renameItem();
#endif

    if (!d->currentItem && !item && d->firstItem) {
        d->currentItem = d->firstItem;
        repaintItem(d->firstItem);
    }

    if (item && item->dragEnabled())
        d->startDragItem = item;
    else
        d->startDragItem = 0;

    if (e->button() == LeftButton && !(e->state() & ShiftButton) &&
         !(e->state() & ControlButton) && item && item->isSelected() &&
         item->textRect(false).contains(e->pos())) {

        if (!item->renameEnabled()) {
            d->mousePressed = true;
#ifndef QT_NO_TEXTEDIT
        } else {
            ensureItemVisible(item);
            setCurrentItem(item);
            item->rename();
            goto emit_signals;
#endif
        }
    }

    d->pressedSelected = item && item->isSelected();

    if (item && item->isSelectable()) {
        if (d->selectionMode == Single)
            item->setSelected(true, e->state() & ControlButton);
        else if (d->selectionMode == Multi)
            item->setSelected(!item->isSelected(), e->state() & ControlButton);
        else if (d->selectionMode == Extended) {
            if (e->state() & ShiftButton) {
                d->pressedSelected = false;
                bool block = signalsBlocked();
                blockSignals(true);
                viewport()->setUpdatesEnabled(false);
                QRect r;
                bool select = true;
                if (d->currentItem)
                    r = QRect(qMin(d->currentItem->x(), item->x()),
                               qMin(d->currentItem->y(), item->y()),
                               0, 0);
                else
                    r = QRect(0, 0, 0, 0);
                if (d->currentItem) {
                    if (d->currentItem->x() < item->x())
                        r.setWidth(item->x() - d->currentItem->x() + item->width());
                    else
                        r.setWidth(d->currentItem->x() - item->x() + d->currentItem->width());
                    if (d->currentItem->y() < item->y())
                        r.setHeight(item->y() - d->currentItem->y() + item->height());
                    else
                        r.setHeight(d->currentItem->y() - item->y() + d->currentItem->height());
                    r = r.normalize();
                    QIconViewPrivate::ItemContainer *c = d->firstContainer;
                    bool alreadyIntersected = false;
                    QRect redraw;
                    for (; c; c = c->n) {
                        if (c->rect.intersects(r)) {
                            alreadyIntersected = true;
                            for (int i = 0; i < c->items.size(); ++i) {
                                QIconViewItem *item = c->items.at(i);
                                if (r.intersects(item->rect())) {
                                    redraw = redraw.unite(item->rect());
                                    item->setSelected(select, true);
                                }
                            }
                        } else {
                            if (alreadyIntersected)
                                break;
                        }
                    }
                    redraw = redraw.unite(item->rect());
                    viewport()->setUpdatesEnabled(true);
                    repaintContents(redraw);
                }
                blockSignals(block);
                viewport()->setUpdatesEnabled(true);
                item->setSelected(select, true);
                emit selectionChanged();
            } else if (e->state() & ControlButton) {
                d->pressedSelected = false;
                item->setSelected(!item->isSelected(), e->state() & ControlButton);
            } else {
                item->setSelected(true, e->state() & ControlButton);
            }
        }
    } else if ((d->selectionMode != Single || e->button() == RightButton)
                && !(e->state() & ControlButton))
        selectAll(false);

    setCurrentItem(item);

    if (e->button() == LeftButton) {
        if (!item && (d->selectionMode == Multi ||
                                  d->selectionMode == Extended)) {
            d->tmpCurrentItem = d->currentItem;
            d->currentItem = 0;
            repaintItem(d->tmpCurrentItem);
            if (d->rubber)
                delete d->rubber;
            d->rubber = 0;
            d->rubber = new QRect(e->x(), e->y(), 0, 0);
            d->selectedItems.clear();
            if ((e->state() & ControlButton) == ControlButton) {
                for (QIconViewItem *i = firstItem(); i; i = i->nextItem())
                    if (i->isSelected())
                        d->selectedItems.insert(i, i);
            }
        }

        d->mousePressed = true;
    }

 emit_signals:
    if (!d->rubber) {
        emit mouseButtonPressed(e->button(), item, e->globalPos());
        emit pressed(item);
        emit pressed(item, e->globalPos());

        if (e->button() == RightButton)
            emit rightButtonPressed(item, e->globalPos());
    }
}

/*!
    \reimp
*/

void QIconView::contentsContextMenuEvent(QContextMenuEvent *e)
{
    if (!receivers(SIGNAL(contextMenuRequested(QIconViewItem*,QPoint)))) {
        e->ignore();
        return;
    }
    if (e->reason() == QContextMenuEvent::Keyboard) {
        QIconViewItem *item = currentItem();
        QRect r = item ? item->rect() : QRect(0, 0, visibleWidth(), visibleHeight());
        emit contextMenuRequested(item, viewport()->mapToGlobal(contentsToViewport(r.center())));
    } else {
        d->mousePressed = false;
        QIconViewItem *item = findItem(e->pos());
        emit contextMenuRequested(item, e->globalPos());
    }
}

/*!
    \reimp
*/

void QIconView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    QIconViewItem *item = findItem(e->pos());
    d->selectedItems.clear();

    bool emitClicked = true;
    d->mousePressed = false;
    d->startDragItem = 0;

    if (d->rubber) {
        QPainter p;
        p.begin(viewport());
        p.setRasterOp(NotROP);
        p.setPen(QPen(color0, 1));
        p.setBrush(NoBrush);

        drawRubber(&p);
        d->dragging = false;
        p.end();

        if ((d->rubber->topLeft() - d->rubber->bottomRight()).manhattanLength() >
             QApplication::startDragDistance())
            emitClicked = false;
        delete d->rubber;
        d->rubber = 0;
        d->currentItem = d->tmpCurrentItem;
        d->tmpCurrentItem = 0;
        if (d->currentItem)
            repaintItem(d->currentItem);
    }

    if (d->scrollTimer) {
        disconnect(d->scrollTimer, SIGNAL(timeout()), this, SLOT(doAutoScroll()));
        d->scrollTimer->stop();
        delete d->scrollTimer;
        d->scrollTimer = 0;
    }

    if (d->selectionMode == Extended &&
         d->currentItem == d->pressedItem &&
         d->pressedSelected && d->currentItem) {
        bool block = signalsBlocked();
        blockSignals(true);
        clearSelection();
        blockSignals(block);
        if (d->currentItem->isSelectable()) {
            d->currentItem->selected = true;
            repaintItem(d->currentItem);
        }
        emit selectionChanged();
    }
    d->pressedItem = 0;

    if (emitClicked) {
        emit mouseButtonClicked(e->button(), item, e->globalPos());
        emit clicked(item);
        emit clicked(item, e->globalPos());
        if (e->button() == RightButton)
            emit rightButtonClicked(item, e->globalPos());
    }
}

/*!
    \reimp
*/

void QIconView::contentsMouseMoveEvent(QMouseEvent *e)
{
    QIconViewItem *item = findItem(e->pos());
    if (d->highlightedItem != item) {
        if (item)
            emit onItem(item);
        else
            emit onViewport();
        d->highlightedItem = item;
    }

    if (d->mousePressed && e->state() == NoButton)
        d->mousePressed = false;

    if (d->startDragItem)
        item = d->startDragItem;

    if (d->mousePressed && d->startDragItem && item && item == d->currentItem &&
        (item->isSelected() || d->selectionMode == NoSelection) && item->dragEnabled()) {
        if ((d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
            d->mousePressed = false;
            d->cleared = false;
#ifndef QT_NO_DRAGANDDROP
            startDrag();
#endif
            if (d->tmpCurrentItem)
                repaintItem(d->tmpCurrentItem);
        }
    } else if (d->mousePressed && !d->currentItem && d->rubber) {
        doAutoScroll();
    }
}

/*!
    \reimp
*/

void QIconView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    QIconViewItem *item = findItem(e->pos());
    if (item) {
        selectAll(false);
        item->setSelected(true, true);
        emit doubleClicked(item);
    }
}

/*!
    \reimp
*/

#ifndef QT_NO_DRAGANDDROP
void QIconView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    d->dragging = true;
    d->drawDragShapes = true;
    d->tmpCurrentItem = 0;
    initDragEnter(e);
    d->oldDragPos = e->pos();
    d->oldDragAcceptAction = false;
    drawDragShapes(e->pos());
    d->dropped = false;
}

/*!
    \reimp
*/

void QIconView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (e->pos() == d->oldDragPos) {
        if (d->oldDragAcceptAction)
            e->acceptAction();
        else
            e->ignore();
        return;
    }

    drawDragShapes(d->oldDragPos);
    d->dragging = false;

    QIconViewItem *old = d->tmpCurrentItem;
    d->tmpCurrentItem = 0;

    QIconViewItem *item = findItem(e->pos());

    if (item) {
        if (old &&
             old->rect().contains(d->oldDragPos) &&
             !old->rect().contains(e->pos())) {
            old->dragLeft();
            repaintItem(old);
        }
        if (!item->rect().contains(d->oldDragPos))
            item->dragEntered();
        if (item->acceptDrop(e)) {
            d->oldDragAcceptAction = true;
            e->acceptAction();
        } else {
            d->oldDragAcceptAction = false;
            e->ignore();
        }

        d->tmpCurrentItem = item;
        QPainter p;
        p.begin(viewport());
        p.translate(-contentsX(), -contentsY());
        item->paintFocus(&p, palette());
        p.end();
    } else {
        e->acceptAction();
        d->oldDragAcceptAction = true;
        if (old) {
            old->dragLeft();
            repaintItem(old);
        }
    }

    d->oldDragPos = e->pos();
    drawDragShapes(e->pos());
    d->dragging = true;
}

/*!
    \reimp
*/

void QIconView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
    if (!d->dropped)
        drawDragShapes(d->oldDragPos);
    d->dragging = false;

    if (d->tmpCurrentItem) {
        repaintItem(d->tmpCurrentItem);
        d->tmpCurrentItem->dragLeft();
    }

    d->tmpCurrentItem = 0;
    d->isIconDrag = false;
    d->iconDragData.clear();
}

/*!
    \reimp
*/

void QIconView::contentsDropEvent(QDropEvent *e)
{
    d->dropped = true;
    d->dragging = false;
    drawDragShapes(d->oldDragPos);

    if (d->tmpCurrentItem)
        repaintItem(d->tmpCurrentItem);

    QIconViewItem *i = findItem(e->pos());

    if (!i && e->source() == viewport() && d->currentItem && !d->cleared) {
        if (!d->rearrangeEnabled)
            return;
        QRect r = d->currentItem->rect();

        d->currentItem->move(e->pos() - d->dragStart);

        int w = d->currentItem->x() + d->currentItem->width() + 1;
        int h = d->currentItem->y() + d->currentItem->height() + 1;

        repaintItem(d->currentItem);
        repaintContents(r.x(), r.y(), r.width(), r.height());

        int dx = d->currentItem->x() - r.x();
        int dy = d->currentItem->y() - r.y();

        QIconViewItem *item = d->firstItem;
        QRect rr;
        for (; item; item = item->next) {
            if (item->isSelected() && item != d->currentItem) {
                rr = rr.unite(item->rect());
                item->moveBy(dx, dy);
                rr = rr.unite(item->rect());
            }
            w = qMax(w, item->x() + item->width() + 1);
            h = qMax(h, item->y() + item->height() + 1);
        }
        repaintContents(rr);
        bool fullRepaint = false;
        if (w > contentsWidth() ||
             h > contentsHeight())
            fullRepaint = true;

        int oldw = contentsWidth();
        int oldh = contentsHeight();

        resizeContents(w, h);


        if (fullRepaint) {
            repaintContents(oldw, 0, contentsWidth() - oldw, contentsHeight());
            repaintContents(0, oldh, contentsWidth(), contentsHeight() - oldh);
        }
        e->acceptAction();
    } else if (!i && (e->source() != viewport() || d->cleared)) {
        QLinkedList<QIconDragItem> lst;
        if (QIconDrag::canDecode(e)) {
            QLinkedList<QIconDragDataItem> l;
            QIconDragPrivate::decode(e, l);
            QLinkedList<QIconDragDataItem>::Iterator it = l.begin();
            for (; it != l.end(); ++it)
                lst << (*it).data;
        }
        emit dropped(e, lst);
    } else if (i) {
        QLinkedList<QIconDragItem> lst;
        if (QIconDrag::canDecode(e)) {
            QLinkedList<QIconDragDataItem> l;
            QIconDragPrivate::decode(e, l);
            QLinkedList<QIconDragDataItem>::Iterator it = l.begin();
            for (; it != l.end(); ++it)
                lst << (*it).data;
        }
        i->dropped(e, lst);
    }
    d->isIconDrag = false;
}
#endif

/*!
    \reimp
*/

void QIconView::resizeEvent(QResizeEvent* e)
{
    QScrollView::resizeEvent(e);
    if (d->resizeMode == Adjust) {
        optimize_layout = true;
        adjustItems();
        optimize_layout = false;
#if 0 // no need for timer delay anymore
        d->oldSize = e->oldSize();
        if (d->adjustTimer->isActive())
            d->adjustTimer->stop();
        d->adjustTimer->start(0, true);
#endif
    }
}

/*!
    Adjusts the positions of the items to the geometry of the icon
    view.
*/

void QIconView::adjustItems()
{
    d->adjustTimer->stop();
    if (d->resizeMode == Adjust)
            arrangeItemsInGrid(true);
}

/*!
    \reimp
*/

void QIconView::keyPressEvent(QKeyEvent *e)
{
    if (!d->firstItem)
        return;

    if (!d->currentItem) {
        setCurrentItem(d->firstItem);
        if (d->selectionMode == Single)
            d->currentItem->setSelected(true, true);
        return;
    }

    bool selectCurrent = true;

    switch (e->key()) {
    case Key_Escape:
        e->ignore();
        break;
#ifndef QT_NO_TEXTEDIT
    case Key_F2: {
        if (d->currentItem->renameEnabled()) {
            d->currentItem->renameItem();
            d->currentItem->rename();
            return;
        }
    } break;
#endif
    case Key_Home: {
        d->currInputString = QString::null;
        if (!d->firstItem)
            break;

        selectCurrent = false;

        QIconViewItem *item = 0;
        QIconViewPrivate::ItemContainer *c = d->firstContainer;
        while (!item && c) {
            QList<QIconViewItem*> &list = c->items;
            for (int j = 0; j < list.size(); ++j) {
                QIconViewItem *i = list.at(j);
                if (!item) {
                    item = i;
                } else {
                    if (d->arrangement == LeftToRight) {
                        // we use pixmap so the items textlength are ignored
                        // find topmost, leftmost item
                        if (i->pixmapRect(false).y() < item->pixmapRect(false).y() ||
                             (i->pixmapRect(false).y() == item->pixmapRect(false).y() &&
                               i->pixmapRect(false).x() < item->pixmapRect(false).x()))
                            item = i;
                    } else {
                        // find leftmost, topmost item
                        if (i->pixmapRect(false).x() < item->pixmapRect(false).x() ||
                             (i->pixmapRect(false).x() == item->pixmapRect(false).x() &&
                               i->pixmapRect(false).y() < item->pixmapRect(false).y()))
                            item = i;
                    }
                }
            }
            c = c->n;
        }

        if (item) {
            QIconViewItem *old = d->currentItem;
            setCurrentItem(item);
            ensureItemVisible(item);
            handleItemChange(old, e->state() & ShiftButton,
                              e->state() & ControlButton, true);
        }
    } break;
    case Key_End: {
        d->currInputString = QString::null;
        if (!d->lastItem)
            break;

        selectCurrent = false;

        QIconViewItem *item = 0;
        QIconViewPrivate::ItemContainer *c = d->lastContainer;
        while (!item && c) {
            QList<QIconViewItem*> &list = c->items;
            for (int j = 0; j < list.size(); ++j) {
                QIconViewItem *i = list.at(j);
                if (!item) {
                    item = i;
                } else {
                    if (d->arrangement == LeftToRight) {
                        // find bottommost, rightmost item
                        if (i->pixmapRect(false).bottom() > item->pixmapRect(false).bottom() ||
                             (i->pixmapRect(false).bottom() == item->pixmapRect(false).bottom() &&
                               i->pixmapRect(false).right() > item->pixmapRect(false).right()))
                            item = i;
                    } else {
                        // find rightmost, bottommost item
                        if (i->pixmapRect(false).right() > item->pixmapRect(false).right() ||
                             (i->pixmapRect(false).right() == item->pixmapRect(false).right() &&
                               i->pixmapRect(false).bottom() > item->pixmapRect(false).bottom()))
                            item = i;
                    }
                }
            }
            c = c->p;
        }

        if (item) {
            QIconViewItem *old = d->currentItem;
            setCurrentItem(item);
            ensureItemVisible(item);
            handleItemChange(old, e->state() & ShiftButton,
                              e->state() & ControlButton, true);
        }
    } break;
    case Key_Right: {
        d->currInputString = QString::null;
        QIconViewItem *item;
        selectCurrent = false;
        Direction dir = DirRight;

        QRect r(0, d->currentItem->y(), contentsWidth(), d->currentItem->height());
        item = findItem(dir, d->currentItem->rect().center(), r);

        // search the row below from the right
        while (!item && r.y() < contentsHeight()) {
            r.moveBy(0, d->currentItem->height());
            item = findItem(dir, QPoint(0, r.center().y()), r);
        }

        if (item) {
            QIconViewItem *old = d->currentItem;
            setCurrentItem(item);
            ensureItemVisible(item);
            handleItemChange(old, e->state() & ShiftButton, e->state() & ControlButton);
        }
    } break;
    case Key_Left: {
        d->currInputString = QString::null;
        QIconViewItem *item;
        selectCurrent = false;
        Direction dir = DirLeft;

        QRect r(0, d->currentItem->y(), contentsWidth(), d->currentItem->height());
        item = findItem(dir, d->currentItem->rect().center(), r);

        // search the row above from the left
        while (!item && r.y() >= 0) {
            r.moveBy(0, - d->currentItem->height());
            item = findItem(dir, QPoint(contentsWidth(), r.center().y()), r);
        }

        if (item) {
            QIconViewItem *old = d->currentItem;
            setCurrentItem(item);
            ensureItemVisible(item);
            handleItemChange(old, e->state() & ShiftButton, e->state() & ControlButton);
        }
    } break;
    case Key_Space: {
        d->currInputString = QString::null;
        if (d->selectionMode == Single)
            break;

        d->currentItem->setSelected(!d->currentItem->isSelected(), true);
    } break;
    case Key_Enter: case Key_Return:
        d->currInputString = QString::null;
        emit returnPressed(d->currentItem);
        break;
    case Key_Down: {
        d->currInputString = QString::null;
        QIconViewItem *item;
        selectCurrent = false;
        Direction dir = DirDown;

        QRect r(d->currentItem->x(), 0, d->currentItem->width(), contentsHeight());
        item = findItem(dir, d->currentItem->rect().center(), r);

        // finding the closest item below and to the right
        while (!item && r.x() < contentsWidth()) {
            r.moveBy(r.width() , 0);
            item = findItem(dir, QPoint(r.center().x(), 0), r);
        }


        QIconViewItem *i = d->currentItem;
        setCurrentItem(item);
        item = i;
        handleItemChange(item, e->state() & ShiftButton, e->state() & ControlButton);
    } break;
    case Key_Up: {
        d->currInputString = QString::null;
        QIconViewItem *item;
        selectCurrent = false;
        Direction dir = DirUp;

        QRect r(d->currentItem->x(), 0, d->currentItem->width(), contentsHeight());
        item = findItem(dir, d->currentItem->rect().center(), r);

        // finding the closest item above and to the left
        while (!item && r.x() >= 0) {
            r.moveBy(- r.width(), 0);
            item = findItem(dir, QPoint(r.center().x(), contentsHeight()), r);
        }

        QIconViewItem *i = d->currentItem;
        setCurrentItem(item);
        item = i;
        handleItemChange(item, e->state() & ShiftButton, e->state() & ControlButton);
    } break;
    case Key_Next: {
        d->currInputString = QString::null;
        selectCurrent = false;
        QRect r;
        if (d->arrangement == LeftToRight)
            r = QRect(0, d->currentItem->y() + visibleHeight(), contentsWidth(), visibleHeight());
        else
            r = QRect(d->currentItem->x() + visibleWidth(), 0, visibleWidth(), contentsHeight());
        QIconViewItem *item = d->currentItem;
        QIconViewItem *ni = findFirstVisibleItem(r );
        if (!ni) {
            if (d->arrangement == LeftToRight)
                r = QRect(0, d->currentItem->y() + d->currentItem->height(), contentsWidth(), contentsHeight());
            else
                r = QRect(d->currentItem->x() + d->currentItem->width(), 0, contentsWidth(), contentsHeight());
            ni = findLastVisibleItem(r );
        }
        if (ni) {
            setCurrentItem(ni);
            handleItemChange(item, e->state() & ShiftButton, e->state() & ControlButton);
        }
    } break;
    case Key_Prior: {
        d->currInputString = QString::null;
        selectCurrent = false;
        QRect r;
        if (d->arrangement == LeftToRight)
            r = QRect(0, d->currentItem->y() - visibleHeight(), contentsWidth(), visibleHeight());
        else
            r = QRect(d->currentItem->x() - visibleWidth(), 0, visibleWidth(), contentsHeight());
        QIconViewItem *item = d->currentItem;
        QIconViewItem *ni = findFirstVisibleItem(r );
        if (!ni) {
            if (d->arrangement == LeftToRight)
                r = QRect(0, 0, contentsWidth(), d->currentItem->y());
            else
                r = QRect(0, 0, d->currentItem->x(), contentsHeight());
            ni = findFirstVisibleItem(r );
        }
        if (ni) {
            setCurrentItem(ni);
            handleItemChange(item, e->state() & ShiftButton, e->state() & ControlButton);
        }
    } break;
    default:
        if (!e->text().isEmpty() && e->text()[0].isPrint()) {
            selectCurrent = false;
            QIconViewItem *i = d->currentItem;
            if (!i)
                i = d->firstItem;
            if (!d->inputTimer->isActive()) {
                d->currInputString = e->text();
                i = i->next;
                if (!i)
                    i = d->firstItem;
                i = findItemByName(i);
            } else {
                d->inputTimer->stop();
                d->currInputString += e->text();
                i = findItemByName(i);
                if (!i) {
                    d->currInputString = e->text();
                    if (d->currentItem && d->currentItem->next)
                        i = d->currentItem->next;
                    else
                        i = d->firstItem;
                    i = findItemByName(i);
                }
            }
            if (i) {
                setCurrentItem(i);
                if (d->selectionMode == Extended) {
                    bool changed = false;
                    bool block = signalsBlocked();
                    blockSignals(true);
                    selectAll(false);
                    blockSignals(block);
                    if (!i->selected && i->isSelectable()) {
                        changed = true;
                        i->selected = true;
                        repaintItem(i);
                    }
                    if (changed)
                        emit selectionChanged();
                }
            }
            d->inputTimer->start(400, true);
        } else {
            selectCurrent = false;
            d->currInputString = QString::null;
            if (e->state() & ControlButton) {
                switch (e->key()) {
                case Key_A:
                    selectAll(true);
                    break;
                }
            }
            e->ignore();
            return;
        }
    }

    if (!(e->state() & ShiftButton) || !d->selectAnchor)
        d->selectAnchor = d->currentItem;

    if (d->currentItem && !d->currentItem->isSelected() &&
         d->selectionMode == Single && selectCurrent) {
        d->currentItem->setSelected(true);
    }

    ensureItemVisible(d->currentItem);
}

/*
  Finds the closest item in the Direction \a dir relative from the point \a relativeTo
  which intersects with the searchRect.

  The function choses the closest item with its center in the searchRect.
*/
QIconViewItem* QIconView::findItem(Direction dir,
                                    const QPoint &relativeTo,
                                    const QRect &searchRect) const
{
    QIconViewItem *centerMatch = 0;
    int centerMatchML = 0;

    // gets list of containers with potential items
    QList<QIconViewPrivate::ItemContainer * >* cList =
        d->findContainers(dir, relativeTo, searchRect);

    for (int i = 0; i < cList->size() && !centerMatch; ++i) {
        QList<QIconViewItem *> &list = (cList->at(i))->items;
        for (int j = 0; j < list.size(); ++j) {
            QIconViewItem *item = list.at(j);
            if (neighbourItem(dir, relativeTo, item) &&
                 searchRect.contains(item->rect().center()) && item != currentItem()) {
                int ml = (relativeTo - item->rect().center()).manhattanLength();
                if (centerMatch) {
                    if (ml < centerMatchML) {
                        centerMatch = item;
                        centerMatchML = ml;
                    }
                } else {
                    centerMatch = item;
                    centerMatchML = ml;
                }
            }
        }
    }
    return centerMatch;
}


/*
  Returns true if the items orientation compared to
  the point \a relativeTo is correct.
*/
bool QIconView::neighbourItem(Direction dir,
                               const QPoint &relativeTo,
                               const QIconViewItem *item) const
{
    switch (dir) {
    case DirUp:
        if (item->rect().center().y() < relativeTo.y())
            return true;
        break;
    case DirDown:
        if (item->rect().center().y() > relativeTo.y())
            return true;
        break;
    case DirLeft:
        if (item->rect().center().x() < relativeTo.x())
            return true;
        break;
    case DirRight:
        if (item->rect().center().x() > relativeTo.x())
            return true;
        break;
    default:
        // nothing
        break;
    }
    return false;
}

/*!
    \reimp
*/

void QIconView::focusInEvent(QFocusEvent*)
{
    d->mousePressed = false;
    d->inMenuMode = false;
    if (d->currentItem) {
        repaintItem(d->currentItem);
    } else if (d->firstItem && QFocusEvent::reason() != QFocusEvent::Mouse) {
        d->currentItem = d->firstItem;
        emit currentChanged(d->currentItem);
        repaintItem(d->currentItem);
    }

    if (style().styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, this))
        repaintSelectedItems();

    if (d->currentItem)
        setMicroFocusHint(d->currentItem->x(), d->currentItem->y(), d->currentItem->width(), d->currentItem->height(), false);
}

/*!
    \reimp
*/

void QIconView::focusOutEvent(QFocusEvent*)
{
    if (style().styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, this)) {
        d->inMenuMode =
            QFocusEvent::reason() == QFocusEvent::Popup ||
            (qApp->focusWidget() && qApp->focusWidget()->inherits("QMenuBar"));
        if (!d->inMenuMode)
            repaintSelectedItems();
    }
    if (d->currentItem)
        repaintItem(d->currentItem);
}

/*!
    Draws the rubber band using the painter \a p.
*/

void QIconView::drawRubber(QPainter *p)
{
    if (!p || !d->rubber)
        return;

    QPoint pnt(d->rubber->x(), d->rubber->y());
    pnt = contentsToViewport(pnt);

    style().drawPrimitive(QStyle::PE_RubberBand, p,
                          QRect(pnt.x(), pnt.y(), d->rubber->width(), d->rubber->height()),
                          palette(), QStyle::Style_Default, QStyleOption(palette().base()));
}

/*!
    Returns the QDragObject that should be used for drag-and-drop.
    This function is called by the icon view when starting a drag to
    get the dragobject that should be used for the drag. Subclasses
    may reimplement this.

    \sa QIconDrag
*/

#ifndef QT_NO_DRAGANDDROP
QDragObject *QIconView::dragObject()
{
    if (!d->currentItem)
        return 0;

    QPoint orig = d->dragStartPos;

    QIconDrag *drag = new QIconDrag(viewport());
    drag->setPixmap((d->currentItem->pixmap() ?
                     *d->currentItem->pixmap() : QPixmap()), // ### QPicture
                     QPoint(d->currentItem->pixmapRect().width() / 2,
                             d->currentItem->pixmapRect().height() / 2));

    if (d->selectionMode == NoSelection) {
        QIconViewItem *item = d->currentItem;
        drag->append(QIconDragItem(),
                      QRect(item->pixmapRect(false).x() - orig.x(),
                             item->pixmapRect(false).y() - orig.y(),
                             item->pixmapRect().width(), item->pixmapRect().height()),
                      QRect(item->textRect(false).x() - orig.x(),
                             item->textRect(false).y() - orig.y(),
                             item->textRect().width(), item->textRect().height()));
    } else {
        for (QIconViewItem *item = d->firstItem; item; item = item->next) {
            if (item->isSelected()) {
                drag->append(QIconDragItem(),
                              QRect(item->pixmapRect(false).x() - orig.x(),
                                     item->pixmapRect(false).y() - orig.y(),
                                     item->pixmapRect().width(), item->pixmapRect().height()),
                              QRect(item->textRect(false).x() - orig.x(),
                                     item->textRect(false).y() - orig.y(),
                                     item->textRect().width(), item->textRect().height()));
            }
        }
    }

    return drag;
}

/*!
    Starts a drag.
*/

void QIconView::startDrag()
{
    if (!d->startDragItem)
        return;

    QPoint orig = d->dragStartPos;
    d->dragStart = QPoint(orig.x() - d->startDragItem->x(),
                           orig.y() - d->startDragItem->y());
    d->startDragItem = 0;
    d->mousePressed = false;
    d->pressedItem = 0;
    d->pressedSelected = 0;

    QDragObject *drag = dragObject();
    if (!drag)
        return;

    if (drag->drag())
        if (drag->target() != viewport())
            emit moved();
}

#endif

/*!
    Inserts the QIconViewItem \a item in the icon view's grid. \e{You
    should never need to call this function.} Instead, insert
    QIconViewItems by creating them with a pointer to the QIconView
    that they are to be inserted into.
*/

void QIconView::insertInGrid(QIconViewItem *item)
{
    if (!item)
        return;

    if (d->reorderItemsWhenInsert) {
        // #### make this efficient - but it's not too dramatic
        int y = d->spacing;

        item->dirty = false;
        if (item == d->firstItem) {
            bool dummy;
            makeRowLayout(item, y, dummy);
            return;
        }

        QIconViewItem *begin = rowBegin(item);
        y = begin->y();
        while (begin) {
            bool dummy;
            begin = makeRowLayout(begin, y, dummy);

            if (!begin || !begin->next)
                break;

            begin = begin->next;
        }
        item->dirty = false;
    } else {
        QRegion r(QRect(0, 0, qMax(contentsWidth(), visibleWidth()),
                          qMax(contentsHeight(), visibleHeight())));

        int y = -1;
        for (QIconViewItem *i = d->firstItem; i; i = i->next) {
            r = r.subtract(i->rect());
            y = qMax(y, i->y() + i->height());
        }

        QVector<QRect> rects = r.rects();
        bool foundPlace = false;
        for (int j = 0; j < rects.size(); ++j) {
            const QRect rect = rects.at(j);
            if (rect.width() >= item->width() &&
                 rect.height() >= item->height()) {
                int sx = 0, sy = 0;
                if (rect.width() >= item->width() + d->spacing)
                    sx = d->spacing;
                if (rect.height() >= item->height() + d->spacing)
                    sy = d->spacing;
                item->move(rect.x() + sx, rect.y() + sy);
                foundPlace = true;
                break;
            }
        }

        if (!foundPlace)
            item->move(d->spacing, y + d->spacing);

        resizeContents(qMax(contentsWidth(), item->x() + item->width()),
                        qMax(contentsHeight(), item->y() + item->height()));
        item->dirty = false;
    }
}

/*!
    Emits a signal to indicate selection changes. \a i is the
    QIconViewItem that was selected or de-selected.

    \e{You should never need to call this function.}
*/

void QIconView::emitSelectionChanged(QIconViewItem *i)
{
    emit selectionChanged();
    if (d->selectionMode == Single)
        emit selectionChanged(i ? i : d->currentItem);
}

/*!
    \internal
*/

void QIconView::emitRenamed(QIconViewItem *item)
{
    if (!item)
        return;

    emit itemRenamed(item, item->text());
    emit itemRenamed(item);
}

/*!
    If a drag enters the icon view the shapes of the objects which the
    drag contains are drawn, usnig \a pos as origin.
*/

void QIconView::drawDragShapes(const QPoint &pos)
{
#ifndef QT_NO_DRAGANDDROP
    if (pos == QPoint(-1, -1))
        return;

    if (!d->drawDragShapes) {
        d->drawDragShapes = true;
        return;
    }

    QStyleOption opt(palette().base());

    QPainter p;
    p.begin(viewport());
    p.translate(-contentsX(), -contentsY());
    p.setRasterOp(NotROP);
    p.setPen(QPen(color0));

    if (d->isIconDrag) {
        QLinkedList<QIconDragDataItem>::Iterator it = d->iconDragData.begin();
        for (; it != d->iconDragData.end(); ++it) {
            QRect ir = (*it).item.pixmapRect();
            QRect tr = (*it).item.textRect();
            tr.moveBy(pos.x(), pos.y());
            ir.moveBy(pos.x(), pos.y());
            if (!ir.intersects(QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight())))
                continue;

            style().drawPrimitive(QStyle::PE_FocusRect, &p, ir, palette(),
                                  QStyle::Style_Default, opt);
            style().drawPrimitive(QStyle::PE_FocusRect, &p, tr, palette(),
                                  QStyle::Style_Default, opt);
        }
    } else if (d->numDragItems > 0) {
        for (int i = 0; i < d->numDragItems; ++i) {
            QRect r(pos.x() + i * 40, pos.y(), 35, 35);
            style().drawPrimitive(QStyle::PE_FocusRect, &p, r, palette(),
                                  QStyle::Style_Default, opt);
        }

    }
    p.end();
#endif
}

/*!
    When a drag enters the icon view, this function is called to
    initialize it. Initializing in this context means getting
    information about the drag, for example so that the icon view
    knows enough about the drag to be able to draw drag shapes for the
    drag data (e.g. shapes of icons which are dragged), etc.
*/

#ifndef QT_NO_DRAGANDDROP
void QIconView::initDragEnter(QDropEvent *e)
{
    if (QIconDrag::canDecode(e)) {
        QIconDragPrivate::decode(e, d->iconDragData);
        d->isIconDrag = true;
    } else if (QUriDrag::canDecode(e)) {
        QList<QByteArray> lst;
        QUriDrag::decode(e, lst);
        d->numDragItems = lst.count();
    } else {
        d->numDragItems = 0;
    }

}
#endif

/*!
    This function is called to draw the rectangle \a r of the
    background using the painter \a p.

    The default implementation fills \a r with the viewport's
    backgroundBrush(). Subclasses may reimplement this to draw custom
    backgrounds.

    \sa contentsX() contentsY() drawContents()
*/

void QIconView::drawBackground(QPainter *p, const QRect &r)
{
    p->fillRect(r, viewport()->backgroundBrush());
}

/*!
    \reimp
*/

bool QIconView::eventFilter(QObject * o, QEvent * e)
{
    if (o == viewport()) {
        switch(e->type()) {
        case QEvent::FocusIn:
            focusInEvent((QFocusEvent*)e);
            return true;
        case QEvent::FocusOut:
            focusOutEvent((QFocusEvent*)e);
            return true;
        case QEvent::Enter:
            enterEvent(e);
            return true;
        case QEvent::Paint:
            if (o == viewport()) {
                if (d->dragging) {
                    if (!d->rubber)
                        drawDragShapes(d->oldDragPos);
                }
                viewportPaintEvent((QPaintEvent*)e);
                if (d->dragging) {
                    if (!d->rubber)
                        drawDragShapes(d->oldDragPos);
                }
            }
            return true;
        default:
            // nothing
            break;
        }
    }

    return QScrollView::eventFilter(o, e);
}


/*!
    \reimp
*/

QSize QIconView::minimumSizeHint() const
{
    return QScrollView::minimumSizeHint();
}

/*!
  \internal
  Finds the next item after the start item beginning
  with \a text.
*/

QIconViewItem* QIconView::findItemByName(QIconViewItem *start)
{
    if (!start)
        return 0;
    QString match = d->currInputString.toLower();
    if (match.length() < 1)
        return start;
    QString curText;
    QIconViewItem *i = start;
    do {
        curText = i->text().toLower();
        if (curText.startsWith(match))
            return i;
        i = i->next;
        if (!i)
            i = d->firstItem;
    } while (i != start);
    return 0;
}

/*!
    Lays out a row of icons (if Arrangement == \c TopToBottom this is
    a column). Starts laying out with the item \a begin. \a y is the
    starting coordinate. Returns the last item of the row (column) and
    sets the new starting coordinate to \a y. The \a changed parameter
    is used internally.

    \warning This function may be made private in a future version of
    Qt. We do not recommend calling it.
*/

QIconViewItem *QIconView::makeRowLayout(QIconViewItem *begin, int &y, bool &changed)
{
    QIconViewItem *end = 0;

    bool reverse = QApplication::reverseLayout();
    changed = false;

    if (d->arrangement == LeftToRight) {

        if (d->rastX == -1) {
            // first calculate the row height
            int h = 0;
            int x = 0;
            int ih = 0;
            QIconViewItem *item = begin;
            for (;;) {
                x += d->spacing + item->width();
                if (x > visibleWidth() && item != begin) {
                    item = item->prev;
                    break;
                }
                h = qMax(h, item->height());
                ih = qMax(ih, item->pixmapRect().height());
                QIconViewItem *old = item;
                item = item->next;
                if (!item) {
                    item = old;
                    break;
                }
            }
            end = item;

            if (d->rastY != -1)
                h = qMax(h, d->rastY);

            // now move the items
            item = begin;
            for (;;) {
                item->dirty = false;
                int x;
                if (item == begin) {
                    if (reverse)
                        x = visibleWidth() - d->spacing - item->width();
                    else
                        x = d->spacing;
                } else {
                    if (reverse)
                        x = item->prev->x() - item->width() - d->spacing;
                    else
                        x = item->prev->x() + item->prev->width() + d->spacing;
                }
                changed = item->move(x, y + ih - item->pixmapRect().height()) || changed;
                if (y + h < item->y() + item->height())
                    h = qMax(h, ih + item->textRect().height());
                if (item == end)
                    break;
                item = item->next;
            }
            y += h + d->spacing;
        } else {
            // first calculate the row height
            int h = begin->height();
            int x = d->spacing;
            int ih = begin->pixmapRect().height();
            QIconViewItem *item = begin;
            int i = 0;
            int sp = 0;
            for (;;) {
                int r = calcGridNum(item->width(), d->rastX);
                if (item == begin) {
                    i += r;
                    sp += r;
                    x = d->spacing + d->rastX * r;
                } else {
                    sp += r;
                    i += r;
                    x = i * d->rastX + sp * d->spacing;
                }
                if (x > visibleWidth() && item != begin) {
                    item = item->prev;
                    break;
                }
                h = qMax(h, item->height());
                ih = qMax(ih, item->pixmapRect().height());
                QIconViewItem *old = item;
                item = item->next;
                if (!item) {
                    item = old;
                    break;
                }
            }
            end = item;

            if (d->rastY != -1)
                h = qMax(h, d->rastY);

            // now move the items
            item = begin;
            i = 0;
            sp = 0;
            for (;;) {
                item->dirty = false;
                int r = calcGridNum(item->width(), d->rastX);
                if (item == begin) {
                    if (d->itemTextPos == Bottom)
                        changed = item->move(d->spacing + (r * d->rastX - item->width()) / 2,
                                              y + ih - item->pixmapRect().height()) || changed;
                    else
                        changed = item->move(d->spacing, y + ih - item->pixmapRect().height()) || changed;
                    i += r;
                    sp += r;
                } else {
                    sp += r;
                    int x = i * d->rastX + sp * d->spacing;
                    if (d->itemTextPos == Bottom)
                        changed = item->move(x + (r * d->rastX - item->width()) / 2,
                                              y + ih - item->pixmapRect().height()) || changed;
                    else
                        changed = item->move(x, y + ih - item->pixmapRect().height()) || changed;
                    i += r;
                }
                if (y + h < item->y() + item->height())
                    h = qMax(h, ih + item->textRect().height());
                if (item == end)
                    break;
                item = item->next;
            }
            y += h + d->spacing;
        }


    } else { // -------------------------------- SOUTH ------------------------------

        int x = y;

        {
            int w = 0;
            int y = 0;
            QIconViewItem *item = begin;
            for (;;) {
                y += d->spacing + item->height();
                if (y > visibleHeight() && item != begin) {
                    item = item->prev;
                    break;
                }
                w = qMax(w, item->width());
                QIconViewItem *old = item;
                item = item->next;
                if (!item) {
                    item = old;
                    break;
                }
            }
            end = item;

            if (d->rastX != -1)
                w = qMax(w, d->rastX);

            // now move the items
            item = begin;
            for (;;) {
                item->dirty = false;
                if (d->itemTextPos == Bottom) {
                    if (item == begin)
                        changed = item->move(x + (w - item->width()) / 2, d->spacing)  || changed;
                    else
                        changed = item->move(x + (w - item->width()) / 2,
                                              item->prev->y() + item->prev->height() + d->spacing) || changed;
                } else {
                    if (item == begin)
                        changed = item->move(x, d->spacing) || changed;
                    else
                        changed = item->move(x, item->prev->y() + item->prev->height() + d->spacing) || changed;
                }
                if (item == end)
                    break;
                item = item->next;
            }
            x += w + d->spacing;
        }

        y = x;
    }

    return end;
}

/*!
  \internal
  Calculates how many cells an item of width \a w needs in a grid with of
  \a x and returns the result.
*/

int QIconView::calcGridNum(int w, int x) const
{
    float r = (float)w / (float)x;
    if ((w / x) * x != w)
        r += 1.0;
    return (int)r;
}

/*!
  \internal
  Returns the first item of the row which contains \a item.
*/

QIconViewItem *QIconView::rowBegin(QIconViewItem *) const
{
    // #### todo
    return d->firstItem;
}

/*!
    Sorts and rearranges all the items in the icon view. If \a
    ascending is true, the items are sorted in increasing order,
    otherwise they are sorted in decreasing order.

    QIconViewItem::compare() is used to compare pairs of items. The
    sorting is based on the items' keys; these default to the items'
    text unless specifically set to something else.

    Note that this function sets the sort order to \a ascending.

    \sa QIconViewItem::key(), QIconViewItem::setKey(),
    QIconViewItem::compare(), QIconView::setSorting(),
    QIconView::sortDirection()
*/

void QIconView::sort(bool ascending)
{
    if (count() == 0)
        return;

    d->sortDirection = ascending;
    QIconViewPrivate::SortableItem *items = new QIconViewPrivate::SortableItem[count()];

    QIconViewItem *item = d->firstItem;
    int i = 0;
    for (; item; item = item->next)
        items[i++].item = item;

    qsort(items, count(), sizeof(QIconViewPrivate::SortableItem), cmpIconViewItems);

    QIconViewItem *prev = 0;
    item = 0;
    if (ascending) {
        for (i = 0; i < (int)count(); ++i) {
            item = items[i].item;
            if (item) {
                item->prev = prev;
                if (item->prev)
                    item->prev->next = item;
                item->next = 0;
            }
            if (i == 0)
                d->firstItem = item;
            if (i == (int)count() - 1)
                d->lastItem = item;
            prev = item;
        }
    } else {
        for (i = (int)count() - 1; i >= 0 ; --i) {
            item = items[i].item;
            if (item) {
                item->prev = prev;
                if (item->prev)
                    item->prev->next = item;
                item->next = 0;
            }
            if (i == (int)count() - 1)
                d->firstItem = item;
            if (i == 0)
                d->lastItem = item;
            prev = item;
        }
    }

    delete [] items;

    arrangeItemsInGrid(true);
}

/*!
    \reimp
*/

QSize QIconView::sizeHint() const
{
    ensurePolished();

    if (!d->firstItem)
        return QScrollView::sizeHint();

    if (d->dirty && d->firstSizeHint) {
        ((QIconView*)this)->resizeContents(qMax(400, contentsWidth()),
                                              qMax(400, contentsHeight()));
        if (autoArrange())
            ((QIconView*)this)->arrangeItemsInGrid(false);
        d->firstSizeHint = false;
    }

    d->dirty = true;
    int extra = style().pixelMetric(QStyle::PM_ScrollBarExtent,
                                    verticalScrollBar()) + 2*frameWidth();
    QSize s(qMin(400, contentsWidth() + extra),
             qMin(400, contentsHeight() + extra));
    return s;
}

/*!
  \internal
*/

void QIconView::updateContents()
{
    viewport()->update();
}

/*!
    \reimp
*/

void QIconView::enterEvent(QEvent *e)
{
    QScrollView::enterEvent(e);
    emit onViewport();
}

/*!
  \internal
  This function is always called when the geometry of an item changes.
  This function moves the item into the correct area in the internal
  data structure.
*/

void QIconView::updateItemContainer(QIconViewItem *item)
{
    if (!item || d->containerUpdateLocked || (!isVisible() && autoArrange()))
        return;

    if (item->d->container1 && d->firstContainer) {
        //Special-case to check if we can use removeLast otherwise use removeAll (slower)
        if (item->d->container1->items.last() == item)
            item->d->container1->items.removeLast();
        else
            item->d->container1->items.removeAll(item);
    }
    item->d->container1 = 0;
    if (item->d->container2 && d->firstContainer) {
        //Special-case to check if we can use removeLast otherwise use removeAll (slower)
        if (item->d->container2->items.last() == item)
            item->d->container2->items.removeLast();
        else
            item->d->container2->items.removeAll(item);
    }
    item->d->container2 = 0;

    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    if (!c) {
        appendItemContainer();
        c = d->firstContainer;
    }

    const QRect irect = item->rect();
    bool contains = false;
    for (;;) {
        if (c->rect.intersects(irect)) {
            contains = c->rect.contains(irect);
            break;
        }

        c = c->n;
        if (!c) {
            appendItemContainer();
            c = d->lastContainer;
        }
    }

    if (!c) {
        qWarning("QIconViewItem::updateItemContainer(): No fitting container found!");
        return;
    }

    c->items.append(item);
    item->d->container1 = c;

    if (!contains) {
        c = c->n;
        if (!c) {
            appendItemContainer();
            c = d->lastContainer;
        }
        c->items.append(item);
        item->d->container2 = c;
    }
    if (contentsWidth() < irect.right() || contentsHeight() < irect.bottom())
        resizeContents(qMax(contentsWidth(), irect.right()), qMax(contentsHeight(), irect.bottom()));
}

/*!
  \internal
  Appends a new rect area to the internal data structure of the items.
*/

void QIconView::appendItemContainer()
{
    QSize s;
    // #### We have to find out which value is best here
    if (d->arrangement == LeftToRight)
        s = QSize(INT_MAX - 1, RECT_EXTENSION);
    else
        s = QSize(RECT_EXTENSION, INT_MAX - 1);

    if (!d->firstContainer) {
        d->firstContainer = new QIconViewPrivate::ItemContainer(0, 0, QRect(QPoint(0, 0), s));
        d->lastContainer = d->firstContainer;
    } else {
        if (d->arrangement == LeftToRight)
            d->lastContainer = new QIconViewPrivate::ItemContainer(
                d->lastContainer, 0, QRect(d->lastContainer->rect.bottomLeft(), s));
        else
            d->lastContainer = new QIconViewPrivate::ItemContainer(
                d->lastContainer, 0, QRect(d->lastContainer->rect.topRight(), s));
    }
}

/*!  \internal

  Rebuilds the whole internal data structure. This is done when it's
  likely that most/all items change their geometry (e.g. in
  arrangeItemsInGrid()), because calling this is then more efficient
  than calling updateItemContainer() for each item.
*/

void QIconView::rebuildContainers()
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while (c) {
        tmpc = c->n;
        delete c;
        c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    QIconViewItem *item = d->firstItem;
    appendItemContainer();
    c = d->lastContainer;
    while (item) {
        if (c->rect.contains(item->rect())) {
            item->d->container1 = c;
            item->d->container2 = 0;
            c->items.append(item);
            item = item->next;
        } else if (c->rect.intersects(item->rect())) {
            item->d->container1 = c;
            c->items.append(item);
            c = c->n;
            if (!c) {
                appendItemContainer();
                c = d->lastContainer;
            }
            c->items.append(item);
            item->d->container2 = c;
            item = item->next;
            c = c->p;
        } else {
            if (d->arrangement == LeftToRight) {
                if (item->y() < c->rect.y() && c->p) {
                    c = c->p;
                    continue;
                }
            } else {
                if (item->x() < c->rect.x() && c->p) {
                    c = c->p;
                    continue;
                }
            }

            c = c->n;
            if (!c) {
                appendItemContainer();
                c = d->lastContainer;
            }
        }
    }
}

/*!
  \internal
*/

void QIconView::movedContents(int, int)
{
    if (d->drawDragShapes) {
        drawDragShapes(d->oldDragPos);
        d->oldDragPos = QPoint(-1, -1);
    }
}

void QIconView::handleItemChange(QIconViewItem *old, bool shift,
                                  bool control, bool homeend)
{
    if (d->selectionMode == Single) {
        bool block = signalsBlocked();
        blockSignals(true);
        if (old)
            old->setSelected(false);
        blockSignals(block);
        d->currentItem->setSelected(true, true);
    } else if (d->selectionMode == Extended) {
         if (shift) {
            if (!d->selectAnchor) {
                if (old && !old->selected && old->isSelectable()) {
                    old->selected = true;
                    repaintItem(old);
                }
                d->currentItem->setSelected(true, true);
            } else {
                QIconViewItem *from = d->selectAnchor, *to = d->currentItem;
                if (!from || !to)
                    return;

                // checking if it's downwards and if we span rows
                bool downwards = false;
                bool spanning = false;
                if (d->arrangement == LeftToRight) {
                    if (from->rect().center().y() < to->rect().center().y())
                        downwards = true;
                } else {
                    if (from->rect().center().x() < to->rect().center().x())
                        downwards = true;
                }

                QRect fr = from->rect();
                QRect tr = to->rect();
                if (d->arrangement == LeftToRight) {
                    fr.moveTopLeft(QPoint(tr.x(), fr.y()));
                    if (!tr.intersects(fr))
                        spanning = true;
                } else {
                    fr.moveTopLeft(QPoint(fr.x(), tr.y()));
                    if (!tr.intersects(fr))
                        spanning = true;
                }


                // finding the rectangles
                QRect topRect, bottomRect, midRect;
                if (!spanning) {
                    midRect = from->rect().unite(to->rect());
                } else {
                    if (downwards) {
                        topRect = from->rect();
                        bottomRect = to->rect();
                    } else {
                        topRect = to->rect();
                        bottomRect = from->rect();
                    }
                    if (d->arrangement == LeftToRight) {
                        topRect.setRight(contentsWidth());
                        bottomRect.setLeft(0);
                        midRect.setRect(0, topRect.bottom(),
                                         contentsWidth(),
                                         bottomRect.top() - topRect.bottom());
                    } else {
                        topRect.setBottom(contentsHeight());
                        bottomRect.setTop(0);
                        midRect.setRect(topRect.right(),
                                         0,
                                         bottomRect.left() - topRect.right(),
                                         contentsHeight());
                    }
                }

                // finding contained items and selecting them
                QIconViewItem *item = 0;
                bool changed = false;
                bool midValid = midRect.isValid();
                bool topValid = topRect.isValid();
                bool bottomValid = bottomRect.isValid();
                QRect selectedRect, unselectedRect;
                for (item = d->firstItem; item; item = item->next) {
                    bool contained = false;
                    QPoint itemCenter = item->rect().center();
                    if (midValid && midRect.contains(itemCenter))
                        contained = true;
                    if (!contained && topValid && topRect.contains(itemCenter))
                        contained = true;
                    if (!contained && bottomValid && bottomRect.contains(itemCenter))
                        contained = true;

                    if (contained) {
                        if (!item->selected && item->isSelectable()) {
                            changed = true;
                            item->selected = true;
                            selectedRect = selectedRect.unite(item->rect());
                        }
                    } else if (item->selected && !control) {
                        item->selected = false;
                        unselectedRect = unselectedRect.unite(item->rect());
                        changed = true;
                    }
                }

                QRect viewRect(contentsX(), contentsY(),
                                visibleWidth(), visibleHeight());

                if (viewRect.intersects(selectedRect)) {
                    if (homeend)
                        QScrollView::updateContents(viewRect.intersect(selectedRect));
                    else
                        repaintContents(viewRect.intersect(selectedRect));
                }
                if (viewRect.intersects(unselectedRect)) {
                    if (homeend)
                        QScrollView::updateContents(viewRect.intersect(unselectedRect));
                    else
                        repaintContents(viewRect.intersect(unselectedRect));
                }

                if (changed)
                    emit selectionChanged();
            }
        } else if (!control) {
            blockSignals(true);
            selectAll(false);
            blockSignals(false);
            d->currentItem->setSelected(true, true);
        }
    } else {
        if (shift)
            d->currentItem->setSelected(!d->currentItem->isSelected(), true);
    }
}

QBitmap QIconView::mask(QPixmap *pix) const
{
    QBitmap m;
    if (d->maskCache.find(QString::number(pix->serialNumber()), m))
        return m;
    if (pix->mask())
       m = *pix->mask();
    else
       m = pix->createHeuristicMask();
    d->maskCache.insert(QString::number(pix->serialNumber()), m);
    return m;
}

/*!
    \reimp
    \internal

    (Implemented to get rid of a compiler warning.)
*/
void QIconView::drawContents(QPainter *)
{
}

/*!
    Returns true if an iconview item is being renamed; otherwise
    returns false.
*/

bool QIconView::isRenaming() const
{
#ifndef QT_NO_TEXTEDIT
    return d->renamingItem && d->renamingItem->renameBox;
#else
    return false;
#endif
}

#endif // QT_NO_ICONVIEW
