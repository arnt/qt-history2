#ifndef QWIDGETRESIZEHANDLER_H
#define QWIDGETRESIZEHANDLER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QMouseEvent;
class QKeyEvent;

class QWidgetResizeHandler : public QObject
{
    Q_OBJECT
    
public:
    QWidgetResizeHandler( QWidget *parent, QWidget *cw = 0, const char *name = 0 );
    void setActive( bool b ) { active = b; }
    bool isActive() const { return active; }
    bool isButtonDown() const { return buttonDown; }
    
    void setExtraHeight( int h ) { extrahei = h; }
    
    void doResize();
    void doMove();

signals:
    void activate();
    
protected:
    bool eventFilter( QObject *o, QEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    
private:
    enum MousePosition {
	Nowhere,
	TopLeft, BottomRight, BottomLeft, TopRight,
	Top, Bottom, Left, Right,
	Center
    };

    QWidget *widget;
    QWidget *childWidget;
    bool buttonDown;
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    MousePosition mode;
    bool moveResizeMode;
    bool active;
    int extrahei;
    
    void setMouseCursor( MousePosition m );
    bool isMove() const {
	return moveResizeMode && mode == Center;
    }
    bool isResize() const {
	return moveResizeMode && !isMove();
    }
    
};

#endif
