/****************************************************************************
** $Id: //depot/qt/main/src/tools/qsmartptr.h#2 $
**
** Definition of QSmartPtr class
**
** Created : 990128
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
#ifndef QSMARTPTR_H
#define QSMARTPTR_H

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
