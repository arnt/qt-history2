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
#include <qapplication.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdatetime.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

// see ### in widgetdatabase.cpp
extern bool dbnounload;
extern QStringList *dbpaths;

int main( int argc, char * argv[] )
{
    bool impl = FALSE;
    bool subcl = FALSE;
    bool imagecollection = FALSE;
    QStringList images;
    const char *error = 0;
    const char* fileName = 0;
    const char* className = 0;
    const char* headerFile = 0;
    const char* outputFile = 0;
    const char* projectName = 0;
    const char* trmacro = 0;
    bool nofwd = FALSE;
    bool fix = FALSE;
    QApplication app(argc, argv, FALSE);
    for ( int n = 1; n < argc && error == 0; n++ ) {
	QCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt[0] == 'i' || opt == "impl" ) {
		impl = TRUE;
		if ( opt == "impl" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of header file.";
			break;
		    }
		    headerFile = argv[++n];
		} else
		    headerFile = &opt[1];
	    } else if ( opt[0] == 'e' || opt == "embed" ) {
		imagecollection = TRUE;
		if ( opt == "embed" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of project.";
			break;
		    }
		    projectName = argv[++n];
		} else
		    projectName = &opt[1];
	    } else if ( opt == "nofwd" ) {
		nofwd = TRUE;
	    } else if ( opt == "nounload" ) {
		dbnounload = TRUE;
	    } else if ( opt == "subdecl" ) {
		subcl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments.";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "subimpl" ) {
		subcl = TRUE;
		impl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments.";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "tr" ) {
		if ( opt == "tr" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing tr macro.";
			break;
		    }
		    trmacro = argv[++n];
		} else {
		    trmacro = &opt[1];
		}
	    } else if ( opt == "L" ) {
		if ( !(n < argc-1) ) {
		    error = "Missing plugin path.";
		    break;
		}
		if ( !dbpaths )
		    dbpaths = new QStringList();
		dbpaths->append( QFile::decodeName( argv[++n] ) );
	    } else if ( opt == "version" ) {
		fprintf( stderr,
			 "User Interface Compiler for Qt version %s\n",
			 QT_VERSION_STR );
		exit( 1 );
	    } else if ( opt == "help" ) {
		break;
	    } else if ( opt == "fix" ) {
		fix = TRUE;
	    } else {
		error = "Unrecognized option";
	    }
	} else {
	    if ( imagecollection )
		images << argv[n];
	    else if ( fileName )		// can handle only one file
		error	 = "Too many input files specified";
	    else
		fileName = argv[n];
	}
    }

    if ( argc < 2 || error || (!fileName && !imagecollection ) ) {
	fprintf( stderr, "Qt user interface compiler.\n" );
	if ( error )
	    fprintf( stderr, "uic: %s\n", error );

	fprintf( stderr, "Usage: %s  [options] [mode] <uifile>\n\n"
		 "Generate declaration:\n"
		 "   %s  [options] <uifile>\n"
		 "Generate implementation:\n"
		 "   %s  [options] -impl <headerfile> <uifile>\n"
		 "\t<headerfile>    name of the declaration file\n"
		 "Generate image collection:\n"
		 "   %s  [options] -embed <project> <image1> <image2> <image3> ...\n"
		 "\t<project>       project name\n"
		 "\t<image[1-N]>    image files\n"
		 "Generate subclass declaration:\n"
		 "   %s  [options] -subdecl <subclassname> <baseclassheaderfile> <uifile>\n"
		 "\t<subclassname>     name of the subclass to generate\n"
		 "\t<baseclassheaderfile>    declaration file of the baseclass\n"
		 "Generate subclass implementation:\n"
		 "   %s  [options] -subimpl <subclassname> <subclassheaderfile> <uifile>\n"
		 "\t<subclassname>     name of the subclass to generate\n"
		 "\t<subclassheaderfile>    declaration file of the subclass\n"
		 "Options:\n"
		 "\t-o file         Write output to file rather than stdout\n"
		 "\t-nofwd          Omit forward declarations of custom classes\n"
		 "\t-nounload       Don't unload plugins after processing\n"
		 "\t-tr func        Use func() instead of tr() for i18n\n"
		 "\t-L path         Additional plugin search path\n"
		 "\t-version        Display version of uic\n"
		 "\t-help           Display this information\n"
		 , argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
	exit( 1 );
    }

    QFile fileOut;
    if ( outputFile ) {
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) )
	    qFatal( "uic: Could not open output file '%s'", outputFile );
    } else {
	fileOut.open( IO_WriteOnly, stdout );
    }
    QTextStream out( &fileOut );
    out.setEncoding( QTextStream::UnicodeUTF8 );

    if ( imagecollection ) {
	Uic::embed( out, projectName, images );
	return 0;
    }


    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) )
	qFatal( "uic: Could not open file '%s' ", fileName );

    QDomDocument doc;
    QString errMsg;
    int errLine;
    if ( !doc.setContent( &file, &errMsg, &errLine ) )
	qFatal( QString("uic: Failed to parse %s: ") + errMsg + QString (" in line %d\n"), fileName, errLine );

    DomTool::fixDocument( doc );

    if ( fix ) {
	out << doc.toString();
	return 0;
    }

    if ( !subcl ) {
	out << "/****************************************************************************" << endl;
	out << "** Form "<< (impl? "implementation" : "interface") << " generated from reading ui file '" << fileName << "'" << endl;
	out << "**" << endl;
	out << "** Created: " << QDateTime::currentDateTime().toString() << endl;
	out << "**      by:  The User Interface Compiler (uic)" << endl;
	out << "**" << endl;
	out << "** WARNING! All changes made in this file will be lost!" << endl;
	out << "****************************************************************************/" << endl << endl;
    }

    QString protector;
    if ( subcl && className && !impl )
	protector = QString::fromLocal8Bit( className ).upper() + "_H";

    if ( !protector.isEmpty() ) {
	out << "#ifndef " << protector << endl;
	out << "#define " << protector << endl;
    }

    if ( headerFile ) {
	out << "#include \"" << headerFile << "\"" << endl << endl;
    }

    Uic( fileName, out, doc, !impl, subcl, trmacro, className, nofwd );

    if ( !protector.isEmpty() ) {
	out << endl;
	out << "#endif // " << protector << endl;
    }
    return 0;
}
