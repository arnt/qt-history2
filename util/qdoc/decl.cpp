/*
  decl.cpp
*/

#include <qregexp.h>
#include <qstring.h>
#include <qtl.h>
#include <qvaluestack.h>

#include "config.h"
#include "decl.h"
#include "english.h"
#include "html.h"
#include "htmlwriter.h"
#include "messages.h"

static QString gulbrandsen( "::" );
static QString parenParen( "()" );

static void printHtmlDataType( HtmlWriter& out, const CodeChunk& type,
			       const Decl *context,
			       const QString& var = QString::null )
{
    QString href;
    Decl *decl = context->resolvePlain( type.base() );
    if ( decl != 0 && decl->doc() != 0 ) {
	if ( decl->kind() == Decl::Class )
	    href = config->classRefHref( decl->fullName() );
	else if ( decl->kind() == Decl::Enum || decl->kind() == Decl::Typedef )
	    href = config->classRefHref( decl->relatesContext()->fullName() ) +
		   QChar( '#' ) + decl->uniqueName();
    }
    type.printHtml( out, href, var );
}

static void printHtmlListEntry( HtmlWriter& out, const QString& funcName,
				const QString& className )
{
    out.printfMeta( "<li><a href=\"%s#%s\">", 
		    config->classRefHref(className).latin1(),
		    Decl::ref(funcName).latin1() );
    out.puts( funcName.latin1() );
    out.putsMeta( "</a>()\n" );
}

static void printHtmlShortMembers( HtmlWriter& out,
				   const QMap<QString, Decl *>& members,
				   const QString& header )
{
    bool metAny = FALSE;

    QMap<QString, Decl *>::ConstIterator m = members.begin();
    while ( m != members.end() ) {
	if ( !config->isInternal() && (*m)->isInternal() ) {
	    ++m;
	    continue;
	}

	if ( !metAny ) {
	    out.printfMeta( "<h2>%s</h2>\n", header.latin1() );
	    out.putsMeta( "<ul>\n" );
	    metAny = TRUE;
	}

	out.putsMeta( "<li><div class=fn>" );
	(*m)->printHtmlShort( out );
	if ( (*m)->isInternal() && (*m)->access() != Decl::Private )
	    out.putsMeta( " &nbsp;<em>(internal)</em>" );
	else if ( (*m)->isObsolete() )
	    out.putsMeta( " &nbsp;<em>(obsolete)</em>" );
	out.putsMeta( "</div></li>\n" );
	++m;
    }
    if ( metAny )
	out.putsMeta( "</ul>\n" );
}

static QString htmlShortName( const Decl *decl )
{
    QString html = htmlProtect( decl->name() );
    if ( !decl->isObsolete() )
	html = QString( "<b>" ) + html + QString( "</b>" );

    if ( decl->doc() == 0 ) {
	if ( decl->kind() == Decl::Function )
	    warning( 2, decl->location(), "Undocumented function '%s'",
		     decl->fullMangledName().latin1() );
	else if ( decl->kind() == Decl::Property )
	    warning( 2, decl->location(), "Undocumented property '%s'",
		     decl->fullName().latin1() );
	else
	    warning( 3, decl->location(), "Undocumented member '%s'",
		     decl->fullName().latin1() );
    } else if ( !decl->isObsolete() &&
		(config->isInternal() || !decl->isInternal()) ) {
	html = QString( "<a href=\"#%1\">%2</a>" ).arg( decl->ref() )
	       .arg( html );
    }
    return html;
}

static void printHtmlShortName( HtmlWriter& out, const Decl *decl )
{
    out.putsMeta( htmlShortName(decl).latin1() );
}

static void printHtmlLongMembers( HtmlWriter& out,
				  const QMap<QString, Decl *>& members,
				  const QString& header )
{
    bool metAny = FALSE;

    QMap<QString, Decl *>::ConstIterator m = members.begin();
    while ( m != members.end() ) {
	if ( !config->isInternal() && (*m)->isInternal() ) {
	    ++m;
	    continue;
	}

	if ( !metAny ) {
	    out.printfMeta( "<hr><h2>%s</h2>\n", header.latin1() );
	    metAny = TRUE;
	}

	out.putsMeta( "<h3 class=fn>" );
	(*m)->printHtmlLong( out );
	out.putsMeta( "</h3>" );
	(*m)->doc()->printHtml( out );

	if ( (*m)->reimplements() != 0 && (*m)->reimplements()->doc() != 0 &&
	     !(*m)->reimplements()->doc()->isInternal() &&
	     !(*m)->reimplements()->context()->isInternal() ) {
	    QString className = (*m)->reimplements()->context()->name();
	    out.printfMeta( "<p>Reimplemented from <a href=\"%s#%s\">%s</a>.\n",
			    config->classRefHref(className).latin1(),
			    (*m)->reimplements()->ref().latin1(),
			    className.latin1() );
	}

	QValueList<Decl *> by = (*m)->reimplementedBy();
	if ( !by.isEmpty() ) {
	    /*
	      We don't want totally uninteresting reimplementations
	      in this list.
	    */
	    QValueList<Decl *>::ConstIterator r;
	    r = by.begin();
	    while ( r != by.end() ) {
		Decl *d = *r;
		++r;
		if ( d->doc() == 0 || d->isInternal() ||
		     d->context()->isInternal() )
		    by.remove( d );
	    }
	}
	if ( !by.isEmpty() ) {
	    // ... so we probably never get here
	    QValueList<Decl *>::ConstIterator r;
	    QValueStack<QString> seps = separators( by.count(),
						    QString(".\n") );
	    out.putsMeta( "<p>Reimplemented in " );
	    r = by.begin();
	    while ( r != by.end() ) {
		QString className = (*r)->context()->fullName();

		if ( (*r)->context()->doc() == 0 ) {
		    out.puts( className.latin1() );
		} else {
		    out.printfMeta( "<a href=\"%s",
				    config->classRefHref(className)
				    .latin1() );
		    if ( (*r)->doc() != 0 )
			out.printfMeta( "#%s", (*r)->ref().latin1() );
		    out.printfMeta( "\">%s</a>", className.latin1() );
		}

		out.puts( seps.pop() );
		++r;
	    }
	}
	++m;
    }
}

