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

#ifndef FORMFILE_H
#define FORMFILE_H

#include <qobject.h>
#include "timestamp.h"

class Project;
class FormWindow;
class SourceEditor;

class FormFile : public QObject
{
    Q_OBJECT

public:
    enum Who {
	WFormWindow = 1,
	WFormCode = 2,
	WAnyOrAll = WFormWindow | WFormCode
    };

    FormFile( const QString &fn, bool temp, Project *p );

    void setFormWindow( FormWindow *f );
    void setEditor( SourceEditor *e );
    void setFileName( const QString &fn );
    void setCode( const QString &c );
    void setModified( bool m, int who = WAnyOrAll );

    FormWindow *formWindow() const;
    SourceEditor *editor() const;
    QString fileName() const;
    QString codeFile() const;
    QString code() const;

    bool load();
    bool save();
    bool saveAs();
    bool close();
    bool closeEvent();
    bool isModified( int who = WAnyOrAll );
    bool hasFormCode() const;
    void createFormCode();

    void showFormWindow();
    void showEditor();

    static QString createUnnamedFileName();

private:
    bool isFormWindowModified() const;
    bool isCodeModified() const;
    void setFormWindowModified( bool m );
    void setCodeModified( bool m );
    QString codeExtension() const;
    bool loadCode();

private:
    QString filename;
    bool fileNameTemp;
    Project *pro;
    FormWindow *fw;
    SourceEditor *ed;
    QString cod;
    TimeStamp timeStamp;
    bool hFormCode;

};

#endif
