#include <qapplication.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qdir.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qwidgetstack.h>

#include "qinstallationwizard.h"
#include "data.h"

// Interfaces
#include "interface/welcome.h"
#include "interface/license.h"
#include "interface/destination.h"
#include "interface/configuration.h"
#include "interface/customize.h"
#include "interface/review.h"
#include "interface/install.h"

#ifndef Q_OS_WIN32
#include <sys/vfs.h> // needed to get free space
#else
// Need to find out how to do this on windows
#include <windows.h>
#endif


QString fileSizeToString( unsigned long long size )
{
    QString suffix = "bytes";
    if ( size > 10*1024*1024*1024*1024ULL ) {
	size /= 1024*1024*1024*1024ULL;
	suffix = "GBytes";
    } else if ( size > 10*1024*1024 ) {
	size /= 1024*1024;
	suffix = "MBytes";
    } else if ( size > 10*1024 ) {
	size /= 1024;
	suffix = "kBytes";
    }
    QString sizeStr;
    sizeStr.sprintf("%lu %s", (unsigned long)size, suffix.latin1() );
    return sizeStr;
}


unsigned long long freeSpace( const QString& path )
{
#ifndef Q_OS_WIN32
    struct statfs buf;
    QDir d( path );
    d.cdUp();
    statfs( QDir::cleanDirPath( d.absPath() ).latin1(), &buf );
    return (unsigned long long)buf.f_bsize * buf.f_bavail;
#else
    // ### Need to fill this in for Windows
    return GetFreeSpace();
#endif
}


class Component;


class ComponentListItem : public QCheckListItem {
    public:
	ComponentListItem( Component *comp, ComponentListItem *parentItem, const QString& name );
	ComponentListItem( Component *comp, QListView *listView, const QString& name );
	virtual void stateChange( bool on );
	virtual void setSelected( bool on );
    private:
	Component *component;
};

    
class Component /* : public QCheckListItem */ {
    public:
	Component();
	Component( const QString& name, const QString& desc, const QString& attr, const QString& filesStr, Component *parentItem, QListView *listView );
	bool isComponentInstalled( const QString& configType );
	void changeFeature( Component *feature, bool select );
	static void calcRequiredSize( Component *parent, unsigned long long *requiredSize, const QString& configType );
	static void updateRequiredSize( Component *parent, const QString& configType );
	QString name;
	QString description;
	ComponentListItem *viewItem;
	QString attribute;
	QStringList files;
	QPtrList<Component> children;
	Component *parent;
	unsigned long long requiredSize;
};


ComponentListItem::ComponentListItem( Component *comp, ComponentListItem *parentItem, const QString& name )
    : QCheckListItem( parentItem, name, QCheckListItem::CheckBox ), component( comp )
{
}


ComponentListItem::ComponentListItem( Component *comp, QListView *listView, const QString& name )
    : QCheckListItem( listView, name, QCheckListItem::CheckBox ), component( comp )
{
}


Customize *customizeWidget;
void ComponentListItem::setSelected( bool on )
{
    if ( on )
	customizeWidget->componentDescription->setText( component->description );    
}


void ComponentListItem::stateChange( bool on )
{
    static bool inProgress = FALSE;
    if ( inProgress )
	return;
    inProgress = TRUE;
    if ( component->attribute == "feature" )
	component->changeFeature( component, on );
    else {
	Component *parent = component->parent;
	while ( parent ) {
	    if ( parent->attribute == "feature" ) {
		bool parentOn = TRUE;
		Component *comp = parent->children.first();
		while ( comp ) {
		    if ( comp->attribute != "hidden" && comp->attribute != "required" && !comp->viewItem->isOn() )
			parentOn = FALSE;
		    comp = parent->children.next();
		}
		if ( parent->viewItem )
		   parent->viewItem->setOn( parentOn );
	    }
	    parent = parent->parent;
	}
    }

    // Update the required size display
    Component::updateRequiredSize( component, "Custom" );
 
    inProgress = FALSE;
}


Component::Component() : viewItem( 0 ), parent( 0 ), requiredSize( 0 )
{
}


