/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
** EDITIONS: FREE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GAMMAVIEW_H
#define GAMMAVIEW_H

#include <qwidget.h>

class GammaView: public QWidget
{
    Q_OBJECT
public:
    GammaView( QWidget *parent = 0,
		const char *name = 0, WFlags f = 0 ) :
	QWidget(parent,name,f)
    { }
};

#endif
