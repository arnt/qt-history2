/****************************************************************************
** $Id: //depot/qt/main/src/tools/qvaluelist.h#9 $
**
** Definition of QValueList class
**
** Created : 990406
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QVALUELIST_H
#define QVALUELIST_H

#ifndef QT_H
#include "qglobal.h"
#include "qshared.h"
#endif // QT_H

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
  typedef QValueListIterator< T, const T&, const T* > ConstIterator;
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

  NodePtr find( NodePtr start, const T& x ) const {
    ConstIterator first( start );
    ConstIterator last( node );
    while( first != last) {
      if ( *first == x )
	return first.node;
      ++first;
    }
    return last.node;
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

  NodePtr at( uint i ) const {
    ASSERT( i <= nodes );
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
  typedef QValueListIterator< T, const T&, const T* > ConstIterator;
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
    return *this;
  }

  QValueList<T> operator+ ( const QValueList<T>& _l ) const
  {
    QValueList<T> l( *this );
    for( ConstIterator it = _l.begin(); it != _l.end(); ++it )
      l.append( *it );
    return l;
  }

  QValueList<T>& operator+= ( const QValueList<T>& _l )
  {
    for( ConstIterator it = _l.begin(); it != _l.end(); ++it )
      append( *it );
    return *this;
  }

  bool operator== ( const QValueList<T>& _l ) const
  {
    if ( count() != _l.count() )
      return FALSE;
    ConstIterator it2 = begin();
    ConstIterator it = _l.begin();
    for( ; it != _l.end(); ++it, ++it2 )
      if ( !( *it == *it2 ) )
	return FALSE;
    return TRUE;
  }
  
  bool operator!= ( const QValueList<T>& _l ) const { return !( *this == _l ); }

  Iterator begin() { detach(); return Iterator( sh->node->next ); }
  ConstIterator begin() const { return ConstIterator( sh->node->next ); }
  Iterator end() { detach(); return Iterator( sh->node ); }
  ConstIterator end() const { return ConstIterator( sh->node ); }
  Iterator last() { detach(); return Iterator( sh->node->prev ); }
  ConstIterator last() const { return ConstIterator( sh->node->prev ); }

  bool isEmpty() const { return ( sh->nodes == 0 ); }

  Iterator insert( Iterator it, const T& x ) { detach(); return sh->insert( it, x ); }

  Iterator append( const T& x ) { detach(); return sh->insert( end(), x ); }
  Iterator prepend( const T& x ) { detach(); return sh->insert( begin(), x ); }

  Iterator remove( Iterator it ) { detach(); return sh->remove( it ); }
  void remove( const T& x ) { detach(); sh->remove( x ); }

  T& getFirst() { detach(); return sh->node->next->data; }
  const T& getFirst() const { return sh->node->next->data; }
  T& getLast() { detach(); return sh->node->prev->data; }
  const T& getLast() const { return sh->node->prev->data; }

  T& operator[] ( uint i ) { detach(); return sh->at(i)->data; }
  const T& operator[] ( uint i ) const { return sh->at(i)->data; }
  Iterator at( uint i ) { detach(); return Iterator( sh->at(i) ); }
  ConstIterator at( uint i ) const { return ConstIterator( sh->at(i) ); }
  Iterator find ( const T& x ) { detach(); return Iterator( sh->find( sh->node->next, x) ); }
  ConstIterator find ( const T& x ) const { return ConstIterator( sh->find( sh->node->next, x) ); }
  Iterator find ( Iterator it, const T& x ) { detach(); return Iterator( sh->find( it.node, x ) ); }
  ConstIterator find ( Iterator it, const T& x ) const { return ConstIterator( sh->find( it.node, x ) ); }
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

#endif
