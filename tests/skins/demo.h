/****************************************************************************
** Form interface generated from reading ui file '/home/ianw/dev/qt/main/examples/skins/demo.ui'
**
** Created: Tue Oct 23 12:25:33 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SKINABLE_H
#define SKINABLE_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QPushButton;
class QSkinDial;

class Skinable : public QWidget
{ 
    Q_OBJECT

public:
    Skinable( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Skinable();

    QPushButton* Forward;
    QPushButton* Stop;
    QPushButton* Back;
    QPushButton* Play;
    QPushButton* QuitButton;
    QSkinDial* Volume;


public slots:
    virtual void init();

};

#endif // SKINABLE_H
