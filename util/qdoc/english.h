/*
  english.h
*/

#ifndef ENGLISH_H
#define ENGLISH_H

#include <qvaluestack.h>

QValueStack<QString> separators( int n,
				 const QString& terminator = QString::null );

#endif
