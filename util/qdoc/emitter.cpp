/*
  emitter.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include "binarywriter.h"
#include "config.h"
#include "doc.h"
#include "emitter.h"
#include "htmlwriter.h"
#include "messages.h"
#include "stringset.h"

static QString protect( const QString& str, QChar metaCh )
{
    /*
      Suppose metaCh is '|' and str is '\ is not |'. The result should be
      '\\ is not \|' or, in C++ notation, "\\\\ is not \\|".
    */
    QString t = str;
    t.replace( QRegExp(QChar('\\')), QString("\\\\") );
    t.replace( QRegExp(QString("\\") + metaCh), QString("\\") + metaCh );
    return t;
}

static void emitHtmlHeaderFile( const QString& headerFilePath,
				const QString& htmlFileName )
{
    QFile f( headerFilePath );
    if ( !f.open(IO_ReadOnly) ) {
	warning( 1, "Cannot open %s", headerFilePath.latin1() );
	return;
    }

    QTextStream t( &f );
    QString fullText = t.read();
    f.close();

    HtmlWriter out( htmlFileName );
    QString headerFileName = QFileInfo( f ).fileName();

    out.setTitle( headerFileName + QString(" Include File") );
    out.setHeading( headerFileName );
    out.printfMeta( "<p>This is the verbatim text of the %s include file. It"
		   " is provided only for illustration; the copyright remains"
		   " with %s.\n", headerFileName.latin1(),
		   config->company().latin1() );
    out.putsMeta( "<hr>\n<pre>\n" );
    out.puts( fullText );
    out.putsMeta( "</pre>\n" );
}

void Emitter::addGroup( DefgroupDoc *doc )
{
    groupdefs.insert( doc->name(), doc );
    addHtmlFile( doc->fileName() );
}

void Emitter::addGroupie( Doc *groupie )
{
    StringSet::ConstIterator group = groupie->groups().begin();
    while ( group != groupie->groups().end() ) {
	groupiemap[*group].insert( groupie->name(), groupie );
	++group;
    }
}

void Emitter::addPage( PageDoc *doc )
{
    pages.append( doc );
    addHtmlFile( doc->fileName() );
}

void Emitter::addExample( ExampleDoc *doc )
{
    examples.append( doc );
    addHtmlFile( config->verbatimHref(doc->fileName()) );
    eglist.insert( doc->fileName() );
}

void Emitter::addHtmlChunk( const QString& link, const HtmlChunk& chk )
{
    chkmap.insert( link, chk );
}

void Emitter::addLink( const QString& link, const QString& text )
{
    lmap[text].insert( link );
}

void Emitter::nailDownDecls()
{
    root.buildMangledSymbolTables();
    root.buildPlainSymbolTables( FALSE );
    root.fillInDecls();

    resolver = new DeclResolver( &root );
    Doc::setResolver( resolver );
}

void Emitter::nailDownDocs()
{
    root.destructSymbolTables();
    root.fillInDocs();
    root.buildPlainSymbolTables( TRUE );

    /*
      Extract miscellaneous information about classes.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while ( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;

	    /*
	      Fetch properties for class.
	    */
	    QValueList<PropertyDecl *>::ConstIterator p =
		    classDecl->properties().begin();
	    while ( p != classDecl->properties().end() ) {
		QString key = classDecl->name() + QChar( '/' ) + (*p)->name();
		QString link = config->classRefHref( classDecl->name() );
		if ( (*p)->name() == (*p)->readFunction() )
		    link += QChar( '#' ) + Decl::ref( (*p)->name() );

		// avoid Q_OVERRIDE
		if ( !(*p)->readFunction().isEmpty() )
		    pmap.insert( key, link );
		++p;
	    }

	    /*
	      Add header files to list.
	    */
	    if ( !classDecl->headerFile().isEmpty() )
		hlist.insert( classDecl->headerFile() );
	    if ( classDecl->classDoc() != 0 &&
		 !classDecl->classDoc()->headers().isEmpty() )
		hlist = reunion( hlist, classDecl->classDoc()->headers() );

	    clist.insert( classDecl->name(), classDecl->whatsThis() );
	    wmap[classDecl->whatsThis()].insert( classDecl->name() );

	    QString parent;

	    /*
	      Build the class hierarchy.
	    */
	    if ( !classDecl->superTypes().isEmpty() ) {
		QString firstp = classDecl->superTypes().first().base();
		Decl *parentDecl = resolvePlain( firstp );
		if ( parentDecl != 0 )
		    parent = parentDecl->name();
	    }
	    chierarchy[parent].insert( classDecl->name() );

	    /*
	      Build the function index.
	    */
	    QValueList<Decl *>::ConstIterator grandChild =
		    classDecl->children().begin();
	    while ( grandChild != classDecl->children().end() ) {
		if ( (*grandChild)->kind() == Decl::Function ) {
		    FunctionDecl *funcDecl = (FunctionDecl *) *grandChild;
		    if ( !funcDecl->isConstructor() &&
			 !funcDecl->isDestructor() )
			findex[funcDecl->name()].insert( classDecl->name() );
		}
		++grandChild;
	    }

	    addHtmlFile( config->classRefHref(classDecl->name()) );
	    addHtmlFile( config->classMembersHref(classDecl->name()) );
	}
	++child;
    }

    StringSet::ConstIterator s = hlist.begin();
    while ( s != hlist.end() ) {
	addHtmlFile( config->verbatimHref(*s) );
	++s;
    }
}

