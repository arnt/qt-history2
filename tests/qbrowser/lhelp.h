/****************************************************************************
** $Id: //depot/qt/main/tests/qbrowser/lhelp.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef LHELP_H
#define LHELP_H
#include <qml.h>
#include <qmainwindow.h>

class LHelp : public QMainWindow
{
    Q_OBJECT
public:
    LHelp( const QString& home_,  const QString& path, QWidget* parent = 0, const char *name=0 );
    ~LHelp();


 private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void contentsChanged();

private:
    QMLBrowser* browser;
    int backwardId, forwardId;

};





#endif

