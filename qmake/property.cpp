
#include "property.h"
#include "qsettings.h"
#include "qdir.h"
#include "qmap.h"
#include "qstringlist.h"
#include "option.h"
#include <stdio.h>

QMakeProperty::QMakeProperty() : sett(NULL)
{
}

QMakeProperty::~QMakeProperty()
{
    delete sett;;
    sett = NULL;
}


bool QMakeProperty::initSettings()
{
    if(sett)
	return TRUE;
    sett = new QSettings;
    return TRUE;
}

QString
QMakeProperty::keyBase() const
{
    return "/QMake/3.1/";
}


QString
QMakeProperty::value(const QString &v)
{
    if(v == "QT_INSTALL_PREFIX") {
#ifdef QT_INSTALL_PREFIX
	return QT_INSTALL_PREFIX;
#elif defined(HAVE_QCONFIG_CPP)
	return qInstallPath();
#endif
    }
    if(v == "QT_INSTALL_DATA") {
#ifdef QT_INSTALL_DATA
	return QT_INSTALL_DATA;
#elif defined(HAVE_QCONFIG_CPP)
	return qInstallPathData();
#endif
    }

    if(initSettings()) {
	bool ok;
	QString ret = sett->readEntry(keyBase() + v, QString::null, &ok);
	return ok ? ret : QString::null;
    }
    return QString::null;
}

bool
QMakeProperty::hasValue(const QString &v)
{
    if(initSettings())
	return !value(v).isNull();
    return FALSE;
}

void
QMakeProperty::setValue(const QString &var, const QString &val)
{
    if(initSettings())
	sett->writeEntry(keyBase() + var, val);
}

bool
QMakeProperty::exec()
{
    bool ret = TRUE;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY) {
	for(QStringList::Iterator it = Option::prop::properties.begin(); 
	    it != Option::prop::properties.end(); it++) {
	    if(Option::prop::properties.count() > 1)
		fprintf(stdout, "%s: ", (*it).latin1());
	    if(!hasValue((*it))) {
		ret = FALSE;
		fprintf(stdout, "**Unknown**\n");
	    } else {
		fprintf(stdout, "%s\n", value((*it)).latin1());
	    }
	}
    } else if(Option::qmake_mode == Option::QMAKE_SET_PROPERTY) {
	for(QStringList::Iterator it = Option::prop::properties.begin(); 
	    it != Option::prop::properties.end(); it++) {
	    QString var = (*it);
	    it++;
	    if(it == Option::prop::properties.end()) {
		ret = FALSE;
		break;
	    }
	    if(!var.startsWith("."))
		setValue(var, (*it));
	}
    }
    if(Option::debug_level && initSettings()) {
	QStringList lst = sett->entryList(keyBase());
	for(QStringList::Iterator it = lst.begin();  it != lst.end(); it++) 
	    qDebug("%s === %s", (*it).latin1(), value((*it)).latin1());
    }
    return ret;
}