static void tangle( Decl *child, QMap<QString, Decl *> *membersp,
		    QMap<QString, Decl *> *slotsp,
		    QMap<QString, Decl *> *signalsp,
		    QMap<QString, Decl *> *staticMembersp,
		    QMap<QString, Decl *> *memberTypesp,
		    QMap<QString, Decl *> *memberPropertiesp,
		    QMap<QString, Decl *> *memberFunctionsp )
{
#if 1
    static int unique = 0;
#endif

    // enum items are documented as part of the enum
    if ( child->kind() == Decl::EnumItem )
	return;

    // protect from anonymous enums
    if ( child->name().isEmpty() )
	return;

    if ( child->doc() != 0 ) {
	switch ( child->kind() ) {
	case ClassDecl::Enum:
	case ClassDecl::Typedef:
	    if ( memberTypesp != 0 )
		memberTypesp->insert( child->sortName(), child );
	    break;
	case ClassDecl::Function:
	    if ( memberFunctionsp != 0 )
		memberFunctionsp->insert( child->sortName(), child );
	    break;
	case ClassDecl::Property:
	    if ( memberPropertiesp != 0 )
		memberPropertiesp->insert( child->sortName(), child );
	    break;
	default:
	    ;
	}
    }

    QString sortKey;
#if 1
    sortKey.sprintf( "%.8x", unique++ );
#else
    sortKey = child->sortName();
#endif

    if ( child->kind() == ClassDecl::Function ) {
	FunctionDecl *funcDecl = (FunctionDecl *) child;

	if ( funcDecl->isSlot() ) {
	    if ( slotsp != 0 )
		slotsp->insert( sortKey, funcDecl );
	    return;
	} else if ( funcDecl->isSignal() ) {
	    if ( signalsp != 0 )
		signalsp->insert( sortKey, funcDecl );
	    return;
	} else if ( funcDecl->isStatic() ) {
	    if ( staticMembersp != 0 )
		staticMembersp->insert( sortKey, funcDecl );
	    return;
	}
    }
    if ( membersp != 0 && child->kind() != Decl::Property )
	membersp->insert( sortKey, child );
}

static void fillInImportantChildren( ClassDecl *classDecl,
				     QValueList<Decl *> *importantChildren )
{
    if ( classDecl->classDoc() == 0 ||
	 classDecl->classDoc()->important().isEmpty() )
	return;

    StringSet important = classDecl->classDoc()->important();
    StringSet importantMet;
    StringSet importantSignatures;

    QValueStack<ClassDecl *> stack;
    QValueList<Decl *>::ConstIterator ch;

    stack.push( classDecl );
    while ( !stack.isEmpty() ) {
	QValueList<CodeChunk>::ConstIterator st;
	const ClassDecl *c = stack.pop();

	st = c->superTypes().begin();
	while ( st != c->superTypes().end() ) {
	    Decl *bd = c->rootContext()->resolvePlain( (*st).base() );
	    if ( bd != 0 && bd->kind() == Decl::Class )
		stack.push( (ClassDecl *) bd );
	    ++st;
	}

	ch = c->children().begin();
	while ( ch != c->children().end() ) {
	    if ( important.contains((*ch)->name()) &&
		 !importantSignatures.contains((*ch)->mangledName()) ) {
		importantMet.insert( (*ch)->name() );
		importantSignatures.insert( (*ch)->mangledName() );
		if ( c != classDecl && (*ch)->doc() != 0 &&
		     !(*ch)->isInternal() && !(*ch)->isObsolete() )
		    importantChildren->append( *ch );
	    }
	    ++ch;
	}
    }

    StringSet importantNotMet = difference( important, importantMet );
    StringSet::ConstIterator i;

    i = importantNotMet.begin();
    while ( i != importantNotMet.end() ) {
	warning( 2, classDecl->classDoc()->location(),
		 "Important inherited member '%s' not found in header files",
		 (*i).latin1() );
	++i;
    }
}

QString Decl::ref( const QString& name )
{
    static QRegExp op( QString("^operator") );
    static QRegExp sp( QChar(' ') );
    static QRegExp amp( QChar('&') );
    static QRegExp lt( QChar('<') );
    static QRegExp eq( QChar('=') );
    static QRegExp gt( QChar('>') );

    if ( name.find(op) == 0 ) {
	QString t = name;
	t.replace( sp, QChar('-') );
	t.replace( amp, QString("-and") );
	t.replace( lt, QString("-lt") );
	t.replace( eq, QString("-eq") );
	t.replace( gt, QString("-gt") );
	return t;
    } else {
	return name;
    }
}

void Decl::setDoc( Doc *doc )
{
    if ( d != 0 ) {
	warning( 3, doc->location(), "Overrides a previous doc" );
	warning( 3, d->location(), "(the previous doc is here)" );
	delete d;
    }
    d = doc;
}

void Decl::buildMangledSymbolTables()
{
    QValueList<Decl *>::ConstIterator child;

    if ( symTable[MangledSymTable].isEmpty() ) {
	child = children().begin();
	while ( child != children().end() ) {
	    symTable[MangledSymTable].insert( (*child)->mangledName(), *child );
	    ++child;
	}
    }

    child = children().begin();
    while ( child != children().end() ) {
	(*child)->buildMangledSymbolTables();
	++child;
    }
}

void Decl::buildPlainSymbolTables( bool omitUndocumented )
{
    QValueList<Decl *>::ConstIterator child;

    if ( symTable[PlainSymTable].isEmpty() ) {
	/*
	  We want '::func' to mean 'func' in the root context.
	*/
	if ( this == rootContext() )
	    symTable[PlainSymTable].insert( QString(""), this );

	child = children().begin();
	while ( child != children().end() ) {
	    if ( !omitUndocumented ||
		 ((*child)->doc() != 0 && (config->isInternal() ||
					  !(*child)->isInternal())) ) {
		QString name = (*child)->name();
		if ( (*child)->kind() == Function )
		    name += parenParen;
		symTable[PlainSymTable].insert( name, *child );
	    }
	    ++child;
	}
    }

    child = children().begin();
    while ( child != children().end() ) {
	if ( !omitUndocumented ||
	     ((*child)->doc() != 0 && (config->isInternal() ||
				      !(*child)->isInternal())) )
	    (*child)->buildPlainSymbolTables( omitUndocumented );
	++child;
    }
}

