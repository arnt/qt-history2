

#include <qmainwindow.h>

class QVFbView;
class QVFbRateDialog;
class QPopupMenu;

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    QVFb( int w, int h, int d, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFb();

    void enableCursor( bool e );

protected slots:
    void slotCursor();
    void slotRateDlg();

protected:
    void createMenu();

protected:
    QVFbView *view;
    QVFbRateDialog *rateDlg;
    QPopupMenu *viewMenu;
    int cursorId;
};

