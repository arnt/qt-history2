#ifndef __Q_VALUELIST_H__
#define __Q_VALUELIST_H__

#include "qglobal.h"
#include "qshared.h"
#include "qstring.h"

template <class T>
struct QValueListNode
{
  QValueListNode( const T& t ) : data( t ) { }
  QValueListNode() { }

  QValueListNode<T>* next;
  QValueListNode<T>* prev;
  T data;
};

template<class T, class Ref, class Ptr>
struct QValueListIterator
{
  /**
   * Typedefs
   */
  typedef QValueListIterator<T, T&, T*> Iterator;
  // typedef QValueListIterator<T, const T&, const T*> ConstIterator;
  typedef QValueListIterator<T, Ref, Ptr> Type;
  typedef QValueListNode<T>* NodePtr;

  /**
   * Variables
   */
  NodePtr node;

  /**
   * Functions
   */
  QValueListIterator() : node( 0 ) {}
  QValueListIterator( NodePtr p ) : node( p ) {}
  // QValueListIterator( ConstIterator& c ) : node( c.node ) {}
  QValueListIterator( const Iterator& i ) : node( i.node ) {}

  bool operator==( const Type& x ) const { return node == x.node; }
  bool operator!=( const Type& x ) const { return node != x.node; }
  Ref operator*() const { return node->data; }
  Ptr operator->() const { return &(node->data); }

  Type& operator++() {
    node = node->next;
    return *this;
  }

  Type operator++(int) {
    Type tmp = *this;
    node = node->next;
    return tmp;
  }

  Type& operator--() {
    node = node->prev;
    return *this;
  }

  Type operator--(int) {
    Type tmp = *this;
    node = node->prev;
    return tmp;
  }
};

template <class T>
class QValueListPrivate : public QShared
{
public:
  /**
   * Typedefs
   */
  typedef QValueListIterator< T, T&, T* > Iterator;
  typedef QValueListNode<T> Node;
  typedef QValueListNode<T>* NodePtr;

  /**
   * Functions
   */
  QValueListPrivate() { node = new Node; node->next = node->prev = node; nodes = 0; }
  QValueListPrivate( const QValueListPrivate& _p ) : QShared() {
    node = new Node; node->next = node->prev = node; nodes = 0;
    Iterator b( _p.node->next );
    Iterator e( _p.node );
    Iterator i( node );
    while( b != e )
      insert( i, *b++ );
  }

  Iterator insert( Iterator it, const T& x ) {
    NodePtr p = new Node( x );
    p->next = it.node;
    p->prev = it.node->prev;
    it.node->prev->next = p;
    it.node->prev = p;
    nodes++;
    return p;
  }

  Iterator remove( Iterator it ) {
    ASSERT ( it.node != node );
    NodePtr next = it.node->next;
    NodePtr prev = it.node->prev;
    prev->next = next;
    next->prev = prev;
    delete it.node;
    nodes--;
    return Iterator( next );
  }

  Iterator find( Iterator it, const T& x ) const {
    Iterator first = it;
    Iterator last = Iterator( node );
    while( first != last) {
      if ( *first == x )
	return first;
      ++first;
    }
    return last;
  }

  uint contains( const T& x ) const {
    uint result = 0;
    Iterator first = Iterator( node->next );
    Iterator last = Iterator( node );
    while( first != last) {
      if ( *first == x )
	++result;
      ++first;
    }
    return result;
  }

  void remove( const T& x ) {
    Iterator first = Iterator( node->next );
    Iterator last = Iterator( node );
    while( first != last) {
      if ( *first == x )
	first = remove( first );
      else
	++first;
    }
  }

  Iterator at( uint i ) const {
    ASSERT( i < nodes );
    NodePtr p = node->next;
    for( uint x = 0; x < i; ++x )
      p = p->next;
    return p;
  }

  void clear() {
    nodes = 0;
    NodePtr p = node->next;
    while( p != node ) {
      NodePtr next = p->next;
      delete p;
      p = next;
    }
    node->next = node->prev = node;
  }

  NodePtr node;
  uint nodes;
};

template <class T>
class QValueList
{
public:
  /**
   * Typedefs
   */
  typedef QValueListIterator< T, T&, T* > Iterator;
  // typedef QValueListIterator< T, const T&, const T* > ConstIterator;
  typedef T ValueType;

  /**
   * API
   */
  QValueList() { sh = new QValueListPrivate<T>; }
  QValueList( const QValueList& _l ) { sh = _l.sh; sh->ref(); }
  ~QValueList() { if ( sh->deref() ) delete sh; }

  QValueList<T>& operator= ( const QValueList<T>& _list )
  {
    if ( sh->deref() ) delete sh;
    sh = _list.sh;
    sh->ref();
  }

  Iterator begin() { detach(); return Iterator( sh->node->next ); }
  Iterator end() { detach(); return Iterator( sh->node ); }
  Iterator last() { detach(); return Iterator( sh->node->prev ); }

  bool isEmpty() const { return ( sh->nodes == 0 ); }

  Iterator insert( Iterator it, const T& x ) { detach(); return sh->insert( it, x ); }

  Iterator append( const T& x ) { detach(); return sh->insert( end(), x ); }
  Iterator prepend( const T& x ) { detach(); return sh->insert( begin(), x ); }

  Iterator remove( Iterator it ) { detach(); return sh->remove( it ); }
  void remove( const T& x ) { detach(); sh->remove( x ); }

  T& getFirst() { detach(); detach(); return sh->node->next->data; }
  T& getLast() { detach(); detach(); return sh->node->prev->data; }

  T& operator[] ( uint i ) { detach(); return *sh->at(i); }
  const T& operator[] ( uint i ) const { return *sh->at(i); }
  Iterator at( uint i ) { detach(); return sh->at(i); }
  Iterator find ( const T& x ) { detach(); return sh->find( begin(), x); }
  Iterator find ( Iterator it, const T& x ) { detach(); return sh->find( it, x ); }
  uint contains( const T& x ) const { return sh->contains( x ); }

  uint count() const { return sh->nodes; }

  void clear() { if ( sh->count == 1 ) sh->clear(); else { sh->deref(); sh = new QValueListPrivate<T>; } }

protected:
  /**
   * Helpers
   */
  void detach() { if ( sh->count > 1 ) { sh->deref(); sh = new QValueListPrivate<T>( *sh ); } }

  /**
   * Variables
   */
  QValueListPrivate<T>* sh;
};

typedef QValueList<QString> QStringList;

#endif
