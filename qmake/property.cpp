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
#include <stdio.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <qlibraryinfo.h>

QStringList qmake_mkspec_paths(); //project.cpp

QMakeProperty::QMakeProperty() : settings(0)
{
}

QMakeProperty::~QMakeProperty()
{
    delete settings;
    settings = 0;
}

void QMakeProperty::initSettings()
{
    if(!settings) {
        settings = new QSettings(QSettings::UserScope, "trolltech.com", "QMake");
        settings->setFallbacksEnabled(false);
    }
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
    if(v == "QT_INSTALL_PREFIX") 
        return QLibraryInfo::location(QLibraryInfo::PrefixPath);
    else if(v == "QT_INSTALL_DATA") 
        return QLibraryInfo::location(QLibraryInfo::DataPath);
    else if(v == "QT_INSTALL_DOCS")
        return QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    else if(v == "QT_INSTALL_HEADERS") 
        return QLibraryInfo::location(QLibraryInfo::HeadersPath);
    else if(v == "QT_INSTALL_LIBS") 
        return QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    else if(v == "QT_INSTALL_BINS") 
        return QLibraryInfo::location(QLibraryInfo::BinariesPath);
    else if(v == "QT_INSTALL_PLUGINS") 
        return QLibraryInfo::location(QLibraryInfo::PluginsPath);
    else if(v == "QT_INSTALL_TRANSLATIONS") 
        return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    else if(v == "QT_INSTALL_CONFIGURATION")
        return QLibraryInfo::location(QLibraryInfo::SettingsPath);
    else if(v == "QMAKE_MKSPECS") 
        return qmake_mkspec_paths().join(Option::target_mode == Option::TARG_WIN_MODE ? ";" : ":");
    else if(v == "QMAKE_VERSION") 
        return qmake_version();
#ifdef QT_VERSION_STR
    else if(v == "QT_VERSION") 
        return QT_VERSION_STR;
#endif

    initSettings();
    int slash = v.lastIndexOf('/');
    QCoreVariant var = settings->value(keyBase(slash == -1) + v);
    bool ok = var.isValid();
    QString ret = var.toString();
    if(!ok) {
        QString version = qmake_version();
        if(slash != -1) {
            version = v.left(slash-1);
            v = v.mid(slash+1);
        }
        settings->beginGroup(keyBase(false));
        QStringList subs = settings->childGroups();
        settings->endGroup();
        subs.sort();
        for (int x = subs.count() - 1; x >= 0; x--) {
            QString s = subs[x];
            if(s.isEmpty() || s > version)
                continue;
            var = settings->value(keyBase(false) + s + "/" + v);
            ok = var.isValid();
            ret = var.toString();
            if (ok) {
                if(!just_check)
                    debug_msg(1, "Fell back from %s -> %s for '%s'.", version.toLatin1().constData(),
                              s.toLatin1().constData(), v.toLatin1().constData());
                return ret;
            }
        }
    }
    return ok ? ret : QString();
}

bool
QMakeProperty::hasValue(QString v)
{
    return !value(v, true).isNull();
}

void
QMakeProperty::setValue(QString var, const QString &val)
{
    initSettings();
    settings->setValue(keyBase() + var, val);
}

bool
QMakeProperty::exec()
{
    bool ret = true;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY) {
        if(Option::prop::properties.isEmpty()) {
            initSettings();
            settings->beginGroup(keyBase(false));
            QStringList subs = settings->childGroups();
            settings->endGroup();
            subs.sort();
            for(int x = subs.count() - 1; x >= 0; x--) {
                QString s = subs[x];
                if(s.isEmpty())
                    continue;
                settings->beginGroup(keyBase(false) + s);
                QStringList keys = settings->childKeys();
                settings->endGroup();
                for(QStringList::Iterator it2 = keys.begin(); it2 != keys.end(); it2++) {
                    QString ret = settings->value(keyBase(false) + s + "/" + (*it2)).toString();
                    if(s != qmake_version())
                        fprintf(stdout, "%s/", s.toLatin1().constData());
                    fprintf(stdout, "%s:%s\n", (*it2).toLatin1().constData(), ret.toLatin1().constData());
                }
            }
            return true;
        }
        for(QStringList::Iterator it = Option::prop::properties.begin();
            it != Option::prop::properties.end(); it++) {
            if(Option::prop::properties.count() > 1)
                fprintf(stdout, "%s:", (*it).toLatin1().constData());
            if(!hasValue((*it))) {
                ret = false;
                fprintf(stdout, "**Unknown**\n");
            } else {
                fprintf(stdout, "%s\n", value((*it)).toLatin1().constData());
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
