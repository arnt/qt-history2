/**********************************************************************
**
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

#ifndef CPPCOMPLETION_H
#define CPPCOMPLETION_H

#include <completion.h>
#include <qguardedptr.h>

class CppEditorCompletion : public EditorCompletion
{
    Q_OBJECT

public:
    CppEditorCompletion( Editor *e );

    bool doObjectCompletion( const QString &object );
    QValueList<QStringList> functionParameters( const QString &func, QChar &, QString &prefix, QString &postfix );
    void setContext( QObjectList *toplevels, QObject *this_ );

private:
    QGuardedPtr<QObject> ths;

};

#endif
