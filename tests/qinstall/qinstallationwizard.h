#ifndef QINSTALLATIONWIZARD_H
#define QINSTALLATIONWIZARD_H


#include "interface/installationwizard.h"

#include <qstring.h>
#include <qdom.h>
#include <qlistview.h>


enum WizardPages {
    WelcomePage = 0,
    LicensePage,
    DestinationPage,
    ConfigurationPage,
    CustomizePage,
    ReviewPage,
    InstallPage
};


class Welcome;
class License;
class Destination;
class Configuration;
class Customize;
class Review;
class Install;
class QWidget;
class QWidgetStack;
class QProgressBar;
class Component;


class QInstallationWizard : public InstallationWizard {
Q_OBJECT
    public:
	QInstallationWizard( QWidget *parent, QProgressBar *pb, const QDomDocument& setupData );
	
    public slots:
	void back();
	void next();
	void cancel();
	void selectDestination();
	bool destinationInstallable();

    protected:
	void timerEvent( QTimerEvent *te );
	
    private:
	void installFiles();
	void advanceProgressBar();
	void buildFileList( Component *parent, QStringList& filelist, const QString& configType );
	void buildTree( const QDomElement& parentElement, Component *parent );
	void addWidgetToStack( QWidget *w, const QString& nextText = QString::null, const QString& backText = QString::null );
	void setWidget( int id );
	void installFile( const QString& file );
	
	QMap<int,QString> backTextMap;
	QMap<int,QString> nextTextMap;
	int lastId;

	QString programCompany;
	QString programName;
	QString programVersion;
	QString programDate;
	QString destinationDefaultAbsPath;

	QStringList fileList;
	QStringList::Iterator fileListIterator;
	int percentInstalled;
	int installTimerId;

	Component *features;

	int progress;
	QProgressBar *progressBar;

	QWidgetStack *stack;
	
	Welcome *welcome;
	License *license;
	Destination *destination;
	Configuration *configuration;
	Customize *customize;
	Review *review;
	Install *install;
};


#endif

