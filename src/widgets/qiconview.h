/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qiconview.h#22 $
**
** Definition of QIconView widget class
**
** Created : 990707
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QICONVIEW_H
#define QICONVIEW_H

#include <qscrollview.h>
#include <qiconset.h>
#include <qstring.h>
#include <qrect.h>
#include <qpoint.h>
#include <qsize.h>
#include <qfont.h>
#include <qlist.h>
#include <qdragobject.h>
#include <qstringlist.h>

class QIconView;
class QFontMetrics;
class QPainter;
class QMimeSource;
class QDragObject;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QKeyEvent;
class QFocusEvent;

struct QIconViewPrivate;
class QIconViewItem;
class QIconViewItemLineEdit;

/*****************************************************************************
 *
 * Class QIconDragItem
 *
 *****************************************************************************/

class Q_EXPORT QIconDragItem
{
public:
    QIconDragItem();
    QIconDragItem( const QRect &r );
    virtual ~QIconDragItem();

    virtual bool operator<( const QIconDragItem &icon );
    virtual bool operator==( const QIconDragItem &icon );
	
    virtual QRect rect() const;
    virtual QString key() const;

    virtual void setRect( const QRect &r );

protected:
    virtual void makeKey();

    QRect rect_;
    QString key_;
	
};

/*****************************************************************************
 *
 * Class QIconDrag
 *
 *****************************************************************************/

class Q_EXPORT QIconDrag : public QDragObject
{
    Q_OBJECT

public:
    typedef QValueList<QIconDragItem> QIconList;

    QIconDrag( const QIconList &icons_, QWidget * dragSource, const char* name = 0 );
    QIconDrag( QWidget * dragSource, const char* name = 0 );
    virtual ~QIconDrag();

    void setIcons( const QIconList &list_ );
    void append( const QIconDragItem &icon_ );

    virtual const char* format( int i ) const;
    virtual QByteArray encodedData( const char* mime ) const;

    static bool canDecode( QMimeSource* e );

    static bool decode( QMimeSource* e, QIconList &list_ );

protected:
    QIconList icons;

};

/*****************************************************************************
 *
 * Class QIconViewItem
 *
 *****************************************************************************/

class Q_EXPORT QIconViewItem : public QObject
{
    friend class QIconView;
    friend class QIconViewItemLineEdit;

    Q_OBJECT

public:
    QIconViewItem( QIconView *parent );
    QIconViewItem( QIconView *parent, QIconViewItem *after );
    QIconViewItem( QIconView *parent, const QString &text );
    QIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text );
    QIconViewItem( QIconView *parent, const QString &text, const QIconSet &icon );
    QIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text, const QIconSet &icon );
    virtual ~QIconViewItem();

    virtual void setAllowRename( bool allow );
    virtual void setAllowDrag( bool allow );
    virtual void setAllowDrop( bool allow );

    virtual QString text();
    virtual QIconSet icon();

    virtual bool allowRename();
    virtual bool allowDrag();
    virtual bool allowDrop();

    virtual QIconView *iconView()  const;
    virtual QIconViewItem *prevItem()  const;
    virtual QIconViewItem *nextItem()  const;

    virtual int index();

    virtual void setSelected( bool s, bool cb = FALSE );
    virtual void setSelectable( bool s );

    virtual bool isSelected();
    virtual bool isSelectable();

    virtual void repaint();

    virtual void move( int x, int y );
    virtual void moveBy( int dx, int dy );
    virtual void move( const QPoint &pnt );
    virtual void moveBy( const QPoint &pnt );

    virtual QRect rect();
    virtual int x();
    virtual int y();
    virtual int width();
    virtual int height();
    virtual QSize size();
    virtual QPoint pos();
    virtual QRect textRect( bool relative = TRUE );
    virtual QRect iconRect( bool relative = TRUE );
    virtual bool contains( QPoint pnt );
    virtual bool intersects( QRect r );

    virtual void setFont( const QFont &font );
    virtual QFont font();

    virtual void setViewMode( QIconSet::Size mode );

    virtual bool acceptDrop( QMimeSource *mime );

    virtual void rename();

public slots:
    virtual void setText( const QString &text );
    virtual void setIcon( const QIconSet &icon );

protected slots:
    virtual void renameItem();
    virtual void cancelRenameItem();

