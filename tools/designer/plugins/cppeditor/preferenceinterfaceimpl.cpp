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

#include "preferenceinterfaceimpl.h"
#include <preferences.h>

PreferenceInterfaceImpl::PreferenceInterfaceImpl()
    : ref( 0 )
{
    cppEditorSyntax = 0;
}

QUnknownInterface *PreferenceInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_PreferenceInterface )
	iface = (PreferenceInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long PreferenceInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long PreferenceInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QStringList PreferenceInterfaceImpl::featureList() const
{
    QStringList lst;
    lst << "C++ Editor";
    return lst;
}

PreferenceInterface::Preference *PreferenceInterfaceImpl::globalPreference( const QString &feature )
{
    if ( !cppEditorSyntax ) {
	cppEditorSyntax = new PreferencesBase( 0 );
	( (PreferencesBase*)cppEditorSyntax )->setPath( "/Software/Trolltech/CppEditor" );
	cppEditorSyntax->hide();
    }
    Preference *pf = 0;
    if ( feature == "C++ Editor" ) {
	pf = new Preference;
	pf->tab = cppEditorSyntax;
	pf->title = feature;
	pf->receiver = pf->tab;
	pf->init_slot = SLOT( reInit() );
	pf->accept_slot = SLOT( save() );
    }
    return pf;
}

