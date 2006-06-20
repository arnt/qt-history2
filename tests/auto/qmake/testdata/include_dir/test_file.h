#include <qwidget.h>

#include "ui_untitled.h"

class SomeObject : public QWidget, public Ui_Form
{
    Q_OBJECT
public:
    SomeObject();
signals:
    void someSignal();
};
