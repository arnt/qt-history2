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

    ~SimpleFlow();

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSizePolicy::ExpandData expanding() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);

protected:
    void setGeometry( const QRect& );

private:
    int doLayout( const QRect&, bool testonly = FALSE );
    QList<QLayoutItem*> list;
    int cached_width;
    int cached_hfw;

};

#endif