Component::Component( const QString& nameStr, const QString& desc, const QString& attr, const QString& filesStr, Component *par, QListView *listView )
    : name( nameStr ), description( desc ), viewItem( 0 ), attribute( attr ), files( QStringList::split( ";", filesStr ) ), parent( par ), requiredSize( 0 )
{
    if ( parent->viewItem )
	viewItem = new ComponentListItem( this, parent->viewItem, name );
    else
	viewItem = new ComponentListItem( this, listView, name );

    for ( unsigned int i = 0; i < files.count(); i++ )
	requiredSize += embeddedDataSize( files[i] ); 
}


void Component::changeFeature( Component *feature, bool select )
{
    if ( feature->attribute != "hidden" && feature->attribute != "required" )
	feature->viewItem->setOn( select );
    Component *comp = feature->children.first();
    while ( comp ) {
	changeFeature( comp, select );
	comp = feature->children.next();
    }
}


bool Component::isComponentInstalled( const QString& configType )
{
    bool compSelected = FALSE;

    // For all configuration types	
    if ( attribute == "required" || attribute == "hidden" )
	compSelected = TRUE;

    // Certain other configurations add additional other component types	
    if ( configType == "Typical" ) {
	if ( attribute == "default" )
	    compSelected = TRUE;
    } else if ( configType == "Custom" ) {
	if ( viewItem->isOn() )
	    compSelected = TRUE;
    } else if ( configType == "Complete" ) {
	compSelected = TRUE;
    }

    return compSelected;
}


void Component::updateRequiredSize( Component *parent, const QString& configType )
{
    // find top level component
    while ( parent->parent )
	parent = parent->parent;
    unsigned long long requiredSize = 0;
    // Get total size
    calcRequiredSize( parent, &requiredSize, configType );
    // Update display of size requried
    customizeWidget->diskSpaceRequired->setText( fileSizeToString( requiredSize ) );    
}


void Component::calcRequiredSize( Component *parent, unsigned long long *requiredSize, const QString& configType )
{
    Component *comp = parent->children.first();
    while ( comp ) {
	if ( comp->isComponentInstalled( configType ) )
	    *requiredSize += comp->requiredSize;
	calcRequiredSize( comp, requiredSize, configType );
	comp = parent->children.next();
    }
}


