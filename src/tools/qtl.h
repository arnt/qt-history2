#ifndef __QTL_H__
#define __QTL_H__

#include <qtextstream.h>
#include <qstring.h>

template <class T>
class QStreamIterator
{
protected:
  QTextStream& stream;
  QString separator;

public:
  QStreamIterator( QTextStream& s) : stream( s ) {}
  QStreamIterator( QTextStream& s, const QString& sep ) : stream( s ), separator( sep )  {}
  QStreamIterator<T>& operator= ( const T& x ) { 
    stream << x;
    if ( !separator.isEmpty() )
      stream << separator;
    return *this;
  }
  QStreamIterator<T>& operator*() { return *this; }
  QStreamIterator<T>& operator++() { return *this; } 
  QStreamIterator<T>& operator++(int) { return *this; } 
};

template <class InputIterator, class OutputIterator>
inline OutputIterator qCopy( InputIterator _begin, InputIterator _end, OutputIterator _dest )
{
  while( _begin != _end )
     *_dest++ = *_begin++;
  return _dest;
}

template <class Container, class OutputIterator>
inline OutputIterator qCopy( Container& _container, OutputIterator _dest )
{
  typename Container::Iterator begin = _container.begin();
  typename Container::Iterator end = _container.end();
  while( begin != end )
     *_dest++ = *begin++;
  return _dest;
}

template <class T>
inline void qSwap( T& _value1, T& _value2 )
{
  T tmp = _value1;
  _value1 = _value2;
  _value2 = tmp;
}

template <class InputIterator>
inline void qBubbleSort( InputIterator b, InputIterator e )
{
  // Goto last element;
  InputIterator last = e;
  --last;
  // only one element or no elements ?
  if ( last == b )
    return;

  // So we have at least two elements in here
  while( b != last )
  {
    bool swapped = false;
    InputIterator swap_pos = b;
    InputIterator x = e;
    InputIterator y = x;
    y--;
    do
    {
      --x;
      --y;
      if ( *x < *y )
      {
	swapped = true;
	qSwap( *x, *y );
	swap_pos = y;
      }
    } while( y != b );
    if ( !swapped )
      return;
    b = swap_pos;
    b++;
  }
}

template <class Container>
inline void qBubbleSort( const Container &c )
{
  qBubbleSort( c.begin(), c.end() );
}

template <class HeapPtr>
void qHeapSortPushDown( HeapPtr heap, int first, int last )
{
  int r = first;
  while( r <= last/2 )
  {
    // Node r has only one child ?
    if ( last == 2*r )
    {
      // Need for swapping ?
      if ( heap[r] > heap[ 2*r ] )
	qSwap( heap[r], heap[ 2*r ] );
      // That's it ...
      r = last;
    }
    // Node has two children
    else
    {
      if ( heap[r] > heap[ 2*r ] && heap[ 2*r ] <= heap[ 2*r+1 ] )
      {
        // Swap with left child
        qSwap( heap[r], heap[ 2*r ] );
        r *= 2;
      }
      else if ( heap[r] > heap[ 2*r+1 ] && heap[ 2*r+1 ] < heap[ 2*r ] )
      {
        // Swap with right child
        qSwap( heap[r], heap[ 2*r+1 ] );
        r = 2*r+1;
      }
      else
        // We are done
        r = last;
    }
  }
}

template <class InputIterator, class Value>
void qHeapSortHelper( InputIterator b, InputIterator e, Value, int n )
{
  // Create the heap
  InputIterator insert = b;
  Value* realheap = new Value[ n ];
  // Wow, what a fake. But I want the heap to be indexed as 1...n
  Value* heap = realheap - 1;
  int size = 0;
  for( ; insert != e; ++insert )
  {
    heap[++size] = *insert;
    int i = size;
    while( i > 1 && heap[i] < heap[ i / 2 ] )
    {
      qSwap( heap[i], heap[ i / 2 ] );
      i /= 2;
    }
  }

  // Now do the sorting
  for( int i = n; i > 0; i-- )
  {
    *b++ = heap[1];
    if ( i > 1 )
    {
      heap[1] = heap[i];
      qHeapSortPushDown( heap, 1, i - 1 );
    }
  }

  delete[] realheap;
}

template <class InputIterator>
inline void qHeapSort( InputIterator b, InputIterator e )
{
  // Empty ?
  if ( b == e )
    return;

  // How many entries have to be sorted ?
  InputIterator it = b;
  int n = 0;
  while ( it != e ) { ++n; ++it; }

  // The last parameter is a hack to retrieve the value type
  // Do the real sorting here
  qHeapSortHelper( b, e, *b );
}

template <class Container>
inline void qHeapSort( Container c )
{
  if ( c.isEmpty() )
    return;

  // The last parameter is a hack to retrieve the value type
  // Do the real sorting here
  qHeapSortHelper( c.begin(), c.end(), *(c.begin()), c.count() );
}

template <class Container>
class QInsertIterator
{
protected:
  Container& container;
  typename Container::Iterator it;

public:
  QInsertIterator( Container& x, typename Container::Iterator i ) 
    : container( x ), it(i) {}

  QInsertIterator<Container>&  operator= ( const typename Container::ValueType& x ) { 
    it = container.insert( it, x );
    ++it;
    return *this;
  }

  QInsertIterator<Container>& operator*() { return *this; }
  QInsertIterator<Container>& operator++() { return *this; }
  QInsertIterator<Container>& operator++(int) { return *this; }
};

#endif
