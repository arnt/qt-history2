#ifndef __Q_MAP_H__
#define __Q_MAP_H__

#include <qshared.h>

struct QMapNodeBase
{
  enum Color { Red, Black };

  QMapNodeBase* left;
  QMapNodeBase* right;
  QMapNodeBase* parent;

  Color color;

  QMapNodeBase* minimum()
  {
    QMapNodeBase* x = this;
    while ( x->left )
      x = x->left;
    return x;
  }

  QMapNodeBase* maximum()
  {
    QMapNodeBase* x = this;
    while ( x->right )
      x = x->right;
    return x;
  }
};

template <class K, class T>
struct QMapNode : public QMapNodeBase
{
  QMapNode( const K& _key, const T& _data ) { data = _data; key = _key; }
  QMapNode( const K& _key ) { key = _key; }
  QMapNode( const QMapNode& _n ) { key = _n.key; data = _n.data; }
  QMapNode() { }
  T data;
  K key;
};

template<class K, class T, class Ref, class Ptr>
struct QMapIterator
{
  /**
   * Typedefs
   */
  typedef QMapIterator< K, T, Ref, Ptr > Type;
  typedef QMapIterator< K, const T, const Ref, const Ptr > ConstType;
  typedef QMapNode< K, T >* NodePtr;
  typedef QMapNode< K, T > Node;
  typedef K& KeyRef;

  /**
   * Variables
   */
  NodePtr node;

  /**
   * Functions
   */
  QMapIterator() : node( 0 ) {}
  QMapIterator( NodePtr p ) : node( p ) {}
  QMapIterator( const Type& i ) : node( i.node ) {}

  bool operator==( const Type& x ) const { return node == x.node; }
  bool operator!=( const Type& x ) const { return node != x.node; }
  Ref operator*() const { return node->data; }
  Ptr operator->() const { return &(node->data); }

  const KeyRef key() const { return node->key; }
  Ref data() const { return node->data; }

  void inc()
  {
    QMapNodeBase* tmp = node;
    if ( tmp->right )
    {
      tmp = tmp->right;
      while ( tmp->left )
        tmp = tmp->left;
    }
    else
    {
      QMapNodeBase* y = tmp->parent;
      while (tmp == y->right)
      {
        tmp = y;
        y = y->parent;
      }
      if (tmp->right != y)
        tmp = y;
    }
    node = (NodePtr)tmp;
  }

  void dec()
  {
    QMapNodeBase* tmp = node;
    if (tmp->color == Node::Red &&
        tmp->parent->parent == tmp )
      tmp = tmp->right;
    else if (tmp->left != 0)
    {
      QMapNodeBase* y = tmp->left;
      while ( y->right )
        y = y->right;
      tmp = y;
    }
    else
    {
      QMapNodeBase* y = tmp->parent;
      while (tmp == y->left)
      {
        tmp = y;
        y = y->parent;
      }
      tmp = y;
    }
    node = (NodePtr)tmp;
  }

  Type& operator++() { 
    inc();
    return *this;
  }

  Type operator++(int) { 
    Type tmp = *this;
    inc();
    return tmp;
  }

  Type& operator--() { 
    dec();
    return *this;
  }

  Type operator--(int) { 
    Type tmp = *this;
    dec();
    return tmp;
  }
};

class QMapPrivateBase : public QShared
{
public:
  QMapPrivateBase() {
    node_count = 0;
  }
  QMapPrivateBase( const QMapPrivateBase* _map) {
    node_count = _map->node_count;
  }

  /**
   * Implementations of basic tree algorithms
   */
  void rotateLeft( QMapNodeBase* x, QMapNodeBase*& root);
  void rotateRight( QMapNodeBase* x, QMapNodeBase*& root );
  void rebalance( QMapNodeBase* x, QMapNodeBase*& root );
  QMapNodeBase* removeAndRebalance( QMapNodeBase* z, QMapNodeBase*& root,
				    QMapNodeBase*& leftmost, QMapNodeBase*& rightmost );

  /**
   * Variables
   */
  int node_count;
};

template <class Key, class T>
class QMapPrivate : public QMapPrivateBase
{
public:
  /**
   * Typedefs
   */
  typedef QMapIterator< Key, T, T&, T* > Iterator;
  typedef QMapIterator< Key, T, const T&, const T* > ConstIterator;
  typedef QMapNode< Key, T > Node;
  typedef QMapNode< Key, T >* NodePtr;

  /**
   * Functions
   */
  QMapPrivate() {
    header = new Node;
    header->color = Node::Red; // Mark the header
    header->parent = 0;
    header->left = header->right = header;
  }
  QMapPrivate( const QMapPrivate< Key, T >* _map ) : QMapPrivateBase( _map ) {
    header = new Node;
    header->color = Node::Red; // Mark the header
    header->parent = copy( (NodePtr)(_map->header->parent) );
    header->parent->parent = header;
    if ( header->parent )
    {
      header->left = header->parent->minimum();
      header->right = header->parent->maximum();
    }
    else
      header->left = header->right = 0;
  }
  ~QMapPrivate() { clear(); delete header; }

  NodePtr copy( NodePtr p )
  {
    if ( !p )
      return 0;
    NodePtr n = new Node( *p );
    if ( p->left )
    {
      n->left = copy( (NodePtr)(p->left) );
      n->left->parent = n;
    }
    else
      n->left = 0;
    if ( p->right )
    {
      n->right = copy( (NodePtr)(p->right) );
      n->right->parent = n;
    }
    else
      n->right = 0;
    return n;
  }

  void clear()
  {
    clear( (NodePtr)(header->parent) );
    header->color = Node::Red;
    header->parent = 0;
    header->left = header->right = header;
  }

