#include <qwidget.h>

class QLineEdit;
class QLabel;

class Main : public QWidget {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);

public slots:
    void bang();
    void bop();
    void ratatatat();
    void whoosh();
    void boom();
    void platsch();
    void tusch();

private:
    QLabel *lab;
    QLineEdit* initEd;
    QLineEdit* filtEd;

};
