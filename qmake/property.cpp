/****************************************************************************
**
** Implementation of QMakeProperty class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "property.h"
#include "option.h"
#include <qsettings.h>
#include <qdir.h>
#include <qmap.h>
#include <qstringlist.h>
#include <stdio.h>

QStringList qmake_mkspec_paths(); //project.cpp

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
QMakeProperty::keyBase(bool version) const
{
    QString ret = "/QMake/properties/";
    if(version)
	ret += QString(qmake_version()) + "/";
    return ret;
}


QString
QMakeProperty::value(QString v, bool just_check)
{
    if(v == "QT_INSTALL_PREFIX") {
#ifdef QT_INSTALL_PREFIX
	return QT_INSTALL_PREFIX;
#elif defined(HAVE_QCONFIG_CPP)
	return qInstallPath();
#endif
    } else if(v == "QT_INSTALL_DATA") {
#ifdef QT_INSTALL_DATA
	return QT_INSTALL_DATA;
#elif defined(HAVE_QCONFIG_CPP)
	return qInstallPathData();
#endif
    } else if(v == "QMAKE_MKSPECS") {
	return qmake_mkspec_paths().join(":");
    } else if(v == "QMAKE_VERSION") {
	return qmake_version();
#ifdef QT_VERSION_STR
    } else if(v == "QT_VERSION") {
	return QT_VERSION_STR;
#endif
    }

    if(initSettings()) {
	bool ok;
	int slash = v.findRev('/');
	QString ret = sett->readEntry(keyBase(slash == -1) + v, QString::null, &ok);
	if(!ok) {
	    QString version = qmake_version();
	    if(slash != -1) {
		version = v.left(slash-1);
		v = v.mid(slash+1);
	    }
	    QStringList subs = sett->subkeyList(keyBase(FALSE));
	    subs.sort();
	    for(QStringList::Iterator it = subs.fromLast(); it != subs.end(); --it) {
		if((*it).isEmpty() || (*it) > version)
		    continue;
		ret = sett->readEntry(keyBase(FALSE) + (*it) + "/" + v, QString::null, &ok);
		if(ok) {
		    if(!just_check)
			debug_msg(1, "Fell back from %s -> %s for '%s'.", version.latin1(),
				  (*it).latin1(), v.latin1());
		    return ret;
		}
	    }
	}
	return ok ? ret : QString();
    }
    return QString::null;
}

bool
QMakeProperty::hasValue(QString v)
{
    if(initSettings())
	return !value(v, TRUE).isNull();
    return FALSE;
}

void
QMakeProperty::setValue(QString var, const QString &val)
{
    if(initSettings())
	sett->writeEntry(keyBase() + var, val);
}

bool
QMakeProperty::exec()
{
    bool ret = TRUE;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY) {
	if(Option::prop::properties.isEmpty() && initSettings()) {
	    QStringList subs = sett->subkeyList(keyBase(FALSE));
	    subs.sort();
	    for(QStringList::Iterator it = subs.fromLast(); it != subs.end(); --it) {
		if((*it).isEmpty())
		    continue;
		QStringList keys = sett->entryList(keyBase(FALSE) + (*it));
		for(QStringList::Iterator it2 = keys.begin(); it2 != keys.end(); it2++) {
		    QString ret = sett->readEntry(keyBase(FALSE) + (*it) + "/" + (*it2));
		    if((*it) != qmake_version())
			fprintf(stdout, "%s/", (*it).latin1());
		    fprintf(stdout, "%s:%s\n", (*it2).latin1(), ret.latin1());
		}
	    }
	    return TRUE;
	}
	for(QStringList::Iterator it = Option::prop::properties.begin(); 
	    it != Option::prop::properties.end(); it++) {
	    if(Option::prop::properties.count() > 1)
		fprintf(stdout, "%s:", (*it).latin1());
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
    return ret;
}