  void clear( NodePtr p ) 
  {
    if ( p && p != header )
    {
      if ( p->left && p->left != header )
	clear( (NodePtr)p->left );
      else if ( p->right && p->right != header )
	clear( (NodePtr)p->right );
    }
    delete p;
  }

  Iterator begin() { return Iterator( (NodePtr)(header->left ) ); }
  Iterator end() { return Iterator( header ); }
  ConstIterator begin() const { return ConstIterator( (NodePtr)(header->left ) ); }
  ConstIterator end() const { return ConstIterator( header ); }

  ConstIterator find(const Key& k) const
  {
    QMapNodeBase* y = header;        // Last node
    QMapNodeBase* x = header->parent; // Root node. 

    while ( x != 0 ) 
    {
      // If as k <= key(x) go left
      if ( !( key(x) < k ) )
      {
	y = x;
	x = x->left;
      }
      else
      {
	x = x->right;
      }
    }

    // Was k bigger/smaller then the biggest/smallest
    // element of the tree ? Return end()
    if ( y == header || k < key(y) )
      return ConstIterator( header );
    return ConstIterator( (NodePtr)y );
  }

  void remove( Iterator it )
  {
    NodePtr del = (NodePtr) removeAndRebalance( it.node, header->parent, header->left, header->right );
    delete del;
    --node_count;
  }

#ifdef DEBUG
  void inorder( QMapNodeBase* x = 0, int level = 0 )
  {
    if ( !x ) x = header->parent;

    if ( x->left )
      inorder( x->left, level + 1 );
    cout << level << " Key=" << key(x) << " Value=" << ((NodePtr)x)->data << endl;
    if ( x->right )
      inorder( x->right, level + 1 );      
  }
#endif

  Iterator insertMulti(const Key& v)
  {
    QMapNodeBase* y = header;
    NodePtr x = header->parent;
    while (x != 0)
    {
      y = x;
      x = ( v < key(x) ) ? x->left : x->right;
    }
    return insert(x, y, v);
  }

  Iterator insertSingle( const Key& k )
  {
    // Search correct position in the tree
    QMapNodeBase* y = header;
    QMapNodeBase* x = header->parent;
    bool result = true;
    while ( x != 0 )
    {
      result = ( k < key(x) );
      y = x;
      x = result ? x->left : x->right;
    }
    // Get iterator on the last not empty one
    Iterator j( (NodePtr)y );
    if ( result )
    {
      // Smaller then the leftmost one ?
      if ( j == begin() )
      {
	return insert(x, y, k );
      }
      // Perhaps daddy is the right one ?
      else
      {
	--j;
      }
    }
    // Really bigger ?
    if ( j.node->key < k )
      return insert(x, y, k );
    // We are going to replace a node
    return j;
  }

  Iterator insert( QMapNodeBase* x, QMapNodeBase* y, const Key& k )
  {
    NodePtr z = new Node( k );
    if (y == header || x != 0 || k < key(y) ) {
      y->left = z;                // also makes leftmost = z when y == header
      if ( y == header ) {
	header->parent = z;
	header->right = z;
      }
      else if ( y == header->left )
	header->left = z;           // maintain leftmost pointing to min node
    } else {
      y->right = z;
      if ( y == header->right )
	header->right = z;          // maintain rightmost pointing to max node
    }
    z->parent = y;
    z->left = 0;
    z->right = 0;
    rebalance( z, header->parent );
    ++node_count;
    return Iterator(z);
  }

protected:
  /**
   * Helpers
   */
  const Key& key( QMapNodeBase* b ) const { return ((NodePtr)b)->key; }

  /**
   * Variables
   */
  NodePtr header;
};

template<class Key, class T>
class QMap
{
public:
  /**
   * Typedefs
   */
  typedef QMapIterator< Key, T, T&, T* > Iterator;
  typedef QMapIterator< Key, T, const T&, const T* > ConstIterator;
  typedef T ValueType;
  typedef QMapPrivate< Key, T > Priv;

  /**
   * API
   */
  QMap() { sh = new QMapPrivate< Key, T >; }
  QMap( const QMap<Key,T>& _node ) { sh = _node.sh; sh->ref(); }
  ~QMap() { if ( sh->deref() ) delete sh; }

  Iterator begin() { detach(); return sh->begin(); }
  Iterator end() { detach(); return sh->end(); }
  ConstIterator begin() const { return ((const Priv*)sh)->begin(); }
  ConstIterator end() const { return ((const Priv*)sh)->end(); }

  Iterator find ( const Key& k ) { detach(); return Iterator( sh->find( k ).node ); }
  ConstIterator find ( const Key& k ) const { return sh->find( k ); }
  T& operator[] ( const Key& k ) { detach(); return sh->find( k ).node->data; }
  const T& operator[] ( const Key& k ) const { return sh->find( k ).data(); }
  bool contains ( const Key& k ) const { return sh->find( k ) != ((const Priv*)sh)->end(); }
  
  uint count() const { return sh->node_count; }

  Iterator insert( const Key& key, const T& value )
  {
    detach();
    Iterator it = sh->insertSingle( key );
    it.data() = value;
    return it;
  }

  void remove( Iterator it ) { detach(); sh->remove( it ); }
  void remove( const Key& k ) 
  {
    detach();
    Iterator it = Iterator( sh->find( k ).node );
    if ( it != end() )
      sh->remove( it );
  }

  void clear() { if ( sh->count == 1 ) sh->clear(); else { sh->deref(); sh = new QMapPrivate<Key,T>; } }

protected:
  /**
   * Helpers
   */
  void detach() { if ( sh->count > 1 ) { sh->deref(); sh = new QMapPrivate<Key,T>( sh ); } }
  
  Priv* sh;
};

#endif
