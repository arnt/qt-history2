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

#ifndef PREFERENCEINTERFACEIMPL_H
#define PREFERENCEINTERFACEIMPL_H

#include <preferenceinterface.h>
class QWidget;

class PreferenceInterfaceImpl : public PreferenceInterface
{
public:
    PreferenceInterfaceImpl();
    virtual ~PreferenceInterfaceImpl();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    Preference *globalPreference( const QString &feature );
    Preference *projectSetting( const QString & ) { return 0; }
    QStringList projectSettings() const { return QStringList(); }
    void connectTo( QUnknownInterface * ) {}
    void deletePreferenceObject( Preference * );

private:
    ulong ref;
    QWidget *cppEditorSyntax;

};

#endif

