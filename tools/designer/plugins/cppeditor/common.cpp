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

#include "editorinterfaceimpl.h"
#include "languageinterfaceimpl.h"
#include "preferenceinterfaceimpl.h"
#include "projectsettingsinterfaceimpl.h"
#include "sourcetemplateinterfaceimpl.h"

class CommonInterface : public QComponentInterface
{
public:
    CommonInterface();
    virtual ~CommonInterface();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    unsigned long addRef();
    unsigned long release();

    QString name() const { return "C++"; }
    QString description() const { return "C++ Integration"; }
    QString version() const { return "0.1"; }
    QString author() const { return "Trolltech AS"; }

private:
    unsigned long ref;
    LanguageInterfaceImpl *langIface;
    PreferenceInterfaceImpl *prefIface;
    ProjectSettingsInterfaceImpl *proIface;
    SourceTemplateInterfaceImpl *srcIface;

};

CommonInterface::CommonInterface()
    : QComponentInterface(), ref( 0 )
{
    langIface = new LanguageInterfaceImpl;
    langIface->addRef();
    prefIface = new PreferenceInterfaceImpl;
    prefIface->addRef();
    proIface = new ProjectSettingsInterfaceImpl;
    proIface->addRef();
    srcIface = new SourceTemplateInterfaceImpl;
    srcIface->addRef();
}

CommonInterface::~CommonInterface()
{
    langIface->release();
    prefIface->release();
    proIface->release();
    srcIface->release();
}

QRESULT CommonInterface::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QComponent )
	*iface = (QComponentInterface*)this;
    else if ( uuid == IID_Editor )
	*iface = new EditorInterfaceImpl;
    else if ( uuid == IID_Language )
	*iface = langIface;
    else if ( uuid == IID_Preference )
	*iface = prefIface;
    else if ( uuid == IID_ProjectSettings )
	*iface = proIface;
    else if ( uuid == IID_SourceTemplate )
	*iface = srcIface;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

unsigned long CommonInterface::addRef()
{
    return ref++;
}

unsigned long CommonInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
     Q_CREATE_INSTANCE( CommonInterface )
}
