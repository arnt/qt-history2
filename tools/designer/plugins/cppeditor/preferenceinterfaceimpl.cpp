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

#include "preferenceinterfaceimpl.h"
#include <preferences.h>

PreferenceInterfaceImpl::PreferenceInterfaceImpl()
    : ref( 0 )
{
    cppEditorSyntax = 0;
}

PreferenceInterfaceImpl::~PreferenceInterfaceImpl()
{
}

QRESULT PreferenceInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_Preference )
	*iface = (PreferenceInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
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

PreferenceInterface::Preference *PreferenceInterfaceImpl::preference()
{
    if ( !cppEditorSyntax ) {
	cppEditorSyntax = new PreferencesBase( 0, "cppeditor_syntax" );
	( (PreferencesBase*)cppEditorSyntax )->setPath( "/Trolltech/CppEditor/" );
	cppEditorSyntax->hide();
    }
    Preference *pf = 0;
    pf = new Preference;
    pf->tab = cppEditorSyntax;
    pf->title = "C++ Editor";
    pf->receiver = pf->tab;
    pf->init_slot = SLOT( reInit() );
    pf->accept_slot = SLOT( save() );
    return pf;
}

void PreferenceInterfaceImpl::deletePreferenceObject( Preference *p )
{
    delete p;
}
