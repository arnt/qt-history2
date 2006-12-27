/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWINDOWSVISTASTYLE_P_H
#define QWINDOWSVISTASTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qwindowsvistastyle.h"

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
#include <private/qwindowsxpstyle_p.h>
#include <private/qpaintengine_raster_p.h>
#include <qlibrary.h>
#include <qpaintengine.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qpixmapcache.h>
#include <qstyleoption.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qprogressbar.h>
#include <qdockwidget.h>

#if !defined(SCHEMA_VERIFY_VSSYM32)
#define TMT_ANIMATIONDURATION	    5006
#define TMT_TRANSITIONDURATIONS	    6000
#define	EP_EDITBORDER_NOSCROLL      6
#define	PBS_DEFAULTED_ANIMATING     6
#define	MBI_NORMAL                  1
#define	MBI_HOT                     2
#define	MBI_PUSHED                  3
#define	MBI_DISABLED                4
#define PP_FILL                     5
#define PP_FILLVERT                 6
#define PP_MOVEOVERLAY              8
#define PP_MOVEOVERLAYVERT          10
#define MENU_BARBACKGROUND          7
#define MENU_BARITEM                8
#define MENU_POPUPCHECK             11
#define MENU_POPUPCHECKBACKGROUND   12
#define MENU_POPUPGUTTER            13
#define MENU_POPUPITEM              14
#define MENU_POPUPSEPARATOR         15
#define MC_CHECKMARKNORMAL          1
#define MC_CHECKMARKDISABLED        2
#define MC_BULLETNORMAL             3
#define MC_BULLETDISABLED           4
#define ABS_UPHOVER                 17
#define ABS_DOWNHOVER               18
#define ABS_LEFTHOVER               19
#define ABS_RIGHTHOVER              20
#define CP_DROPDOWNBUTTONRIGHT      6
#define CP_DROPDOWNBUTTONLEFT       7
#define SCRBS_HOVER                 5
#define SPI_GETCLIENTAREAANIMATION  0x1042
#endif

class Animation
{
public :
    Animation() : _running(true) { }
    virtual ~Animation() { }
    QWidget * widget() const { return _widget; }
    bool running() const { return _running; }
    const QTime &startTime() const { return _startTime; }
    void setRunning(bool val) { _running = val; }
    void setWidget(QWidget *widget) { _widget = widget; }
    void setStartTime(const QTime &startTime) { _startTime = startTime; }
    virtual void paint(QPainter *painter, const QStyleOption *option);

protected:
    void drawBlendedImage(QPainter *painter, QRect rect, float value);
    QTime _startTime;
    QPointer<QWidget> _widget;
    QImage _primaryImage;
    QImage _secondaryImage;
    QImage _tempImage;
    bool _running;
};


// Handles state transition animations
class Transition : public Animation
{
public :
    Transition() : Animation() {}
    virtual ~Transition() { }
    void setDuration(int duration) { _duration = duration; }
    void setStartImage(const QImage &image) { _primaryImage = image; }
    void setEndImage(const QImage &image) { _secondaryImage = image; }
    virtual void paint(QPainter *painter, const QStyleOption *option);
    int duration() const { return _duration; }
    int _duration; //set time in ms to complete a state transition
};


// Handles pulse animations (default buttons)
class Pulse: public Animation
{
public :
    Pulse() : Animation() {}
    virtual ~Pulse() { }
    void setDuration(int duration) { _duration = duration; }
    void setPrimaryImage(const QImage &image) { _primaryImage = image; }
    void setAlternateImage(const QImage &image) { _secondaryImage = image; }
    virtual void paint(QPainter *painter, const QStyleOption *option);
    int duration() const { return _duration; }
    int _duration; //time in ms to complete a pulse cycle
};


class QWindowsVistaStylePrivate :  public QObject, public QWindowsXPStylePrivate
{
    Q_DECLARE_PUBLIC(QWindowsVistaStyle)

public:
    QWindowsVistaStylePrivate() : 
        QWindowsXPStylePrivate(), timerId(-1)
    {   
        resolveSymbols();
    }

    static bool resolveSymbols();
    static inline bool useVista();
    void startAnimation(Animation *);
    void stopAnimation(const QWidget *);
    Animation* widgetAnimation(const QWidget *) const;
    void timerEvent(QTimerEvent *);
    bool transitionsEnabled() const;

private:
    int timerId;
    QList <Animation*> animations;
};

#endif // QT_NO_STYLE_WINDOWSVISTA
#endif // QWINDOWSVISTASTYLE_P_H
