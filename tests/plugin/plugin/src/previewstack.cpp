/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#include "previewstack.h"
#include <qlayout.h>
#include <qobjectlist.h>
#include <qapplication.h>

static const int pc = 1;

PreviewStack::PreviewStack( QWidget* parent, const char* name, WFlags f )

: QWidgetStack( parent, name )

{
    setWFlags( f );
    int i;
    QWidget* page[ pc ];
    QBoxLayout* layout[ pc ];

    for ( i = 0; i < pc; i++) {
	page[i] = new QWidget( );
	layout[i] = new QVBoxLayout( page[i] );
	addWidget( page[i], i );
    }

    layout[0]->addWidget( new QWidget( page[0] ), 1, AlignHCenter );

    
    raiseWidget(0);

}



PreviewStack::~PreviewStack()
{
}

void PreviewStack::nextWidget()
{
    int newid = id(visibleWidget()) + 1;
    raiseWidget( newid % pc );
}

void PreviewStack::previousWidget()
{
    int newid = id(visibleWidget()) - 1;
    raiseWidget( newid < 0 ? pc-1 : newid );
}

/*!
    \reimp
*/

void PreviewStack::setPreviewPalette( QPalette pal )
{
    QObjectList* list = queryList("QWidget");
    QObjectListIt it(*list);

    for ( ; it.current(); ++it)
        ((QWidget*) it.current())->setPalette(pal);
}
