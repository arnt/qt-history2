#include <qwidget.h>
class QWidgetStack;

class Main : public QWidget {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);

public slots:
    void raise42();
    void raise69();
    
private:
    QWidgetStack * ws;
};
