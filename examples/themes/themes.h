/****************************************************************************
** $Id: //depot/qt/main/examples/themes/themes.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef THEMES_H
#define THEMES_H

#include <qmainwindow.h>

class QTabWidget;

class Themes: public QMainWindow
{
    Q_OBJECT

public:
    Themes( QWidget *parent = 0L, const char *name = 0L, WFlags f = WType_TopLevel );

protected:
    QTabWidget *tabwidget;

    int sWood, sMetal, sPlatinum, sWindows, sCDE, sMotif;

protected slots:
    void styleWood();
    void styleMetal();
    void stylePlatinum();
    void styleWindows();
    void styleCDE();
    void styleMotif();

};


#endif
