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

#include <qwidget.h>

class QImage;
class QTimer;
class QAnimationWriter;
class QLock;
struct QVFbHeader;

class QVFbView : public QWidget
{
    Q_OBJECT
public:
    QVFbView( int display_id, int w, int h, int d, QWidget *parent = 0, Qt::WFlags = 0 );
    ~QVFbView();

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;

    bool touchScreenEmulation() const { return emulateTouchscreen; }
    int rate() { return refreshRate; }
    bool animating() const { return !!animation; }
    QImage image() const;

    void setGamma(double gr, double gg, double gb);
    double gammaRed() const { return gred; }
    double gammaGreen() const { return ggreen; }
    double gammaBlue() const { return gblue; }
    void getGamma(int i, QRgb& rgb);
    void skinKeyPressEvent( QKeyEvent *e ) { keyPressEvent(e); }
    void skinKeyReleaseEvent( QKeyEvent *e ) { keyReleaseEvent(e); }

    double zoom() const { return zm; }

    QSize sizeHint() const;
public slots:
    void setTouchscreenEmulation( bool );

    void setRate( int );
    void setZoom( double );
    void startAnimation( const QString& );
    void stopAnimation();

protected slots:
    void timeout();

protected:
    QImage getBuffer( const QRect &r, int &leading ) const;
    void drawScreen();
    void sendMouseData( const QPoint &pos, int buttons, int wheel );
    void sendKeyboardData( int unicode, int keycode, Qt::KeyboardModifiers modifiers,
			   bool press, bool repeat );
    virtual void paintEvent( QPaintEvent *pe );
    virtual void contextMenuEvent( QContextMenuEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    bool emulateTouchscreen;
    void setDirty( const QRect& );
    int shmId;
    unsigned char *data;
    QVFbHeader *hdr;
    int viewdepth; // "faked" depth
    int rsh;
    int gsh;
    int bsh;
    int rmax;
    int gmax;
    int bmax;
    int contentsWidth;
    int contentsHeight;
    double gred, ggreen, gblue;
    QRgb* gammatable;
    QLock *qwslock;
    QTimer *timer;
    int mouseFd;
    int keyboardFd;
    int refreshRate;
    QString mousePipe;
    QString keyboardPipe;
    QAnimationWriter *animation;
    int displayid;
    double zm;
};

