/****************************************************************************
** $Id: //depot/qt/main/examples/qwerty/qwerty.h#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QWERTY_H
#define QWERTY_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qprinter.h>

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor( QWidget *parent=0, const char *name="qwerty" );
   ~Editor();

public slots:
    void newDoc();
    void load();
    void load( const QString& fileName );
    void save();
    void print();
    void closeDoc();
    void addEncoding();

protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );

private slots:    
    void saveAsEncoding( int );

private:
    void saveAs( const QString& fileName, int code=-1 );
    void rebuildCodecList();
    QMenuBar 	   *m;
    QMultiLineEdit *e;
    QPrinter        printer;
    QPopupMenu	   *save_as;
};

#endif // QWERTY_H
