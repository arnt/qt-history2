#include "qxmlparser.h"

#include <ctype.h>

#include <iostream.h>

/*********************************************
 *
 * QXMLParser
 *
 *********************************************/

QXMLParser::QXMLParser()
{
  tree = 0;
}

QXMLParser::~QXMLParser()
{
}

bool QXMLParser::addTag( QXMLParser::Tag* _tag )
{
  if ( tree )
    return tree->addTag( _tag );

  delete _tag;
  return true;
}

void QXMLParser::addText( const QString& _text )
{
  if ( tree )
    tree->addText( _text );  
}

/*
  Returns -1 if everything went fine or the position in the text
  where the error was detected.
 */

int QXMLParser::parse( const QString &text, QXMLParseTree* _tree )
{
  tree = _tree;
  if ( tree )
    tree->clear();

  int len = text.length();
  int pos = 0;
  int start = 0;
  Tag* tag = 0;
  QString attrib;

 Node1: // accepts
  tag = 0;
  if ( pos == len )
    goto Ok;
  else if ( text[pos] == '<' )
  {
    pos++;
    goto Node2;
  }
  else
  {
    start = pos++;
    goto Node20;
  }
 Node2: // Tag
  if ( pos + 3 <= len && text[pos] == '!' && text[pos+1] == '-' && text[pos+2] == '-' )
  {
    pos += 3;
    goto Node30;
  }
  tag = new Tag;
  if ( pos == len )
    goto Failed;
  if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node2;
  }
  if ( text[pos] == '/' )
  {
    tag->preclosed = true;
    pos++;
    goto Node3;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node4;
  }
  else
    goto Failed;
 Node3:
  if ( pos == len )
    goto Failed;
  if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node3;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node4;
  }
  else
    goto Failed;
 Node4:
  if ( pos == len )
    goto Failed;
  else if ( isalpha( text[pos] ) )
  {
    ++pos;
    goto Node4;
  }
  else if ( text[pos] == '>' )
  {
    tag->name = text.mid( start, pos - start );
    cout << "Tag1=" << tag->name << endl;
    pos++;
    goto Node15;
  }
  else if ( text[pos] == '/' )
  {
    if ( tag->preclosed )
      goto Failed;
    tag->name = text.mid( start, pos - start );
    cout << "Tag2=" << tag->name << endl;
    tag->postclosed = true;
    pos++;
    goto Node14;
  }
  else if ( text[pos].isSpace() )
  {
    tag->name = text.mid( start, pos - start );
    cout << "Tag3=" << tag->name << endl;
    pos++;
    goto Node5;
  }
  else
    goto Failed;
 Node5:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node5;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node6;
  }
  else if ( text[pos] == '>' )
  {
    pos++;
    goto Node15;
  }
  else if ( text[pos] == '/' )
  {
    if ( tag->preclosed )
      goto Failed;
    tag->postclosed = true;
    pos++;
    goto Node14;
  }
  else
    goto Failed;
 Node6:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    attrib = text.mid( start, pos - start );
    cout << "Attrib=" << attrib << endl;
    ++pos;
    goto Node7;
  }
  else if ( isalpha( text[pos] ) )
  {
    ++pos;
    goto Node6;
  }
  else if ( text[pos] == '=' )
  {
    attrib = text.mid( start, pos - start );
    cout << "Attrib=" << attrib << endl;
    ++pos;
    goto Node8;
  }
  else
    goto Failed;
 Node7:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node7;
  }
  else if ( text[pos] == '=' )
  {
    ++pos;
    goto Node8;
  }
  else
    goto Failed;
 Node8:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node10;
  }
  else if ( text[pos] == '"' )
  {
    ++pos;
    goto Node9;
  }
  else
    goto Failed;
 Node9:
  if ( pos == len )
    goto Failed;
  start = pos;
  if ( text[pos] == '"' )
  {
    QString value = text.mid( start, pos - start );
    tag->attribs.insert( attrib, value );
    cout << "Value=" << value << endl;
    pos++;
    goto Node12;
  }
  else
  {
    ++pos;
    goto Node11;
  }
 Node10:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node10;
  }
  else if ( text[pos] == '"' )
  {
    ++pos;
    goto Node9;
  }
  else
    goto Failed;
 Node11:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '"' )
  {
    QString value = text.mid( start, pos - start );
    tag->attribs.insert( attrib, value );
    cout << "Value=" << value << endl;
    pos++;
    goto Node12;
  }
  else
  {
    ++pos;
    goto Node11;
  }
 Node12:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node13;
  }
  else if ( text[pos] == '/' )
  {
    if ( tag->preclosed )
      goto Failed;
    tag->postclosed = true;
    ++pos;
    goto Node14;
  }
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else
    goto Failed;
 Node13:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node13;
  }
  else if ( text[pos] == '/' )
  {
    if ( tag->preclosed )
      goto Failed;
    tag->postclosed = true;
    ++pos;
    goto Node14;
  }
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node6;
  }
  else
    goto Failed;
 Node14:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else
    goto Failed;
 Node15: // accepts
  if ( !addTag( tag ) )
    goto Failed;
  tag = 0;
  if ( pos == len )
    goto Ok;
  goto Node1;
 Node20: // Text
  tag = 0;
  if ( pos == len )
  {
    if ( pos != start )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	addText( tmp );
    }
    goto Ok;
  }
  else if ( text[pos] == '<' )
  {
    if ( pos != start )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	addText( tmp );
    }
    ++pos;
    goto Node2;
  }
  ++pos;
  goto Node20;
 Node30: // Comments
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node31;
  }
  else 
  {
    ++pos;
    goto Node30;
  }
 Node31:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node32;
  }
  else 
  {
    ++pos;
    goto Node30;
  }
 Node32:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node1;
  }
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node32;
  }
  ++pos;
  goto Node30;

 Ok:
  return -1;
 Failed:
  if ( tag )
    delete tag;
  return pos;
}

