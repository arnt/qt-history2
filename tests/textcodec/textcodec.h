#include <qlabel.h>

class QTextCodec;

class Main : public QLabel {
    Q_OBJECT
    QString hex;
    QTextCodec *codec;
public:
    Main(const char* enc, QWidget* parent=0, const char* name=0, int f=0);
    void keyPressEvent(QKeyEvent*);
};
