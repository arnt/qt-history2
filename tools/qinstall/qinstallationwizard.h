#ifndef QINSTALLATION_WIZARD_H
#define QINSTALLATION_WIZARD_H


#include <qstring.h>
#include <qdom.h>
#include <qlistview.h>
#include "interface/installationwizard.h"


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
	
    private:
	void installFiles();
	void advanceProgressBar();
	void buildFileList( Component *parent, QString *componentList, QString *fileList, const QString& seperator, const QString& configType );
	void buildTree( const QDomElement& parentElement, Component *parent );
	void addWidgetToStack( QWidget *w, const QString& nextText = "Next >", const QString& backText = "< Back" );
	void setWidget( int id );
	
	QMap<int,QString> backTextMap;
	QMap<int,QString> nextTextMap;
	int lastId;

//	QMap<QString,QString> attirbutes;
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

