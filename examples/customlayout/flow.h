/****************************************************************************
** $Id: //depot/qt/main/examples/customlayout/flow.h#2 $
**
** Definition of simple flow layout for custom layout example
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef FLOW_H
#define FLOW_H

#include <qlayout.h>
#include <qlist.h>

class SimpleFlow : public QLayout
{
public:
    SimpleFlow( QWidget *parent, int border=0, int space=-1,
		const char *name=0 )
	: QLayout( parent, border, space, name ),
	cached_width(0) {}
    SimpleFlow( QLayout* parent, int space=-1, const char *name=0 )
	: QLayout( parent, space, name ),
	cached_width(0) {}
    SimpleFlow( int space=-1, const char *name=0 )
	: QLayout( space, name ),
	cached_width(0) {}

    ~SimpleFlow()
    {}
    
    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const { return TRUE; }
    int heightForWidth( int ) const;
    QSize sizeHint() const { return minimumSize(); }
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const
	{ return QSizePolicy::NoDirection; }
protected:
    void setGeometry( const QRect& );
private:
    int layout( const QRect&, bool testonly = FALSE );
    QList<QLayoutItem> list;
    int cached_width;
    int cached_hfw;
};

#endif
