/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "property.h"
#include "option.h"
#include <qdir.h>
#include <qmap.h>
#include <qstringlist.h>
#include <stdio.h>

QStringList qmake_mkspec_paths(); //project.cpp

QMakeProperty::QMakeProperty()
{
}

QMakeProperty::~QMakeProperty()
{
}

QString
QMakeProperty::keyBase(bool version) const
{
    if (version)
        return QString(qmake_version()) + "/";
    return QString();
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
        return qmake_mkspec_paths().join(Option::target_mode == Option::TARG_WIN_MODE ? ";" : ":");
    } else if(v == "QMAKE_VERSION") {
        return qmake_version();
#ifdef QT_VERSION_STR
    } else if(v == "QT_VERSION") {
        return QT_VERSION_STR;
#endif
    }
    return QString::null;
}

bool
QMakeProperty::hasValue(QString v)
{
    return !value(v, true).isNull();
}

void
QMakeProperty::setValue(QString var, const QString &val)
{
}

bool
QMakeProperty::exec()
{
    bool ret = true;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY) {
        for(QStringList::Iterator it = Option::prop::properties.begin();
            it != Option::prop::properties.end(); it++) {
            if(Option::prop::properties.count() > 1)
                fprintf(stdout, "%s:", (*it).latin1());
            if(!hasValue((*it))) {
                ret = false;
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
                ret = false;
                break;
            }
            if(!var.startsWith("."))
                setValue(var, (*it));
        }
    }
    return ret;
}
