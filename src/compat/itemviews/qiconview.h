/****************************************************************************
**
** Definition of QIconView widget class.
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

#ifndef QICONVIEW_H
#define QICONVIEW_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstring.h"
#include "qrect.h"
#include "qpoint.h"
#include "qsize.h"
#include "qfont.h" // QString->QFont conversion
#include "qdragobject.h"
#include "qbitmap.h"
#include "qpicture.h"
#include "qlinkedlist.h"
#endif // QT_H

#ifndef QT_NO_ICONVIEW

class QIconView;
class QPainter;
class QMimeSource;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QKeyEvent;
class QFocusEvent;
class QShowEvent;
class QIconViewItem;
class QIconViewItemLineEdit;
class QStringList;
class QIconDragPrivate;

#ifndef QT_NO_DRAGANDDROP

class Q_COMPAT_EXPORT QIconDragItem
{
public:
    QIconDragItem();
    virtual ~QIconDragItem();
    virtual QByteArray data() const;
    virtual void setData(const QByteArray &d);
    bool operator== (const QIconDragItem&) const;

private:
    QByteArray ba;

};

class Q_COMPAT_EXPORT QIconDrag : public QDragObject
{
    Q_OBJECT
public:
    QIconDrag(QWidget * dragSource, const char* name = 0);
    virtual ~QIconDrag();

    void append(const QIconDragItem &item, const QRect &pr, const QRect &tr);

    virtual const char* format(int i) const;
    static bool canDecode(QMimeSource* e);
    virtual QByteArray encodedData(const char* mime) const;

private:
    QIconDragPrivate *d;
    QChar endMark;

    friend class QIconView;
    friend class QIconViewPrivate;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QIconDrag(const QIconDrag &);
    QIconDrag &operator=(const QIconDrag &);
#endif
};

#endif

class QIconViewToolTip;
class QIconViewItemPrivate;

class Q_COMPAT_EXPORT QIconViewItem : public Qt
{
    friend class QIconView;
    friend class QIconViewToolTip;
    friend class QIconViewItemLineEdit;

public:
    QIconViewItem(QIconView *parent);
    QIconViewItem(QIconView *parent, QIconViewItem *after);
    QIconViewItem(QIconView *parent, const QString &text);
    QIconViewItem(QIconView *parent, QIconViewItem *after, const QString &text);
    QIconViewItem(QIconView *parent, const QString &text, const QPixmap &icon);
    QIconViewItem(QIconView *parent, QIconViewItem *after, const QString &text, const QPixmap &icon);
#ifndef QT_NO_PICTURE
    QIconViewItem(QIconView *parent, const QString &text, const QPicture &picture);
    QIconViewItem(QIconView *parent, QIconViewItem *after, const QString &text, const QPicture &picture);
#endif
    virtual ~QIconViewItem();

    virtual void setRenameEnabled(bool allow);
    virtual void setDragEnabled(bool allow);
    virtual void setDropEnabled(bool allow);

    virtual QString text() const;
    virtual QPixmap *pixmap() const;
#ifndef QT_NO_PICTURE
    virtual QPicture *picture() const;
#endif
    virtual QString key() const;

    bool renameEnabled() const;
    bool dragEnabled() const;
    bool dropEnabled() const;

    QIconView *iconView() const;
    QIconViewItem *prevItem() const;
    QIconViewItem *nextItem() const;

    int index() const;

    virtual void setSelected(bool s, bool cb);
    virtual void setSelected(bool s);
    virtual void setSelectable(bool s);

    bool isSelected() const;
    bool isSelectable() const;

    virtual void repaint();

    virtual bool move(int x, int y);
    virtual void moveBy(int dx, int dy);
    virtual bool move(const QPoint &pnt);
    virtual void moveBy(const QPoint &pnt);

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QSize size() const;
    QPoint pos() const;
    QRect textRect(bool relative = true) const;
    QRect pixmapRect(bool relative = true) const;
    bool contains(const QPoint& pnt) const;
    bool intersects(const QRect& r) const;

    virtual bool acceptDrop(const QMimeSource *mime) const;

#ifndef QT_NO_TEXTEDIT
    void rename();
#endif

    virtual int compare(QIconViewItem *i) const;

    virtual void setText(const QString &text);
    virtual void setPixmap(const QPixmap &icon);
#ifndef QT_NO_PICTURE
    virtual void setPicture(const QPicture &icon);
#endif
    virtual void setText(const QString &text, bool recalc, bool redraw = true);
    virtual void setPixmap(const QPixmap &icon, bool recalc, bool redraw = true);
    virtual void setKey(const QString &k);

    virtual int rtti() const;
    static int RTTI;

protected:
#ifndef QT_NO_TEXTEDIT
    virtual void removeRenameBox();
#endif
    virtual void calcRect(const QString &text_ = QString::null);
    virtual void paintItem(QPainter *p, const QPalette &pal);
    virtual void paintFocus(QPainter *p, const QPalette &pal);
#ifndef QT_NO_DRAGANDDROP
    virtual void dropped(QDropEvent *e, const QLinkedList<QIconDragItem> &lst);
#endif
    virtual void dragEntered();
    virtual void dragLeft();
    void setItemRect(const QRect &r);
    void setTextRect(const QRect &r);
    void setPixmapRect(const QRect &r);
    void calcTmpText();
    QString tempText() const;

private:
    void init(QIconViewItem *after = 0
#ifndef QT_NO_PICTURE
               , QPicture *pic = 0
#endif
              );
#ifndef QT_NO_TEXTEDIT
    void renameItem();
    void cancelRenameItem();
#endif
    void checkRect();

    QIconView *view;
    QString itemText, itemKey;
    QString tmpText;
    QPixmap *itemIcon;
#ifndef QT_NO_PICTURE
    QPicture *itemPic;
#endif
    QIconViewItem *prev, *next;
    uint allow_rename : 1;
    uint allow_drag : 1;
    uint allow_drop : 1;
    uint selected : 1;
    uint selectable : 1;
    uint dirty : 1;
    uint wordWrapDirty : 1;
    QRect itemRect, itemTextRect, itemIconRect;
#ifndef QT_NO_TEXTEDIT
    QIconViewItemLineEdit *renameBox;
#endif
    QRect oldRect;

    QIconViewItemPrivate *d;

};

class QIconViewPrivate;          /* don't touch */

