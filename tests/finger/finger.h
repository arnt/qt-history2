#include <qwidget.h>

class QLineEdit;
class QSocket;
class QTextStream;
class QMultiLineEdit;


class Main : public QWidget {
    Q_OBJECT
public:
    Main( QWidget* parent=0, const char* name=0, int f=0 );

private slots:
    void dataArrived();
    void timeToUpdate();
    void makeConnection();

private:
    QLineEdit * input;
    QSocket * socket;
    QTextStream * s;
    QMultiLineEdit * output;
};
