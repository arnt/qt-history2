#include "../../../qactioninterface.h"
#include "../../../qcleanuphandler.h"

#include <qaction.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qdialog.h>
#include <qapplication.h>

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
    TestInterface();
    QString name() { return "Test Actionplugin"; }
    QString description() { return "Test implementation of the QActionInterface"; }
    QString author() { return "vohi"; }

    QStringList featureList();
    QAction* create( const QString &actionname, QObject* parent = 0 );

    QCleanUpHandler<QAction>* actions;

public slots:
    void onOpenModalDialog();
    void onOpenNonmodalDialog();
    void openFile();
};

TestInterface* that;

TestInterface::TestInterface()
{
    that = this;
    actions = new QCleanUpHandler<QAction>();
}


QStringList TestInterface::featureList()
{
    QStringList list;

    list << "Open Modal Dialog";
    list << "Open non-Modal Dialog";
    list << "Open file Dialog";

    return list;
}

QAction* TestInterface::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "Open Modal Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open &modal dialog", Qt::CTRL + Qt::Key_M, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenModalDialog()) );
	actions->addCleanUp( a );
	return a;
    } else if ( actionname == "Open non-Modal Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open &non-modal dialog", Qt::CTRL + Qt::Key_N, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenNonmodalDialog()) );
	actions->addCleanUp( a );
	return a;
    } else if ( actionname == "Open file Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open file-dialog", Qt::CTRL + Qt::Key_F, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(openFile()) );
	actions->addCleanUp( a );
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

void TestInterface::openFile()
{
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QActionInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect( QApplication* myapp )
{
    qDebug("I've been loaded by %p", myapp );
    return TRUE;
}

LIBEXPORT bool onDisconnect( QApplication* myapp )
{
    if ( myapp ) {
	delete that->actions;
        qDebug("I've been unloaded by %p", myapp);
    } else {
	qDebug("I've been unloaded by operation system" );
    }
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "main.moc"
