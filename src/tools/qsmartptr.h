#ifndef __qsmartptr_h__
#define __qsmartptr_h__

#include "qshared.h"

template< class T >
class QSmartPtrPrivate : public QShared
{
public:
  QSmartPtrPrivate( T* t ) : QShared() { addr = t; }
  ~QSmartPtrPrivate() { delete addr; }
  
  T* addr;
};

template< class T >
struct QSmartPtr
{
public:
  QSmartPtr() { ptr = new QSmartPtrPrivate<T>( 0 ); }
  QSmartPtr( T* t ) { ptr = new QSmartPtrPrivate<T>( t ); }
  QSmartPtr( const QSmartPtr& p ) { ptr = p.ptr; ptr->ref(); }
  ~QSmartPtr() { if ( ptr->deref() ) delete ptr; }

  QSmartPtr<T>& operator= ( const QSmartPtr<T>& p ) {
    if ( ptr->deref() ) delete ptr;
    ptr = p.ptr; ptr->ref();
    return *this;
  }
  QSmartPtr<T>& operator= ( T* p ) { 
    if ( ptr->deref() ) delete ptr;
    ptr = new QSmartPtrPrivate<T>( p );
    return *this;
  }
  bool operator== ( const QSmartPtr<T>& p ) const { return ( ptr->addr == p.ptr->addr ); }
  bool operator!= ( const QSmartPtr<T>& p ) const { return ( ptr->addr != p.ptr->addr ); }
  bool operator== ( const T* p ) const { return ( ptr->addr == p ); }
  bool operator!= ( const T* p ) const { return ( ptr->addr != p ); }
  bool operator!() const { return ( ptr->addr == 0 ); }
  operator bool() const { return ( ptr->addr != 0 ); }
  operator T*() { return ptr->addr; }
  operator const T*() const { return ptr->addr; }

  const T& operator*() const { return *(ptr->addr); }
  T& operator*() { return *(ptr->addr); }
  const T* operator->() const { return ptr->addr; }
  T* operator->() { return ptr->addr; }

private:
  QSmartPtrPrivate<T>* ptr;
};

#endif
