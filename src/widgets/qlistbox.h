/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.h#72 $
**
** Definition of QListBox widget class
**
** Created : 941121
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLISTBOX_H
#define QLISTBOX_H

#ifndef QT_H
#include "qscrollview.h"
#include "qpixmap.h"
#include "qtimer.h"
#endif // QT_H


class QListBoxPrivate;
class QListBoxItem;
class QString;
class QStrList;


class Q_EXPORT QListBox : public QScrollView
{
    Q_OBJECT
public:
    QListBox( QWidget *parent=0, const char *name=0, WFlags f=0  );
   ~QListBox();

    virtual void setFont( const QFont & );

    uint count() const;

    // ### uses const char *, not QString
    void insertStrList( const QStrList *, int index=-1 );
    void insertStrList( const QStrList &, int index=-1 );
    void insertStrList( const char **,
			int numStrings=-1, int index=-1 );

    void insertItem( const QListBoxItem *, int index=-1 );
    void insertItem( const QString &text, int index=-1 );
    void insertItem( const QPixmap &pixmap, int index=-1 );

    void removeItem( int index );
    void clear();

    QString text( int index )	const;
    const QPixmap *pixmap( int index )	const;

    void changeItem( const QListBoxItem *, int index );
    void changeItem( const QString &text, int index );
    void changeItem( const QPixmap &pixmap, int index );

    int numItemsVisible() const;

    int currentItem() const;
    void setCurrentItem( int index );
    void ensureCurrentVisible();
    virtual void setCurrentItem( QListBoxItem * );
    void centerCurrentItem() { ensureCurrentVisible(); }
    int topItem() const;
    virtual void setTopItem( int index );
    virtual void setBottomItem( int index );

    bool smoothScrolling() const;
    virtual void setSmoothScrolling( bool );

    long maxItemWidth() const;

    enum SelectionMode { Single, Multi, Extended };
    virtual void setSelectionMode( SelectionMode );
    SelectionMode selectionMode() const;

    void setMultiSelection( bool multi );
    bool isMultiSelection() const;

    virtual void setSelected( int, bool );
    bool isSelected( int ) const;

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

    QListBoxItem *item( int index ) const;
    int index( QListBoxItem * ) const;

    void triggerUpdate( bool doLayout );

    bool itemVisible( int index );
    bool itemVisible( QListBoxItem * );

    enum LayoutMode { FixedNumber,
		      FitWidth, FitToHeight = FitWidth,
		      Variable };
    virtual void setColumnMode( LayoutMode );
    virtual void setColumnMode( int );
    virtual void setRowMode( LayoutMode );
    virtual void setRowMode( int );

    LayoutMode columnMode() const;
    LayoutMode rowMode() const;

    int numColumns() const;
    int	numCols() const { return numColumns(); } // for QTableView users
    int	numRows() const;

    bool variableWidth() const;
    virtual void setVariableWidth( bool );

    bool variableHeight() const;
    virtual void setVariableHeight( bool );

    void drawContents( QPainter*, int, int, int, int );

public slots:
    virtual void clearSelection();

signals:
    void highlighted( int index );
    void selected( int index );
    void highlighted( const QString &);
    void selected( const QString &);
    void highlighted( QListBoxItem * );
    void selected( QListBoxItem * );

    void selectionChanged();

protected:
    void viewportMousePressEvent( QMouseEvent * );
    void viewportMouseReleaseEvent( QMouseEvent * );
    void viewportMouseDoubleClickEvent( QMouseEvent * );
    void viewportMouseMoveEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );

    void updateItem( int index );
    void updateItem( QListBoxItem * );
    void updateCellWidth();

    void toggleCurrentItem();

    void updateVector();
    void doLayout() const;

    QListBoxItem * itemAt( QPoint ) const;

private slots:
    void refreshSlot();
    void autoScroll();

private:
    void tryGeometry( int, int ) const;
    int currentRow() const;
    int currentColumn() const;
    void verticalScrollMagic( QListBoxItem *, int );

    void emitChangedSignal( bool );

    QListBoxPrivate * d;

    static QListBox * changedListBox;

    friend class QListBoxItem; // ### for access to d.  regrettable.

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBox( const QListBox & );
    QListBox &operator=( const QListBox & );
#endif
};


class Q_EXPORT QListBoxItem
{
public:
    QListBoxItem();
    virtual ~QListBoxItem();

    virtual QString text() const;
    virtual const QPixmap *pixmap() const;

    virtual int	 height() const = 0;
    virtual int	 width()  const = 0;

    QListBox * listBox() const { return parent; }

protected:
    virtual void paint( QPainter * ) = 0;
    void	 setText( const QString &text ) { txt = text; }
    virtual void update( bool doLayout = FALSE );

private:
    QString txt;
    QListBox * parent;
    uint selected:1;
    uint dirty:1;
    int x, y;
    QListBoxItem * p, * n;

    friend class QListBox;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxItem( const QListBoxItem & );
    QListBoxItem &operator=( const QListBoxItem & );
#endif
};


class Q_EXPORT QListBoxText : public QListBoxItem
{
public:
    QListBoxText( const QString & text=QString::null );
   ~QListBoxText();
    void  paint( QPainter * );
    int	  height() const;
    int	  width()  const;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxText( const QListBoxText & );
    QListBoxText &operator=( const QListBoxText & );
#endif
};


class Q_EXPORT QListBoxPixmap : public QListBoxItem
{
public:
    QListBoxPixmap( const QPixmap & );
   ~QListBoxPixmap();
    const QPixmap *pixmap() const { return &pm; }
protected:
    void paint( QPainter * );
    int height() const;
    int width() const;
private:
    QPixmap pm;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxPixmap( const QListBoxPixmap & );
    QListBoxPixmap &operator=( const QListBoxPixmap & );
#endif
};


#endif // QLISTBOX_H
