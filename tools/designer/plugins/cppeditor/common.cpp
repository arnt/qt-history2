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

#include "editorinterfaceimpl.h"
#include <qcomponentinterface.h>
#include "languageinterfaceimpl.h"
#include "preferenceinterfaceimpl.h"

class CommonInterface : public QComponentInterface
{
public:
    CommonInterface();
    virtual ~CommonInterface();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QString name() const { return "C++"; }
    QString description() const { return "C++ Integration"; }
    QString version() const { return "0.1"; }
    QString author() const { return "Trolltech AS"; }

private:
    unsigned long ref;
    EditorInterface *editorIface;
    LanguageInterfaceImpl *langIface;
    PreferenceInterfaceImpl *prefIface;

};

CommonInterface::CommonInterface()
    : QComponentInterface(), ref( 0 )
{
    editorIface = new EditorInterfaceImpl;
    editorIface->addRef();
    langIface = new LanguageInterfaceImpl;
    langIface->addRef();
    prefIface = new PreferenceInterfaceImpl;
    prefIface->addRef();
}

CommonInterface::~CommonInterface()
{
    editorIface->release();
    langIface->release();
    prefIface->release();
}

QUnknownInterface *CommonInterface::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    if ( uuid == IID_QComponentInterface )
	iface = (QComponentInterface*)this;
    else if ( uuid == IID_EditorInterface )
	iface = editorIface;
    else if ( uuid == IID_LanguageInterface )
	iface = langIface;
    else if ( uuid == IID_PreferenceInterface )
	iface = prefIface;

    if ( iface )
	iface->addRef();
    return iface;
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
