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

#include "preferenceinterfaceimpl.h"
#include <preferences.h>

PreferenceInterfaceImpl::PreferenceInterfaceImpl( QUnknownInterface *outer )
    : parent( outer ),
      ref( 0 ),
      cppEditorSyntax( 0 )
{
}

PreferenceInterfaceImpl::~PreferenceInterfaceImpl()
{
}

ulong PreferenceInterfaceImpl::addRef()
{
    return parent ? parent->addRef() : ref++;
}

ulong PreferenceInterfaceImpl::release()
{
    if ( parent )
	return parent->release();
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QRESULT PreferenceInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    if ( parent )
	return parent->queryInterface( uuid, iface );

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
