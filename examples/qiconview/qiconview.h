/****************************************************************************
** $Id: //depot/qt/main/examples/qiconview/qiconview.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QTICONVIEW_H
#define QTICONVIEW_H

#include <qscrollview.h>
#include <qiconset.h>
#include <qstring.h>
#include <qrect.h>
#include <qpoint.h>
#include <qsize.h>
#include <qfont.h>
#include <qlist.h>
#include <qdragobject.h>
#include <qmultilinedit.h>
#include <qstringlist.h>

class QtIconView;
class QFontMetrics;
class QPainter;
class QMimeSource;
class QDragObject;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QKeyEvent;

struct QtIconViewPrivate;
class QtIconViewItem;

/*****************************************************************************
 *
 * Class QtIconViewItemDrag
 *
 *****************************************************************************/

class QtIconViewItemDrag : public QDragObject
{
public:
    QtIconViewItemDrag( QWidget *source = 0, const char *name = 0 )
        : QDragObject( source, name )  {}

    virtual const char *format( int )  const
    { return 0L; }

    virtual QByteArray encodedData( const char * )  const
    { return QByteArray(); }

};

/*****************************************************************************
 *
 * Class QtIconViewItemLineEdit
 *
 *****************************************************************************/

class QtIconViewItemLineEdit : public QMultiLineEdit
{
    Q_OBJECT

public:
    QtIconViewItemLineEdit( const QString &text, QWidget *parent, QtIconViewItem *theItem, const char *name = 0 );

signals:
    void escapePressed();

protected:
    void keyPressEvent( QKeyEvent *e );

protected:
    QtIconViewItem *item;
    QString startText;

};

/*****************************************************************************
 *
 * Class QtIconViewItem
 *
 *****************************************************************************/

class QtIconViewItem : public QObject
{
    friend class QtIconView;

    Q_OBJECT

public:
    QtIconViewItem( QtIconView *parent );
    QtIconViewItem( QtIconView *parent, QtIconViewItem *after );
    QtIconViewItem( QtIconView *parent, const QString &text );
    QtIconViewItem( QtIconView *parent, QtIconViewItem *after, const QString &text );
    QtIconViewItem( QtIconView *parent, const QString &text, const QIconSet &icon );
    QtIconViewItem( QtIconView *parent, QtIconViewItem *after, const QString &text, const QIconSet &icon );
    virtual ~QtIconViewItem();

    virtual void setAllowRename( bool allow );
    virtual void setAllowDrag( bool allow );
    virtual void setAllowDrop( bool allow );

    virtual QString text();
    virtual QIconSet icon();

    virtual bool allowRename();
    virtual bool allowDrag();
    virtual bool allowDrop();

    virtual QtIconView *iconView()  const;
    virtual QtIconViewItem *prevItem()  const;
    virtual QtIconViewItem *nextItem()  const;

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
    virtual QDragObject *dragObject();

    virtual void rename();

public slots:
    virtual void setText( const QString &text );
    virtual void setIcon( const QIconSet &icon );

protected slots:
    virtual void renameItem();
    virtual void cancelRenameItem();

protected:
    virtual void removeRenameBox();
    virtual void calcRect();
    virtual void paintItem( QPainter *p );
    virtual void paintFocus( QPainter *p );
    virtual void makeActiveIcon();
    virtual void dropped( QMimeSource *mime );
    virtual void breakLines( const QString text, QStringList &lst, int width );
    virtual void dragEntered();
    virtual void dragLeft();

    QtIconView *view;
    QString itemText;
    QIconSet itemIcon;
    QtIconViewItem *prev, *next;
    bool allow_rename, allow_drag, allow_drop;
    bool selected, selectable;
    QRect itemRect, itemTextRect, itemIconRect;
    QFontMetrics *fm;
    QFont f;
    QIconSet::Size viewMode;
    QtIconViewItemLineEdit *renameBox;

};


/*****************************************************************************
 *
 * Class QtIconView
 *
 *****************************************************************************/

class QtIconView : public QScrollView
{
    friend class QtIconViewItem;
    friend struct QtIconViewPrivate;

    Q_OBJECT

public:
    enum SelectionMode {
        Single = 0,
        Multi,
        StrictMulti
    };

    QtIconView( QWidget *parent = 0, const char *name = 0 );
    virtual ~QtIconView();

    virtual void insertItem( QtIconViewItem *item, QtIconViewItem *after = 0L );
    virtual void removeItem( QtIconViewItem *item );

    virtual int index( QtIconViewItem *item );

    virtual QtIconViewItem *firstItem()  const;
    virtual QtIconViewItem *lastItem()  const;
    virtual QtIconViewItem *currentItem()  const;
    virtual void setCurrentItem( QtIconViewItem *item );

    virtual unsigned int count();

    virtual void setViewMode( QIconSet::Size mode );
    virtual QIconSet::Size viewMode();

    virtual void orderItemsInGrid();
    virtual void show();

    virtual void setSelectionMode( SelectionMode m );
    virtual SelectionMode selectionMode();

    virtual QtIconViewItem *findItem( const QPoint &pos );
    virtual void selectAll( bool select );

    virtual void repaintItem( QtIconViewItem *item );

    virtual void ensureItemVisible( QtIconViewItem *item );

    virtual void clear();

    virtual void setRastX( int rx );
    virtual void setRastY( int ry );
    virtual int rastX();
    virtual int rastY();
    virtual void setSpacing( int sp );
    virtual int spacing();

signals:
    void dropped( QMimeSource *mime );
    void moved();
    void doubleClicked( QtIconViewItem *item );
    void itemRightClicked( QtIconViewItem *item );
    void viewportRightClicked();

protected slots:
    virtual void doAutoScroll();

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
    virtual void keyPressEvent( QKeyEvent *e );

    virtual void selectByRubber( QRect oldRubber );
    virtual void drawRubber( QPainter *p );
    virtual QDragObject *dragObject();
    virtual void startDrag( bool move = FALSE );
    virtual void insertInGrid( QtIconViewItem *item );

    QtIconViewPrivate *d;

};

#endif
