/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qwidget.h>


class QVFb;
class QVFbView;


class Skin : public QWidget
{
public:
    Skin( QVFb *p, const QString &skinFile, int &viewW, int &viewH );
    ~Skin( );
    void setView( QVFbView *v );
protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent * );
private:
    QVFb *parent;
    QVFbView *view;
    QPoint clickPos;
    bool buttonPressed;
    int buttonCode;
    int buttonIndex;
    float transparancy;

    QString skinImageUpFileName;
    QString skinImageDownFileName;
    QPixmap *skinImageUp;
    QPixmap *skinImageDown;
    int viewX1, viewY1;
    int numberOfAreas;

    typedef struct {
	QString	name;
        int	keyCode;
        int	x1, y1;
        int	x2, y2;
    } ButtonAreas;

    ButtonAreas *areas;
};


