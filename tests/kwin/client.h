#ifndef CLIENT_H
#define CLIENT_H

#include "options.h"
#include <qframe.h>
#include <qvbox.h>
#include <qpixmap.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class Workspace;


class KWM
{
public:
    static QPixmap miniIcon(Window w, int width=0, int height=0);
    static QPixmap icon(Window w, int width=0, int height=0);
};

class WindowWrapper : public QWidget
{
    Q_OBJECT
public:
    WindowWrapper( WId w, QWidget *parent=0, const char* name=0);
    ~WindowWrapper();

    inline WId window() const;
    void invalidateWindow();
    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent* );
    void hideEvent( QHideEvent* );
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    bool x11Event( XEvent * );		// X11 event

private:
    WId win;
    Time lastMouseEventTime;
};

inline WId WindowWrapper::window() const
{
    return win;
}



class Client : public QWidget
{
    Q_OBJECT
public:
    Client( Workspace *ws, WId w, QWidget *parent=0, const char *name=0, WFlags f = 0);
    ~Client();

    inline WId window() const;
    inline WindowWrapper* windowWrapper() const;
    inline Workspace* workspace() const;
    void invalidateWindow();
    inline WId transientFor() const;

    virtual bool windowEvent( XEvent * );

    void manage( bool isMapped = FALSE );
    void setMappingState( int s );
    int mappingState() const;

    void requestActivation();
    void withdraw();

    QSize adjustedSize( const QSize& ) const;
    QSize minimumSize() const;
    int minimumWidth() const;
    int minimumHeight() const;
    QSize maximumSize() const;
    int maximumWidth() const;
    int maximumHeight() const;

    inline QPixmap icon() const;
    inline QPixmap miniIcon() const;


    // is the window in withdrawn state?
    bool isWithdrawn(){
	return state == WithdrawnState;
    }
    // is the window in iconic state?
    bool isIconified(){
	return state == IconicState;
    }
    // is the window in normal state?
    bool isNormal(){
	return state == NormalState;
    }

    inline bool isActive() const;
    void setActive( bool );

    int desktop() const;
    bool isOnDesktop( int d ) const;

    bool isShade() const;
    virtual void setShade( bool );

    enum MaximizeMode { MaximizeVertical, MaximizeHorizontal, MaximizeFull };
public slots:
    void iconify();
    void maximize( MaximizeMode);

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void moveEvent( QMoveEvent * );
    void showEvent( QShowEvent* );
    void hideEvent( QHideEvent* );
    bool x11Event( XEvent * );		// X11 event

    bool eventFilter( QObject *, QEvent * );

    virtual void captionChange( const QString& name );

    virtual void activeChange( bool );


    enum MousePosition {
	Nowhere, TopLeft , BottomRight, BottomLeft, TopRight, Top, Bottom, Left, Right, Center
    };

    virtual MousePosition mousePosition( const QPoint& ) const;
    virtual void setMouseCursor( MousePosition m );

    // handlers for X11 events
    bool mapRequest( XMapRequestEvent& e );
    bool unmapNotify( XUnmapEvent& e );
    bool configureRequest( XConfigureRequestEvent& e );
    bool propertyNotify( XPropertyEvent& e );

private:
    QSize sizeForWindowSize( const QSize&, bool ignore_height = FALSE ) const;
    void getWmNormalHints();
    void fetchName();
    void gravitate( bool invert );


    WId win;
    WindowWrapper* wwrap;
    Workspace* wspace;
    int desk;
    bool buttonDown;
    MousePosition mode;
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    QSize clientSize;
    XSizeHints  xSizeHint;
    void sendSynteticConfigureNotify();
    int state;
    bool active;
    int ignore_unmap;
    QRect original_geometry;
    bool shaded;
    WId transient_for;
    void getIcons();
    QPixmap icon_pix;
    QPixmap miniicon_pix;
};

inline WId Client::window() const
{
    return win;
}

inline WindowWrapper* Client::windowWrapper() const
{
    return wwrap;
}

inline Workspace* Client::workspace() const
{
    return wspace;
}

inline WId Client::transientFor() const
{
    return transient_for;
}

inline int Client::mappingState() const
{
    return state;
}


inline bool Client::isActive() const
{
    return active;
}

/*!
  Returns the virtual desktop within the workspace() the client window
  is located in, -1 if it isn't located on any special desktop. This may be
  if the window wasn't mapped yet or if the window is sticky. Do not use
  desktop() directly, use isOnDesktop() instead.
 */
inline int Client::desktop() const
{
    return desk;
 }

/*!
  Returns whether the client is on visible or iconified on the virtual
  desktop \a d. This is always TRUE for sticky clients.
 */
inline bool Client::isOnDesktop( int d ) const
{
    return desk == d; // TODO sticky
}


inline QPixmap Client::icon() const
{
    return icon_pix;
}

inline QPixmap Client::miniIcon() const
{
    return miniicon_pix;
}


class NoBorderClient : public Client
{
    Q_OBJECT
public:
    NoBorderClient( Workspace *ws, WId w, QWidget *parent=0, const char *name=0 );
    ~NoBorderClient();
};

#endif
