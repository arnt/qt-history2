/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qabstractlayout.h#13 $
**
** Definition of the abstract layout base class
**
** Created : 960416
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

#ifndef QABSTRACTLAYOUT_H
#define QABSTRACTLAYOUT_H

#ifndef QT_H
#include "qobject.h"
#include "qsizepolicy.h"
#endif // QT_H

class QMenuBar;
class QWidget;
struct QLayoutData;
class QLayoutItem;

class Q_EXPORT QGLayoutIterator : public QShared
{
public:
    virtual uint count() const = 0;
    virtual void toFirst() = 0;
    virtual void next() = 0;
    virtual QLayoutItem *current() = 0;
    virtual void removeCurrent() = 0;
};

class Q_EXPORT QLayoutIterator
{
public:
    QLayoutIterator( QGLayoutIterator *i ) :it(i) {}
    QLayoutIterator( const QLayoutIterator &i ) :it( i.it )
    { if ( it ) it->ref(); }
    ~QLayoutIterator() { if ( it && it->deref() ) delete it; }
    QLayoutIterator &operator=( const QLayoutIterator &i )
    {
	if ( it && it->deref() ) delete it;
	it = i.it;
	if ( it ) it->ref();
	return *this;
    }

    uint count() const { return it ? it->count() : 0; }
    bool isNull() const { return it == 0; }
    bool isEmpty() const { return count() == 0; }
    void toFirst() { if ( it ) it->toFirst(); }
    void next() { if ( it ) it->next(); }
    QLayoutItem *current() { return it ? it->current() : 0; }
    void removeCurrent() { if ( it ) it->removeCurrent(); }
private:
    QGLayoutIterator *it;
};

class Q_EXPORT QLayoutItem
{
public:
    QLayoutItem( int alignment = 0 ) :align(alignment) {}
    virtual ~QLayoutItem();
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual QSizePolicy::ExpandData expanding() const =0;
    virtual void setGeometry( const QRect& ) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth( int ) const;
    virtual void invalidate();

    virtual QWidget *widget();
    virtual QLayoutIterator iterator();

    int alignment() const { return align; }
    virtual void setAlignment( int a );
protected:
    int align;
};


class Q_EXPORT QSpacerItem : public QLayoutItem
{
 public:
    QSpacerItem( int w, int h, QSizePolicy::SizeType hData=QSizePolicy::Minimum,
		 QSizePolicy::SizeType vData= QSizePolicy::Minimum )
	:width(w), height(h), sizeP(hData, vData )
	{}
    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& );
    QRect geometry() const;
 private:
    int width, height;
    QSizePolicy sizeP;
    QRect rect;
};


class Q_EXPORT QWidgetItem : public QLayoutItem
{
public:
    QWidgetItem( QWidget *w ) : wid(w) {}
    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& ) ;
    QRect geometry() const;
    virtual QWidget *widget();

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

private:
    QWidget *wid;
};


class Q_EXPORT QLayout : public QObject, public QLayoutItem
{
    Q_OBJECT
public:
    QLayout( QWidget *parent, int border=0, int autoBorder=-1,
	     const char *name=0 );
    QLayout( QLayout *parentLayout, int autoBorder=-1, const char *name=0 );
    QLayout( int autoBorder=-1, const char *name=0 );

    ~QLayout();
    int defaultBorder() const { return insideSpacing; }
    int margin() const { return outsideBorder; }

    enum { unlimited = QCOORD_MAX };

    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

    virtual void  setMenuBar( QMenuBar *w );

    QWidget *mainWidget();
    QMenuBar *menuBar() const { return menubar; }
    bool isTopLevel() const { return topLevel; }
    QRect geometry() const;
#if 1	//OBSOLETE
    bool activate();
#endif

    void add( QWidget *w ) { addItem( new QWidgetItem( w ) ); }
    virtual void addItem ( QLayoutItem * ) = 0;

    QSizePolicy::ExpandData expanding() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void setGeometry( const QRect& )=0;
    QLayoutIterator iterator()=0;
    bool isEmpty() const;

    int totalHeightForWidth( int w ) const;
protected:
    bool  eventFilter( QObject *, QEvent * );
    void addChildLayout( QLayout *l );
private:
    void setWidgetLayout( QWidget *, QLayout * );
    int insideSpacing;
    int outsideBorder;
    bool    topLevel;
    QRect rect;
    QLayoutData *extraData;
    QMenuBar *menubar;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLayout( const QLayout & );
    QLayout &operator=( const QLayout & );
#endif

};


#endif
