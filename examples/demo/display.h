/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef QT_H
#include <qwidget.h>
#include <qframe.h>
#endif // QT_H

class QTimer;
class QDial;
class QLCDNumber;
class QProgressBar;
class QSpinBox;
class Screen;
class Curve;

class DisplayWidget : public QWidget {
    Q_OBJECT
public:
    DisplayWidget( QWidget *parent=0, const char *name=0 );

    void run();
    void stop();

protected:
    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );

private slots:
    void tick();

private:
    Screen *screen;
    QDial *dial;
    Curve *curve;
    QSpinBox *spin;
    QLCDNumber *lcd;
    int lcdval;
    QProgressBar *bar;
    int tbar;
    QTimer *timer;
};

class Screen : public QFrame {
    Q_OBJECT
public:
    enum { MaxSamples = 400, Range = 100, FrameWidth = 3 };
    Screen( QWidget *parent=0, const char *name=0 );

    void animate();

public slots:
    void setStep( int s );

protected:
    virtual void drawContents( QPainter * );

private:
    int yval[ MaxSamples ];
    int pos0; // buffer pointer for x == 0
    int t0;   // time parameter at x == 0
    int step;
};

class Curve : public QFrame {
    Q_OBJECT
    enum { FrameWidth = 3, Radius = 100 };
public:
    Curve( QWidget *parent=0, const char *name=0 );

    void animate();
public slots:
    void setFactor( int );

protected:
    virtual void drawContents( QPainter * );

private:
    int shift, n;
};

#endif // PLOT_H
