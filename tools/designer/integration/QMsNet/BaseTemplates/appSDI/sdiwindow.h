#include "mainwindow.h"

class QTextEdit;
class SDIWindow : public MainWindow
{
    Q_OBJECT
public:
    SDIWindow( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );

protected:
    QTextEdit *doc;
};