void Decl::destructSymbolTables()
{
    symTable[MangledSymTable].clear();
    symTable[PlainSymTable].clear();
    QValueList<Decl *>::ConstIterator child = children().begin();
    while ( child != children().end() ) {
	(*child)->destructSymbolTables();
	++child;
    }
}

void Decl::fillInDecls()
{
    QValueList<Decl *>::ConstIterator child;

    if ( !declsFilledIn ) {
	fillInDeclsForThis();
	declsFilledIn = TRUE;
    }
    child = children().begin();
    while ( child != children().end() ) {
	(*child)->fillInDecls();
	++child;
    }
}

void Decl::fillInDocs()
{
    if ( !declsFilledIn )
	fillInDecls();

    if ( !docsFilledIn ) {
	fillInDocsForThis();
	docsFilledIn = TRUE;
    }

    QValueList<Decl *>::ConstIterator child = children().begin();
    while ( child != children().end() ) {
	(*child)->fillInDocs();
	++child;
    }
}

bool Decl::isInternal() const
{
    return access() == Private || ( doc() != 0 && doc()->isInternal() );
}

void Decl::setImportantChildren( const QValueList<Decl *>& important )
{
    imp = important;

    QValueList<Decl *>::ConstIterator i = important.begin();
    while ( i != important.end() ) {
	all.append( *i );
	++i;
    }
}

QString Decl::fullName() const
{
    if ( context() == 0 ) {
	return name();
    } else {
	QString m = context()->fullName();
	if ( !m.isEmpty() )
	    m += gulbrandsen;
	m += name();
	return m;
    }
}

QString Decl::mangledName() const
{
    return n;
}

QString Decl::fullMangledName() const
{
    if ( context() == 0 ) {
	return mangledName();
    } else {
	QString m = context()->fullMangledName();
	if ( !m.isEmpty() )
	    m += gulbrandsen;
	m += mangledName();
	return m;
    }
}

QString Decl::uniqueName() const
{
    return name();
}

QString Decl::sortName() const
{
    return uniqueName();
}

Decl *Decl::resolveMangled( const QString& relOrFullMangledName ) const
{
    Decl *d = resolveHere( MangledSymTable, relOrFullMangledName );
    if ( d == 0 && rootContext() != this )
	d = rootContext()->resolveHere( MangledSymTable, relOrFullMangledName );
    return d;
}

Decl *Decl::resolvePlain( const QString& relOrFullName ) const
{
    Decl *d = resolveHere( PlainSymTable, relOrFullName );
    if ( d == 0 && rootContext() != this )
	d = rootContext()->resolveHere( PlainSymTable, relOrFullName );
    return d;
}

void Decl::printHtmlShort( HtmlWriter& /* out */ ) const
{
}

void Decl::printHtmlLong( HtmlWriter& /* out */ ) const
{
}

Decl::Decl( Kind kind, const Location& loc, const QString& name, Decl *context )
    : k( kind ), a( Public ), d( 0 ), lo( loc ), n( name ), cura( Public ),
      c( context ), relc( context ), declsFilledIn( FALSE ),
      docsFilledIn( FALSE ), reimp( 0 )
{
    rootc = this;
    if ( context != 0 ) {
	// properties are always public, no matter where they are declared
	if ( kind != Property )
	    a = context->cura;
	rootc = context->rootc;

	QValueList<Decl *> *cat = 0;

	switch ( a ) {
	case Public:
	    cat = &context->pub;
	    break;
	case Protected:
	    cat = &context->prot;
	    break;
	case Private:
	    cat = &context->priv;
	}
	cat->append( this );
	context->all.append( this );
    }
}

void Decl::setReimplements( Decl *superDecl )
{
    if ( reimp != 0 ) {
	if ( doc() != 0 )
	    warning( 2, doc()->location(),
		     "Function '%s' somehow reimplements two functions at once",
		     name().latin1() );
    } else {
	reimp = superDecl;
	superDecl->reimpby.append( this );
    }
}

void Decl::setRelates( Decl *context )
{
    context->rel.append( this );
    context->all.append( this );
    relc = context;
}

Decl *Decl::resolveHere( int whichSymTable, const QString& name ) const
{
    if ( name.isEmpty() )
	return (Decl *) this;

    /*
      We have to be careful about 'a(B::C)'.
    */
    QMap<QString, Decl *>::ConstIterator s;
    QString left = name;
    QString right;

    int k = left.find( gulbrandsen );
    if ( k != -1 ) {
	int ell = left.find( QChar('(') );
	if ( ell == -1 || ell > k ) {
	    right = left.mid( k + 2 );
	    left.truncate( k );
	}
    }

    s = symTable[whichSymTable].find( left );
    if ( s == symTable[whichSymTable].end() )
	return 0;
    else
	return (*s)->resolveHere( whichSymTable, right );
}

ClassDecl::ClassDecl( const Location& loc, const QString& name, Decl *context )
    : Decl( Class, loc, name, context )
{
}

void ClassDecl::buildPlainSymbolTables( bool omitUndocumented )
{
    QValueStack<ClassDecl *> stack;
    QValueList<Decl *>::ConstIterator ch;

    stack.push( this );
    while ( !stack.isEmpty() ) {
	const ClassDecl *c = stack.pop();

	QValueList<CodeChunk>::ConstIterator st = c->superTypes().begin();
	while ( st != c->superTypes().end() ) {
	    Decl *bd = rootContext()->resolvePlain( (*st).base() );
	    if ( bd != 0 && bd->kind() == Decl::Class )
		stack.push( (ClassDecl *) bd );
	    ++st;
	}

	ch = c->children().begin();
	while ( ch != c->children().end() ) {
	    QString name = (*ch)->name();
	    bool documented;
	    if ( (*ch)->kind() == EnumItem )
		documented = ( ((EnumItemDecl *) *ch)->parentEnum()->doc() !=
			       0 );
	    else
		documented = ( (*ch)->doc() != 0 );

	    bool omit = ( omitUndocumented && !documented ) ||
			( !config->isInternal() && (*ch)->isInternal() );

	    if ( (*ch)->kind() == Decl::Function ) {
		FunctionDecl *funcDecl = (FunctionDecl *) *ch;
		if ( (c != this &&
		      (funcDecl->isConstructor() ||
		       funcDecl->isDestructor())) ||
		     funcDecl->overloadNumber() > 1 )
		    omit = TRUE;
		else
		    name += parenParen;
	    }
	    if ( !omit )
		symTable[PlainSymTable].insert( name, *ch, FALSE );
	    ++ch;
	}
    }
}

