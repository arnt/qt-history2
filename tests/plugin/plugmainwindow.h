#ifndef PLUGMAINWINDOW_H
#define PLUGMAINWINDOW_H

#include <qmainwindow.h>

class PlugMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlugMainWindow( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );

public slots:
    void fileOpen();
};

#endif // PLUGMAINWINDOW_H
