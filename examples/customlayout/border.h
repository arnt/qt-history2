/****************************************************************************
**
** Definition of simple flow layout for custom layout example.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BORDER_H
#define BORDER_H

#include <qlayout.h>
#include <qlist.h>
#include <qwidget.h>

class BorderWidgetItem : public QWidgetItem
{
public:
    BorderWidgetItem( QWidget *w )
	: QWidgetItem( w )
    {}

    void setGeometry( const QRect &r )
    { widget()->setGeometry( r ); }

};

class BorderLayout : public QLayout
{
public:
    enum Position {
	West = 0,
	North,
	South,
	East,
	Center
    };

    struct BorderLayoutStruct
    {
	BorderLayoutStruct( QLayoutItem *i, Position p ) {
	    item = i;
	    pos = p;
	}

	QLayoutItem *item;
	Position pos;
    };

    enum SizeType {
	Minimum = 0,
	SizeHint
    };

    BorderLayout( QWidget *parent, int border = 0, int autoBorder = -1,
		  const char *name = 0 )
	: QLayout( parent, border, autoBorder, name ), cached( 0, 0 ), mcached( 0, 0 )
	{}

    BorderLayout( QLayout* parent, int autoBorder = -1, const char *name = 0 )
	: QLayout( parent, autoBorder, name  ), cached( 0, 0 ), mcached( 0, 0 )
    {}

    BorderLayout( int autoBorder = -1, const char *name = 0 )
	: QLayout( autoBorder, name ), cached( 0, 0 ), mcached( 0, 0 )
    {}

    ~BorderLayout();

    void addItem( QLayoutItem *item );

    void addWidget( QWidget *widget, Position pos );
    void add( QLayoutItem *item, Position pos );

    bool hasHeightForWidth() const;

    QSize sizeHint() const;
    QSize minimumSize() const;

    QLayoutItem *itemAt(int);
    QLayoutItem *takeAt(int);
    QSizePolicy::ExpandData expanding() const;

protected:
    void setGeometry( const QRect &rect );

private:
    void doLayout( const QRect &rect );
    void calcSize();

    QList<BorderLayoutStruct*> list;
    QSize cached, mcached;
};

#endif
