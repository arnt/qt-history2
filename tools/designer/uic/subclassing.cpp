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

#include "uic.h"
#include "parser.h"
#include "widgetdatabase.h"
#include "domtool.h"
#include <qfile.h>
#include <qstringlist.h>
#include <qdatetime.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>

/*!
  Creates a declaration ( headerfile ) for a subclass \a subClass
  of the form given in \a e

  \sa createSubImpl()
 */
void Uic::createSubDecl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QStringList::Iterator it, it2, it3;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    out << "class " << subClass << " : public " << nameOfClass << endl;
    out << "{ " << endl;

/* tmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
    } else { // standard QWidget
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
    }

    // destructor
    out << "    ~" << subClass << "();" << endl;
    out << endl;

    // find additional functions
    QStringList publicSlots, protectedSlots, privateSlots;
    QStringList publicSlotTypes, protectedSlotTypes, privateSlotTypes;
    QStringList publicSlotSpecifier, protectedSlotSpecifier, privateSlotSpecifier;
    QStringList publicFuncts, protectedFuncts, privateFuncts;
    QStringList publicFunctRetTyp, protectedFunctRetTyp, privateFunctRetTyp;
    QStringList publicFunctSpec, protectedFunctSpec, privateFunctSpec;

    // for compatibility
    nl = e.parentNode().toElement().elementsByTagName( "slot" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "slots"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedSlots += functionName;
	    protectedSlotTypes += returnType;
	    protectedSlotSpecifier += specifier;
	} else if ( access == "private" ) {
	    privateSlots += functionName;
	    privateSlotTypes += returnType;
	    privateSlotSpecifier += specifier;
	} else {
	    publicSlots += functionName;
	    publicSlotTypes += returnType;
	    publicSlotSpecifier += specifier;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "function" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "functions" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString type = n.attribute( "type", "function" );
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( type == "slot" ) {
	    if ( access == "protected" ) {
		protectedSlots += functionName;
		protectedSlotTypes += returnType;
		protectedSlotSpecifier += specifier;
	    } else if ( access == "private" ) {
		privateSlots += functionName;
		privateSlotTypes += returnType;
		privateSlotSpecifier += specifier;
	    } else {
		publicSlots += functionName;
		publicSlotTypes += returnType;
		publicSlotSpecifier += specifier;
	    }
	} else {
	    if ( access == "protected" ) {
		protectedFuncts += functionName;
		protectedFunctRetTyp += returnType;
		protectedFunctSpec += specifier;
	    } else if ( access == "private" ) {
		privateFuncts += functionName;
		privateFunctRetTyp += returnType;
		privateFunctSpec += specifier;
	    } else {
		publicFuncts += functionName;
		publicFunctRetTyp += returnType;
		publicFunctSpec += specifier;
	    }
	}
    }

    if ( !publicFuncts.isEmpty() ) {
	for ( it = publicFuncts.begin(), it2 = publicFunctRetTyp.begin(), it3 = publicFunctSpec.begin();
	      it != publicFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    // create public additional slots
    if ( !publicSlots.isEmpty() ) {
	out << "public slots:" << endl;
	for ( it = publicSlots.begin(), it2 = publicSlotTypes.begin(), it3 = publicSlotSpecifier.begin();
	      it != publicSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    if ( !protectedFuncts.isEmpty() ) {
	out << "protected:" << endl;
	for ( it = protectedFuncts.begin(), it2 = protectedFunctRetTyp.begin(), it3 = protectedFunctSpec.begin();
	      it != protectedFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    // create protected additional slots
    if ( !protectedSlots.isEmpty() ) {
	out << "protected slots:" << endl;
	for ( it = protectedSlots.begin(), it2 = protectedSlotTypes.begin(), it3 = protectedSlotSpecifier.begin();
	      it != protectedSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    if ( !privateFuncts.isEmpty() ) {
	out << "private:" << endl;
	for ( it = privateFuncts.begin(), it2 = privateFunctRetTyp.begin(), it3 = privateFunctSpec.begin();
	      it != privateFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    // create private additional slots
    if ( !privateSlots.isEmpty() ) {
	out << "private slots:" << endl;
	for ( it = privateSlots.begin(), it2 = privateSlotTypes.begin(), it3 = privateSlotSpecifier.begin();
	      it != privateSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "    " << type << " " << (*it) << ";" << endl;
	}
	out << endl;
    }

    out << "};" << endl;
}

/*!
  Creates an implementation for a subclass \a subClass of the form
  given in \a e

  \sa createSubDecl()
 */