/*********************************************
 *
 * QXMLIterator
 *
 *********************************************/

QXMLIterator::QXMLIterator( const QXMLParseTree& _tree ) : 
  tag( _tree.root() )
{
}

QXMLIterator::~QXMLIterator()
{
}

QXMLParser::Tag* QXMLIterator::operator* () const
{
  return tag;
}

QXMLParser::Tag* QXMLIterator::operator-> () const
{
  return tag;
}

QXMLIterator QXMLIterator::operator++ (int)
{
  if ( !tag )
    return *this;

  QXMLIterator tmp = *this;
  tag = tag->nextSibling;
  return tmp;
}

QXMLIterator& QXMLIterator::operator++ ()
{
  if ( tag )
    tag = tag->nextSibling;
  return *this;
}

QXMLIterator QXMLIterator::firstChild() const
{
  return QXMLIterator( tag->firstChild );
}

QXMLIterator QXMLIterator::nextSibling() const
{
  return QXMLIterator( tag->nextSibling );
}

QXMLIterator QXMLIterator::parent() const
{
  return QXMLIterator( tag->parent );
}

uint QXMLIterator::childCount() const
{
  if ( !isValid() )
    return 0;

  uint c = 0;
  QXMLParser::Tag* t = tag->firstChild;
  while( t )
  {
    ++c;
    t = t->nextSibling;
  }

  return c;
}

QString QXMLIterator::textAttrib( const QString& _name ) const
{
  if ( !isValid() )
    return QString::null;

  return tag->attribs[ _name ];
}

int QXMLIterator::intAttrib( const QString& _name ) const
{
  if ( !isValid() )
    return 0;

  return tag->attribs[ _name ].toInt();
}

double QXMLIterator::doubleAttrib( const QString& _name ) const
{
  if ( !isValid() )
    return 0.0;

  return tag->attribs[ _name ].toDouble();
}

bool QXMLIterator::boolAttrib( const QString& _name ) const
{
  if ( isValid() && strcasecmp( tag->attribs[ _name ].ascii(), "true" ) == 0 )
    return true;
  return false;
}

QColor QXMLIterator::colorAttrib( const QString& _name ) const
{
  if ( !isValid() )
    return QColor();

  return QColor( tag->attribs[ _name ].ascii() );
}

bool QXMLIterator::hasAttrib( const QString& _name ) const
{
  return ( isValid() && tag->attribs.contains( _name ) );
}

/*********************************************
 *
 * QXMLParseTree
 *
 *********************************************/

QXMLParseTree::QXMLParseTree()
{
  root_tag = new QXMLParser::Tag;
  current_tag = root_tag;
  current_sibling = 0;
}

QXMLParseTree::~QXMLParseTree()
{
  if ( root_tag )
    delete root_tag;
}
 
void QXMLParseTree::clear()
{
  if ( root_tag->firstChild == 0 )
    return;

  if ( root_tag )
    delete root_tag;
  root_tag = new QXMLParser::Tag;
  current_tag = root_tag;
  current_sibling = 0;
}

bool QXMLParseTree::addTag( QXMLParser::Tag* _tag )
{
  ASSERT( current_tag != 0 );
  if ( _tag->postclosed )
  {
    if ( current_sibling )
      current_sibling->nextSibling = _tag;
    else
      current_tag->firstChild = _tag;

    _tag->nextSibling = 0;
    current_sibling = _tag;
    _tag->parent = current_tag;
    
    return true;
  }
  if ( _tag->preclosed )
  {
    if ( _tag->name != current_tag->name )
      return false;
    current_sibling = current_tag;
    current_tag = current_tag->parent;
    delete _tag;
    return true;
  }

  if ( current_sibling )
    current_sibling->nextSibling = _tag;
  else
    current_tag->firstChild = _tag;

  _tag->nextSibling = 0;
  _tag->parent = current_tag;
  current_tag = _tag;
  current_sibling = 0;
  return true;
}

void QXMLParseTree::addText( const QString& _str )
{
  QXMLParser::Tag* tag = new QXMLParser::Tag;
  tag->name = _str;
  tag->isText = true;

  if ( current_sibling )
    current_sibling->nextSibling = tag;
  else
    current_tag->firstChild = tag;
  
  tag->nextSibling = 0;
  current_sibling = tag;
  tag->parent = current_tag;  
}
