/****************************************************************************
** $Id: //depot/qt/main/examples/progressbar/main.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "progressbar.h"
#include <qapplication.h>

int main(int argc,char **argv)
{
    QApplication a(argc,argv);

    ProgressBar progressbar;
    progressbar.resize(300,200);
    progressbar.setCaption("Example for a ProgressBar");
    a.setMainWidget(&progressbar);
    progressbar.show();

    return a.exec();
}
