#ifndef PLUGMAINWINDOW_H
#define PLUGMAINWINDOW_H

#include <qmainwindow.h>
#include <qmap.h>

#include <qapplication.h>
#include <qguardedptr.h>
#include <qapplicationinterface.h>

class QHBox;
class QScrollView;
class WidgetPlugInManager;
class QActionPlugInManager;
class QPopupMenu;
class QToolBar;
class QAction;

class PlugMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlugMainWindow( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    ~PlugMainWindow();

public slots:
    void fileOpen();
    void fileClose();
    void runWidget( int );

protected:
    bool addAction( QAction* );

    QPopupMenu* actionMenu;
    QGuardedPtr<QPopupMenu> pluginMenu;
    QGuardedPtr<QToolBar> pluginTool;
    QHBox *box;
    QScrollView *sv;
    WidgetPlugInManager* widgetManager;
    QActionPlugInManager* actionManager;

private:
    QMap<int, QString> menuIDs;
};

class PlugApplicationInterface : public QApplicationInterface
{
public:
    PlugApplicationInterface();
    ~PlugApplicationInterface();

    QComponentInterface* queryInterface( const QCString& );

private:
    QGuardedPtr<QComponentInterface> iMainWindow;
};

class PlugApplication : public QApplication
{
public:
    PlugApplication( int argc, char** argv )
	: QApplication( argc, argv )
    {
	appIface = 0;
    }

    QApplicationInterface* queryInterface();

protected:
    QGuardedPtr<QApplicationInterface> appIface;
};

#endif // PLUGMAINWINDOW_H
