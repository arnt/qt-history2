#ifndef TEST_H
#define TEST_H

#include "qtextlayout.h"
#include <qwidget.h>

class MyArea : public QTextArea {
public:
    MyArea();

    virtual int lineWidth(int x, int y, int h = 0) const;
    virtual QRect lineRect(int x, int y, int h) const;

    QRegion r;
};


class MyView : public QWidget {
    Q_OBJECT
public:
    MyView();
protected:
    virtual void paintEvent(QPaintEvent *);

    QTextArea *area;
};




#endif
