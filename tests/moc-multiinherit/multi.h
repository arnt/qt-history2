#ifndef MULTI_H
#define MULTI_H

#include <qwidget.h>

class Something
{
public:
    virtual void foo();
};

class VisualSomething : public QWidget, public Something
{
    Q_OBJECT
public:
    VisualSomething( QWidget* parent = 0, const char* name = 0, int f = 0 );
public slots:
    void foo();
};

class Main : public QWidget {
    Q_OBJECT
public:
    Main( QWidget* parent = 0, const char* name = 0, int f = 0 );
};


#endif
