#ifndef PLUGMAINWINDOW_H
#define PLUGMAINWINDOW_H

#include <qmainwindow.h>
#include <qdict.h>

class QHBox;
class QScrollView;
class QWidgetPlugInManager;
class QPopupMenu;

class PlugMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlugMainWindow( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );

public slots:
    void fileOpen();
    void fileClose();
    void runWidget( int );
    void runAction( int );

protected:
    QPopupMenu* actionMenu;
    QPopupMenu* widgetMenu;
    QHBox *box;
    QScrollView *sv;
    QWidgetPlugInManager* manager;

private:
    QDict<int> menuIDs;
};

#endif // PLUGMAINWINDOW_H
