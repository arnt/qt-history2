/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
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
#include "profile.h"
#include <qxml.h>

class ProfileHandler : public QXmlDefaultHandler
{
public:
    enum State { StateNone, StateProfile, StateProperty, StateDocfile };

    ProfileHandler( Profile *prof ) :
	profile( prof ), state( StateNone ) { }

    bool startElement( const QString &namespaceURI, const QString &localName,
		       const QString &qName, const QXmlAttributes &attr );
    bool endElement( const QString & namespaceURI, const QString & localName,
		     const QString & qName);
    bool characters( const QString &chars );

    Profile *profile;
    State state;
    QString propertyName;
};

bool ProfileHandler::startElement( const QString &namespaceURI,
				   const QString &localName,
				   const QString &qName,
				   const QXmlAttributes &attr )
{
    switch( state ) {
    case StateNone:
	if ( qName.lower()!="profile" )
	    return FALSE;
	state = StateProfile;
	break;
    case StateProfile:
	if ( qName.lower()=="property" ) {
	    propertyName = attr.value( "name" );
	    if( propertyName.isNull() ) {
		return FALSE;
	    }
	    state = StateProperty;
	} else if ( qName.lower() == "docfile" ) {
	    propertyName = attr.value( "icon" );
	    state = StateDocfile;
	}
	break;
    case StateProperty:
	return FALSE;
    case StateDocfile:
	return FALSE;
    }
    return TRUE;
}

bool ProfileHandler::endElement( const QString & namespaceURI,
				 const QString & localName,
				 const QString & qName)
{
    switch( state ) {
    case StateNone:
	return FALSE;
    case StateProfile:
	break;
    case StateProperty:
	state = StateProfile;
    case StateDocfile:
	state = StateProfile;
    }
    return TRUE;
}

bool ProfileHandler::characters( const QString &chars )
{
    switch( state ) {
    case StateProperty:
	profile->props.insert( propertyName, chars );
	break;
    case StateDocfile:
	profile->docs.append( chars );
	if( !propertyName.isNull() )
	    profile->icons[chars] = propertyName;
	break;
    default:
	break;
    }
    return TRUE;
}


Profile::Profile( const QString &filename )
    : valid( FALSE )
{
    if ( !filename.isNull() )
	load( filename );
}

void Profile::load( const QString &name )
{
    QFile file( name );
    if( !file.exists() ) {
	qWarning( "Profile does not exist: %s", name.latin1() );
	return;
    } else if( !file.open( IO_ReadOnly ) ) {
	qWarning( "Profile could not be opened: %s", name.latin1() );
	return;
    }

    QXmlInputSource insrc( &file );
    QXmlSimpleReader read;
    ProfileHandler handler( this );
    read.setContentHandler( &handler );
    valid = read.parse( &insrc, FALSE );

    if( !valid ) {
	qWarning( "Failed to parse profile: ", name.latin1() );
    } else {
	QValueList<QString> keys = props.keys();
	QValueList<QString>::ConstIterator it = keys.begin();
	while( it != keys.end() ) {
	    qDebug( "Property: %15s = %s", (*it).latin1(), props[*it].latin1() );
	    it++;
	}

	QStringList::ConstIterator docfiles = docs.begin();
	while( docfiles != docs.end() ) {
	    qDebug( "Docfile: %s", (*docfiles).latin1() );
	    docfiles++;
	}

	keys = icons.keys();
	it = keys.begin();
	while( it != keys.end() ) {
	    qDebug( "Icons: %15s = %s", (*it).latin1(), icons[*it].latin1() );
	    it++;
	}
    }
}


