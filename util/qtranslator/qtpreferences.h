/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtpreferences.h#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTPREFERENCES
#define QTPREFERENCES

#include <qstringlist.h>
#include <qstring.h>

class QTConfig;

/****************************************************************************
 *
 * Class: QTPreferences
 *
 ****************************************************************************/

class QTPreferences
{
public:
    QTPreferences();
    ~QTPreferences();

    struct Sources
    {
        Sources();

        QStringList directories;
        QStringList extensions;
    } sources;

    struct Traslation
    {
        Traslation();

        QString directory;
        QString prefix;
        bool folders;
    } translation;

    QStringList languages;
    QString projectFile;

    void createProjectConfig();
    void saveProjectConfig();
    void readProjectConfig();

protected:
    void getLanguages();

    QTConfig *projectConfig;

};

#endif
