/****************************************************************************
** $Id: $
**
** Copyright ( C ) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef _TABLET_STATS_
#define _TABLET_STATS_

#include <qwidget.h>

class QLabel;

class TabletStats : public QWidget
{
	Q_OBJECT
public:
	TabletStats( QWidget *parent, const char* name );
	~TabletStats();

protected:
	void tabletEvent( QTabletEvent *e );

protected:
	QLabel *lblXTilt,
		   *lblYTilt,
		   *lblPressure;
};

#endif