QString ClassDecl::whatsThis() const // ### needed?
{
    if ( classDoc() == 0 )
	return QString( "" );
    else
	return classDoc()->whatsThis();
}

void ClassDecl::printHtmlShort( HtmlWriter& out ) const
{
    out.putsMeta( "class" );
    if ( !name().isEmpty() ) {
	out.puts( " " );
	printHtmlShortName( out, this );
    }
    out.putsMeta( " { }" );
}

void ClassDecl::printHtmlLong( HtmlWriter& out ) const
{
    QMap<QString, Decl *> publicMembers;
    QMap<QString, Decl *> publicSlots;
    QMap<QString, Decl *> publicSignals;
    QMap<QString, Decl *> staticPublicMembers;
    QMap<QString, Decl *> importantInheritedMembers;
    QMap<QString, Decl *> properties;
    QMap<QString, Decl *> protectedMembers;
    QMap<QString, Decl *> protectedSlots;
    QMap<QString, Decl *> staticProtectedMembers;
    QMap<QString, Decl *> privateMembers;
    QMap<QString, Decl *> privateSlots;
    QMap<QString, Decl *> staticPrivateMembers;
    QMap<QString, Decl *> related;

    QMap<QString, Decl *> memberTypes;
    QMap<QString, Decl *> memberProperties;
    QMap<QString, Decl *> memberFunctions;
    QMap<QString, Decl *> relatedFunctions;

    QValueList<Decl *>::ConstIterator child;

    emitHtmlListOfAllMemberFunctions();

    out.setTitle( name() + QString(" Class") );
    out.setHeading( name() + QString(" Class Reference") );
    if ( classDoc() != 0 && !classDoc()->module().isEmpty() )
	out.setSubHeading( QString("[<a href=\"%1.html\">%2 module</a>]")
			   .arg(classDoc()->module().lower())
			   .arg(classDoc()->module()) );
    else if ( isObsolete() )
	out.setSubHeading( QString("[obsolete]") );

    if ( classDoc() != 0 ) {
	if ( classDoc()->isPreliminary() )
	    out.putsMeta( "<p><center><font color=\"red\"><b>The API for this"
			  " class is under development and is subject to"
			  " change.</b><br>\n"
			  " We do not recommend the use of this class for"
			  " production work at this time.</font></center>\n" );

#if 0
	// not yet
	QString img = config->classImageHref( name() );
	if ( !config->findDepth(img, config->imageDirList()).isEmpty() ) {
	    config->needImage( location(), img );
	    out.printfMeta( "<img align=\"right\" src=\"%s\">\n",
			    img.latin1() );
	}
#endif

	out.putsMeta( "<p>" );
	out.putsMeta( classDoc()->brief() );
	out.puts( "\n" );
	if ( !classDoc()->extension().isEmpty() )
	    out.printfMeta( "<p>This class is part of the"
			    " <b>%s %s Extension</b>.\n",
			    config->product().latin1(),
			    classDoc()->extension().latin1() );
    }

    out.putsMeta( "<a href=\"#details\">More...</a>\n" );

    if ( !hfile.isEmpty() )
	Doc::printHtmlIncludeHeader( out, hfile );

    QValueList<CodeChunk> mySupert;
    QValueList<CodeChunk> mySubt;
    QValueList<CodeChunk>::ConstIterator st;

    st = supert.begin();
    while ( st != supert.end() ) {
	if ( rootContext()->resolvePlain((*st).base()) != 0 )
	    mySupert.append( *st );
	++st;
    }

    st = subt.begin();
    while ( st != subt.end() ) {
	if ( rootContext()->resolvePlain((*st).base()) != 0 )
	    mySubt.append( *st );
	++st;
    }

    int n = mySupert.count();
    QValueStack<QString> seps = separators( n, QString(".\n") );

    if ( n > 0 ) {
	out.putsMeta( "<p>Inherits " );
	st = mySupert.begin();
	while ( st != mySupert.end() ) {
	    printHtmlDataType( out, *st, this );
	    out.puts( seps.pop() );
	    ++st;
	}
    }

    n = mySubt.count();
    seps = separators( n, QString(".\n") );

    if ( n > 0 ) {
	out.putsMeta( "<p>Inherited by " );
	qHeapSort( ((ClassDecl *) this)->subt );
	st = mySubt.begin();
	while ( st != mySubt.end() ) {
	    printHtmlDataType( out, *st, this );
	    out.puts( seps.pop() );
	    ++st;
	}
    }

    out.printfMeta( "<p><a href=\"%s\">List of all member functions.</a>\n",
		    config->classMembersHref(name()).latin1() );

    child = publicChildren().begin();
    while ( child != publicChildren().end() ) {
	tangle( *child, &publicMembers, &publicSlots, &publicSignals,
		&staticPublicMembers, &memberTypes, &memberProperties,
		&memberFunctions );
	++child;
    }

    child = importantChildren().begin();
    while ( child != importantChildren().end() ) {
	tangle( *child, &importantInheritedMembers, &importantInheritedMembers,
		&importantInheritedMembers, &importantInheritedMembers,
		&memberTypes, &memberProperties, &memberFunctions );
	++child;
    }

    child = protectedChildren().begin();
    while ( child != protectedChildren().end() ) {
	tangle( *child, &protectedMembers, &protectedSlots, 0,
		&staticProtectedMembers, &memberTypes, 0, &memberFunctions );
	++child;
    }

    if ( config->isInternal() ) {
	child = privateChildren().begin();
	while ( child != privateChildren().end() ) {
	    tangle( *child, &privateMembers, &privateSlots, 0,
		    &staticPrivateMembers, &memberTypes, 0, &memberFunctions );
	    ++child;
	}
    }

    child = relatedChildren().begin();
    while ( child != relatedChildren().end() ) {
	tangle( *child, &related, 0, 0, 0, 0, 0, &relatedFunctions );
	++child;
    }

    QValueList<PropertyDecl *>::ConstIterator p = props.begin();
    while ( p != props.end() ) {
	properties.insert( (*p)->name(), *p );
	++p;
    }

    printHtmlShortMembers( out, publicMembers, "Public Members" );
    printHtmlShortMembers( out, publicSlots, "Public Slots" );
    printHtmlShortMembers( out, publicSignals, "Signals" );
    printHtmlShortMembers( out, staticPublicMembers, "Static Public Members" );
    printHtmlShortMembers( out, importantInheritedMembers,
			   "Important Inherited Members" );
    printHtmlShortMembers( out, properties, "Properties" );
    printHtmlShortMembers( out, protectedMembers, "Protected Members" );
    printHtmlShortMembers( out, protectedSlots, "Protected Slots" );
    printHtmlShortMembers( out, staticProtectedMembers,
			  "Static Protected Members" );
    printHtmlShortMembers( out, privateMembers, "Private Members" );
    printHtmlShortMembers( out, privateSlots, "Private Slots" );
    printHtmlShortMembers( out, staticPrivateMembers,
			   "Static Private Members" );
    printHtmlShortMembers( out, related, "Related Functions" );

    out.putsMeta( "<hr><a name=\"details\"></a>"
		  "<h2>Detailed Description</h2>\n" );

    if ( classDoc() != 0 )
	classDoc()->printHtml( out );

    printHtmlLongMembers( out, memberTypes, "Member Type Documentation" );
    printHtmlLongMembers( out, memberFunctions,
			  "Member Function Documentation" );
    printHtmlLongMembers( out, memberProperties, "Property Documentation" );
    printHtmlLongMembers( out, relatedFunctions, "Related Functions" );

    if ( !config->footer().isEmpty() ) {
	out.enterFooter();
	out.putsMeta( config->footer().latin1() );
    }
}

