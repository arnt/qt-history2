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
#include <qcoresettings.h>
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
        return true;
    sett = new QCoreSettings(Qt::UserScope, "Trolltech", "QMake");
    return true;
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

    if(initSettings()) {
        int slash = v.lastIndexOf('/');
        QCoreVariant var = sett->value(keyBase(slash == -1) + v);
        bool ok = var.isValid();
        QString ret = var.toString();
        if(!ok) {
            QString version = qmake_version();
            if(slash != -1) {
                version = v.left(slash-1);
                v = v.mid(slash+1);
            }
            sett->beginGroup(keyBase(false));
            QStringList subs = sett->childGroups();
            sett->endGroup();
            subs.sort();
            for (int x = subs.count() - 1; x >= 0; x--) {
                QString s = subs[x];
                if(s.isEmpty() || s > version)
                    continue;
                var = sett->value(keyBase(false) + s + "/" + v);
                ok = var.isValid();
                ret = var.toString();
                if (ok) {
                    if(!just_check)
                        debug_msg(1, "Fell back from %s -> %s for '%s'.", version.latin1(),
                                  s.latin1(), v.latin1());
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
        return !value(v, true).isNull();
    return false;
}

void
QMakeProperty::setValue(QString var, const QString &val)
{
    if(initSettings())
        sett->setValue(keyBase() + var, val);
}

bool
QMakeProperty::exec()
{
    bool ret = true;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY) {
        if(Option::prop::properties.isEmpty() && initSettings()) {
            sett->beginGroup(keyBase(false));
            QStringList subs = sett->childGroups();
            sett->endGroup();
            subs.sort();
            for(int x = subs.count() - 1; x >= 0; x--) {
                QString s = subs[x];
                if(s.isEmpty())
                    continue;
                sett->beginGroup(keyBase(false) + s);
                QStringList keys = sett->childKeys();
                sett->endGroup();
                for(QStringList::Iterator it2 = keys.begin(); it2 != keys.end(); it2++) {
                    QString ret = sett->value(keyBase(false) + s + "/" + (*it2)).toString();
                    if(s != qmake_version())
                        fprintf(stdout, "%s/", s.latin1());
                    fprintf(stdout, "%s:%s\n", (*it2).latin1(), ret.latin1());
                }
            }
            return true;
        }
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
