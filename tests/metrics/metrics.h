#include <qmainwindow.h>
#include <qprinter.h>


class Paper : public QWidget {
    Q_OBJECT
public:
    Paper(QWidget* parent=0, const char* name=0, int f=0);
    ~Paper();

    void paintEvent(QPaintEvent* e);
    void paint(QPainter& p);

public slots:
    void changeFont1();
    void changeFont2();
    void print();
    void sizeToA5();
    void sizeToA4();

private:
    void setRichText();
    QFont font1, font2;
    QPrinter::PageSize ps;
};

class Main : public QMainWindow {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
};
