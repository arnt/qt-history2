#ifndef QDOM_H
#define QDOM_H

#include "qstring.h"

struct QDOM
{
  typedef QString String;

  class Namespace
  {
    static const unsigned short INDEX_SIZE_ERR     = 1;
    static const unsigned short DOMSTRING_SIZE_ERR = 2;
    static const unsigned short HIERARCHY_REQUEST_ERR = 3;
    static const unsigned short WRONG_DOCUMENT_ERR = 4;
    static const unsigned short INVALID_CHARACTER_ERR = 5;
    static const unsigned short NO_DATA_ALLOWED_ERR = 6;
    static const unsigned short NO_MODIFICATION_ALLOWED_ERR = 7;
    static const unsigned short NOT_FOUND_ERR      = 8;
    static const unsigned short NOT_SUPPORTED_ERR  = 9;
    static const unsigned short INUSE_ATTRIBUTE_ERR = 10;
  };
  
  class Exception : public Namespace
  {
  public:
    unsigned short code;
  };

  class Implementation : public Namespace
  {
  public:
    bool hasFeature( const QString& feature, const QString& version ) = 0;
  };

  class DocumentFragment : public Node
  {
  public:
  };

  class Document : public Node
  {
  public:
    // Attributes
    const DocumentType* doctype() const = 0;
    const Implementation* implementation() const = 0;
    const Element* documentElement() const = 0;

    // Factories
    Element*               createElement( const QString& tagName ) = 0;
    DocumentFragment*      createDocumentFragment() = 0;
    Text*                  createTextNode( const QString& data ) = 0;
    Comment*               createComment( const QString& data ) = 0;
    CDATASection*          createCDATASection( const QString& data ) = 0;
    ProcessingInstruction* createProcessingInstruction( const QString& target, const QString& data ) = 0;
    Attr*                  createAttribute(i const QString& name ) = 0;
    EntityReference*       createEntityReference( const QString& name ) = 0;
    NodeList*              getElementsByTagName( const QString& tagname ) = 0;
  };

  class Node : public Namespace
  {
    static const unsigned short      ELEMENT_NODE       = 1;
    static const unsigned short      ATTRIBUTE_NODE     = 2;
    static const unsigned short      TEXT_NODE          = 3;
    static const unsigned short      CDATA_SECTION_NODE = 4;
    static const unsigned short      ENTITY_REFERENCE_NODE = 5;
    static const unsigned short      ENTITY_NODE        = 6;
    static const unsigned short      PROCESSING_INSTRUCTION_NODE = 7;
    static const unsigned short      COMMENT_NODE       = 8;
    static const unsigned short      DOCUMENT_NODE      = 9;
    static const unsigned short      DOCUMENT_TYPE_NODE = 10;
    static const unsigned short      DOCUMENT_FRAGMENT_NODE = 11;
    static const unsigned short      NOTATION_NODE      = 12;

    QString  nodeName() const = 0;
    QString& nodeValue() = 0;

    unsigned short      nodeType() const = 0;
    const Node*         parentNode() const = 0;
    const NodeList*     childNodes() const = 0;
    const Node*         firstChild() const = 0;
    const Node*         lastChild() const = 0;
    const Node*         previousSibling() const = 0;
    const Node*         nextSibling() const = 0;
    const NamedNodeMap* attributes() const = 0;
    const Document*     ownerDocument() const = 0;

    Node* insertBefore( Node* newChild, Node* refChild ) = 0;
    Node* replaceChild( Node* newChild, Node* oldChild ) = 0;
    Node* removeChild( Node* oldChild ) = 0;
    Node* appendChild( Node* newChild ) = 0;
    bool  hasChildNodes() const = 0;
    Node* cloneNode(in boolean deep) const = 0;
  };

  class NodeList : public Namespace
  {
  public:
    Node*        item( unsigned long index ) = 0;
    unsigne long length() const = 0;
  };

  class NamedNodeMap : public Namespace
  {
  public:
    Node*         getNamedItem( const QString& name ) = 0;
    Node*         setNamedItem( Node* arg ) = 0;
    Node*         removeNamedItem( const QString& name ) = 0;
    Node*         item( unsigned long index ) = 0;
    unsigned long length() const = 0;
  };

  class CharacterData : public Node
  {
  public:
    QString&      data() = 0;
    unsigned long length() const = 0;

    QString substringData( unsigned long offset, unsigned long count ) = 0;
    void    appendData( const QString& arg ) = 0;
    void    insertData( unsigned long offset, const QString& arg ) = 0;
    void    deleteData( unsigned long offset, unsigned long count ) = 0;
    void    replaceData(in unsigned long offset, unsigned long count, const QString& arg ) = 0;
  };

  class Attr : public Node
  {
  public:
    QString  name() const = 0;
    bool     specified() const = 0;
    QString& value() = 0;
  };

  class Element : public Node
  {
  public:
    QString   tagName() const = 0;
    QString&  getAttribute( const QString& name ) = 0;
    void      setAttribute( const QString& name, const QString& value ) = 0;
    void      removeAttribute( const QString& name ) = 0;
    Attr*     getAttributeNode( const QString& name) = 0;
    Attr*     setAttributeNode( Attr* newAttr ) = 0;
    Attr*     removeAttributeNode( Attr* oldAttr ) = 0;
    NodeList* getElementsByTagName( const QString name ) = 0;
    void      normalize() = 0;
  };

  class Text : public CharacterData
  {
  public:
    Text* splitText(in unsigned long offset) = 0;
  };

  class Comment : public CharacterData
  {
  };

  class CDATASection : public Text
  {
  };

  class DocumentType : public Node
  {
  public:
    QString             name() const = 0;
    const NamedNodeMap* entities() const = 0;
    const NamedNodeMap* notations() const = 0;
  };

  class Notation : public Node
  {
  public:
    QString publicId() const = 0;
    QString systemId() const = 0;
  };

  class Entity : public Node
  {
  public:
    QString publicId() const = 0;
    QString systemId() const = 0;
    QString notationName() const = 0;
  };

  class EntityReference : public Node
  {
  public:
    QString  target() const = 0;
    QString& data() = 0;
  };
};

class QDOMDocument : public QDOM::Document
{
public:
    QDOMDocument();
    QDOMDocument( const QString& filename );

    // Attributes
    const DocumentType* doctype() const;
    const Implementation* implementation() const;
    const Element* documentElement() const;

    // Factories
    Element*               createElement( const QString& tagName );
    DocumentFragment*      createDocumentFragment();
    Text*                  createTextNode( const QString& data );
    Comment*               createComment( const QString& data );
    CDATASection*          createCDATASection( const QString& data );
    ProcessingInstruction* createProcessingInstruction( const QString& target, const QString& data );
    Attr*                  createAttribute(i const QString& name );
    EntityReference*       createEntityReference( const QString& name );
    NodeList*              getElementsByTagName( const QString& tagname );
};

#endif