QInstallationWizard::QInstallationWizard( QWidget *parent, QProgressBar *pb, const QDomDocument& setupData ) : InstallationWizard( parent )
{
    lastId = 0;
    progress = 10;
    progressBar = pb;

    connect( NextButton, SIGNAL( clicked() ), this, SLOT( next() ) );
    connect( BackButton, SIGNAL( clicked() ), this, SLOT( back() ) );
    connect( CancelButton, SIGNAL( clicked() ), this, SLOT( cancel() ) );
    BackButton->setEnabled( FALSE );
    
    // Create the Installation Wizard 
    QGridLayout *l = new QGridLayout( Frame );
    stack = new QWidgetStack( Frame );
    l->addWidget( stack, 0, 0 );
    advanceProgressBar();
    
    QDomElement data;

    // Create Welcome Page
    data = setupData.elementsByTagName("Welcome").item(0).toElement();
    welcome = new Welcome( stack );
    welcome->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    welcome->title->setText( data.attribute( "title" ) );
    welcome->instructions->setText( data.attribute( "instructions" ) );
    addWidgetToStack( welcome );
    advanceProgressBar();

    // Create License Page
    data = setupData.elementsByTagName("License").item(0).toElement();
    license = new License( stack );
    license->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    license->title->setText( data.attribute( "title" ) );
    license->instructions->setText( data.attribute( "instructions" ) );
    license->file->setText( embeddedData( data.attribute( "file" ) ) );
    license->question->setText( data.attribute( "question" ) );
    addWidgetToStack( license, "Yes" );
    advanceProgressBar();
   
    // Create Target Page
    data = setupData.elementsByTagName("Destination").item(0).toElement();
    destination = new Destination( stack ); 
    destination->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    destination->title->setText( data.attribute( "title" ) );
    destination->instructions->setText( data.attribute( "instructions" ) );
    // ### Could be platform specific, perhaps need a platform base path to use then add a path from the program name
    // However this could be a plugin (eg a kicker plugin) or something special which needs to go to a specific path
    // in which case a specific full path might be needed instead. Need a way to either make it automatic or if one
    // is specified it is obviously needed.
    destination->destination->setText( data.attribute( "destination" ) );

    addWidgetToStack( destination );
    advanceProgressBar();
    
    // Create Configuration Page
    data = setupData.elementsByTagName("Configuration").item(0).toElement();
    configuration = new Configuration( stack ); 
    configuration->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    configuration->title->setText( data.attribute( "title" ) );
    configuration->instructions->setText( data.attribute( "instructions" ) );
    configuration->option1->setText( data.attribute( "option1" ) );
    configuration->option2->setText( data.attribute( "option2" ) );
    configuration->option3->setText( data.attribute( "option3" ) );
    configuration->option4->setText( data.attribute( "option4" ) );
    configuration->description1->setText( data.attribute( "description1" ) );
    configuration->description2->setText( data.attribute( "description2" ) );
    configuration->description3->setText( data.attribute( "description3" ) );
    configuration->description4->setText( data.attribute( "description4" ) );
    addWidgetToStack( configuration );
    advanceProgressBar();
    
    // Create Customize Page
    data = setupData.elementsByTagName("Customize").item(0).toElement();
    customize = new Customize( stack ); 
    customizeWidget = customize;
    customize->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    customize->title->setText( data.attribute( "title" ) );
    customize->instructions->setText( data.attribute( "instructions" ) );
    customize->componentTree->header()->hide();
    features = new Component;
    buildTree( setupData.elementsByTagName("ComponentTree").item(0).toElement(), features );
    Component::updateRequiredSize( features, "Custom" );
    addWidgetToStack( customize );
    advanceProgressBar();
    
    // Create Review Page
    data = setupData.elementsByTagName("Review").item(0).toElement();
    review = new Review( stack ); 
    review->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    review->title->setText( data.attribute( "title" ) );
    review->instructions->setText( data.attribute( "instructions" ) );
    addWidgetToStack( review, "Install" );
    advanceProgressBar();
    
    // Create Install Page
    data = setupData.elementsByTagName("Install").item(0).toElement();
    install = new Install( stack ); 
    install->image->setPixmap( QPixmap( embeddedData( data.attribute( "image" ) ) ) );
    install->title->setText( data.attribute( "title" ) );
    install->instructions->setText( data.attribute( "instructions" ) );
    addWidgetToStack( install, "HIDE", "HIDE" );
    advanceProgressBar();
   
    stack->raiseWidget( welcome );
}


void QInstallationWizard::back()
{
    if ( stack->id( stack->visibleWidget() ) == 5 && !configuration->option3->isChecked() )
	setWidget( 3 );
    else
	setWidget( stack->id( stack->visibleWidget() ) - 1 );
}


void QInstallationWizard::buildFileList( Component *parent, QString *components, QString *filelist, const QString& seperator, const QString& configType )
{
    Component *comp = parent->children.first();
    while ( comp ) {
	if ( comp->isComponentInstalled( configType ) ) {
	    if ( comp->attribute != "feature" ) {
		if ( !components->isNull() )
		    *components += seperator; 
		*components += comp->name;
	    }
	    QString files = comp->files.join( seperator );
	    if ( !files.isNull() ) {
		if ( !filelist->isNull() )
		    *filelist += seperator;
		*filelist += files;
	    }
	}	
	buildFileList( comp, components, filelist, seperator, configType );
	comp = parent->children.next();
    }
}


