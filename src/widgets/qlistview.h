/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.h#49 $
**
** Definition of QListView widget class
**
** Created : 970809
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

class QStrList;
class QPixmap;
class QFont;

class QListView;
struct QListViewPrivate;


#ifndef QT_H
#include "qscrollview.h"
#endif // QT_H


class QListViewItem
{
public:
    QListViewItem( QListView * parent );
    QListViewItem( QListViewItem * parent );
    QListViewItem( QListView * parent,
		   const char *,     const char * = 0,
		   const char * = 0, const char * = 0,
		   const char * = 0, const char * = 0,
		   const char * = 0, const char * = 0 );
    QListViewItem( QListViewItem * parent,
		   const char *,     const char * = 0,
		   const char * = 0, const char * = 0,
		   const char * = 0, const char * = 0,
		   const char * = 0, const char * = 0 );
    virtual ~QListViewItem();

    virtual void insertItem( QListViewItem * );
    virtual void removeItem( QListViewItem * );

    int height() const { return ownHeight; }
    virtual void invalidateHeight();
    int totalHeight() const;
    virtual int width( const QFontMetrics&,
		       const QListView*, int column) const;
    void widthChanged(int column=-1) const;
    int depth() const;

    virtual void setText( int, const char * );
    virtual const char * text( int ) const;

    virtual void setPixmap( int, const QPixmap & );
    virtual const QPixmap * pixmap( int ) const;

    virtual const char * key( int, bool ) const;
    virtual void sortChildItems( int, bool );

    int childCount() const { return nChildren; }

    bool isOpen() const { return open && nChildren>0; } // ###
    virtual void setOpen( bool );
    virtual void setup();

    virtual void setSelected( bool );
    bool isSelected() const { return selected; }

    virtual void paintCell( QPainter *,  const QColorGroup & cg,
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

    QListView *listView() const;

    virtual void setSelectable( bool enable );
    bool isSelectable() const { return selectable; }

    virtual void setExpandable( bool );
    bool isExpandable() { return expandable; }

    void repaint() const;

protected:
    virtual void enforceSortOrder() const;
    virtual void setHeight( int );
    virtual void activate();

private:
    void init();
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


class QListView: public QScrollView
{
    Q_OBJECT
public:
    QListView( QWidget * parent = 0, const char * name = 0 );
    ~QListView();

    int treeStepSize() const;
    virtual void setTreeStepSize( int );

    virtual void insertItem( QListViewItem * );
    virtual void clear();

    virtual int addColumn( const char * label, int size = -1);
    virtual void setColumnText( int column, const char * label );
    const char* columnText( int column ) const;
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
    virtual void setSelected( QListViewItem *, bool );
    bool isSelected( QListViewItem * ) const;
    virtual void setOpen( QListViewItem *, bool );
    bool isOpen( QListViewItem * ) const;

    virtual void setCurrentItem( QListViewItem * );
    QListViewItem * currentItem() const;

    QListViewItem * firstChild() const;

    virtual void setAllColumnsShowFocus( bool );
    bool allColumnsShowFocus() const;

    void setItemMargin( int );
    int itemMargin() const;

    virtual void setRootIsDecorated( bool );
    bool rootIsDecorated() const;

    virtual void setSorting( int column, bool increasing = TRUE );

    void setStyle( GUIStyle );
    void setFont( const QFont & );
    void setPalette( const QPalette & );

    bool eventFilter( QObject * o, QEvent * );

    QSize sizeHint() const;

public slots:
    void triggerUpdate();

signals:
    void selectionChanged();
    void selectionChanged( QListViewItem * );
    void currentChanged( QListViewItem * );

    void doubleClicked( QListViewItem * );
    void returnPressed( QListViewItem * );
    void rightButtonClicked( QListViewItem *, const QPoint&, int );
    void rightButtonPressed( QListViewItem *, const QPoint&, int );

protected:
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );

    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void keyPressEvent( QKeyEvent *e );

    void resizeEvent( QResizeEvent *e );

    void drawContentsOffset( QPainter *, int ox, int oy,
			     int cx, int cy, int cw, int ch );

    virtual void paintEmptyArea( QPainter *, const QRect & );
    void enabledChange( bool );

protected slots:
    void updateContents();

private slots:
    void changeSortColumn( int );
    void updateDirtyItems();
    void handleSizeChange( int, int, int );

private:
    void updateGeometries();
    void buildDrawableList() const;
    void reconfigureItems();
    void widthChanged(const QListViewItem*, int c);

    QListViewPrivate * d;

    friend class QListViewItem;
};


class QCheckListItem : public QListViewItem
{
public:
    enum Type { RadioButton, CheckBox, Controller };

    QCheckListItem( QCheckListItem *parent, const char *text,
		    Type = Controller );
    QCheckListItem( QListView *parent, const char *text,
		    Type = Controller );
    QCheckListItem( QListViewItem *parent, const char *text,
		    const QPixmap & );
    QCheckListItem( QListView *parent, const char *text,
		    const QPixmap & );

    void paintCell( QPainter *,  const QColorGroup & cg,
		    int column, int width, int alignment );
    int width( const QFontMetrics&, const QListView*, int column) const;
    void setup();

    void setOn( bool );
    bool isOn() const { return on; }
    Type type() const { return myType; }
    const char *text() const { return QListViewItem::text( 0 ); }
    const char *text( int n ) const { return QListViewItem::text( n ); }

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

    QPixmap *pix;
};


#endif // QLISTVIEW_H