void ClassDecl::fillInDeclsForThis()
{
    QValueStack<ClassDecl *> stack;
    QMap<QString, Decl *>::ConstIterator ent;

    /*
      Explore the ancestry of this class.
    */
    stack.push( this );
    while ( !stack.isEmpty() ) {
	ClassDecl *c = stack.pop();

	QValueList<CodeChunk>::ConstIterator st = c->superTypes().begin();
	while ( st != c->superTypes().end() ) {
	    Decl *bd = rootContext()->resolveMangled( (*st).base() );
	    if ( bd != 0 && bd->kind() == Decl::Class ) {
		stack.push( (ClassDecl *) bd );
		if ( c == this )
		    stack.top()->subt.append( CodeChunk(name()) );
	    }
	    ++st;
	}

	if ( c != this ) {
	    c->fillInDecls();

	    ent = symTable[MangledSymTable].begin();
	    while ( ent != symTable[MangledSymTable].end() ) {
		if ( (*ent)->kind() == Decl::Function ) {
		    FunctionDecl *funcDecl = (FunctionDecl *) *ent;
		    Decl *superDecl = c->resolveMangled( ent.key() );
		    if ( superDecl != 0 &&
			 superDecl->kind() == Decl::Function ) {
			FunctionDecl *superFunc = (FunctionDecl *) superDecl;
			if ( superFunc->isVirtual() ) {
			    funcDecl->setVirtual( TRUE );
			    funcDecl->setReimplements( superFunc );
			}
		    }
		}
		++ent;
	    }
	}
    }
}

/*
  Reports undocumented parameters.
*/
static void checkParams( const FunctionDecl *funcDecl,
			 const StringSet& declared )
{
    FnDoc *fn = funcDecl->fnDoc();
    if ( fn == 0 )
	return;

    if ( funcDecl->isInternal() || funcDecl->isObsolete() )
	return;

    StringSet diff;
    StringSet::ConstIterator s;

    diff = difference( declared, fn->documentedParameters() );
    s = diff.begin();
    while ( s != diff.end() ) {
	warning( 3, fn->location(), "Undocumented parameter '%s'",
		 (*s).latin1() );
	++s;
    }
}