void QInstallationWizard::next()
{
    int nextId;

    if ( stack->id( stack->visibleWidget() ) == 3 && !configuration->option3->isChecked() )
	nextId = 5;
    else
	nextId = stack->id( stack->visibleWidget() ) + 1;

    if ( nextId == 5 ) {

	// find top level component
	Component *parent = features;
	while ( parent->parent )
	    parent = parent->parent;
	unsigned long long freeSize = freeSpace( destination->destination->text() );
	unsigned long long requiredSize = 0;
	QButton *but = configuration->ButtonGroup1->selected();
	if ( but )
	    Component::calcRequiredSize( parent, &requiredSize, but->text() );

	if ( requiredSize >= freeSize ) {
	    QString message;
	    message.sprintf( "<b><red>Warning</red></b>, the required disk space (%s) exceeds the calculated free disk space (%s)."
		    "<br>Please free up some disk space or choose a different configuration option such as 'Compact'."
		    "<br>If you still wish to proceed at your own risk anyway, click 'Yes', otherwise click 'No' to change the options.",
		    fileSizeToString( requiredSize ).latin1(), fileSizeToString( freeSize ).latin1() );
	    QMessageBox mb( "Installation Wizard", message, QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton );
	    if ( mb.exec() == QMessageBox::Yes )
		setWidget( nextId );
	    else
		return;
	}

    }    

    setWidget( nextId );
}


void QInstallationWizard::cancel()
{
    QMessageBox mb( "Installation Wizard", "This will quit the installation immediately. Are you sure you wish to quit?", QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton );
    if ( mb.exec() == QMessageBox::Yes )
	exit(0);    
}


void QInstallationWizard::addWidgetToStack( QWidget *w, const QString& nextText = "Next >", const QString& backText = "< Back" )
{
    backTextMap[lastId] = backText;
    nextTextMap[lastId] = nextText;
    stack->addWidget( w, lastId );
    lastId++;
}


void QInstallationWizard::installFiles()
{

}


void QInstallationWizard::setWidget( int id )
{
    // Customize Configuration Page
    if ( id == 4 ) {
	// Set the available space text based on the set destination directory
	customize->diskSpaceFree->setText( fileSizeToString( freeSpace( destination->destination->text() ) ) );
    }

    // Review Configuration Page
    if ( id == 5 ) {
	QString destdir = destination->destination->text();
	QString config;
	QButton *but = configuration->ButtonGroup1->selected();
	if ( but )
	    config = but->text();
	else 
	    config = "Error determining the configuration type";
	QString filelist, complist;
	QString seperator = "<br>";
	buildFileList( features, &complist, &filelist, seperator, config );
	QString settings = "<b>Destination directory</b><br>" + destdir + "<p>"
	    + "<b>Configuration type</b><br>" + config + "<p>"
//	    + "<b>Components which will be installed</b><br>" + complist + "<p>"
	    + "<b>Files about to be installed</b><br>" + filelist;
	review->settings->setText( settings );
    }
    
    stack->raiseWidget( id );
    NextButton->setText( nextTextMap[ id ] );
    BackButton->setText( backTextMap[ id ] );
    NextButton->setEnabled( id != lastId - 1 );
    BackButton->setEnabled( id != 0 );
    if ( nextTextMap[ id ] == "HIDE" )
	NextButton->hide();
    else
	NextButton->show();
    if ( backTextMap[ id ] == "HIDE" )
	BackButton->hide();
    else
	BackButton->show();
}


void QInstallationWizard::advanceProgressBar()
{
    progress += 10;
    if ( progressBar ) {
	progressBar->setProgress( progress );
	qApp->processEvents();
    }
}


void QInstallationWizard::buildTree( const QDomElement& parentElement, Component *parent )
{
    QDomNode node = parentElement.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() ) {
	    QDomElement e = node.toElement();
	    if ( node.nodeName() == "Component" ) {
		QString attr = e.attribute( "attribute" );
		Component *item = new Component( e.attribute( "name" ), e.attribute( "description" ), attr, e.attribute( "files" ), parent, customize->componentTree );
		parent->children.append( item );	
		item->viewItem->setOpen( attr == "feature" );
		item->viewItem->setOn( ( attr == "default" || attr == "required" ) );
		item->viewItem->setEnabled( attr != "required" );
		item->viewItem->setVisible( attr != "hidden" );
		// recursively build the tree
		buildTree( node.toElement(), item );
	    }
	}
	node = node.nextSibling();
    }
}

