/**********************************************************************
** Copyright (C) 2000-2003 Trolltech AS.  All rights reserved.
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
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qdir.h>

//#define PROFILE_DEBUG

class ProfileHandler : public QXmlDefaultHandler
{
public:
    enum State { StateNone, StateProfile, StateProperty, StateDoc,
		 StateDocTitle, StateDocIcon, StateDocDir };

    ProfileHandler( Profile *prof ) :
	profile( prof ), state( StateNone ) { }

    bool startElement( const QString &namespaceURI, const QString &localName,
		       const QString &qName, const QXmlAttributes &attr );
    bool endElement( const QString & namespaceURI, const QString & localName,
		     const QString & qName);
    bool characters( const QString &chars );

    Profile *profile;
    State state;
    QString propertyName, curDoc;
};

bool ProfileHandler::startElement( const QString & /*namespaceURI*/,
				   const QString & /*localName*/,
				   const QString &qName,
				   const QXmlAttributes &attr )
{
    switch( state ) {
    case StateNone:
	if ( qName.lower() != "adp" )
	    return FALSE;
	state = StateProfile;
	break;
    case StateProfile:
	if ( qName.lower() == "property" ) {
	    propertyName = attr.value( "name" );
	    if( propertyName.isNull() ) {
		return FALSE;
	    }
	    state = StateProperty;
	} else if ( qName.lower() == "documentation" ) {
	    curDoc = attr.value( "file" );
	    if ( curDoc.isEmpty() ) {
		qWarning( "Cannot handle documentation without a file name" );
		return FALSE;
	    }
	    profile->docs.append( curDoc );
	    state = StateDoc;
	}
	break;
    case StateDoc:
	if ( qName.lower() == "title" )
	    state = StateDocTitle;
	else if ( qName.lower() == "icon" )
	    state = StateDocIcon;
	else if ( qName.lower() == "imgdir" )
	    state = StateDocDir;
	break;
    case StateProperty:
	return FALSE;
    }
    return TRUE;
}

bool ProfileHandler::endElement( const QString & /*namespaceURI*/,
				 const QString & /*localName*/,
				 const QString & /*qName*/)
{
    switch( state ) {
    case StateNone:
	return FALSE;
    case StateProfile:
	break;
    case StateProperty:
    case StateDoc:
	state = StateProfile;
	break;
    case StateDocTitle:
    case StateDocIcon:
    case StateDocDir:
	state = StateDoc;
    }
    return TRUE;
}

bool ProfileHandler::characters( const QString &chars )
{
    switch( state ) {
    case StateNone:
    case StateProfile:
    case StateDoc:
	break;
    case StateProperty:
	profile->props[propertyName.lower()] = chars;
	break;
    case StateDocTitle:
	profile->titles[curDoc] = chars;
	break;
    case StateDocIcon:
	profile->icons[curDoc] = chars;
	break;
    case StateDocDir:
	profile->imageDirs[curDoc] = chars;
    }
    return TRUE;
}

static QString entitize( const QString &s )
{
    QString str = s;
    str = str.replace( "&", "&amp;" );
    str = str.replace( ">", "&gt;" );
    str = str.replace( "<", "&lt;" );
    return str;
}

Profile *Profile::createProfile( const QString &filename )
{
    Profile *profile = new Profile;
    profile->load( filename );
    return profile;
}

Profile *Profile::createDefaultProfile()
{
    QString path = QString( qInstallPathDocs() ) + "/html/";
    Profile *profile = new Profile;
    profile->valid = TRUE;
    profile->changed = TRUE;
    profile->props["name"] = "default";
    profile->props["applicationicon"] = "appicon.png";
    profile->props["aboutmenutext"] = "About Qt";
    profile->props["abouturl"] = "about_qt";
    profile->props["title"] = "Qt Assistant";
    profile->props["basepath"] = path;
    profile->props["startpage"] = path + "/index.html";

    profile->addDocFile( path + "qt.dcf" );
    profile->addDocFile( path + "designer.dcf" );
    profile->addDocFile( path + "assistant.dcf" );
    profile->addDocFile( path + "linguist.dcf" );
    profile->addDocFile( path + "qmake.dcf" );

    profile->addDocFileIcon( path + "qt.dcf", "qt.png" );
    profile->addDocFileIcon( path + "designer.dcf", "designer.png" );
    profile->addDocFileIcon( path + "assistant.dcf", "assistant.png" );
    profile->addDocFileIcon( path + "linguist.dcf", "linguist.png" );
    profile->addDocFileIcon( path + "qmake.dcf", "" );

    profile->addDocFileTitle( path + "qt.dcf", "Qt Reference Documentation" );
    profile->addDocFileTitle( path + "designer.dcf", "Qt Designer Manual" );
    profile->addDocFileTitle( path + "assistant.dcf", "Qt Assistant Manual" );
    profile->addDocFileTitle( path + "linguist.dcf", "Guide to the Qt Translation Tools" );
    profile->addDocFileTitle( path + "qmake.dcf", "qmake User Guide" );

    profile->addDocFileImageDir( path + "qt.dcf", "../../gif/" );
    profile->addDocFileImageDir( path + "designer.dcf", "../../gif/" );
    profile->addDocFileImageDir( path + "assistant.dcf", "../../gif/" );
    profile->addDocFileImageDir( path + "linguist.dcf", "../../gif/" );
    profile->addDocFileImageDir( path + "qmake.dcf", "../../gif/" );

    return profile;
}