void ClassDecl::fillInDocsForThis()
{
    /*
      Provide standard documentation for property getters and
      (re)setters. Also append the standard text "Get the property's
      value..."
    */
    QMap<QString, PropertyDecl *> propertyMap;
    QMap<QString, PropertyDecl *>::Iterator q;

    QValueList<PropertyDecl *>::ConstIterator p = props.begin();
    while ( p != props.end() ) {
	PropertyDoc *pd = (*p)->propertyDoc();
	if ( pd != 0 ) {
	    if ( !(*p)->readFunction().isEmpty() )
		propertyMap.insert( (*p)->readFunction(), *p );
	    if ( !(*p)->writeFunction().isEmpty() )
		propertyMap.insert( (*p)->writeFunction(), *p );
	    if ( !(*p)->resetFunction().isEmpty() )
		propertyMap.insert( (*p)->resetFunction(), *p );

	    pd->setFunctions( (*p)->readFunction(),
			      Decl::ref((*p)->readFunction()),
			      (*p)->writeFunction(),
			      Decl::ref((*p)->writeFunction()),
			      (*p)->resetFunction(),
			      Decl::ref((*p)->resetFunction()) );
	}
	++p;
    }

    QValueList<Decl *>::ConstIterator child = children().begin();
    while ( child != children().end() ) {
	if ( (*child)->kind() == Function ) {
	    FunctionDecl *func = (FunctionDecl *) *child;
	    q = propertyMap.find( func->name() );
	    if ( q != propertyMap.end() ) {
		QString html;
		StringSet documentedParams;

		QString brief = (*q)->propertyDoc()->brief();
		bool whether = brief.startsWith( QString("whether ") );

		/*
		  The function has the right name. Let's see if it
		  also has the right parameter type.
		*/
		if ( func->name() == (*q)->readFunction() ) {
		    if ( func->parameters().count() == 0 ) {
			if ( whether )
			    html = QString( "Returns TRUE if %1; otherwise"
					    " returns FALSE" )
				   .arg( brief.mid(8) );
			else
			    html = QString( "Returns " ) + brief;
		    }
		} else if ( func->name() == (*q)->resetFunction() ) {
		    if ( func->parameters().count() == 0 )
			html = QString( "Resets " ) + brief;
		} else {
		    if ( func->parameters().count() == 1 ) {
			QString type = func->parameters().first().dataType()
				       .toString();
			if ( type.find((*q)->dataType().toString()) != -1 ||
			     type == QString("int") ||
			     type == QString("uint") ) {
			    html = QString( "Sets " ) + brief;
			    if ( !func->parameters().first().name().isEmpty() )
				html += QString( " to <em>" ) +
					func->parameters().first().name() +
					QString( "</em>" );
			}
		    }
		}

		// pretend all parameters are documented, for now
		documentedParams = func->parameterNames();

		if ( !html.isEmpty() ) {
		    html.prepend( QString("<p>") );
		    html += QString( ".\nSee the <a href=\"%1#%2\">\"%3\"</a>"
				     " property for details.\n" );
		    html = html.arg( config->classRefHref(name()) )
			       .arg( (*q)->ref() )
			       .arg( (*q)->name() );

		    Doc *doc = new FnDoc( (*q)->location(), html, QString::null,
					  QString::null, documentedParams,
					  FALSE );
		    doc->setObsolete( (*q)->isObsolete() );
		    func->setDoc( doc );
		    func->setRelatedProperty( *q );
		}
	    }
	}
	++child;
    }

    /*
      Do some other stuff.
    */
    QValueList<Decl *> importantChildren;

    QMap<QString, QValueList<FunctionDecl *> > fmap;
    QMap<QString, QValueList<FunctionDecl *> >::Iterator f;
    QValueList<FunctionDecl *>::ConstIterator g;

    fillInImportantChildren( this, &importantChildren );
    setImportantChildren( importantChildren );

    /*
      Build fmap to map a function name to its associated functions.
    */
    child = children().begin();
    while ( child != children().end() ) {
	if ( (*child)->kind() == Function )
	    fmap[(*child)->name()].append( (FunctionDecl *) *child );
	++child;
    }

    /*
      Constructors are a special case. If a class possesses many
      constructors, none is a reimplementation of the others.
      Internally, we'll elect the first constructor as the canonical
      one.
    */
    f = fmap.find( name() );
    if ( f != fmap.end() ) {
	/*
	  First pass: Get rid of extra '\overload's.
	*/
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    if ( (*g)->fnDoc() != 0 && (*g)->fnDoc()->overloads() ) {
#if 0
		// this might be right or wrong
		warning( 3, (*g)->fnDoc()->location(),
			 "Suspicious '\\overload' in doc for constructor" );
#endif
		(*g)->fnDoc()->setOverloads( FALSE );
	    }

	    // a great place to do something unrelated
	    if ( !isInternal() )
		checkParams( *g, (*g)->parameterNames() );
	    ++g;
	}

	/*
	  Second pass: Put '\overload's for all functions but the first one.
	*/
	int overloadNo = 1;
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    (*g)->setOverloadNumber( overloadNo );
	    if ( overloadNo > 1 && (*g)->fnDoc() != 0 )
		(*g)->fnDoc()->setOverloads( TRUE );
	    overloadNo++;
	    ++g;
	}

	fmap.remove( f );
    }

    /*
      General case: anything but constructors.
    */
    f = fmap.begin();
    while ( f != fmap.end() ) {
	const int NumCandidateLists = 3;
	QValueList<FunctionDecl *> candidates[NumCandidateLists];

	FnDoc *scapeGoat = 0;
	int overloadNo = 2;

	/*
	  First pass: Fill in the candidate lists. Among the
	  candidates, we'll choose a canonical version and make sure
	  all the others are '\overload's

	  Ideally, the situation is this: All versions of the
	  function except one are marked '\overload'. Unfortunately,
	  on Qt, this policy led to hundreds of warnings about
	  missing '\overload's. To avoid that, we distinguish
	  candidates according to their quality (or rather badness).
	  A normal function has badness 0, an obsolete one has
	  badness 1, and an internal one has badness 2. So if there
	  are five versions of 'Foo::foo()' (without any '\overload')
	  of badness 0, 1, 1, 2, 2, qdoc elects the version with
	  badness 0 as the canonical one, without complaining about
	  missing '\overload's.

	  There are three candidate lists, to distinguish between the
	  three quality levels. The best candidate should fall in
	  candidates[0].
	*/
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    /*
	      Make sure there is no clash with overload numbers for
	      '\important' members.
	    */
	    if ( (*g)->overloadNumber() >= overloadNo )
		overloadNo = (*g)->overloadNumber() + 1;

	    if ( (*g)->fnDoc() != 0 ) {
		if ( (*g)->fnDoc()->overloads() ) {
		    if ( scapeGoat == 0 )
			scapeGoat = (*g)->fnDoc();
		} else if ( (*g)->isInternal() ) {
		    candidates[2].append( *g );
		} else if ( (*g)->isObsolete() ) {
		    candidates[1].append( *g );
		} else if ( (*g)->doc() != 0 ) {
		    candidates[0].append( *g );
		}
	    }
	    ++g;
	}

	/*
	  Skip to the less bad nonempty list of candidates.
	*/
	int badness = 0;
	while ( badness < NumCandidateLists && candidates[badness].isEmpty() )
	    badness++;

	/*
	  No candidates at all?
	*/
	FunctionDecl *canonical = 0;
	if ( badness == NumCandidateLists ) {
	    if ( scapeGoat != 0 ) {
		warning( 2, scapeGoat->location(),
			 "All documented versions of this function are"
			 " '\\overload'" );
		scapeGoat->setOverloads( FALSE );
	    }
	} else {
	    /*
	      Ideally, there would be a single best candidate.
	    */
	    g = candidates[badness].begin();
	    canonical = *g;

	    /*
	      Complain about candidates that are as good as the
	      elected one and make them implicitly '\overload'.
	    */
	    ++g;
	    while ( g != candidates[badness].end() ) {
		if ( badness < 2 )
		    warning( 4, (*g)->fnDoc()->location(),
			     "Missing '\\overload'" );
		(*g)->fnDoc()->setOverloads( TRUE );
		++g;
	    }

	    /*
	      Make bad candidates implicitly '\overload'.
	    */
	    badness++;
	    while ( badness < NumCandidateLists ) {
		g = candidates[badness].begin();
		while ( g != candidates[badness].end() ) {
		    (*g)->fnDoc()->setOverloads( TRUE );
		    ++g;
		}
		badness++;
	    }
	}

	/*
	  Second pass: Assign overload numbers. If the canonical
	  function is internal or obsolete, the other documented
	  functions inherit this property.
	*/
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    if ( (*g)->overloadNumber() == 1 &&
		 ((*g)->fnDoc() == 0 || (*g)->fnDoc()->overloads()) )
		(*g)->setOverloadNumber( overloadNo++ );

	    if ( canonical != 0 && (*g)->fnDoc() != 0 ) {
		if ( canonical->isInternal() )
		    (*g)->fnDoc()->setInternal( TRUE );
		if ( canonical->isObsolete() )
		    (*g)->fnDoc()->setObsolete( TRUE );
	    }

	    /*
	      Here is another great place to do something unrelated
	      to assigning overload numbers. Parameters inherited
	      from the canonical function need no documentation.
	    */
	    if ( canonical == 0 || (*g) == canonical )
		checkParams( *g, (*g)->parameterNames() );
	    else
		checkParams( *g, difference((*g)->parameterNames(),
					    canonical->parameterNames()) );
	    ++g;
	}
	++f;
    }
}

