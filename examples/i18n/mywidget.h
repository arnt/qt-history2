/****************************************************************************
** $Id: //depot/qt/main/examples/i18n/mywidget.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <qvbox.h>
#include <qstring.h>

class MyWidget : public QVBox
{
	Q_OBJECT

public:
	MyWidget( QWidget* parent, const QString &language, const char* name = 0 );

private:
	void initChoices();

};

#endif
