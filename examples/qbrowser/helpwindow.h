/****************************************************************************
** $Id: //depot/qt/main/examples/qbrowser/helpwindow.h#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H
#include <qtextbrowser.h>
#include <qmainwindow.h>

class HelpWindow : public QMainWindow
{
    Q_OBJECT
public:
    HelpWindow( const QString& home_,  const QString& path, QWidget* parent = 0, const char *name=0 );
    ~HelpWindow();


    void setupSlideshow( const QString& file);

 private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void textChanged();
    void about();
    void aboutQt();
    void open();

private:
    QTextBrowser* browser;
    int backwardId, forwardId;

};





#endif