class Q_COMPAT_EXPORT QIconView : public QScrollView
{
    friend class QIconViewItem;
    friend class QIconViewPrivate;
    friend class QIconViewToolTip;

    Q_OBJECT
    // #### sorting and sort direction do not work
    Q_ENUMS(SelectionMode ItemTextPos Arrangement ResizeMode)
    Q_PROPERTY(bool sorting READ sorting)
    Q_PROPERTY(bool sortDirection READ sortDirection)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(int gridX READ gridX WRITE setGridX)
    Q_PROPERTY(int gridY READ gridY WRITE setGridY)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(ItemTextPos itemTextPos READ itemTextPos WRITE setItemTextPos)
    Q_PROPERTY(QBrush itemTextBackground READ itemTextBackground WRITE setItemTextBackground)
    Q_PROPERTY(Arrangement arrangement READ arrangement WRITE setArrangement)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(int maxItemWidth READ maxItemWidth WRITE setMaxItemWidth)
    Q_PROPERTY(int maxItemTextLength READ maxItemTextLength WRITE setMaxItemTextLength)
    Q_PROPERTY(bool autoArrange READ autoArrange WRITE setAutoArrange)
    Q_PROPERTY(bool itemsMovable READ itemsMovable WRITE setItemsMovable)
    Q_PROPERTY(bool wordWrapIconText READ wordWrapIconText WRITE setWordWrapIconText)
    Q_PROPERTY(bool showToolTips READ showToolTips WRITE setShowToolTips)
    Q_PROPERTY(uint count READ count)

public:
    enum SelectionMode {
        Single = 0,
        Multi,
        Extended,
        NoSelection
    };
    enum Arrangement {
        LeftToRight = 0,
        TopToBottom
    };
    enum ResizeMode {
        Fixed = 0,
        Adjust
    };
    enum ItemTextPos {
        Bottom = 0,
        Right
    };

    QIconView(QWidget* parent=0, const char* name=0, WFlags f = 0);
    virtual ~QIconView();

    virtual void insertItem(QIconViewItem *item, QIconViewItem *after = 0L);
    virtual void takeItem(QIconViewItem *item);

    int index(const QIconViewItem *item) const;

    QIconViewItem *firstItem() const;
    QIconViewItem *lastItem() const;
    QIconViewItem *currentItem() const;
    virtual void setCurrentItem(QIconViewItem *item);
    virtual void setSelected(QIconViewItem *item, bool s, bool cb = false);

    uint count() const;

public:
    virtual void showEvent(QShowEvent *);

    virtual void setSelectionMode(SelectionMode m);
    SelectionMode selectionMode() const;

    QIconViewItem *findItem(const QPoint &pos) const;

    enum StringComparisonMode {
        CaseSensitive   = 0x00001, // 0 0001
        BeginsWith      = 0x00002, // 0 0010
        EndsWith        = 0x00004, // 0 0100
        Contains        = 0x00008, // 0 1000
        ExactMatch      = 0x00010  // 1 0000
    };
    typedef uint ComparisonFlags;
    QIconViewItem *findItem(const QString &text, ComparisonFlags = BeginsWith | Qt::CaseSensitive) const;
    virtual void selectAll(bool select);
    virtual void clearSelection();
    virtual void invertSelection();

    virtual void repaintItem(QIconViewItem *item);
    void repaintSelectedItems();

    void ensureItemVisible(QIconViewItem *item);
    QIconViewItem* findFirstVisibleItem(const QRect &r) const;
    QIconViewItem* findLastVisibleItem(const QRect &r) const;

    virtual void clear();

