/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include <qobject.h>
#include "timestamp.h"

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

    bool save( bool ignoreModified = FALSE );
    bool saveAs( bool ignoreModified = FALSE );
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