void ClassDecl::emitHtmlListOfAllMemberFunctions() const
{
    QMap<QString, Decl *> all = symTable[PlainSymTable];
    QMap<QString, Decl *>::ConstIterator f;

    HtmlWriter out( location(), config->classMembersHref(name()) );
    out.setTitle( name() + QString(" Member List") );
    out.setHeading( QString("Complete Member List for ") + name() );

    out.printfMeta( "<p>This is the complete list of member functions for\n"
		    "<a href=\"%s\">%s</a>, including inherited members.\n\n",
		    config->classRefHref(fullName()).latin1(),
		    name().latin1() );
    out.putsMeta( "<ul>\n" );

    /*
      Put the constructor and destructor first.
    */
    if ( all.contains(name() + parenParen) )
	printHtmlListEntry( out, name(), name() );
    if ( all.contains(QChar('~') + name() + parenParen) )
	printHtmlListEntry( out, QChar('~') + name(), name() );

    f = all.begin();
    while ( f != all.end() ) {
	if ( (*f)->kind() == Decl::Function ) {
	    FunctionDecl *funcDecl = (FunctionDecl *) *f;
	    if ( !funcDecl->isConstructor() && !funcDecl->isDestructor() &&
		 funcDecl->context() == funcDecl->relatesContext() )
		printHtmlListEntry( out, funcDecl->name(),
				    funcDecl->context()->name() );
	}
	++f;
    }

    out.putsMeta( "</ul>\n" );
}

Parameter::Parameter( const CodeChunk& type, const QString& name,
		      const CodeChunk& defaultValue )
    : t( type ), n( name ), d( defaultValue )
{
}

Parameter::Parameter( const Parameter& p )
    : t( p.t ), n( p.n ), d( p.d )
{
}

Parameter& Parameter::operator=( const Parameter& p )
{
    t = p.t;
    n = p.n;
    d = p.d;
    return *this;
}

void Parameter::printHtmlShort( HtmlWriter& out ) const
{
    dataType().printHtml( out, QString::null, name() );
    if ( !defaultValue().isEmpty() ) {
	out.printfMeta( " = " );
	defaultValue().printHtml( out );
    }
}

void Parameter::printHtmlLong( HtmlWriter& out, const Decl *context ) const
{
    printHtmlDataType( out, dataType(), context, name() );
    if ( !defaultValue().isEmpty() ) {
	out.printfMeta( " = " );
	defaultValue().printHtml( out );
    }
}

FunctionDecl::FunctionDecl( const Location& loc, const QString& name,
			    Decl *context, const CodeChunk& returnType )
    : Decl( Function, loc, name, context ), r( returnType ), c( FALSE ),
      st( FALSE ), v( FALSE ), p( FALSE ), si( FALSE ), sl( FALSE ), ovo( 1 ),
      prop( 0 )
{
}

QString FunctionDecl::mangledName() const
{
    QString m = name();

    m += QChar( '(' );
    ParameterList::ConstIterator param = parameters().begin();
    if ( param != parameters().end() ) {
	m += (*param).dataType().toString();
	param++;
    }
    while ( param != parameters().end() ) {
	m += QString( ", " );
	m += (*param).dataType().toString();
	++param;
    }
    m += QChar( ')' );
    if ( isConst() )
	m += QString( " const" );
    return m;
}

QString FunctionDecl::uniqueName() const
{
    // the overload number is in base 36 to get decent alphabetical order
    return overloadNumber() == 1 ? name()
	   : name() + QChar( '-' ) + QString::number( overloadNumber(), 36 );
}

QString FunctionDecl::sortName() const
{
    int prio;
    if ( isConstructor() )
	prio = 1;
    else if ( isDestructor() )
	prio = 2;
    else
	prio = 3;

    // functions go to the end, if they are mixed with other things
    return QChar( 'z' + prio ) + uniqueName();
}

void FunctionDecl::setOverloadNumber( int no )
{
    ovo = no;
    if ( ovo != 1 && doc() != 0 )
	doc()->setLink( config->classRefHref(relatesContext()->name()) +
			QChar('#') + ref(),
			fullName() );
}

void FunctionDecl::addParameter( const Parameter& param )
{
    pl.append( param );
    if ( !param.name().isEmpty() )
	ps.insert( param.name() );
}

void FunctionDecl::borrowParameterNames( ParameterList::ConstIterator p )
{
    QValueList<Parameter>::Iterator oldp;

    ps.clear();
    oldp = pl.begin();
    while ( oldp != pl.end() ) {
	if ( !(*p).name().isEmpty() && (*p).name() != (*oldp).name() )
	    (*oldp).setName( (*p).name() );

	if ( !(*oldp).name().isEmpty() )
	    ps.insert( (*oldp).name() );
	++oldp;
	++p;
    }
}

bool FunctionDecl::isConstructor() const
{
    return context() != 0 && context()->name() == name();
}

