#ifndef PLUGMAINWINDOW_H
#define PLUGMAINWINDOW_H

#include <qmainwindow.h>
#include <qmap.h>

#include <qapplication.h>
#include <qguardedptr.h>
#include "qapplicationinterfaces.h"

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
    ~PlugMainWindow();

public slots:
    void fileOpen();
    void fileClose();
    void runWidget( int );

protected:
    bool addAction( QAction* );

    QPopupMenu* actionMenu;
    QPopupMenu* widgetMenu;
    QGuardedPtr<QPopupMenu> pluginMenu;
    QGuardedPtr<QToolBar> pluginTool;
    QHBox *box;
    QScrollView *sv;
    QWidgetPlugInManager* widgetManager;
    QActionPlugInManager* actionManager;

private:
    QMap<int, QString> menuIDs;
};

class PlugMainWindowInterface : public QApplicationInterface
{
public:
    PlugMainWindow* mainWindow() const;

    void requestProperty( const QCString&, QVariant& );
    void requestSetProperty( const QCString&, const QVariant& );
};

class PlugApplication : public QApplication
{
public:
    PlugApplication( int argc, char** argv )
	: QApplication( argc, argv )
    {
	mwIface = 0;
    }

    QApplicationInterface* requestApplicationInterface( const QCString& request );
    QStrList queryInterfaceList() const;

protected:
    PlugMainWindowInterface* mwIface;
};

#endif // PLUGMAINWINDOW_H
