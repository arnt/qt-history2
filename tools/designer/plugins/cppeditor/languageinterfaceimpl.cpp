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

#include "languageinterfaceimpl.h"
#include <qobject.h>
#include <designerinterface.h>
#include "yyreg.h"

LanguageInterfaceImpl::LanguageInterfaceImpl()
    : ref( 0 )
{
}

QRESULT LanguageInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    if ( uuid == IID_QUnknownInterface )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_LanguageInterface )
	*iface = (LanguageInterface*)this;

    if ( *iface )
	(*iface)->addRef();
}

unsigned long LanguageInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long LanguageInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void LanguageInterfaceImpl::functions( const QString &code, QValueList<Function> *functionMap ) const
{
    QValueList<CppFunction> l;
    extractCppFunctions( code, &l );
    for ( QValueList<CppFunction>::Iterator it = l.begin(); it != l.end(); ++it ) {
	Function func;
	func.name = (*it).prototype();
	func.name.remove( 0, func.name.find( "::" ) + 2 );
	func.body = (*it).body();
	func.returnType = (*it).returnType();
	functionMap->append( func );
    }
}

QString LanguageInterfaceImpl::createFunctionStart( const QString &className, const QString &func, const QString &returnType )
{
    return returnType + " " + className + "::" + func;
}

QStringList LanguageInterfaceImpl::definitions() const
{
    QStringList lst;
    lst << "Includes (in Implementation)" << "Includes (in Declaration)" << "Forward Declarations" << "Class Variables";
    return lst;
}

QStringList LanguageInterfaceImpl::definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const
{
    DesignerInterface *iface = 0;
    designerIface->queryInterface( IID_DesignerInterface, (QUnknownInterface**) &iface );
    if ( !iface )
	return QStringList();
    DesignerFormWindow *fw = iface->currentForm();
    if ( !fw )
	return QStringList();
    QStringList lst;
    if ( definition == "Includes (in Implementation)" ) {
	lst = fw->implementationIncludes();
    } else if ( definition == "Includes (in Declaration)" ) {
	lst = fw->declarationIncludes();
    } else if ( definition == "Forward Declarations" ) {
	lst = fw->forwardDeclarations();
    } else if ( definition == "Class Variables" ) {
	lst = fw->variables();
    }
    iface->release();
    return lst;
}

void LanguageInterfaceImpl::setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface )
{
    DesignerInterface *iface = 0;
    designerIface->queryInterface( IID_DesignerInterface, (QUnknownInterface**) &iface );
    if ( !iface )
	return;
    DesignerFormWindow *fw = iface->currentForm();
    if ( !fw )
	return;
    if ( definition == "Includes (in Implementation)" ) {
	fw->setImplementationIncludes( entries );
    } else if ( definition == "Includes (in Declaration)" ) {
	fw->setDeclarationIncludes( entries );
    } else if ( definition == "Forward Declarations" ) {
	fw->setForwardDeclarations( entries );
    } else if ( definition == "Class Variables" ) {
	fw->setVariables( entries );
    }
    iface->release();
}

QString LanguageInterfaceImpl::createArguments( const QStringList &args )
{
    QString s;
    for ( QStringList::ConstIterator it = args.begin(); it != args.end(); ++it ) {
	if ( it != args.begin() )
	    s += ",";
	s += *it;
    }
    return s;
}

QString LanguageInterfaceImpl::createEmptyFunction()
{
    return "{\n\n}";
}

bool LanguageInterfaceImpl::supports( Support s ) const
{
    if ( s == ReturnType )
	return TRUE;
    if ( s == ConnectionsToCustomSlots )
	return TRUE;
    if ( s == AdditionalFiles )
	return FALSE;
    return FALSE;
}