bool FunctionDecl::isDestructor() const
{
    return name()[0] == QChar( '~' );
}

void FunctionDecl::printHtmlShort( HtmlWriter& out ) const
{
    if ( isVirtual() )
	out.putsMeta( "virtual " );
    if ( !returnType().isEmpty() ) {
	returnType().printHtml( out );
	out.putsMeta( " " );
    }
    printHtmlShortName( out, this );
    out.putsMeta( " (" );

    ParameterList::ConstIterator param = parameters().begin();
    if ( param != parameters().end() ) {
	out.putsMeta( " " );
	(*param).printHtmlShort( out );
	while ( ++param != parameters().end() ) {
	    out.putsMeta( ", " );
	    (*param).printHtmlShort( out );
	}
	out.putsMeta( " " );
    }
    out.putsMeta( ")" );

    if ( isConst() )
	out.putsMeta( " const" );
}

void FunctionDecl::printHtmlLong( HtmlWriter& out ) const
{
    if ( !returnType().isEmpty() ) {
	printHtmlDataType( out, returnType(), context() );
	out.putsMeta( " " );
    }
    out.printfMeta( "<a name=\"%s\"></a>%s (", ref().latin1(),
		    htmlProtect(fullName()).latin1() );

    ParameterList::ConstIterator param = parameters().begin();
    if ( param != parameters().end() ) {
	out.putsMeta( " " );
	(*param).printHtmlLong( out, context() );
	while ( ++param != parameters().end() ) {
	    out.putsMeta( ", " );
	    (*param).printHtmlLong( out, context() );
	}
	out.putsMeta( " " );
    }
    out.putsMeta( ")" );

    if ( isConst() )
	out.putsMeta( " const" );

    QString bracketedStuff;
    if ( isVirtual() )
	bracketedStuff += QString( " virtual" );
    if ( isStatic() )
	bracketedStuff += QString( " static" );

    if ( access() == Decl::Protected )
	bracketedStuff += QString( " protected" );
    else if ( access() == Decl::Private )
	bracketedStuff += QString( " private" );

    if ( isSignal() )
	bracketedStuff += QString( " signal" );
    if ( isSlot() )
	bracketedStuff += QString( " slot" );

    if ( !bracketedStuff.isEmpty() ) {
	bracketedStuff[0] = QChar( '[' );
	bracketedStuff.prepend( QString("<tt> ") );
	bracketedStuff.append( QString("]</tt>") );
	out.putsMeta( bracketedStuff.latin1() );
    }
    out.putsMeta( "\n" );
}

EnumItemDecl::EnumItemDecl( const Location& loc, const QString& name,
			    EnumDecl *parentEnum, const CodeChunk& value )
    : Decl( EnumItem, loc, name, parentEnum->context() ),
      enumDecl( parentEnum ), v( value )
{
}

QString EnumItemDecl::uniqueName() const
{
    // use parent's name as the link
    return enumDecl->uniqueName();
}

void EnumItemDecl::printHtmlShort( HtmlWriter& out ) const
{
    out.putsMeta( name().latin1() );
    if ( !value().isEmpty() ) {
	out.putsMeta( " = " );
	value().printHtml( out );
    }
}

EnumDecl::EnumDecl( const Location& loc, const QString& name, Decl *context )
    : Decl( Enum, loc, name, context )
{
}

QString EnumDecl::uniqueName() const
{
    return name() + QString( "-enum" );
}

void EnumDecl::printHtmlShort( HtmlWriter& out ) const
{
    out.putsMeta( "enum" );
    if ( !name().isEmpty() ) {
	out.printfMeta( " " );
	printHtmlShortName( out, this );
    }
    out.putsMeta( " {" );

    ItemIterator i = itemBegin();
    if ( i != itemEnd() ) {
	out.putsMeta( " " );
	(*i)->printHtmlShort( out );
	while ( ++i != itemEnd() ) {
	    out.putsMeta( ", " );
	    (*i)->printHtmlShort( out );
	}
    }
    out.putsMeta( " }" );
}

void EnumDecl::printHtmlLong( HtmlWriter& out ) const
{
    out.printfMeta( "<a name=\"%s\"></a>%s", ref().latin1(),
		    fullName().latin1() );
}

TypedefDecl::TypedefDecl( const Location& loc, const QString& name,
			  Decl *context, const CodeChunk& type )
    : Decl( Typedef, loc, name, context ), t( type )
{
}

void TypedefDecl::printHtmlShort( HtmlWriter& out ) const
{
    out.putsMeta( "typedef " );
    t.printHtml( out, QString::null, htmlShortName(this) );
}

void TypedefDecl::printHtmlLong( HtmlWriter& out ) const
{
    out.printfMeta( "<a name=\"%s\"></a>%s", ref().latin1(),
		    fullName().latin1() );
}

PropertyDecl::PropertyDecl( const Location& loc, const QString& name,
			    Decl *context, const CodeChunk& type )
    : Decl( Property, loc, name, context ), t( type ), store( Tdef ),
      design( Tdef )
{
    if ( context->kind() == Class )
	((ClassDecl *) context)->addProperty( this );
}

QString PropertyDecl::uniqueName() const
{
    return name() + QString( "-prop" );
}

void PropertyDecl::printHtmlShort( HtmlWriter& out ) const
{
    dataType().printHtml( out );
    out.puts( " " );
    printHtmlShortName( out, this );
    if ( propertyDoc() != 0 && !propertyDoc()->brief().isEmpty() ) {
	out.putsMeta( "&nbsp;- " );
	out.putsMeta( propertyDoc()->brief() );
    }
    if ( writeFunction().isEmpty() )
	out.putsMeta( " &nbsp;<em>(read only)</em>" );
}

void PropertyDecl::printHtmlLong( HtmlWriter& out ) const
{
    printHtmlDataType( out, dataType(), context() );
    out.printfMeta( " <a name=\"%s\"></a>%s", ref().latin1(), name().latin1() );
}

PropertyDecl::Trool PropertyDecl::toTrool( bool b )
{
    return b ? Ttrue : Tfalse;
}

bool PropertyDecl::fromTrool( Trool tr, bool def )
{
    return tr == Tdef ? def : tr == Ttrue;
}
