#include "../../../qactioninterface.h"

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
    TestInterface() 
	: iface(0) {}

    ~TestInterface()
    {
	delete iface;
	iface = 0;
    }

    QString name() { return "Test Actionplugin"; }
    QString description() { return "Test implementation of the QActionInterface"; }
    QString author() { return "vohi"; }

    QStringList featureList();
    QAction* create( const QString &actionname, QObject* parent = 0 );
    QApplicationInterface* appInterface() { return iface ? iface : ( iface = new QApplicationInterface() ); }

public slots:
    void onOpenModalDialog();
    void onOpenNonmodalDialog();

protected:
    QApplicationInterface* iface;
};

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
	return a;
    } else if ( actionname == "Open non-Modal Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open &non-modal dialog", Qt::CTRL + Qt::Key_N, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenNonmodalDialog()) );
	return a;
    } else if ( actionname == "Open file Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open file-dialog", Qt::CTRL + Qt::Key_F, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), appInterface(), SIGNAL( openFile() ) );
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
    return new TestInterface();;
}

LIBEXPORT bool onConnect( QApplication* myapp )
{
    qDebug("I've been loaded by %p", myapp );
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

#include "main.moc"
