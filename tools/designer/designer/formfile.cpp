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

#include "formfile.h"
#include "timestamp.h"

FormFile::FormFile( Project *p )
    : pro( p ), fw( 0 ), ed( 0 ), timeStamp( this, "" )
{
}

void FormFile::setFormWindow( FormWindow *f )
{
    fw = f;
}

void FormFile::setEditor( SourceEditor *e )
{
    ed = e;
}

void FormFile::setUiFile( const QString &fn )
{
    uifile = fn;
}

void FormFile::setCodeFile( const QString &fn )
{
    codefile = fn;
    timeStamp.setFileName( fn );
}

void FormFile::setCode( const QString &c )
{
    cod = c;
}

FormWindow *FormFile::formWindow() const
{
    return fw;
}

SourceEditor *FormFile::editor() const
{
    return ed;
}

QString FormFile::uiFile() const
{
    return uifile;
}

QString FormFile::codeFile() const
{
    return codefile;
}

QString FormFile::code() const
{
    return cod;
}

bool FormFile::load()
{
    return FALSE;
}

bool FormFile::save()
{
    return FALSE;
}

bool FormFile::saveAs()
{
    return FALSE;
}

bool FormFile::close()
{
    return FALSE;
}

bool FormFile::closeEvent()
{
    return FALSE;
}

void FormFile::setModified( bool m )
{
}

bool FormFile::isModified()
{
    return FALSE;
}

