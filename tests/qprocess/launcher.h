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

private slots:
    void nextInfo();
    void execute();
    void executeOther( int i );

private:
    QLabel* info;
};
