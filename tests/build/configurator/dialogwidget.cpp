/*
** Implementation file for the configurator main dialog widget
*/

#include <qlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <stdlib.h>

#include "dialogwidget.h"

extern QApplication* pApp;

CDialogWidget::CDialogWidget( QWidget* pParent, const char* pName, WFlags f ) :
	QWidget( pParent, pName, f ),
	m_bShared( true ),
	m_bThreaded( false )
{
	QHBoxLayout* pTopLevelLayout;
	QVBox* pProjectSettingsBox;
	QHBox* pButtonBox;

	QButtonGroup* pThreadGroup;
	QButtonGroup* pLibGroup;
	QGroupBox* pConfigGroup;
	QGroupBox* pOutputGroup;
	QRadioButton* pThreaded;
	QRadioButton* pNonThreaded;
	QRadioButton* pSharedLib;
	QRadioButton* pStaticLib;
	QLabel* pOutNameLabel;
	QLabel* pOutFormatLabel;
	QPushButton* pCloseButton;
	QPushButton* pGenerateButton;

/*
** Create widgets and layouts
*/
	pTopLevelLayout = new QHBoxLayout( this );
	pConfigGroup = new QGroupBox( 1, Qt::Horizontal, "Available configurations", this );
	m_pConfigView = new CConfigView( pConfigGroup );
	pProjectSettingsBox = new QVBox( this);
	pThreadGroup = new QButtonGroup( 1, Qt::Horizontal, "Threading options", pProjectSettingsBox );
	pNonThreaded = new QRadioButton( "Non-threaded", pThreadGroup );
	pThreaded = new QRadioButton( "Threaded", pThreadGroup );
	pThreadGroup->insert( pNonThreaded, 0 );
	pThreadGroup->insert( pThreaded, 1 );
	pLibGroup = new QButtonGroup( 1, Qt::Horizontal, "Linkage options", pProjectSettingsBox );
	pStaticLib = new QRadioButton( "Static library", pLibGroup );
	pSharedLib = new QRadioButton( "Shared library", pLibGroup );
	pLibGroup->insert( pStaticLib, 0 );
	pLibGroup->insert( pSharedLib, 1 );
	pOutputGroup = new QGroupBox( 2, Qt::Horizontal, "Output options", pProjectSettingsBox );
	pOutNameLabel = new QLabel( "Output filename", pOutputGroup );
	m_pOutNameEdit = new QLineEdit( "Makefile", pOutputGroup );
	pOutFormatLabel = new QLabel( "Output format", pOutputGroup );
	m_pOutFormat = new QComboBox( false, pOutputGroup );
	m_pOutFormat->insertItem( "Makefile", 0 );
	m_pOutFormat->insertItem( "Visual Studio project", 1 );
	pButtonBox = new QHBox( pProjectSettingsBox );
	pGenerateButton = new QPushButton( "Generate", pButtonBox );
	pCloseButton = new QPushButton( "Close", pButtonBox );

	pTopLevelLayout->setSpacing( 3 );
	pTopLevelLayout->addWidget( pConfigGroup );
	pTopLevelLayout->addWidget( pProjectSettingsBox );
	
/*
** Set the initial default state
*/
	pSharedLib->setChecked( true );
	pNonThreaded->setChecked( true );

/*
** Do the plumbing work
*/
	connect( pCloseButton, SIGNAL( clicked() ), (QObject*)pApp, SLOT( closeAllWindows() ) );
	connect( pGenerateButton, SIGNAL( clicked() ), this, SLOT( generate() ) );
	connect( pLibGroup, SIGNAL( clicked( int ) ), this, SLOT( clickedLib( int ) ) );
	connect( pThreadGroup, SIGNAL( clicked( int ) ), this, SLOT( clickedThread( int ) ) );
}

CDialogWidget::~CDialogWidget( )
{
}

void CDialogWidget::generate()
{
	QStringList* pModules;
	QString strQMake;

	pModules = m_pConfigView->activeModules();

	strQMake = "qmake qt";

	if ( !pModules->isEmpty() || m_bThreaded )
	{
		strQMake += " \"CONFIG+=";
		for ( QStringList::Iterator it = pModules->begin(); it != pModules->end(); ++it )
		{
			strQMake += *it + " ";
		}

		if ( m_bThreaded )
		{
			strQMake += "thread ";
		}

		strQMake.truncate( strQMake.length() - 1 ); // Remove the trailing space
		strQMake += "\""; // Terminate the CONFIG+= part
	}

	if ( m_bShared )
	{
		strQMake += " \"DEFINES+=QT_MAKEDLL\"";
	}

	if ( m_pOutFormat->currentItem() )
	{
		strQMake += " -win32 -t vclib";
	}
	strQMake += " -o " + m_pOutNameEdit->text();
	qDebug( strQMake );
	system( strQMake.latin1() );
}

void CDialogWidget::clickedLib( int nID )
{
	m_bShared = nID;
}

void CDialogWidget::clickedThread( int nID )
{
	m_bThreaded = nID;
}
