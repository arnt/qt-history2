#include "main.h"

#include <qresource.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <stdio.h>

struct Object
{
  QString name;
  QString className;
};

QValueList<Object> findObjects( const QResourceItem* t )
{
  QValueList<Object> lst;

  const QResourceItem* it = t->firstChild();
  for( ; it; it = it->nextSibling() )
  {
    if ( it->hasAttrib( "name" ) )
    {
      Object o;
      o.name = it->attrib( "name" );
      o.className = it->type();
      lst.append( o );
    }
    lst += findObjects( it );
  }

  return lst;
}

void generateHeader( QFile& out, const QResourceItem* it )
{
  QString name = it->attrib( "name" );

  QTextStream str( &out );

  QStringList done;
  QValueList<Object> objects = findObjects( it );
  QValueList<Object>::Iterator oit = objects.begin();
  for( ; oit != objects.end(); ++oit )
    if ( !done.contains( oit->className ) )
    {
      str << "class " << oit->className << ";" << endl;
      done.append( oit->className );
    }

  str << endl << "#include <" << it->type().lower() << ".h>" << endl << endl;
  str << "class " << name << "_skel : public " << it->type() << endl << "{" << endl;
  str << "  Q_OBJECT" << endl << "public:" << endl << "  " << name << "_skel( QWidget* parent, const QResource& resource );" << endl;
  str << "  ~" << name << "_skel();" << endl << endl << "protected:" << endl;

  oit = objects.begin();
  for( ; oit != objects.end(); ++oit )
    str << "  " << oit->className << "*  " << oit->name << ";" << endl;

  str << endl << "protected slots:" << endl;
  const QResourceItem* t = it->firstChild();
  for( ; t; t = t->nextSibling() )
    if ( t->type() == "CustomSlot" )
      str << "  virtual void " << t->attrib("signature" ) << " = 0;" << endl;

  str << endl << "signals:" << endl;
  t = it->firstChild();
  for( ; t; t = t->nextSibling() )
    if ( t->type() == "CustomSignal" )
      str << "  void " << t->attrib("signature" ) << ";" << endl;

  str << "};" << endl << endl;
}

void generateImpl( QFile& out, QResourceItem* it, const QString& basename )
{
  QString name = it->attrib( "name" );

  QTextStream str( &out );
  str << "#include \"" << basename << "_skel.h\"" << endl << endl;
  str << name << "_skel::" << name << "_skel( QWidget* parent, const QResource& resource )" << endl;
  str << "  : " << it->type() << "( parent, resource )" << endl << "{" << endl;
  str << "  configure( resource );" << endl << endl;
  QValueList<Object> objects = findObjects( it );
  QValueList<Object>::Iterator oit = objects.begin();
  for( ; oit != objects.end(); ++oit )
  {
    str << "  " << oit->name << " = (" << oit->className << "*)child( \"" << oit->name << "\", \"" << oit->className << "\" );" << endl;
    str << "  ASSERT( " << oit->name << " );" << endl; 
  }
  str << "}" << endl << endl;

  str << name << "_skel::~" << name << "_skel()" << endl << "{" << endl << "}" << endl << endl;
}

int main( int argc, char **argv )
{
  if ( argc != 2 )
    qFatal( "Syntax: qgenerator resourcefile\n" );

  QResource resource( argv[1] );
  if ( resource.isEmpty() )
    qFatal( "Could not open file %s\n", argv[1] );

  QString basename = argv[1];
  int i = basename.findRev( "." );
  if ( i != -1 )
    basename = basename.left( i );

  QFile outh( basename + "_skel.h" );
  if ( !outh.open( IO_WriteOnly ) )
    qFatal( "Could not write file %s", (basename+"_skel.h").ascii() );

  QFile outc( basename + "_skel.cpp" );
  if ( !outc.open( IO_WriteOnly ) )
    qFatal( "Could not write file %s", (basename+"_skel.cpp").ascii() );

  {
    QTextStream str( &outh );
    str << "#ifndef __" << basename.upper() << "__H_" << endl;
    str << "#define __" << basename.upper() << "__H_" << endl << endl;
    str << "class QResource;" << endl << endl;;
  }

  QResourceItem* r = resource.tree();
  QString name = r->name();
  if ( !name.isEmpty() )
  {
    printf("Writing class %s\n", name.ascii() );
    generateHeader( outh, r );
    
    generateImpl( outc, r, basename );
  }

  {
    QTextStream str( &outh );
    str << "#endif" << endl;
  }

  outh.close();
  outc.close();
}
