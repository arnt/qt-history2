/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.h#97 $
**
** Definition of QListView widget class
**
** Created : 970809
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

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

class QPixmap;
class QFont;
class QHeader;
class QIconSet;
class QDomElement;

class QListView;
struct QListViewPrivate;
class QListViewItemIterator;

#ifndef QT_H
#include "qscrollview.h"
#endif // QT_H


class Q_EXPORT QListViewItem: public Qt
{
friend class QListViewItemIterator;

public:
    QListViewItem( QListView * parent );
    QListViewItem( QListViewItem * parent );
    QListViewItem( QListView * parent, QListViewItem * after );
    QListViewItem( QListViewItem * parent, QListViewItem * after );

    QListViewItem( QListView * parent,
		   QString,     QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null );
    QListViewItem( QListViewItem * parent,
		   QString,     QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null );

    QListViewItem( QListView * parent, QListViewItem * after,
		   QString,     QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null );
    QListViewItem( QListViewItem * parent, QListViewItem * after,
		   QString,     QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null,
		   QString = QString::null, QString = QString::null );
    virtual ~QListViewItem();

    virtual void insertItem( QListViewItem * );
    virtual void takeItem( QListViewItem * );
    virtual void removeItem( QListViewItem * ); //obsolete, use takeItem instead

    int height() const;
    virtual void invalidateHeight();
    int totalHeight() const;
    virtual int width( const QFontMetrics&,
		       const QListView*, int column) const;
    void widthChanged(int column=-1) const;
    int depth() const;

    virtual void setText( int, const QString &);
    virtual QString text( int ) const;

    virtual void setPixmap( int, const QPixmap & );
    virtual const QPixmap * pixmap( int ) const;

    virtual QString key( int, bool ) const;
    virtual void sortChildItems( int, bool );

    int childCount() const { return nChildren; }

    bool isOpen() const { return open; }
    virtual void setOpen( bool );
    virtual void setup();

    virtual void setSelected( bool );
    bool isSelected() const { return selected; }

    virtual void paintCell( QPainter *, const QColorGroup & cg,
			    int column, int width, int alignment );
    virtual void paintBranches( QPainter * p, const QColorGroup & cg,
				int w, int y, int h, GUIStyle s );
    virtual void paintFocus( QPainter *, const QColorGroup & cg,
			     const QRect & r );

    QListViewItem * firstChild() const;
    QListViewItem * nextSibling() const { return siblingItem; }
    QListViewItem * parent() const;

    QListViewItem * itemAbove();
    QListViewItem * itemBelow();

    int itemPos() const;

    QListView *listView() const;

    virtual void setSelectable( bool enable );
    bool isSelectable() const { return selectable; }

    virtual void setExpandable( bool );
    bool isExpandable() const { return expandable; }

    void repaint() const;

#ifdef QT_BUILDER
    bool setConfiguration( const QDomElement& item, int columns );
#endif // QT_BUILDER

protected:
    virtual void enforceSortOrder() const;
    virtual void setHeight( int );
    virtual void activate();

private:
    void init();
    void moveToJustAfter( QListViewItem * );
    int ownHeight;
    int maybeTotalHeight;
    int nChildren;

    uint lsc: 14;
    uint lso: 1;
    uint open : 1;
    uint selected : 1;
    uint selectable: 1;
    uint configured: 1;
    uint expandable: 1;
    uint is_root: 1;

    QListViewItem * parentItem;
    QListViewItem * siblingItem;
    QListViewItem * childItem;

    void * columns;

    friend class QListView;
};


class Q_EXPORT QListView: public QScrollView
{
friend class QListViewItemIterator;

    Q_OBJECT
public:
    QListView( QWidget * parent = 0, const char *name = 0 );
    ~QListView();

    int treeStepSize() const;
    virtual void setTreeStepSize( int );

    virtual void insertItem( QListViewItem * );
    virtual void takeItem( QListViewItem * );
    virtual void removeItem( QListViewItem * ); // obsolete, use takeItem instead

    virtual void clear();

    QHeader * header() const;

    virtual int addColumn( const QString &label, int size = -1);
    virtual int addColumn( const QIconSet& iconset, const QString &label, int size = -1);
    virtual void setColumnText( int column, const QString &label );
    virtual void setColumnText( int column, const QIconSet& iconset, const QString &label );
    QString columnText( int column ) const;
    virtual void setColumnWidth( int column, int width );
    int columnWidth( int column ) const;
    enum WidthMode { Manual, Maximum };
    virtual void setColumnWidthMode( int column, WidthMode );
    WidthMode columnWidthMode( int column ) const;

    virtual void setColumnAlignment( int, int );
    int columnAlignment( int ) const;

    void show();

    QListViewItem * itemAt( const QPoint & screenPos ) const;
    QRect itemRect( const QListViewItem * ) const;
    int itemPos( const QListViewItem * );

    void ensureItemVisible( const QListViewItem * );

    void repaintItem( const QListViewItem * ) const;