Profile::Profile()
    : valid( FALSE ), changed( FALSE )
{
}

Profile::Profile( const Profile *p )
{
    props = p->props;
    docs = p->docs;
    icons = p->icons;
    titles = p->titles;
    imageDirs = p->imageDirs;
    valid = p->valid;
    changed = p->changed;
}

bool Profile::load( const QString &name )
{
    QFile file( name );
    if( !file.exists() ) {
	qWarning( "Profile does not exist: %s", name.latin1() );
	return FALSE;
    } else if( !file.open( IO_ReadOnly ) ) {
	qWarning( "Profile could not be opened: %s", name.latin1() );
	return FALSE;
    }

    QXmlInputSource insrc( &file );
    QXmlSimpleReader read;
    ProfileHandler handler( this );
    read.setContentHandler( &handler );
    valid = read.parse( &insrc, FALSE );

    if( !valid ) {
	qWarning( "Failed to parse profile: %s", name.latin1() );
	return FALSE;
    } else if ( props["name"].isNull() ) {
	qWarning( "Profile does not contain required property 'name'" );
	valid = FALSE;
	return FALSE;
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
	keys = imageDirs.keys();
	it = keys.begin();
	while( it != keys.end() ) {
	    qDebug( "Dirs: %15s = %s", (*it).latin1(), imageDirs[*it].latin1() );
	    it++;
	}
    }
#endif
    changed = TRUE;
    return TRUE;
}

QString Profile::makeRelativePath( const QString &base, const QString &path )
{
    QDir bp( base );
    QDir pp( path );
    QStringList bl = QStringList::split( "/", bp.canonicalPath() );
    QStringList pl = QStringList::split( "/", pp.canonicalPath() );

    while ( bl.count() && pl.count() ) {
	if ( bl.first() == pl.first() ) {
	    bl.pop_front();
	    pl.pop_front();
	} else {
	    break;
	}
    }
    QString rel = "";
    if ( bl.count() )
	for ( int i = 0; i < (int)bl.count(); ++i )
	    rel += "../";
    else
	rel = "./";
    rel += pl.join( "/" );
    rel.replace( QRegExp( "/$" ), "" );
    return rel;
}

void Profile::save( const QString &name )
{
    QFile file( name );
    if ( !file.open( IO_WriteOnly ) ) {
	qWarning( "Profile could not be saved" );
	return;
    }
    QString indent( "    " );
    QTextStream s( &file );
    s.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    s << "<!DOCTYPE ADP>"<< endl;
    s << "<ADP>" << endl;

    QValueList<QString> keys = props.keys();
    QValueList<QString>::ConstIterator it = keys.begin();
    QString buf, val;
    while( it != keys.end() ) {
	val = props[*it];
	if ( (*it == "applicationicon" || *it == "abouturl" || *it == "startpage" ) && !val.isEmpty() ) {
	    QFileInfo fi( val );
	    QString relPath = Profile::makeRelativePath( props["basepath"], fi.dirPath( TRUE ) );
	    val = relPath + "/" + fi.fileName();
	}
	buf = QString( "<property name=\"%1\">%2</property>" )
	    .arg( *it ).arg( entitize( val ) );
	s << indent <<  buf << endl;
	it++;
    }
    s << endl;
    QString imgDir;
    QStringList::ConstIterator docfiles = docs.begin();
    while( docfiles != docs.end() ) {
	QFileInfo fi( *docfiles );
	QString val = Profile::makeRelativePath( props["basepath"], fi.dirPath( TRUE ) );
	val = val + "/" + fi.fileName();
	s << indent << "<documentation file=\"" << entitize( val ) << "\">" << endl;
	if ( !titles[*docfiles].isEmpty() )
	    s << indent << indent << "<title>" << entitize( titles[*docfiles] ) << "</title>" << endl;
	if ( !icons[*docfiles].isEmpty() )
	    s << indent << indent << "<icon>" << entitize( icons[*docfiles] ) << "</icon>" << endl;
	if ( !imageDirs[*docfiles].isEmpty() )
	    s << indent << indent << "<imgdir>" << entitize( imageDirs[*docfiles] ) << "</imgdir>" << endl;
	s << indent << "</documentation>" << endl;
	++docfiles;
    }
    s << "</ADP>";
    file.close();
}

void Profile::removeDocFileEntry( const QString &title )
{
    QString docFile = "";
    QMap<QString,QString>::Iterator it = titles.begin();
    for ( ; it != titles.end(); ++it ) {
	if ( it.data() == title ) {
	    docFile = it.key();
	    break;
	}
    }
    if ( docFile.isEmpty() )
	return;
    QStringList::Iterator iter = docs.begin();
    for ( ; iter != docs.end(); ++iter ) {
	if ( *iter == docFile ) {
	    docs.remove( iter );
	    break;
	    }
    }
}