void Uic::createSubImpl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QStringList::Iterator it, it2, it3;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " *" << endl;
	out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, modal, fl )" << endl;
    } else { // standard QWidget
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, fl )" << endl;
    }
    out << "{" << endl;
    out << "}" << endl;
    out << endl;

    // destructor
    out << "/*  " << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << subClass << "::~" << subClass << "()" << endl;
    out << "{" << endl;
    out << "    // no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;


    // find additional functions
    QStringList publicSlots, protectedSlots, privateSlots;
    QStringList publicSlotTypes, protectedSlotTypes, privateSlotTypes;
    QStringList publicSlotSpecifier, protectedSlotSpecifier, privateSlotSpecifier;
    QStringList publicFuncts, protectedFuncts, privateFuncts;
    QStringList publicFunctRetTyp, protectedFunctRetTyp, privateFunctRetTyp;
    QStringList publicFunctSpec, protectedFunctSpec, privateFunctSpec;


    // for compatibility
    nl = e.parentNode().toElement().elementsByTagName( "slot" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "slots"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedSlots += functionName;
	    protectedSlotTypes += returnType;
	    protectedSlotSpecifier += specifier;
	} else if ( access == "private" ) {
	    privateSlots += functionName;
	    privateSlotTypes += returnType;
	    privateSlotSpecifier += specifier;
	} else {
	    publicSlots += functionName;
	    publicSlotTypes += returnType;
	    publicSlotSpecifier += specifier;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "function" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "functions" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString type = n.attribute( "type", "function" );
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( type == "slot" ) {
	    if ( access == "protected" ) {
		protectedSlots += functionName;
		protectedSlotTypes += returnType;
		protectedSlotSpecifier += specifier;
	    } else if ( access == "private" ) {
		privateSlots += functionName;
		privateSlotTypes += returnType;
		privateSlotSpecifier += specifier;
	    } else {
		publicSlots += functionName;
		publicSlotTypes += returnType;
		publicSlotSpecifier += specifier;
	    }
	} else {
	    if ( access == "protected" ) {
		protectedFuncts += functionName;
		protectedFunctRetTyp += returnType;
		protectedFunctSpec += specifier;
	    } else if ( access == "private" ) {
		privateFuncts += functionName;
		privateFunctRetTyp += returnType;
		privateFunctSpec += specifier;
	    } else {
		publicFuncts += functionName;
		publicFunctRetTyp += returnType;
		publicFunctSpec += specifier;
	    }
	}
    }

    if ( !publicFuncts.isEmpty() ) {
	for ( it = publicFuncts.begin(), it2 = publicFunctRetTyp.begin(), it3 = publicFunctSpec.begin();
	      it != publicFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * public function" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }

    // create stubs for public additional slots
    if ( !publicSlots.isEmpty() ) {
	for ( it = publicSlots.begin(), it2 = publicSlotTypes.begin(), it3 = publicSlotSpecifier.begin();
	      it != publicSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * public slot" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }


    if ( !protectedFuncts.isEmpty() ) {
	for ( it = protectedFuncts.begin(), it2 = protectedFunctRetTyp.begin(), it3 = protectedFunctSpec.begin();
	      it != protectedFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * protected function" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }


    // create stubs for protected additional slots
    if ( !protectedSlots.isEmpty() ) {
	for ( it = protectedSlots.begin(), it2 = protectedSlotTypes.begin(), it3 = protectedSlotSpecifier.begin();
	      it != protectedSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * protected slot" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }


    if ( !privateFuncts.isEmpty() ) {
	for ( it = privateFuncts.begin(), it2 = privateFunctRetTyp.begin(), it3 = privateFunctSpec.begin();
	      it != privateFuncts.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * private function" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }

    // create stubs for private additional slots
    if ( !privateSlots.isEmpty() ) {
	for ( it = privateSlots.begin(), it2 = privateSlotTypes.begin(), it3 = privateSlotSpecifier.begin();
	      it != privateSlots.end(); ++it, ++it2, ++it3 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    if ( *it3 == "non virtual" )
		continue;
	    out << "/* " << endl;
	    out << " * private slot" << endl;
	    out << " */" << endl;
	    out << type << " " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl << endl;
	}
	out << endl;
    }
}

