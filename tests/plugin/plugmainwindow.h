#ifndef PLUGMAINWINDOW_H
#define PLUGMAINWINDOW_H

#include <qmainwindow.h>
#include <qdict.h>

class QHBox;
class QScrollView;
class QWidgetPlugInManager;
class QActionPlugInManager;
class QPopupMenu;
class QToolBar;
class QAction;

class PlugMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlugMainWindow( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );

public slots:
    void fileOpen();
    void fileClose();
    void runWidget( int );

protected:
    bool addAction( QAction* );

    QPopupMenu* actionMenu;
    QPopupMenu* widgetMenu;
    QPopupMenu* pluginMenu;
    QToolBar* pluginTool;
    QHBox *box;
    QScrollView *sv;
    QWidgetPlugInManager* widgetManager;
    QActionPlugInManager* actionManager;

private:
    QDict<int> menuIDs;
};

#endif // PLUGMAINWINDOW_H
