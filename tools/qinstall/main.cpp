#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qheader.h>
#include <qiconview.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qvbox.h>
#include <qframe.h>
#include <qdesktopwidget.h>
#include <qmessagebox.h>
#include <qiconview.h>
#include <qxml.h>
#include <qlayout.h>
#include <qdom.h>

// Embedded setup configuration files, images and installable files
#include "data.h"
#include "qinstallationwizard.h"

#include "interface/progressbarwidget.h"


/*
      Turning on profiling causes the atexit function to be needed
         This is just to work around my broken libraries on this machine
*/
typedef void (*funcptr)(int,void*);
int atexit(void (*function)(void)) { return on_exit( (funcptr)function, 0 ); }



int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );

    QRect geometry = QApplication::desktop()->screenGeometry();

    QDomDocument setupData;
    setupData.setContent( embeddedData( "setup.xml" ) );

    // Load project data
    QDomElement projectProgram = setupData.elementsByTagName("Program").item(0).toElement();
    QDomElement projectImages  = setupData.elementsByTagName("InstallationWizard").item(0).toElement();

    QString programName = projectProgram.attribute("name");
    QString programCompany = projectProgram.attribute("company");
    QString programVersion = projectProgram.attribute("version");
    QString programDate = projectProgram.attribute("date");

    // Load splashscreen image and display immediately
    QLabel *splashScreen = 0;
    QPixmap splashImage( embeddedData( projectImages.attribute( "splashScreen" ) ) );
    if ( !splashImage.isNull() ) {
	splashScreen = new QLabel( 0, "Splash Screen", Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool );
	splashScreen->setPixmap( splashImage );
	splashScreen->move( ( geometry.width() - splashImage.width() ) / 2, ( geometry.height() - splashImage.height() ) / 2 );
	splashScreen->show();
	a.processEvents();
    }

    // Show a startup progress indicator
    ProgressBarWidget *startupProgress = new ProgressBarWidget( 0, "StartUp", TRUE, Qt::WStyle_Customize | Qt::WStyle_Title );
    startupProgress->SetupIcon->setPixmap( QPixmap( embeddedData( projectImages.attribute( "setupIcon" ) ) ) );
    startupProgress->InformationLabel->setText( programName + " is preparing the installation wizard which will guide you through the installation process. Please wait.<br>" );
    startupProgress->StatusLabel->setText( "Preparing the setup wizard." );
    int kickerOrTaskbarHeight = 64;
    startupProgress->move( geometry.width() - startupProgress->width() - 64, geometry.height() - startupProgress->height() - 64 - kickerOrTaskbarHeight );
    startupProgress->ProgressBar1->setTotalSteps( 100 );
    startupProgress->ProgressBar1->setProgress( 0 );
    startupProgress->show();
    a.processEvents();
    startupProgress->ProgressBar1->setProgress( 10 );
    a.processEvents();

    // Load data and do processing here while updating the progress bar 
    // Create the Installation Wizard 
    QInstallationWizard *iw = new QInstallationWizard( 0, startupProgress->ProgressBar1, setupData );
    iw->move( ( geometry.width() - iw->width() ) / 2, ( geometry.height() - iw->height() ) / 2 );
    iw->show();
    
    startupProgress->ProgressBar1->setProgress( 100 );
    a.processEvents();
    delete startupProgress;
    if ( splashScreen )
	delete splashScreen;
    
    return a.exec();
}

