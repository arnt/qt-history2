/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsplitter.h>
#include <qlistbox.h>
#include <qiconview.h>
#include <qpixmap.h>

class QDragEnterEvent;
class QDragDropEvent;


class DDListBox : public QListBox
{
    Q_OBJECT
signals:
    void message( const QString& );
public:
    DDListBox( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    void dragEnterEvent( QDragEnterEvent *evt );
    void dropEvent( QDropEvent *evt );
};


class DDIconView : public QIconView
{
public:
    DDIconView( QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) :
	QIconView( parent, name, f ) {}
    QDragObject *dragObject();
};

