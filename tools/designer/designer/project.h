/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PROJECT_H
#define PROJECT_H

#include <qstring.h>
#include <qstringlist.h>

class Project
{
public:
    Project( const QString &fn, const QString &pName = QString::null );
    
    void setFileName( const QString &fn );
    QString fileName() const;
    QString projectName() const;
    
    QStringList uiFiles() const;
    QString databaseDescription() const;

    void addUiFile( const QString &f );

    bool isFormLoaded( const QString &form );
    void setFormLoaded( const QString &form, bool loaded );
    
private:
    void parse();
    void clear();
    
private:
    QString filename;
    QStringList uifiles;
    QString dbFile;
    QString proName;
    QStringList loadedForms;
    
};

#endif
