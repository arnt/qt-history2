/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.h#9 $
**
** Definition of 
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

class QStrList;
class QPixmap;
class QFont;

class QListView;
struct QListViewPrivate;


#include "qscrollview.h"


class QListViewItem
{
public:
    QListViewItem( QListView * parent );
    QListViewItem( QListViewItem * parent );
    QListViewItem( QListView * parent, const char * firstLabel, ... );
    QListViewItem( QListViewItem * parent, const char * firstLabel, ... );
    virtual ~QListViewItem();

    virtual void insertItem( QListViewItem * );
    virtual void removeItem( QListViewItem * );

    virtual int compare( int column, const QListViewItem * with ) const;

    int height() const { return ownHeight; }
    virtual void styleChange();
    virtual void invalidateHeight();
    int totalHeight() const;

    virtual const char * text( int ) const;

    int children() const { return childCount; }

    bool isOpen() const { return open && childCount>0; } // ###
    virtual void setOpen( bool );

    virtual void setSelected( bool );
    bool isSelected() const { return selected; }

    virtual void paintCell( QPainter *,  const QColorGroup & cg,
			    int column, int width, bool showFocus ) const;
    virtual void paintBranches( QPainter * p, const QColorGroup & cg,
				int w, int y, int h, GUIStyle s ) const;

    const QListViewItem * firstChild() const { return childItem; }
    const QListViewItem * nextSibling() const { return siblingItem; }

    virtual QListView *listView() const;

protected:
    void setHeight( int );

private:
    void init();
    int ownHeight;
    int maybeTotalHeight;
    int childCount;

    uint open : 1;
    uint selected : 1;

    QListViewItem * parentItem;
    QListViewItem * siblingItem;
    QListViewItem * childItem;

    QStrList * columnTexts;

    friend QListView;
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

    virtual void setColumn( const char * label, int size, int column=-1 );

    void show();
    void setFont( const QFont & );

    QListViewItem * itemAt( QPoint screenPos ) const;
    QRect itemRect( QListViewItem * ) const;

    virtual void setMultiSelection( bool enable );
    bool isMultiSelection() const;
    virtual void setSelected( QListViewItem *, bool );
    bool isSelected( QListViewItem * ) const;

    virtual void setCurrentItem( QListViewItem * );
    QListViewItem * currentItem() const;

public slots:
    void triggerUpdate();

signals:
    void sizeChanged();

protected:
    bool eventFilter( QObject * o, QEvent * );

    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );

    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void keyPressEvent( QKeyEvent *e );

    void drawContentsOffset( QPainter *, int ox, int oy,
			     int cx, int cy, int cw, int ch );
    
protected slots:
    void updateContents();

private:
    void doStyleChange( QListViewItem * );
    void buildDrawableList() const;

    QListViewPrivate * d;
};


#endif
