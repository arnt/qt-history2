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

#include "languageinterfaceimpl.h"
#include <qobject.h>
#include <designerinterface.h>
#include <qfile.h>
#include "yyreg.h"

LanguageInterfaceImpl::LanguageInterfaceImpl()
    : ref( 0 )
{
}

QRESULT LanguageInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_Language )
	*iface = (LanguageInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
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
	if ( (*it).prototype().find( "::" ) == -1 )
	    continue;
	func.name = (*it).prototype();
	func.name.remove( 0, func.name.find( "::" ) + 2 );
	func.body = (*it).body();
	func.returnType = (*it).returnType();
	func.start = (*it).functionStartLineNum();
	func.end = (*it).closingBraceLineNum();
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
    designerIface->queryInterface( IID_Designer, (QUnknownInterface**) &iface );
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
    designerIface->queryInterface( IID_Designer, (QUnknownInterface**) &iface );
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
    if ( s == SaveFormCodeExternal )
	return TRUE;
    if ( s == StoreFormCodeSeperate )
	return TRUE;
    return FALSE;
}

void LanguageInterfaceImpl::fileFilters( QMap<QString, QString> &extensionFilterMap ) const
{
    extensionFilterMap.insert( "cpp", "QuickScript Files (*.cpp)" );
    extensionFilterMap.insert( "C", "QuickScript Files (*.C)" );
    extensionFilterMap.insert( "cxx", "QuickScript Files (*.cxx)" );
    extensionFilterMap.insert( "c++", "QuickScript Files (*.c++)" );
    extensionFilterMap.insert( "c", "QuickScript Files (*.c)" );
    extensionFilterMap.insert( "h", "QuickScript Files (*.h)" );
    extensionFilterMap.insert( "H", "QuickScript Files (*.H)" );
    extensionFilterMap.insert( "hpp", "QuickScript Files (*.hpp)" );
    extensionFilterMap.insert( "hxx", "QuickScript Files (*.hxx)" );
}

QString LanguageInterfaceImpl::projectKeyForExtenstion( const QString &extension ) const
{
    if ( extension[ 0 ] == 'c' || extension[ 0 ] == 'C' )
	return "SOURCES";
    return "HEADERS";
}

void LanguageInterfaceImpl::sourceProjectKeys( QStringList &keys ) const
{
    keys << "HEADERS" << "SOURCES";
}

 class CheckObject : public QObject
{
public:
    CheckObject() {}
    bool checkConnectArgs( const char *signal, const char *member ) { return QObject::checkConnectArgs( signal, 0, member ); }

};

bool LanguageInterfaceImpl::canConnect( const QString &signal, const QString &slot )
{
    CheckObject o;
    return o.checkConnectArgs( signal.latin1(), slot.latin1() );
}

void LanguageInterfaceImpl::saveFormCode( const QString &form, const QString &filename,
					       const QValueList<Function> &functions,
					       const QStringList &,
					       const QStringList &,
					       const QStringList &,
					       const QStringList &,
					       const QValueList<Connection> & )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream ts( &f );

    if ( !functions.isEmpty() ) {
	for ( QValueList<Function>::ConstIterator it = functions.begin();
	      it != functions.end(); ++it ) {
	    if ( (*it).returnType.isEmpty() )
		ts << "void ";
	    else
		ts << (*it).returnType << " ";
	    ts << form << "::" << (*it).name << endl;;
	    ts <<  (*it).body;
	    ts << endl << endl;
	}
    }
}

void LanguageInterfaceImpl::loadFormCode( const QString &, const QString &filename,
					       QValueList<Function> &functions,
					       QStringList &,
					       QStringList &,
					       QStringList &,
					       QStringList &,
					       QValueList<Connection> & )
{
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    QString code( ts.read() );
    this->functions( code, &functions );
}
