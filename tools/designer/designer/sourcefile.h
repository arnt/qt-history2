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

#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include <qobject.h>

struct DesignerSourceFile;
class SourceEditor;

class SourceFile : public QObject
{
    Q_OBJECT

public:
    SourceFile( const QString &fn );
    ~SourceFile();

    QString text() const;
    void setText( const QString &s );

    bool save();
    bool load();

    void setFileName( const QString &fn ) { filename = fn; save(); }
    QString fileName() const { return filename; }

    DesignerSourceFile *iFace();

    void setOriginalFileName( const QString &f ) { ofn = f; }
    QString originalFileName() const { return ofn.isEmpty() ? filename : ofn; }

    void setEditor( SourceEditor *e );
    bool isModified() const;

private:
    QString filename;
    QString txt;
    DesignerSourceFile *iface;
    QString ofn;
    SourceEditor *editor;

};

#endif