    virtual void setMultiSelection( bool enable );
    bool isMultiSelection() const;
    virtual void clearSelection();
    virtual void setSelected( QListViewItem *, bool );
    bool isSelected( const QListViewItem * ) const;
    QListViewItem * selectedItem() const;
    virtual void setOpen( QListViewItem *, bool );
    bool isOpen( const QListViewItem * ) const;

    virtual void setCurrentItem( QListViewItem * );
    QListViewItem * currentItem() const;

    QListViewItem * firstChild() const;

    int childCount() const;

    virtual void setAllColumnsShowFocus( bool );
    bool allColumnsShowFocus() const;

    virtual void setItemMargin( int );
    int itemMargin() const;

    virtual void setRootIsDecorated( bool );
    bool rootIsDecorated() const;

    virtual void setSorting( int column, bool increasing = TRUE );

    virtual void setFont( const QFont & );
    virtual void setPalette( const QPalette & );

    bool eventFilter( QObject * o, QEvent * );

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

#ifdef QT_BUILDER
    bool setConfiguration( const QDomElement& element );
#endif // QT_BUILDER

public slots:
    void triggerUpdate();

signals:
    void selectionChanged();
    void selectionChanged( QListViewItem * );
    void currentChanged( QListViewItem * );
    void clicked( QListViewItem * );

    void doubleClicked( QListViewItem * );
    void returnPressed( QListViewItem * );
    void rightButtonClicked( QListViewItem *, const QPoint&, int );
    void rightButtonPressed( QListViewItem *, const QPoint&, int );

protected:
    void contentsMousePressEvent( QMouseEvent * e );
    void contentsMouseReleaseEvent( QMouseEvent * e );
    void contentsMouseMoveEvent( QMouseEvent * e );
    void contentsMouseDoubleClickEvent( QMouseEvent * e );

    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void keyPressEvent( QKeyEvent *e );

    void resizeEvent( QResizeEvent *e );

    void showEvent( QShowEvent * );

    void drawContentsOffset( QPainter *, int ox, int oy,
			     int cx, int cy, int cw, int ch );

    virtual void paintEmptyArea( QPainter *, const QRect & );
    void enabledChange( bool );
    void styleChange( QStyle& );

protected slots:
    void updateContents();
    void doAutoScroll();

private slots:
    void changeSortColumn( int );
    void updateDirtyItems();
    void makeVisible();
    void handleSizeChange( int, int, int );

private:
    void updateGeometries();
    void buildDrawableList() const;
    void reconfigureItems();
    void widthChanged(const QListViewItem*, int c);

    QListViewPrivate * d;

    friend class QListViewItem;
};


class Q_EXPORT QCheckListItem : public QListViewItem
{
public:
    enum Type { RadioButton, CheckBox, Controller };

    QCheckListItem( QCheckListItem *parent, const QString &text,
		    Type = Controller );
    QCheckListItem( QListViewItem *parent, const QString &text,
		    Type = Controller );
    QCheckListItem( QListView *parent, const QString &text,
		    Type = Controller );
    QCheckListItem( QListViewItem *parent, const QString &text,
		    const QPixmap & );
    QCheckListItem( QListView *parent, const QString &text,
		    const QPixmap & );

    void paintCell( QPainter *,  const QColorGroup & cg,
		    int column, int width, int alignment );
    virtual void paintFocus( QPainter *, const QColorGroup & cg,
			     const QRect & r );
    int width( const QFontMetrics&, const QListView*, int column) const;
    void setup();

    virtual void setOn( bool );
    bool isOn() const { return on; }
    Type type() const { return myType; }
    QString text() const { return QListViewItem::text( 0 ); }
    QString text( int n ) const { return QListViewItem::text( n ); }

protected:
    void paintBranches( QPainter * p, const QColorGroup & cg,
			int w, int y, int h, GUIStyle s );

    void activate();
    void turnOffChild();
    virtual void stateChange( bool );

private:
    void init();
    Type myType;
    bool on;
    QCheckListItem *exclusive;

    void *reserved;
};

class Q_EXPORT QListViewItemIterator
{
    friend struct QListViewPrivate;
    friend class QListView;
    friend class QListViewItem;

public:
    QListViewItemIterator();
    QListViewItemIterator( QListViewItem *item );
    QListViewItemIterator( const QListViewItemIterator &it );
    QListViewItemIterator( QListView *lv );

    QListViewItemIterator &operator=( const QListViewItemIterator &it );

    ~QListViewItemIterator();

    QListViewItemIterator &operator++();
    const QListViewItemIterator operator++( int );
    QListViewItemIterator &operator+=( int j );

    QListViewItemIterator &operator--();
    const QListViewItemIterator operator--( int );
    QListViewItemIterator &operator-=( int j );

    QListViewItem *current() const;

protected:
    QListViewItem *curr;
    QListView *listView;

private:
    void addToListView();
    void currentRemoved();

};

#endif // QLISTVIEW_H
