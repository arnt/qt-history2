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

#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include "timestamp.h"
#include <qobject.h>

struct DesignerSourceFile;
class SourceEditor;
class Project;

class SourceFile : public QObject
{
    Q_OBJECT

public:
    SourceFile( const QString &fn, bool temp, Project *p );
    ~SourceFile();

    void setText( const QString &s );
    void setModified( bool m );

    bool save( bool ignoreModified = false );
    bool saveAs( bool ignoreModified = false );
    bool load();
    bool close();
    bool closeEvent();
    Project *project() const;

    QString text() const;
    QString fileName() const { return filename; }
    bool isModified() const;

    void checkTimeStamp();

    DesignerSourceFile *iFace();

    void setEditor( SourceEditor *e );
    SourceEditor *editor() const { return ed; }

    static QString createUnnamedFileName( const QString &extension );

    bool isAccepted() const { return accepted; }

private:
    bool checkFileName( bool allowBreak );

private:
    QString filename;
    QString txt;
    DesignerSourceFile *iface;
    SourceEditor *ed;
    bool fileNameTemp;
    TimeStamp timeStamp;
    Project *pro;
    bool pkg;
    bool accepted;

};

#endif
