#include <qapplication.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qmetaobject.h>
#include <qsettings.h>

#include <qactivex.h>


class Browser : public QDialog
{
    Q_OBJECT
public:
    Browser( const QString &control = QString::null, QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0 ) 
	: QDialog( parent, name, modal, f )
    {
	QVBoxLayout *dialoglayout = new QVBoxLayout( this );
	QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
	dialoglayout->addWidget( splitter );
	
	QVBox *vbox = new QVBox( splitter );
	activex = new QActiveX( control, vbox );
	activex->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	QHBox *hbox = new QHBox( vbox );

	QComboBox *cbox = new QComboBox( TRUE, hbox );
	cbox->setAutoCompletion( TRUE );
	cbox->setAutoResize( TRUE );

	QSettings controls;
	QStringList clsids = controls.subkeyList( "/Classes/CLSID" );
	for ( QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it ) {
	    QString clsid = *it;
	    QStringList subkeys = controls.subkeyList( "/Classes/CLSID/" + clsid );
	    if ( subkeys.contains( "Control" ) ) {
		QString name = controls.readEntry( "/Classes/CLSID/" + clsid + "/Default" );
		axcontrols.insert( name, clsid );
		cbox->insertItem( name );
	    }
	}
	cbox->listBox()->sort();
	leControl = cbox->lineEdit();
	leControl->selectAll();

	pbInstantiate = new QPushButton( "Create", hbox );

	connect( pbInstantiate, SIGNAL(clicked()), this, SLOT(instantiate()) );
	
	vbox = new QVBox( splitter );
	listview = new QListView( vbox );
	listview->addColumn( "Name" );
	listview->addColumn( "Value" );
	listview->setRootIsDecorated( TRUE );
	hbox = new QHBox( vbox );
	leSlot = new QLineEdit( hbox );
	pbInvoke = new QPushButton( "Invoke", hbox );

	populate();

	connect( pbInvoke, SIGNAL(clicked()), this, SLOT(invoke()) );
	connect( listview, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(invoke(QListViewItem*)) );

	connect( activex, SIGNAL(signal(const QString&,int,void*)), this, SLOT(slot(const QString&,int,void*)) );
	connect( activex, SIGNAL(propertyChanged(const QString&)), this, SLOT(propertyChanged(const QString&)) );
    }

public slots:
    void invoke()
    {
	QString text = leSlot->text();
	activex->dynamicCall( (const char*)text );

	leSlot->setSelection( 0, text.length() );
    }
    void invoke( QListViewItem *item )
    {
	if ( !item )
	    return;

	leSlot->setText( item->text( 0 ) );
	invoke();
    }

    void slot( const QString &name, int argc, void *args )
    {
	qDebug( "Signal \"%s\" emitted!", name.latin1() );
    }
    void propertyChanged( const QString &name )
    {
	qDebug( "Property \"%s\" changed!", name.latin1() );
    }

    void instantiate() 
    {
	pbInvoke->setDefault( TRUE );
	leSlot->setFocus();
	QString name = leControl->text();
	QString ctrl = axcontrols[name];
	if ( ctrl.isEmpty() )
	    ctrl = name;
	activex->setControl( ctrl );
	populate();
    }

    void populate()
    {
	listview->clear();

	QMetaObject *mo = activex->metaObject();
	QListViewItem *item = new QListViewItem( listview, "Class Info", QString::number( mo->numClassInfo() ) );
	for ( int i = 0; i < mo->numClassInfo(FALSE ); ++i ) {
	    const QClassInfo *info = mo->classInfo( i, FALSE );
	    (void)new QListViewItem( item, info->name, info->value );
	}
	item = new QListViewItem( listview, "Signals", QString::number( mo->numSignals( FALSE ) ) );
	for ( i = 0; i < mo->numSignals(FALSE ); ++i ) {
	    const QMetaData *signal = mo->signal( i, FALSE );
	    (void)new QListViewItem( item, signal->name );
	}
	item = new QListViewItem( listview, "Slots", QString::number( mo->numSlots( FALSE ) ) );
	for ( i = 0; i < mo->numSlots( FALSE ); ++i ) {
	    const QMetaData *slot = mo->slot( i, FALSE );
	    (void)new QListViewItem( item, slot->name );
	}
	item = new QListViewItem( listview, "Properties", QString::number( mo->numProperties( FALSE ) ) );    
	for ( i = 0; i < mo->numProperties( FALSE ); ++i ) {
	    const QMetaProperty *property = mo->property( i, FALSE );
	    QString value = activex->property( property->n ).toString();
	    (void)new QListViewItem( item, QString( "%1 %2" ).arg( property->t ).arg( property->n ), value );
	}
    }

private:
    QListView *listview;
    QLineEdit *leSlot;
    QLineEdit *leControl;
    QPushButton *pbInstantiate;
    QPushButton *pbInvoke;
    QActiveX *activex;

    QMap<QString, QString> axcontrols;
};

#include "tmp/moc/debug_mt_shared/main.moc"

int main( int argc, char **argv ) 
{
    QApplication app( argc, argv );

    Browser browser;

    return browser.exec();
}
