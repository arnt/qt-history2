#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qgroupbox.h>

class Launcher : public QHBox {
    Q_OBJECT
public:
    Launcher();

private:
    void run( const char* path, const char* cmd );
    void showSource( const char* path );

private slots:
    void nextInfo();
    void execute();
    void executeOther( int i );
    void source();
    void sourceOther( QListBoxItem *, const QPoint & );

private:
    QLabel* info;
};