protected:
    virtual void removeRenameBox();
    virtual void calcRect( const QString &text_ = QString::null );
    virtual void paintItem( QPainter *p );
    virtual void paintFocus( QPainter *p );
    virtual void dropped( QDropEvent *e );
    virtual void dragEntered();
    virtual void dragLeft();
    virtual void init();

    QIconView *view;
    QString itemText;
    QIconSet itemIcon;
    QIconViewItem *prev, *next;
    bool allow_rename, allow_drag, allow_drop;
    bool selected, selectable;
    QRect itemRect, itemTextRect, itemIconRect;
    QFontMetrics *fm;
    QFont f;
    QIconSet::Size viewMode;
    QIconViewItemLineEdit *renameBox;
    bool isReady;

};


/*****************************************************************************
 *
 * Class QIconView
 *
 *****************************************************************************/

class Q_EXPORT QIconView : public QScrollView
{
    friend class QIconViewItem;
    friend struct QIconViewPrivate;

    Q_OBJECT

public:
    enum SelectionMode {
	Single = 0,
	Multi,
	StrictMulti
    };
    enum AlignMode {
	East = 0,
	South
    };
    enum ResizeMode {
	Fixed = 0,
	Adjust
    };

    QIconView( QWidget *parent = 0, const char *name = 0 );
    virtual ~QIconView();

    virtual void insertItem( QIconViewItem *item, QIconViewItem *after = 0L );
    virtual void removeItem( QIconViewItem *item );

    virtual int index( QIconViewItem *item );

    virtual QIconViewItem *firstItem()	const;
    virtual QIconViewItem *lastItem()  const;
    virtual QIconViewItem *currentItem()  const;
    virtual void setCurrentItem( QIconViewItem *item );

    virtual unsigned int count();

    virtual void setViewMode( QIconSet::Size mode );
    virtual QIconSet::Size viewMode();

    virtual void orderItemsInGrid();
    virtual void show();

    virtual void setSelectionMode( SelectionMode m );
    virtual SelectionMode selectionMode();

    virtual QIconViewItem *findItem( const QPoint &pos );
    virtual void selectAll( bool select );

    virtual void repaintItem( QIconViewItem *item );

    virtual void ensureItemVisible( QIconViewItem *item );
    virtual QIconViewItem* findFirstVisibleItem();
    
    virtual void clear();

    virtual void setRastX( int rx );
    virtual void setRastY( int ry );
    virtual int rastX();
    virtual int rastY();
    virtual void setSpacing( int sp );
    virtual int spacing();

    virtual void setAlignMode( AlignMode am );
    virtual AlignMode alignMode() const;

    virtual void setResizeMode( ResizeMode am );
    virtual ResizeMode resizeMode() const;

    virtual void setMaxItemWidth( int w );
    virtual int maxItemWidth() const;
    virtual void setMaxItemTextLength( int w );
    virtual int maxItemTextLength() const;

    bool eventFilter( QObject * o, QEvent * );

signals:
    void dropped( QDropEvent *e );
    void moved();
    void doubleClicked( QIconViewItem *item );
    void itemRightClicked( QIconViewItem *item );
    void viewportRightClicked();
    void selectionChanged();
    void selectionChanged( int numItems );
    void currentChanged();
    void currentChanged( QIconViewItem *item );

protected slots:
    virtual void doAutoScroll();
    virtual void adjustItems();
    virtual void slotUpdate();

protected:
    virtual void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent *e );
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual void resizeEvent( QResizeEvent* e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void focusInEvent( QFocusEvent *e );
    virtual void focusOutEvent( QFocusEvent *e );

    virtual void selectByRubber( QRect oldRubber );
    virtual void drawRubber( QPainter *p );
    virtual QDragObject *dragObject();
    virtual void startDrag();
    virtual void insertInGrid( QIconViewItem *item );
    virtual void drawDragShape( const QPoint &pnt );
    virtual void initDrag( QDropEvent *e );
    virtual void drawBackground( QPainter *p, const QRect &r );

    void emitSelectionChanged();
    void emitNewSelectionNumber();

    void setDragObjectIsKnown( bool b );
    void setIconDragData( const QValueList<QIconDragItem> &lst );
    void setNumDragItems( int num );

private:
    QIconViewPrivate *d;

};

#endif