void Emitter::emitHtml() const
{
    QString htmlFileName;

    HtmlWriter::setStyle( config->style() );
    HtmlWriter::setPostHeader( config->postHeader() );
    HtmlWriter::setAddress( config->address() );

    resolver->setExampleFileList( eglist );
    resolver->setHeaderFileList( hlist );
    resolver->setHtmlFileList( htmllist );
    resolver->setHtmlChunkMap( chkmap );

    Doc::setHeaderFileList( hlist );
    Doc::setClassList( clist );
    Doc::setFunctionIndex( findex );
    Doc::setClassHierarchy( chierarchy );

    /*
      Generate the verbatim header files.
    */
    StringSet::ConstIterator s = hlist.begin();
    while ( s != hlist.end() ) {
	htmlFileName = config->verbatimHref( *s );

	if ( config->generateHtmlFile(htmlFileName) ) {
	    QString headerFilePath =
		    config->findDepth( *s, config->includeDirList() );
	    if ( headerFilePath.isEmpty() )
		warning( 1, "Cannot find header file '%s'", (*s).latin1() );
	    else
		emitHtmlHeaderFile( headerFilePath, htmlFileName );
	}
	++s;
    }

    /*
      Examples have to be done first, so that the documentation can link to
      them.
    */
    QValueList<ExampleDoc *>::ConstIterator ex = examples.begin();
    while ( ex != examples.end() ) {
	htmlFileName = config->verbatimHref( (*ex)->fileName() );

	if ( config->generateHtmlFile(htmlFileName) ) {
	    HtmlWriter out( htmlFileName );
	    if ( (*ex)->title().isEmpty() )
		out.setTitle( (*ex)->fileName() + QString(" Example File") );
	    else
		out.setTitle( (*ex)->title() );
	    out.setHeading( (*ex)->heading() );
	    (*ex)->printHtml( out );
	}
	++ex;
    }

    /*
      Generate class documentation.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;
	    htmlFileName = config->classRefHref( classDecl->name() );

	    if ( config->generateHtmlFile(htmlFileName) ) {
		resolver->setCurrentClass( classDecl );
		HtmlWriter out( htmlFileName );
		classDecl->printHtmlLong( out );
	    }
	}
	++child;
    }
    resolver->setCurrentClass( (ClassDecl *) 0 );

    QMap<QString, DefgroupDoc *>::ConstIterator def = groupdefs.begin();
    QMap<QString, QMap<QString, Doc *> >::ConstIterator groupies =
	    groupiemap.begin();
    QMap<QString, Doc *>::ConstIterator c; // ### rename

    /*
      A COBOL programmer wrote this clever loop. If it weren't for C, he would
      be programming in OBOL.
    */
    while ( def != groupdefs.end() || groupies != groupiemap.end() ) {
	if ( def != groupdefs.end() ) {
	    if ( groupies == groupiemap.end() || def.key() < groupies.key() ) {
		warning( 2, (*def)->location(), "Empty group '%s'",
			 def.key().latin1() );
		++def;
	    }
	}
	if ( groupies != groupiemap.end() ) {
	    if ( def == groupdefs.end() || groupies.key() < def.key() ) {
		c = (*groupies).begin();
		while ( c != (*groupies).end() ) {
		    if ( *c != 0 )
			warning( 3, (*c)->location(),
				 "Undefined group '%s'",
				 groupies.key().latin1() );
		    ++c;
		}
		++groupies;
	    } else if ( groupies.key() == def.key() ) {
		/*
		  Bingo! At this point *def is the doc and *groupies is a QMap
		  with class- or page-name keys and Doc * values.
		*/
		htmlFileName = config->defgroupHref( (*def)->name() );

		if ( config->generateHtmlFile(htmlFileName) ) {
		    bool onlyClasses = TRUE;
		    c = (*groupies).begin();
		    while ( c != (*groupies).end() ) {
			if ( (*c)->kind() != Doc::Class ) {
			    onlyClasses = FALSE;
			    break;
			}
			++c;
		    }

		    HtmlWriter out( htmlFileName );
		    out.setTitle( (*def)->title() );
		    out.setHeading( (*def)->heading() );
		    (*def)->printHtml( out );
		    out.enterFooter();
		    out.printfMeta( "<p>%s:\n<ul>\n",
				    onlyClasses ? "Classes" : "Pages" );

		    c = (*groupies).begin();
		    while ( c != (*groupies).end() ) {
			QString link = (*c)->fileName();
			out.printfMeta( "<li><a href=\"%s\">%s</a>\n",
					link.latin1(), (*c)->name().latin1() );
			if ( !(*c)->whatsThis().isEmpty() )
			    out.printfMeta( "   (%s)\n",
					    (*c)->whatsThis().latin1() );
			++c;
		    }
		    out.putsMeta( "</ul>\n" );
		}
		++def;
		++groupies;
	    }
	}
    }

    QValueList<PageDoc *>::ConstIterator pa = pages.begin();
    while ( pa != pages.end() ) {
	htmlFileName = (*pa)->fileName();

	if ( config->generateHtmlFile(htmlFileName) ) {
	    HtmlWriter out( htmlFileName );
	    out.setTitle( (*pa)->title() );
	    out.setHeading( (*pa)->heading() );
	    (*pa)->printHtml( out );
	}
	++pa;
    }

    /*
      Write the four special files: index, propertyindex, titleindex, and
      whatsthis.
    */
    BinaryWriter index( QString("index") );
    QMap<QString, StringSet>::ConstIterator x = lmap.begin();
    while ( x != lmap.end() ) {
	StringSet::ConstIterator s = (*x).begin();
	while ( s != (*x).end() ) {
	    index.puts( QString("\"%1\" %2\n")
				.arg(protect(x.key(), QChar('"')))
				.arg(*s)
				.latin1() );
	    ++s;
	}
	++x;
    }

    BinaryWriter propertyindex( QString("propertyindex") );
    QMap<QString, QString>::ConstIterator p = pmap.begin();
    while ( p != pmap.end() ) {
	propertyindex.puts( QString("\"%1\" %2\n")
				    .arg(p.key()).arg(*p)
				    .latin1() );
	++p;
    }

    BinaryWriter titleindex( QString("titleindex") );
    QMap<QString, StringSet>::ConstIterator t = HtmlWriter::titleMap().begin();
    while ( t != HtmlWriter::titleMap().end() ) {
	StringSet::ConstIterator s = (*t).begin();
	while ( s != (*t).end() ) {
	    titleindex.puts( QString("%1 | %2\n")
				     .arg(protect(t.key(), QChar('|')))
				     .arg(*s)
				     .latin1() );
	    ++s;
	}
	++t;
    }

    BinaryWriter whatsthis( QString("whatsthis") );
    QMap<QString, StringSet>::ConstIterator w = wmap.begin();
    while ( w != wmap.end() ) {
	StringSet::ConstIterator s = (*w).begin();
	while ( s != (*w).end() ) {
	    whatsthis.puts( QString("%1. | %2\n")
				    .arg(protect(w.key(), QChar('|')))
				    .arg(*s)
				    .latin1() );
	    ++s;
	}
	++w;
    }
}

void Emitter::addHtmlFile( const QString& fileName )
{
    if ( htmllist.contains(fileName) )
	warning( 1, "HTML file '%s' overwritten", fileName.latin1() );
    else if ( fileName.right(5) != QString(".html") )
	warning( 1, "HTML file '%s' lacking '.html' extension",
		 fileName.latin1() );
    htmllist.insert( fileName );
}
