
#include "qimpen/qimpeninput.h"

class QWSServer;

class QWSPenInput : public QIMPenInput
{
    Q_OBJECT
public:
    QWSPenInput( QWidget *parent, const char *name, int WFlags );

protected slots:
    void keyPress( unsigned int unicode );
};

