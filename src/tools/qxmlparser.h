#ifndef __QXMLPARSER_H__
#define __QXMLPARSER_H__

#include <qstring.h>
#include <qmap.h>
#include <qlist.h>
#include <qshared.h>
#include <qcolor.h>

class QXMLParser;
class QXMLParseTree;

class QXMLParser
{
public:
  struct Tag
  {
    Tag() { postclosed = preclosed = false; isText = false; firstChild = nextSibling = parent = 0; }
    ~Tag() { if ( firstChild ) delete firstChild; if ( nextSibling ) delete nextSibling; }

    Tag* parent;
    QString name;
    QMap<QString,QString> attribs;
    bool preclosed;
    bool postclosed;
    bool isText;
    Tag* firstChild;
    Tag* nextSibling;
  };

  QXMLParser();
  virtual ~QXMLParser();

  int parse( const QString& _text, QXMLParseTree* _tree = 0 );

protected:
  virtual bool addTag( Tag* _tag );
  virtual void addText( const QString& _text );

private:
  QXMLParseTree* tree;
};

class QXMLParseTree : public QShared
{
  friend QXMLParser;
public:
  QXMLParseTree();
  ~QXMLParseTree();

  void clear();

  QXMLParser::Tag* root() const { return root_tag; }

protected:
  bool addTag( QXMLParser::Tag* _tag );
  void addText( const QString& _text );

  QXMLParser::Tag* root_tag;
  QXMLParser::Tag* current_tag;
  QXMLParser::Tag* current_sibling;
};

class QXMLIterator
{
public:
  QXMLIterator( const QXMLParseTree& _tree );
  QXMLIterator( const QXMLIterator& _it ) { tag = _it.tag; }
  QXMLIterator( QXMLParser::Tag* _tag ) { tag = _tag; }

  ~QXMLIterator();

  QXMLIterator operator++ (int);
  QXMLIterator& operator++ ();

  QXMLParser::Tag* operator*() const;
  QXMLParser::Tag* operator->() const;

  uint childCount() const;

  QXMLIterator firstChild() const;
  QXMLIterator nextSibling() const;
  QXMLIterator parent() const;

  bool isText() const { return tag->isText; }
  bool isClosingTag() const { return tag->preclosed; }
  bool isEmptyTag() const { return tag->postclosed; }
  QString text() const { return tag->name; }

  QString textAttrib ( const QString& _name ) const;
  int intAttrib( const QString& _name ) const;
  bool boolAttrib( const QString& _name ) const;
  QColor colorAttrib( const QString& _name ) const;
  bool hasAttrib( const QString& _name ) const;
  double doubleAttrib( const QString& _name ) const;

  bool isValid() const { return ( tag != 0 ); }

protected:
  QXMLParser::Tag* tag;
};

#endif
