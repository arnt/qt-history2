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
#include "htmlwriter.h"
#include "messages.h"

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
    out.printfMeta( "<li><a href=\"%s#%s\">%s</a>\n",
		    config->classRefHref(className).latin1(),
		    Decl::anchor(funcName).latin1(), funcName.latin1() );
}

static void printHtmlShortMembers( HtmlWriter& out,
				   const QValueList<Decl *>& members,
				   const QString& header )
{
    if ( !members.isEmpty() ) {
	out.printfMeta( "<h2>%s</h2>\n", header.latin1() );
	out.putsMeta( "<ul>\n" );

	QValueList<Decl *>::ConstIterator m = members.begin();
	while( m != members.end() ) {
	    if ( config->isInternal() || !(*m)->internal() ) {
		out.putsMeta( "<li><div class=fn>" );
		(*m)->printHtmlShort( out );
		if ( (*m)->internal() && (*m)->access() != Decl::Private )
		    out.putsMeta( "&nbsp; <em>(internal)</em>" );
		else if ( (*m)->obsolete() )
		    out.putsMeta( "&nbsp; <em>(obsolete)</em>" );
		out.putsMeta( "</div></li>\n" );
	    }
	    ++m;
	}
	out.putsMeta( "</ul>\n" );
    }
}

// ### move inside class?
static QString htmlShortName( const Decl *decl )
{
    QString html = decl->name();
    if ( !decl->obsolete() )
	html = QString( "<b>" ) + html + QString( "</b>" );

    if ( decl->doc() == 0 ) {
	if ( decl->kind() == Decl::Function )
	    warning( 2, decl->location(), "Undocumented function '%s'",
		     decl->fullMangledName().latin1() );
	else
	    warning( 3, decl->location(), "Undocumented member '%s'",
		     decl->fullName().latin1() );
    } else if ( (config->isInternal() || !decl->internal()) &&
		!decl->obsolete() ) {
	html = QString( "<a href=\"#%1\">%2</a>" ).arg( decl->anchor() )
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
    if ( !members.isEmpty() ) {
	out.printfMeta( "<hr><h2>%s</h2>\n", header.latin1() );

	QMap<QString, Decl *>::ConstIterator m = members.begin();
	while ( m != members.end() ) {
	    if ( !config->isInternal() && (*m)->internal() ) {
		++m;
		continue;
	    }

	    out.putsMeta( "<h3 class=fn>" );
	    (*m)->printHtmlLong( out );
	    out.putsMeta( "</h3>" );
	    (*m)->doc()->printHtml( out );

	    if ( (*m)->reimplements() != 0 ) {
		QString className = (*m)->reimplements()->context()->name();
		out.printfMeta( "<p>Reimplemented from <a href=\"%s\"",
				config->classRefHref(className).latin1() );
		if ( (*m)->reimplements()->doc() != 0 )
		    out.printfMeta( "#%s",
				    (*m)->reimplements()->anchor().latin1() );
		out.printfMeta( ">%s</a>.\n", className.latin1() );
	    }

	    QValueList<Decl *> by = (*m)->reimplementedBy();
	    if ( !by.isEmpty() ) {
		// we don't want totally uninteresting
		// reimplementations in this list.
		QValueList<Decl *>::ConstIterator r;
		r = by.begin();
		while( r != by.end() ) {
		    Decl * d = *r;
		    ++r;
		    if ( d->internal() )
			by.remove( d );
		}
	    }
	    if ( !by.isEmpty() ) {
		// ... so we probably never get here.
		QValueList<Decl *>::ConstIterator r;
		QValueStack<QString> seps = separators( by.count(),
							QString(".\n") );
		out.putsMeta( "<p>Reimplemented in " );
		r = by.begin();
		while ( r != by.end() ) {
		    QString className = (*r)->context()->fullName();
		    out.printfMeta( "<a href=\"%s\"",
				    config->classRefHref(className).latin1() );
		    if ( (*r)->doc() != 0 )
			out.printfMeta( "#%s", (*r)->anchor().latin1() );
		    out.printfMeta( ">%s</a>", className.latin1() );
		    out.puts( seps.pop() );
		    ++r;
		}
	    }
	    ++m;
	}
    }
}

static void tangle( Decl *child, QValueList<Decl *> *membersp,
		    QValueList<Decl *> *slotsp, QValueList<Decl *> *signalsp,
		    QValueList<Decl *> *staticMembersp,
		    QMap<QString, Decl *> *memberTypesp,
		    QMap<QString, Decl *> *memberFunctionsp )
{
    if ( child->name().isEmpty() ) // protect from anonymous enums
	return;

    if ( child->doc() != 0 ) {
	if ( child->kind() == ClassDecl::Enum ||
	     child->kind() == ClassDecl::Typedef ) {
	    if ( memberTypesp != 0 )
		memberTypesp->insert( child->sortName(), child );
	} else if ( child->kind() == ClassDecl::Function ) {
	    if ( memberFunctionsp != 0 )
		memberFunctionsp->insert( child->sortName(), child );
	}
    }

    if ( child->kind() == ClassDecl::Function ) {
	FunctionDecl *funcDecl = (FunctionDecl *) child;

	if ( funcDecl->isSlot() ) {
	    if ( slotsp != 0 )
		slotsp->append( funcDecl );
	    return;
	} else if ( funcDecl->isSignal() ) {
	    if ( signalsp != 0 )
		signalsp->append( funcDecl );
	    return;
	} else if ( funcDecl->isStatic() ) {
	    if ( staticMembersp != 0 )
		staticMembersp->append( funcDecl );
	    return;
	}
    }
    if ( membersp != 0 )
	membersp->append( child );
}

static void fillInImportantChildren( ClassDecl *classDecl,
				     QValueList<Decl *> *importantChildren )
{
    StringSet important, importantMet;
    if ( classDecl->classDoc() != 0 )
	important = classDecl->classDoc()->important();
    StringSet importantSignatures;

    QValueStack<ClassDecl *> stack;
    QValueList<Decl *>::ConstIterator ch;

    stack.push( classDecl );
    while ( !stack.isEmpty() ) {
	QValueList<CodeChunk>::ConstIterator st;
	const ClassDecl *c = stack.pop();

	st = c->superTypes().begin();
	while ( st != c->superTypes().end() ) {
	    Decl *bd = classDecl->rootContext()->resolvePlain( (*st).base() );
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
		     !(*ch)->internal() && !(*ch)->obsolete() )
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

QString Decl::anchor( const QString& name )
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

void Decl::destructMangledSymbolTables()
{
    symTable[MangledSymTable].clear();
    QValueList<Decl *>::ConstIterator child = children().begin();
    while ( child != children().end() ) {
	(*child)->destructMangledSymbolTables();
	++child;
    }
}

void Decl::buildPlainSymbolTables()
{
    if ( doc() == 0 && this != rootContext() ) // ### unify
	return;

    QValueList<Decl *>::ConstIterator child;

    if ( symTable[PlainSymTable].isEmpty() ) {
	/*
	  We want '::func' to mean 'func' in the root context.
	*/
	if ( this == rootContext() )
	    symTable[PlainSymTable].insert( QString(""), this );

	child = children().begin();
	while ( child != children().end() ) {
	    symTable[PlainSymTable].insert( (*child)->uniqueName(), *child );
	    ++child;
	}
    }

    child = children().begin();
    while ( child != children().end() ) {
	(*child)->buildPlainSymbolTables();
	++child;
    }
}

void Decl::fillInDecls()
{
    QValueList<Decl *>::ConstIterator child;

    if ( !declsFilledIn ) {
	fillInDeclsThis();
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
	fillInDocsThis();
	docsFilledIn = TRUE;
    }

    QValueList<Decl *>::ConstIterator child = children().begin();
    while ( child != children().end() ) {
	(*child)->fillInDocs();
	++child;
    }
}

bool Decl::internal() const
{
    return access() == Private || ( doc() != 0 && doc()->internal() );
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
	    m += QString( "::" );
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
	    m += QString( "::" );
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

void Decl::printHtmlShort( HtmlWriter& out ) const
{
    out.printFnord();
}

void Decl::printHtmlLong( HtmlWriter& out ) const
{
    out.printFnord();
}

void Decl::fillInDeclsThis()
{
}

void Decl::fillInDocsThis()
{
}

Decl::Decl( Kind kind, const Location& loc, const QString& name, Decl *context )
    : k( kind ), a( Public ), d( 0 ), lo( loc ), n( name ), cura( Public ),
      c( context ), relc( context ), rootc( this ), declsFilledIn( FALSE ),
      docsFilledIn( FALSE ), reimp( 0 )
{
    if ( context != 0 ) {
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
    static QString *gulbrandsen = 0;

    if ( gulbrandsen == 0 )
	gulbrandsen = new QString( "::" );

    if ( name.isEmpty() )
	return (Decl *) this;

    /*
      We have to be careful about 'a(B::C)'.
    */
    QMap<QString, Decl *>::ConstIterator s;
    QString left = name;
    QString right;

    int k = left.find( *gulbrandsen );
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

void ClassDecl::buildPlainSymbolTables()
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
	    bool omit = FALSE;
	    if ( (*ch)->kind() == Decl::Function ) {
		FunctionDecl *funcDecl = (FunctionDecl *) *ch;
		if ( c != this && (funcDecl->isConstructor() ||
				   funcDecl->isDestructor()) )
		    omit = TRUE;
		if ( funcDecl->overloadNumber() > 1 )
		    omit = TRUE;
	    }
	    if ( !omit && !symTable[PlainSymTable].contains((*ch)->name()) )
		symTable[PlainSymTable].insert( (*ch)->name(), *ch );
	    ++ch;
	}
    }
}

QString ClassDecl::whatsThis() const
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
    QValueList<Decl *> publicMembers;
    QValueList<Decl *> publicSlots;
    QValueList<Decl *> publicSignals;
    QValueList<Decl *> staticPublicMembers;
    QValueList<Decl *> importantInheritedMembers;
    QValueList<Decl *> protectedMembers;
    QValueList<Decl *> protectedSlots;
    QValueList<Decl *> staticProtectedMembers;
    QValueList<Decl *> privateMembers;
    QValueList<Decl *> privateSlots;
    QValueList<Decl *> staticPrivateMembers;
    QValueList<Decl *> related;

    QMap<QString, Decl *> memberTypes;
    QMap<QString, Decl *> memberFunctions;
    QMap<QString, Decl *> relatedFunctions;

    QValueList<Decl *>::ConstIterator child;

    emitHtmlListOfAllMemberFunctions();

    out.setTitle( name() + QString(" Class") );
    out.setHeading( name() + QString(" Class Reference") );
    if ( classDoc() != 0 && !classDoc()->module().isEmpty() )
	out.setSubHeading( QString("[ <a href=\"%1.html\">%2 module</a> ]")
			   .arg(classDoc()->module())
			   .arg(classDoc()->module()) );

    if ( classDoc() != 0 ) {
	out.putsMeta( "<p>" );
	out.putsMeta( classDoc()->brief() );
	out.puts( "\n" );
	if ( !classDoc()->extension().isEmpty() )
	    out.printfMeta( "<p>This class is part of the"
			    " <b>Qt %s Extension.</b>\n",
			    classDoc()->extension().latin1() );
    }

    out.putsMeta( "<a href=\"#details\">More...</a>\n" );

    if ( !hfile.isEmpty() )
	Doc::printHtmlIncludeHeader( out, hfile );

    int n = supert.count();
    QValueStack<QString> seps = separators( n, QString(".\n") );
    QValueList<CodeChunk>::ConstIterator st;

    if ( n > 0 ) {
	out.putsMeta( "<p>Inherits " );
	st = supert.begin();
	while ( st != supert.end() ) {
	    printHtmlDataType( out, *st, this );
	    out.puts( seps.pop() );
	    ++st;
	}
    }

    n = subt.count();
    seps = separators( n, QString(".\n") );

    if ( n > 0 ) {
	out.putsMeta( "<p>Inherited by " );
	qHeapSort( ((ClassDecl *) this)->subt );
	st = subt.begin();
	while ( st != subt.end() ) {
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
		&staticPublicMembers, &memberTypes, &memberFunctions );
	++child;
    }

    child = importantChildren().begin();
    while ( child != importantChildren().end() ) {
	tangle( *child, &importantInheritedMembers, &importantInheritedMembers,
		&importantInheritedMembers, &importantInheritedMembers,
		&memberTypes, &memberFunctions );
	++child;
    }

    child = protectedChildren().begin();
    while ( child != protectedChildren().end() ) {
	tangle( *child, &protectedMembers, &protectedSlots, 0,
		&staticProtectedMembers, &memberTypes, &memberFunctions );
	++child;
    }

    if ( config->isInternal() ) {
	child = privateChildren().begin();
	while ( child != privateChildren().end() ) {
	    tangle( *child, &privateMembers, &privateSlots, 0,
		    &staticPrivateMembers, &memberTypes, &memberFunctions );
	    ++child;
	}
    }

    child = relatedChildren().begin();
    while ( child != relatedChildren().end() ) {
	tangle( *child, &related, 0, 0, 0, 0, &relatedFunctions );
	++child;
    }

    printHtmlShortMembers( out, publicMembers, "Public Members" );
    printHtmlShortMembers( out, publicSlots, "Public Slots" );
    printHtmlShortMembers( out, publicSignals, "Signals" );
    printHtmlShortMembers( out, staticPublicMembers, "Static Public Members" );
    printHtmlShortMembers( out, importantChildren(),
			   "Important Inherited Members" );
    printHtmlShortMembers( out, protectedMembers, "Protected Members" );
    printHtmlShortMembers( out, protectedSlots, "Protected Slots" );
    printHtmlShortMembers( out, staticProtectedMembers,
			  "Static Protected Members" );
    printHtmlShortMembers( out, privateMembers, "Private Members" );
    printHtmlShortMembers( out, privateSlots, "Private Slots" );
    printHtmlShortMembers( out, staticPrivateMembers,
			   "Static Private Members" );
    printHtmlShortMembers( out, related, "Related Functions" );

    if ( !properties().isEmpty() ) {
	out.putsMeta( "<h2>Properties</h2>\n" );
	out.putsMeta( "<table border=1 cellpadding=3 cellspacing=0>\n" );
	out.putsMeta( "<tr><th>Type<th>Name<th>READ<th>WRITE<th>Options\n" );

	QValueList<Property>::ConstIterator p = properties().begin();
	while ( p != properties().end() ) {
	    out.printfMeta( "<tr><td>%s<td>%s<td>%s<td>%s\n",
			    (*p).type().latin1(), (*p).name().latin1(),
			    ( (*p).readFunction().isEmpty()
			      ? "&nbsp; " : (*p).readFunction().latin1() ),
			    ( (*p).writeFunction().isEmpty()
			      ? "&nbsp;" : (*p).writeFunction().latin1() ) );

	    QString opts;
	    if ( (*p).stored() != (*p).storedDefault() ) {
		opts += " STORED ";
		opts += (*p).stored() ? "true" : "false";
	    }
	    if ( (*p).designable() != (*p).designableDefault() ) {
		opts += " DESIGNABLE ";
		opts += (*p).designable() ? "true" : "false";
	    }
	    if ( !(*p).resetFunction().isEmpty() ) {
		opts += " RESET ";
		opts += (*p).resetFunction();
	    }
	    opts = opts.stripWhiteSpace();
	    if ( opts.isEmpty() )
		opts = "&nbsp;";
	    out.printfMeta( "<td>\n%s", opts.latin1() );
	    ++p;
	}
	out.putsMeta( "</table>\n" );
    }

    out.putsMeta( "<hr><a name=details></a><h2>Detailed Description</h2>\n" );

    if ( classDoc() != 0 )
	classDoc()->printHtml( out );

    printHtmlLongMembers( out, memberTypes, "Member Type Documentation" );
    printHtmlLongMembers( out, memberFunctions,
			  "Member Function Documentation" );
    printHtmlLongMembers( out, relatedFunctions, "Related Functions" );

    if ( !config->footer().isEmpty() ) {
	out.enterFooter();
	out.putsMeta( config->footer().latin1() );
    }
}

void ClassDecl::fillInDeclsThis()
{
    QValueStack<ClassDecl *> stack;
    QMap<QString, Decl *>::ConstIterator ent;

    stack.push( this );
    while ( !stack.isEmpty() ) {
	ClassDecl *c = stack.pop();

	QValueList<CodeChunk>::ConstIterator st = st = c->superTypes().begin();
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

    int level = 3;
    if ( funcDecl->internal() )
	level++;
    if ( funcDecl->reimplements() != 0 )
	level++;

    if ( level > 4 ) // no warnings for '\reimp'
	return;

    StringSet diff;
    StringSet::ConstIterator s;

    diff = difference( declared, fn->parameterNames() );
    s = diff.begin();
    while ( s != diff.end() ) {
	warning( level, fn->location(), "Undocumented parameter '%s'",
		 (*s).latin1() );
	++s;
    }
}

void ClassDecl::fillInDocsThis()
{
    QValueList<Decl *> importantChildren;

    QMap<QString, QValueList<FunctionDecl *> > fmap;
    QMap<QString, QValueList<FunctionDecl *> >::Iterator f;
    QValueList<FunctionDecl *>::ConstIterator g;
    QValueList<Decl *>::ConstIterator child;

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
      Constructors are a special case.  If a class possesses many constructors,
      none is a reimplementation of the others.  Internally, we'll elect the
      first constructor as the canonical one.
    */
    f = fmap.find( name() );
    if ( f != fmap.end() ) {
	/*
	  First pass:  Get rid of extra '\overload's.
	*/
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    if ( (*g)->fnDoc() != 0 && (*g)->fnDoc()->overloads() ) {
		warning( 3, (*g)->fnDoc()->location(),
			 "Suspicious '\\overload' in doc for constructor" );
		(*g)->fnDoc()->setOverloads( FALSE );
	    }

	    // a great place to do something unrelated
	    checkParams( *g, (*g)->parameterNames() );
	    ++g;
	}

	/*
	  Second pass:  Put '\overload's to all functions, except the first one.
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
	  First pass: Fill in the candidate lists.  Among the
	  candidates, we'll choose a canonical version and make sure
	  all the others are '\overload's

	  Ideally, the situation is this:  All versions of the
	  function except one are marked '\overload'.  Unfortunately,
	  on Qt, this policy led to hundreds of warnings about missing
	  '\overload's.  To avoid that, we distinguish candidates
	  according to their quality (or rather badness).  A normal
	  function has badness 0, an obsolete one has badness 1, and
	  an internal one has badness 2.  So if there are five
	  versions of 'Foo::foo()' (without any '\overload') of
	  badness 0, 1, 1, 2, 2, qdoc elects the version with badness
	  0 as the canonical one, without complaining about missing
	  '\overload's.

	  There are three candidate lists, to distinguish between the
	  three quality levels.  The best candidate should fall in
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
		} else if ( (*g)->internal() ) {
		    candidates[2].append( *g );
		} else if ( (*g)->obsolete() ) {
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
		warning( 3, scapeGoat->location(),
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
	      Complain about candidates that are as good as the one elected and
	      make them implicitly '\overload'.
	    */
	    ++g;
	    while ( g != candidates[badness].end() ) {
		warning( 4, (*g)->fnDoc()->location(), "Missing '\\overload'" );
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
	  Second pass:  Assign overload numbers.  If the canonical function is
	  internal or obsolete, the other documented functions inherit this
	  property.
	*/
	g = (*f).begin();
	while ( g != (*f).end() ) {
	    if ( (*g)->overloadNumber() == 1 &&
		 ((*g)->fnDoc() == 0 || (*g)->fnDoc()->overloads()) )
		(*g)->setOverloadNumber( overloadNo++ );

	    if ( canonical != 0 && (*g)->fnDoc() != 0 ) {
		if ( canonical->internal() )
		    (*g)->fnDoc()->setInternal( TRUE );
		if ( canonical->obsolete() )
		    (*g)->fnDoc()->setObsolete( TRUE );
	    }

	    /*
	      Here is another great place to do something unrelated to assigning
	      overload numbers.  Parameters inherited from the canonical
	      function need no documentation.
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

    HtmlWriter out( config->classMembersHref(name()) );
    out.setTitle( name() + QString(" Member List") );
    out.setHeading( QString("Complete Member List for ") + name() );

    out.printfMeta( "<p>This is the complete list of member functions for\n"
		    "<a href=\"%s\">%s</a>, including inherited members.\n\n",
		    config->classRefHref(fullName()).latin1(),
		    name().latin1() );
    out.putsMeta( "<ul>\n" );

    QString dtorName = QChar( '~' ) + name();

    if ( all.contains(name()) )
	printHtmlListEntry( out, name(), name() );
    if ( all.contains(dtorName) )
	printHtmlListEntry( out, dtorName, name() );

    f = all.begin();
    while ( f != all.end() ) {
	if ( (*f)->kind() == Decl::Function ) {
	    FunctionDecl *funcDecl = (FunctionDecl *) *f;
	    if ( !funcDecl->isConstructor() && !funcDecl->isDestructor() )
		printHtmlListEntry( out, f.key(), funcDecl->context()->name() );
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
      st( FALSE ), v( FALSE ), p( FALSE ), si( FALSE ), sl( FALSE ), ovo( 1 )
{
}

QString FunctionDecl::mangledName() const
{
    QString m = name();

    m += QChar( '(' );
    ParameterIterator param = parameterBegin();
    if ( param != parameterEnd() ) {
	m += (*param).dataType().toString();
	param++;
    }
    while ( param != parameterEnd() ) {
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

    return QChar( '0' + prio ) + uniqueName();
}

void FunctionDecl::setOverloadNumber( int no )
{
    ovo = no;
    if ( ovo != 1 && doc() != 0 )
	doc()->setLink( config->classRefHref(relatesContext()->name()) +
			QChar('#') + anchor(),
			fullName() );
}

void FunctionDecl::addParameter( const Parameter& param )
{
    pl.append( param );
    if ( !param.name().isEmpty() )
	ps.insert( param.name() );
}

void FunctionDecl::borrowParameterNames( ParameterIterator p )
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
    return context() != 0 && QChar( '~' ) + context()->name() == name();
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

    ParameterIterator param = parameterBegin();
    if ( param != parameterEnd() ) {
	out.putsMeta( " " );
	(*param).printHtmlShort( out );
	while ( ++param != parameterEnd() ) {
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
    out.printfMeta( "<a name=\"%s\"></a>%s (", anchor().latin1(),
		    fullName().latin1() );

    ParameterIterator param = parameterBegin();
    if ( param != parameterEnd() ) {
	out.putsMeta( " " );
	(*param).printHtmlLong( out, context() );
	while ( ++param != parameterEnd() ) {
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

EnumItem::EnumItem( const QString& ident, const CodeChunk& value )
    : id( ident ), v( value )
{
}

EnumItem::EnumItem( const EnumItem& item )
    : id( item.id ), v( item.v )
{
}

EnumItem& EnumItem::operator=( const EnumItem& item )
{
    id = item.id;
    v = item.v;
    return *this;
}

void EnumItem::printHtml( HtmlWriter& out ) const
{
    out.putsMeta( ident().latin1() );
    if ( !value().isEmpty() ) {
	out.puts( " = " );
	value().printHtml( out );
    }
}

EnumDecl::EnumDecl( const Location& loc, const QString& name, Decl *context )
    : Decl( Enum, loc, name, context )
{
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
	(*i).printHtml( out );
	while ( ++i != itemEnd() ) {
	    out.putsMeta( ", " );
	    (*i).printHtml( out );
	}
    }
    out.putsMeta( " }" );
}

void EnumDecl::printHtmlLong( HtmlWriter& out ) const
{
    out.printfMeta( "<a name=\"%s\"></a><b>%s</b>", anchor().latin1(),
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
    out.printfMeta( "<a name=\"%s\"></a><b>%s</b>", anchor().latin1(),
		    fullName().latin1() );
}
