/****************************************************************************
** $Id: //depot/qt/main/examples/compact/keyboard.h#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qwidget.h>
class Keyboard : public QWidget {
public:
    Keyboard(QWidget* parent=0, const char* name=0, int f=0);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent* e);
    void drawKeyboard( QPainter &p, int key = -1 );

    QSize sizeHint() const;
    
private:
    int getKey( int &w, int j = -1 );

    uint shift:1;
    uint lock:1;
    uint ctrl:1;
    uint alt:1;
    
    int pressedKey;
};
