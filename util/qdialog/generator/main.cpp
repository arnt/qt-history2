#include "main.h"

#include <qxmlparser.h>
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

QValueList<Object> findObjects( QXMLIterator t )
{
  QValueList<Object> lst;

  QXMLIterator it = t->begin();
  for( ; it != t->end(); ++it )
  {
    if ( it->hasAttrib( "name" ) )
    {
      Object o;
      o.name = it->attrib( "name" );
      o.className = it->tagName();
      lst.append( o );
    }
    lst += findObjects( it );
  }

  return lst;
}

void generateHeader( QFile& out, QXMLIterator it )
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

  str << endl << "#include <" << it->tagName().lower() << ".h>" << endl << endl;
  str << "class " << name << "_skel : public " << it->tagName() << endl << "{" << endl;
  str << "  Q_OBJECT" << endl << "public:" << endl << "  " << name << "_skel( QWidget* parent, const QResource& resource );" << endl;
  str << "  ~" << name << "_skel();" << endl << endl << "protected:" << endl;

  oit = objects.begin();
  for( ; oit != objects.end(); ++oit )
    str << "  " << oit->className << "*  " << oit->name << ";" << endl;

  str << endl << "protected slots:" << endl;
  QXMLIterator t = it->begin();
  for( ; t != it->end(); ++t )
    if ( t->tagName() == "CustomSlot" )
      str << "  virtual void " << t->attrib("signature" ) << " = 0;" << endl;

  str << endl << "signals:" << endl;
  t = it->begin();
  for( ; t != it->end(); ++t )
    if ( t->tagName() == "CustomSignal" )
      str << "  void " << t->attrib("signature" ) << ";" << endl;

  str << "};" << endl << endl;
}

void generateImpl( QFile& out, QXMLIterator it, const QString& basename )
{
  QString name = it->attrib( "name" );

  QTextStream str( &out );
  str << "#include \"" << basename << "_skel.h\"" << endl << endl;
  str << name << "_skel::" << name << "_skel( QWidget* parent, const QResource& resource )" << endl;
  str << "  : " << it->tagName() << "( parent, resource )" << endl << "{" << endl;
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
    fatal( "Syntax: qgenerator resourcefile\n" );

  QXMLParseTree tree;
  QFile file( argv[1] );
  if ( !file.open( IO_ReadOnly ) )
    fatal( "Could not open file %s\n", argv[1] );

  QTextStream str( &file );
  str >> tree;

  QString basename = argv[1];
  int i = basename.findRev( "." );
  if ( i != -1 )
    basename = basename.left( i );

  QFile outh( basename + "_skel.h" );
  if ( !outh.open( IO_WriteOnly ) )
    fatal( "Could not write file %s", (basename+"_skel.h").ascii() );

  QFile outc( basename + "_skel.cpp" );
  if ( !outc.open( IO_WriteOnly ) )
    fatal( "Could not write file %s", (basename+"_skel.cpp").ascii() );

  {
    QTextStream str( &outh );
    str << "#ifndef __" << basename.upper() << "__H_" << endl;
    str << "#define __" << basename.upper() << "__H_" << endl << endl;
    str << "class QResource;" << endl << endl;;
  }

  QXMLTag* r = tree.rootTag();
  QXMLIterator it = r->begin();
  for( ; it != r->end(); ++it )
  {
    QString name;
    if ( it->hasAttrib( "name" ) )
      name = it->attrib( "name" );
    if ( !name.isEmpty() )
    {
      printf("Writing class %s\n", name.ascii() );
      generateHeader( outh, it );

      generateImpl( outc, it, basename );
    }
  }

  {
    QTextStream str( &outh );
    str << "#endif" << endl;
  }

  outh.close();
  outc.close();

  file.close();
}