    virtual void setGridX(int rx);
    virtual void setGridY(int ry);
    int gridX() const;
    int gridY() const;
    virtual void setSpacing(int sp);
    int spacing() const;
    virtual void setItemTextPos(ItemTextPos pos);
    ItemTextPos itemTextPos() const;
    virtual void setItemTextBackground(const QBrush &b);
    QBrush itemTextBackground() const;
    virtual void setArrangement(Arrangement am);
    Arrangement arrangement() const;
    virtual void setResizeMode(ResizeMode am);
    ResizeMode resizeMode() const;
    virtual void setMaxItemWidth(int w);
    int maxItemWidth() const;
    virtual void setMaxItemTextLength(int w);
    int maxItemTextLength() const;
    virtual void setAutoArrange(bool b);
    bool autoArrange() const;
    virtual void setShowToolTips(bool b);
    bool showToolTips() const;

    void setSorting(bool sort, bool ascending = true);
    bool sorting() const;
    bool sortDirection() const;

    virtual void setItemsMovable(bool b);
    bool itemsMovable() const;
    virtual void setWordWrapIconText(bool b);
    bool wordWrapIconText() const;

    bool eventFilter(QObject * o, QEvent *);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    virtual void sort(bool ascending = true);

    virtual void setFont(const QFont &);
    virtual void setPalette(const QPalette &);

    bool isRenaming() const;

public slots:
    virtual void arrangeItemsInGrid(const QSize &grid, bool update = true);
    virtual void arrangeItemsInGrid(bool update = true);
    virtual void setContentsPos(int x, int y);
    virtual void updateContents();

signals:
    void selectionChanged();
    void selectionChanged(QIconViewItem *item);
    void currentChanged(QIconViewItem *item);
    void clicked(QIconViewItem *);
    void clicked(QIconViewItem *, const QPoint &);
    void pressed(QIconViewItem *);
    void pressed(QIconViewItem *, const QPoint &);

    void doubleClicked(QIconViewItem *item);
    void returnPressed(QIconViewItem *item);
    void rightButtonClicked(QIconViewItem* item, const QPoint& pos);
    void rightButtonPressed(QIconViewItem* item, const QPoint& pos);
    void mouseButtonPressed(int button, QIconViewItem* item, const QPoint& pos);
    void mouseButtonClicked(int button, QIconViewItem* item, const QPoint& pos);
    void contextMenuRequested(QIconViewItem* item, const QPoint &pos);

#ifndef QT_NO_DRAGANDDROP
    void dropped(QDropEvent *e, const QLinkedList<QIconDragItem> &lst);
#endif
    void moved();
    void onItem(QIconViewItem *item);
    void onViewport();
    void itemRenamed(QIconViewItem *item, const QString &);
    void itemRenamed(QIconViewItem *item);

protected slots:
    virtual void doAutoScroll();
    virtual void adjustItems();
    virtual void slotUpdate();

private slots:
    void movedContents(int dx, int dy);

protected:
    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void contentsContextMenuEvent(QContextMenuEvent *e);

#ifndef QT_NO_DRAGANDDROP
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDragLeaveEvent(QDragLeaveEvent *e);
    void contentsDropEvent(QDropEvent *e);
#endif

    void resizeEvent(QResizeEvent* e);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void enterEvent(QEvent *e);

    virtual void drawRubber(QPainter *p);
#ifndef QT_NO_DRAGANDDROP
    virtual QDragObject *dragObject();
    virtual void startDrag();
#endif
    virtual void insertInGrid(QIconViewItem *item);
    virtual void drawBackground(QPainter *p, const QRect &r);

    void emitSelectionChanged(QIconViewItem * i = 0);
    void emitRenamed(QIconViewItem *item);

    QIconViewItem *makeRowLayout(QIconViewItem *begin, int &y, bool &changed);

    void changeEvent(QEvent *);

private:
    void contentsMousePressEventEx(QMouseEvent *e);
    virtual void drawDragShapes(const QPoint &pnt);
#ifndef QT_NO_DRAGANDDROP
    virtual void initDragEnter(QDropEvent *e);
#endif
    void drawContents(QPainter*);
    QIconViewItem* findItemByName(QIconViewItem *start);
    void handleItemChange(QIconViewItem *old, bool shift,
                           bool control, bool homeend = false);

    int calcGridNum(int w, int x) const;
    QIconViewItem *rowBegin(QIconViewItem *item) const;
    void updateItemContainer(QIconViewItem *item);
    void appendItemContainer();
    void rebuildContainers();
    enum Direction {
        DirUp = 0,
        DirDown,
        DirLeft,
        DirRight
    };
    QIconViewItem* findItem(Direction dir,
                             const QPoint &relativeTo,
                             const QRect &searchRect) const;
    bool neighbourItem(Direction dir,
                        const QPoint &relativeTo,
                        const QIconViewItem *item) const;
    QBitmap mask(QPixmap *pix) const;

    QIconViewPrivate *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QIconView(const QIconView &);
    QIconView& operator=(const QIconView &);
#endif
};

#endif // QT_NO_ICONVIEW

#endif // QICONVIEW_H
