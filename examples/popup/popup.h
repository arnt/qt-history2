/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef POPUP_H
#define POPUP_H
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>

class FancyPopup : public QLabel
{
    Q_OBJECT
public:
    FancyPopup( QWidget* parent = 0, const char*  name=0);
    
    void popup( QWidget* parent = 0);
protected:
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void closeEvent( QCloseEvent * );
    
private:
    QWidget* popupParent;
    int moves;
};


 class Frame : public QFrame
 {
     Q_OBJECT
 public:
     Frame( QWidget *parent=0, const char*  name=0);

 protected:

 private slots:
     void button1Clicked();
     void button2Pressed();
     
 private:
     QPushButton *button1;
     QPushButton *button2;
     
     QFrame* popup1;
     FancyPopup* popup2;
 };

#endif
