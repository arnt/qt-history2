

#include <qscrollview.h>

class QImage;
class QTimer;
struct QVFbHeader;

class QVFbView : public QScrollView
{
    Q_OBJECT
public:
    QVFbView( int w, int h, int d, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFbView();

    int rate() { return refreshRate; }

public slots:
    void setRate( int );

protected slots:
    void timeout();

protected:
    void initLock();
    void lock();
    void unlock();
    void drawScreen();
    void sendMouseData( const QPoint &pos, int buttons );
    void sendKeyboardData( int unicode, int keycode, int modifiers,
			   bool press, bool repeat );
    virtual bool eventFilter( QObject *obj, QEvent *e );
    virtual void viewportPaintEvent( QPaintEvent *pe );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    int shmId;
    unsigned char *data;
    QVFbHeader *hdr;
    int lockId;
    QTimer *timer;
    int mouseFd;
    int keyboardFd;
    int refreshRate;
};

