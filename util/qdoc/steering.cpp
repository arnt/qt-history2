/*
  steering.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include "binarywriter.h"
#include "config.h"
#include "declresolver.h"
#include "doc.h"
#include "htmlwriter.h"
#include "messages.h"
#include "steering.h"
#include "stringset.h"

static QString protect( const QString& str, QChar metaCh )
{
    /*
      Suppose metaCh is '|' and str is '\ is not |'.  The result should be
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
    out.printfMeta( "<p>This is the verbatim text of the %s include file.  It"
		   " is provided only for illustration; the copyright remains"
		   " with %s.\n", headerFileName.latin1(),
		   config->company().latin1() );
    out.putsMeta( "<hr>\n<pre>\n" );
    out.puts( fullText );
    out.putsMeta( "</pre>\n" );
}

void Steering::addGroup( DefgroupDoc *doc )
{
    groupdefs.insert( doc->groupName(), doc );
    addHtmlFile( config->defgroupHref(doc->groupName()) );
}

void Steering::addClassToGroup( ClassDecl *classDecl, const QString& group )
{
    groupclasses[group].insert( classDecl->name(), classDecl );
}

void Steering::addPage( PageDoc *doc )
{
    pages.append( doc );
    addHtmlFile( doc->fileName() );
}

void Steering::addExample( ExampleDoc *doc )
{
    examples.append( doc );
    addHtmlFile( config->verbatimHref(doc->fileName()) );
    eglist.insert( doc->fileName() );
}

void Steering::addHtmlChunk( const QString& link, const HtmlChunk& chk )
{
    chkmap.insert( link, chk );
}

void Steering::addLink( const QString& link, const QString& text )
{
    lmap[text].insert( link );
}

void Steering::nailDownDecls()
{
    root.buildMangledSymbolTables();
    root.fillInDecls();
}

void Steering::nailDownDocs()
{
    root.destructMangledSymbolTables();
    root.buildPlainSymbolTables();
    root.fillInDocs();

    /*
      Extract miscellaneous information about classes.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while ( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 &&
	     config->processClass((*child)->name()) ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;

	    /*
	      Fetch properties for class.
	    */
	    QValueList<Property>::ConstIterator p =
		    classDecl->properties().begin();
	    while ( p != classDecl->properties().end() ) {
		QString key = classDecl->name() + QChar( '/' ) + (*p).name();
		QString link = config->classRefHref( classDecl->name() );
		if ( (*p).name() == (*p).readFunction() )
		    link += QChar( '#' ) + Decl::anchor( (*p).name() );

		// avoid Q_OVERRIDE
		if ( !(*p).readFunction().isEmpty() )
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

void Steering::emitHtml() const
{
    HtmlWriter::setStyle( config->style() );
    HtmlWriter::setPostHeader( config->postHeader() );
    HtmlWriter::setAddress( config->address() );

    DeclResolver resolver( &root );
    resolver.setExampleFileList( eglist );
    resolver.setHeaderFileList( hlist );
    resolver.setHtmlFileList( htmllist );
    resolver.setHtmlChunkMap( chkmap );

    Doc::setResolver( &resolver );
    Doc::setHeaderFileList( hlist );
    Doc::setClassList( clist );
    Doc::setFunctionIndex( findex );
    Doc::setClassHierarchy( chierarchy );

    /*
      Generate the verbatim header files.
    */
    StringSet::ConstIterator s = hlist.begin();
    while ( s != hlist.end() ) {
	QString headerFilePath = config->findDepth( *s,
						    config->includeDirList() );
	if ( headerFilePath.isEmpty() )
	    warning( 1, "Cannot find header file '%s'", (*s).latin1() );
	else
	    emitHtmlHeaderFile( headerFilePath, config->verbatimHref(*s) );
	++s;
    }

    /*
      Examples have to be done first, so that the documentation can link to
      them.
    */
    QValueList<ExampleDoc *>::ConstIterator ex = examples.begin();
    while ( ex != examples.end() ) {
	HtmlWriter out( config->verbatimHref((*ex)->fileName()) );
	if ( (*ex)->title().isEmpty() )
	    out.setTitle( (*ex)->fileName() + QString(" Example File") );
	else
	    out.setTitle( (*ex)->title() );
	out.setHeading( (*ex)->heading() );
	(*ex)->printHtml( out );
	++ex;
    }

    /*
      Generate class documentation.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 &&
	     config->processClass((*child)->name()) ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;

	    resolver.setCurrentClass( classDecl );
	    HtmlWriter out( config->classRefHref(classDecl->name()) );
	    classDecl->printHtmlLong( out );
	}
	++child;
    }
    resolver.setCurrentClass( (ClassDecl *) 0 );

    // process the rest?
    if ( !config->processClass(QString("")) )
	return;

    QMap<QString, DefgroupDoc *>::ConstIterator def = groupdefs.begin();
    QMap<QString, QMap<QString, ClassDecl *> >::ConstIterator classes =
	    groupclasses.begin();
    QMap<QString, ClassDecl *>::ConstIterator c;

    /*
      A COBOL programmer wrote this clever loop.  If it weren't for C, he would
      be programming in OBOL.
    */
    while ( def != groupdefs.end() || classes != groupclasses.end() ) {
	if ( def != groupdefs.end() ) {
	    if ( classes == groupclasses.end() || def.key() < classes.key() ) {
		warning( 2, (*def)->location(), "Empty group '%s'",
			 def.key().latin1() );
		++def;
	    }
	}
	if ( classes != groupclasses.end() ) {
	    if ( def == groupdefs.end() || classes.key() < def.key() ) {
		c = (*classes).begin();
		while ( c != (*classes).end() ) {
		    if ( (*c)->doc() != 0 )
			warning( 3, (*c)->doc()->location(),
				 "Undefined group '%s'",
				 classes.key().latin1() );
		    ++c;
		}
		++classes;
	    } else if ( classes.key() == def.key() ) {
		/*
		  Bingo!  At this point *def is the doc and *classes is a QMap
		  with class-name keys and ClassDecl * values.
		*/
		HtmlWriter out( config->defgroupHref((*def)->groupName()) );
		out.setTitle( (*def)->title() );
		out.setHeading( (*def)->heading() );
		(*def)->printHtml( out );
		out.enterFooter();
		out.putsMeta( "<p>Classes:\n<ul>\n" );

		c = (*classes).begin();
		while ( c != (*classes).end() ) {
		    out.printfMeta( "<li><a href=\"%s\">%s</a>\n",
				    config->classRefHref((*c)->name()).latin1(),
				    (*c)->name().latin1() );
		    if ( !(*c)->whatsThis().isEmpty() )
			out.printfMeta( "   (%s)\n",
					(*c)->whatsThis().latin1() );
		    ++c;
		}
		out.putsMeta( "</ul>\n" );
		++def;
		++classes;
	    }
	}
    }

    QValueList<PageDoc *>::ConstIterator pa = pages.begin();
    while ( pa != pages.end() ) {
	HtmlWriter out( (*pa)->fileName() );
	out.setTitle( (*pa)->title() );
	out.setHeading( (*pa)->heading() );
	(*pa)->printHtml( out );
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
	    index.putsBase256( QString("\"%1\" %2\n")
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
	propertyindex.putsBase256( QString("\"%1\" %2\n").arg(p.key()).arg(*p)
				   .latin1() );
	++p;
    }

    BinaryWriter titleindex( QString("titleindex") );
    QMap<QString, StringSet>::ConstIterator t = HtmlWriter::titleMap().begin();
    while ( t != HtmlWriter::titleMap().end() ) {
	StringSet::ConstIterator s = (*t).begin();
	while ( s != (*t).end() ) {
	    titleindex.putsBase256( QString("%1 | %2\n")
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
	    whatsthis.putsBase256( QString("%1. | %2\n")
				   .arg(protect(w.key(), QChar('|')))
				   .arg(*s)
				   .latin1() );
	    ++s;
	}
	++w;
    }
}

void Steering::addHtmlFile( const QString& fileName )
{
    if ( htmllist.contains(fileName) )
	warning( 1, "HTML file '%s' overwritten", fileName.latin1() );
    else if ( fileName.right(5) != QString(".html") )
	warning( 1, "HTML file '%s' lacking '.html' extension",
		 fileName.latin1() );
    htmllist.insert( fileName );
}
