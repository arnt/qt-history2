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

QStringList LanguageInterfaceImpl::featureList() const
{
    QStringList lst;
    lst << "C++";
    return lst;
}

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void LanguageInterfaceImpl::functions( const QString &code, QMap<QString, QString> *functionMap ) const
{
    QString text( code );
    QString func;
    QString body;

    int i = 0;
    int j = 0;
    int k = 0;
    while ( i != -1 ) {
	i = text.find( "::", i );
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
	func = text.mid( j, k - j + 1 );
	func = func.stripWhiteSpace();
	func = func.simplifyWhiteSpace();
	func = NormalizeObject::normalizeSignalSlot( func.latin1() );
	
	i = k;
	i = text.find( "{", i );
	if ( i == -1 )
	    break;
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

	functionMap->insert( func, body );
    }
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
    if ( definition == "Includes (in Implementation)" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    return ( (DesignerInterface*)designerIface )->currentForm()->implementationIncludes();
    } else if ( definition == "Includes (in Declaration)" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    return ( (DesignerInterface*)designerIface )->currentForm()->declarationIncludes();
    } else if ( definition == "Forward Declarations" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    return ( (DesignerInterface*)designerIface )->currentForm()->forwardDeclarations();
    } else if ( definition == "Class Variables" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    return ( (DesignerInterface*)designerIface )->currentForm()->variables();
    }
    return QStringList();
}

void LanguageInterfaceImpl::setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface )
{
    if ( definition == "Includes (in Implementation)" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    ( (DesignerInterface*)designerIface )->currentForm()->setImplementationIncludes( entries );
    } else if ( definition == "Includes (in Declaration)" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    ( (DesignerInterface*)designerIface )->currentForm()->setDeclarationIncludes( entries );
    } else if ( definition == "Forward Declarations" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    ( (DesignerInterface*)designerIface )->currentForm()->setForwardDeclarations( entries );
    } else if ( definition == "Class Variables" ) {
	if ( ( (DesignerInterface*)designerIface )->currentForm() )
	    ( (DesignerInterface*)designerIface )->currentForm()->setVariables( entries );
    }
}
