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

// #define PROFILE_DEBUG

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
	profile->props[propertyName.lower()] = chars;
	break;
    case StateDocfile:
	profile->docs.append( chars );
	if( !propertyName.isNull() )
	    profile->icons[chars.lower()] = propertyName;
	break;
    default:
	break;
    }
    return TRUE;
}

Profile *Profile::createProfile( const QString &filename )
{
    Profile *profile = new Profile;
    profile->load( filename );
    return profile;
}

Profile *Profile::createDefaultProfile()
{
    Profile *profile = new Profile;
    profile->valid = TRUE;
    profile->defProf = TRUE;
    profile->props["name"] = "defaultprofile";

    QString path = QString( qInstallPathDocs() ) + "/html/";

    profile->addDocFile( path + "qt.xml" );
    profile->addDocFile( path + "designer.xml" );
    profile->addDocFile( path + "assistant.xml" );
    profile->addDocFile( path + "linguist.xml" );
    profile->addDocFile( path + "qmake.xml" );

    profile->addDocFileIcon( path + "qt.xml", "qt.png" );
    profile->addDocFileIcon( path + "designer.xml", "designer.png" );
    profile->addDocFileIcon( path + "assistant.xml", "assistant.png" );
    profile->addDocFileIcon( path + "linguist.xml", "linguist.png" );

    return profile;
}


Profile::Profile()
    : valid( FALSE ), defProf( FALSE )
{
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
    } else if ( props["name"].isNull() ) {
	qWarning( "Profile does not contain required property 'name'" );
	valid = FALSE;
	return;
    }
#ifdef PROFILE_DEBUG
    else {
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
#endif

}


