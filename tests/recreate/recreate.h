#include <qlabel.h>

class Main : public QLabel {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
public slots:
    void bang();
    void pang();
    void ting();
};
