#include "../../qactioninterface.h"

#include <qaction.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qdialog.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class TestInterface : public QObject, public QActionInterface
{
    Q_OBJECT
public:
    QString queryInterface() { return "QActionInterface"; }

    QString name() { return "Test Actionplugin"; }
    QString description() { return "Test implementation of the QActionInterface"; }
    QString author() { return "vohi"; }

    QStringList actions();
    QAction* create( const QString &actionname, bool& self, QObject* parent = 0 );

public slots:
    void onOpenModalDialog();
    void onOpenNonmodalDialog();
};

QStringList TestInterface::actions()
{
    QStringList list;

    list << "Open Modal Dialog";
    list << "Open non-Modal Dialog";

    return list;
}

QAction* TestInterface::create( const QString& actionname, bool &self, QObject* parent )
{
    if ( actionname == "Open Modal Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open &modal dialog", Qt::CTRL + Qt::Key_M, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenModalDialog()) );
	if ( self ) {
	    if ( parent->inherits("QMainWindow") ) {
		QMainWindow* mw = (QMainWindow*)parent;
		QPopupMenu* popup = new QPopupMenu( mw );
		a->addTo( popup );
		
		QToolBar* toolbar = new QToolBar( mw );
		mw->addToolBar( toolbar, "Dialogs" );
		toolbar->show();
		a->addTo( toolbar );

		mw->menuBar()->insertItem( "&Dialogs", popup );
	    }
	}
	return a;
    } else if ( actionname == "Open non-Modal Dialog" ) {
	self = FALSE;
	QAction* a = new QAction( actionname, QIconSet(), "Open &non-modal dialog", Qt::CTRL + Qt::Key_N, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenNonmodalDialog()) );
	return a;
    } else 
	return 0;
}

void TestInterface::onOpenModalDialog()
{
    QDialog dialog( 0, 0, TRUE );
    dialog.show();
}

void TestInterface::onOpenNonmodalDialog()
{
    QDialog *dialog = new QDialog( 0, 0, FALSE );
    dialog->show();
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QActionInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect()
{
    qDebug("I've been loaded!");
    return TRUE;
}

LIBEXPORT bool onDisconnect()
{
    qDebug("I've been unloaded!");
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "action.moc"
