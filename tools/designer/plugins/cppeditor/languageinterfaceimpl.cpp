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

QUnknownInterface *LanguageInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_LanguageInterface )
	iface = (LanguageInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
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
#if 0
    QValueList<CppFunction> l;
    extractCppFunctions( code, &l );
    for ( QValueList<CppFunction>::Iterator it = l.begin(); it != l.end(); ++it ) {
	Function func;
	func.name = (*it).prototype();
	func.name.remove( 0, func.name.find( "::" ) + 2 );
	func.body = (*it).body();
	functionMap->append( func );
    }

#else

    QString text( code );
    QString func;
    QString body;

    int i = 0;
    int j = 0;
    int k = 0;
    while ( i != -1 ) {
	i = text.find( "::", i );
	int colon = i;
	if ( i == -1 )
	    break;
	int nl = -1;
	if ( ( nl = text.findRev( "\n", i ) ) != -1 &&
	     text[ nl + 1 ] == ' ' || text[ nl + 1 ] == '\t' ) {
	    i += 2;
	    continue;
	}
	for ( j = i + QString( "::").length(); j < (int)text.length(); ++j ) {
	    if ( text[ j ] != ' ' && text[ j ] != '\t' )
		break;
	}
	if ( j == (int)text.length() - 1 )
	    break;
	k = text.find( ")", j );
	int l = text.find( "::", colon + QString( "::" ).length() + 1 );
	if ( l < k && l > colon ) {
	    i = l - 1;
	    continue;
	}
	func = text.mid( j, k - j + 1 );
	func = func.stripWhiteSpace();
	func = func.simplifyWhiteSpace();
	if ( func.isEmpty() )
	    break;
	func = NormalizeObject::normalizeSignalSlot( func.latin1() );
	
	i = k;
	i = text.find( "{", i );
	if ( i == -1 )
	    break;
	l = text.find( "::", k );
	if ( l < i && l > colon ) {
	    i = l - 1;
	    continue;
	}
	int open = 0;
	for ( j = i; j < (int)text.length(); ++j ) {
	    if ( text[ j ] == '{' )
		open++;
	    else if ( text[ j ] == '}' )
		open--;
	    if ( !open )
		break;
	}
	body = text.mid( i, j - i + 1 );

	Function f;
	f.name = func;
	f.body = body;
	functionMap->append( f );
    }
#endif
}

QString LanguageInterfaceImpl::createFunctionStart( const QString &className, const QString &func )
{
    return "void " + className + "::" + func;
}

QStringList LanguageInterfaceImpl::definitions() const
{
    QStringList lst;
    lst << "Includes (in Implementation)" << "Includes (in Declaration)" << "Forward Declarations" << "Class Variables";
    return lst;
}

QStringList LanguageInterfaceImpl::definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const
{
    DesignerInterface *iface = (DesignerInterface*)designerIface->queryInterface( IID_DesignerInterface );
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
    DesignerInterface *iface = (DesignerInterface*)designerIface->queryInterface( IID_DesignerInterface );
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
