/****************************************************************************
** $Id$
**
** Implementation of QGPluginManager class
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgpluginmanager.h"
#ifndef QT_NO_COMPONENT
#include "qmap.h"
#include "qdir.h"

/*
  The following co-occurrence code is borrowed from Qt Linguist.

  How similar are two texts? The approach used here relies on
  co-occurrence matrices and is very efficient.

  Let's see with an example: how similar are "here" and "hither"?  The
  co-occurrence matrix M for "here" is M[h,e] = 1, M[e,r] = 1,
  M[r,e] = 1 and 0 elsewhere; the matrix N for "hither" is N[h,i] = 1,
  N[i,t] = 1, ..., N[h,e] = 1, N[e,r] = 1 and 0 elsewhere.  The union
  U of both matrices is the matrix U[i,j] = max { M[i,j], N[i,j] },
  and the intersection V is V[i,j] = min { M[i,j], N[i,j] }. The score
  for a pair of texts is

      score = (sum of V[i,j] over all i, j) / (sum of U[i,j] over all i, j),

  a formula suggested by Arnt Gulbrandsen. Here we have

      score = 2 / 6,

  or one third.

  The implementation differs from this in a few details.  Most
  importantly, repetitions are ignored; for input "xxx", M[x,x] equals
  1, not 2.
*/

/*
  Every character is assigned to one of 20 buckets so that the
  co-occurrence matrix requires only 20 * 20 = 400 bits, not
  256 * 256 = 65536 bits or even more if we want the whole Unicode.
  Which character falls in which bucket is arbitrary.

  The second half of the table is a replica of the first half, because of
  laziness.
*/
static const char indexOf[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
    0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
//  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
//  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
//  `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
//  p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,

    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
    1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
    15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0
};

/*
  The entry bitCount[i] (for i between 0 and 255) is the number of
  bits used to represent i in binary.
*/
static const char bitCount[256] = {
    0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8
};

class QCoMatrix
{
public:
    /*
      The matrix has 20 * 20 = 400 entries. This requires 50 bytes, or
      13 words. Some operations are performed on words for more
      efficiency.
    */
    union {
	Q_UINT8 b[52];
	Q_UINT32 w[13];
    };

    QCoMatrix() { memset( b, 0, 52 ); }
    QCoMatrix( const char *text ) {
	char c = '\0', d;
	memset( b, 0, 52 );
	while ( (d = *text) != '\0' ) {
	    setCoocc( c, d );
	    if ( (c = *++text) != '\0' ) {
		setCoocc( d, c );
		text++;
	    }
	}
    }

    void setCoocc( char c, char d ) {
	int k = indexOf[(uchar) c] + 20 * indexOf[(uchar) d];
	b[k >> 3] |= k & 0x7;
    }

    int worth() const {
	int result = 0;
	for ( int i = 0; i < 50; i++ )
	    result += bitCount[b[i]];
	return result;
    }

    static QCoMatrix reunion( const QCoMatrix& m, const QCoMatrix& n )
    {
	QCoMatrix p;
	for ( int i = 0; i < 13; i++ )
	    p.w[i] = m.w[i] | n.w[i];
	return p;
    }
    static QCoMatrix intersection( const QCoMatrix& m, const QCoMatrix& n )
    {
	QCoMatrix p;
	for ( int i = 0; i < 13; i++ )
	    p.w[i] = m.w[i] & n.w[i];
	return p;
    }
};

/*
  Returns an integer between 0 (dissimilar) and 15 (very similar)
  depending on  how similar the string is to \a target.

  This function is efficient, but its results might change in future
  versions of Qt as the algorithm evolves.

  \code
    QString s( "color" );
    a = similarity( s, "color" );  // a == 15
    a = similarity( s, "colour" ); // a == 8
    a = similarity( s, "flavor" ); // a == 4
    a = similarity( s, "dahlia" ); // a == 0
  \endcode
*/
static int similarity( const QString& s1, const QString& s2 )
{
    QCoMatrix m1( s1 );
    QCoMatrix m2( s2 );
    return ( 15 * (QCoMatrix::intersection(m1, m2).worth() + 1) ) /
	   ( QCoMatrix::reunion(m1, m2).worth() + 1 );
}

/*!
  \class QPluginManager qpluginmanager.h
  \brief The QPluginManager class provides basic functions to access a certain kind of functionality in libraries.
  \ingroup componentmodel

  A common usage of components is to extend the existing functionality in an application using plugins. The application
  defines interfaces that abstract a certain group of functionality, and a plugin provides a specialized implementation
  of one or more of those interfaces.

  The QPluginManager template has to be instantiated with an interface definition and the IID for this interface.

  \code
  QPluginManager<MyPluginInterface> *manager = new QPluginManager<MyPluginInterface>( IID_MyPluginInterface );
  \endcode

  It searches a specified directory for all shared libraries, queries for components that implement the specific interface and
  reads information about the features the plugin wants to add to the application. The component can provide the set of features
  provided by implementing either the QFeatureListInterface or the QComponentInformationInterface. The strings returned by the implementations
  of

  \code
  QStringList QFeatureListInterface::featureList() const
  \endcode

  or

  \code
  QString QComponentInformationInterface::name() const
  \endcode

  respectively, can then be used to access the component that provides the requested feature:

  \code
  MyPluginInterface *iface;
  manager->queryInterface( "feature", &iface );
  if ( iface )
      iface->execute( "feature" );
  \endcode

  The application can use a QPluginManager instance to create parts of the user interface based on the list of features
  found in plugins:

  \code
  QPluginManager<MyPluginInterface> *manager = new QPluginManager<MyPluginInterface>( IID_ImageFilterInterface );
  manager->addLibraryPath(...);

  QStringList features = manager->featureList();
  for ( QStringList::Iterator it = features.begin(); it != features.end(); ++it ) {
      MyPluginInterface *iface;
      manager->queryInterface( *it, &iface );

      // use QAction to provide toolbuttons and menuitems for each feature...
  }
  \endcode
*/

/*!
  \fn QPluginManager::QPluginManager( const QUuid& id, const QStringList& paths = QString::null, const QString &suffix = QString::null, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE )

  Creates an QPluginManager for interfaces \a id that will load all shared library files in the \a paths + \a suffix.
  The default policy is set to \a pol. If \a cs is FALSE the manager will handle feature strings case insensitive.

  \warning
  Setting the cs flag to FALSE requires that components also convert to lower case when comparing with passed strings, so this has
  to be handled with care and documented very well.

  The \a pol parameter is propagated to the QLibrary object created for each library.

  \sa QApplication::libraryPaths()
*/


/*!
  \overload QPluginManager::QPluginManager( const QUuid &id, const QString &file, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE )

  Creates an QPluginManager for interfaces \a id that will load the shared library \a file. The default policy is set to \a pol. 
  If \a cs is FALSE the manager will handle feature strings case insensitive.
*/

/*!
  \fn void QPluginManager::addLibraryPath( const QString& path )

  Calls addLibrary for all shared library files in \a path.
  The current library policy will be used for all new QLibrary objects.

  \sa addLibrary(), setDefaultPolicy(), QApplication::libraryPaths()
*/

/*!
  \fn QLibrary* QPluginManager::addLibrary( const QString& file )

  Tries to load the library \a file, adds the library to the managed list and
  returns the created QLibrary object if successful, otherwise returns 0. If
  there is already a QLibrary object for \a file, this object will be returned.
  The library will stay in memory if the default policy is Immediately, otherwise
  it gets unloaded again.

  Note that \a file does not have to include the platform dependent file extension.

  \sa removeLibrary(), addLibraryPath()
*/

/*!
  \fn bool QPluginManager::removeLibrary( const QString& file )

  Removes the library \a file from the managed list and returns TRUE if the library could
  be unloaded, otherwise returns FALSE.

  \warning
  The QLibrary object for this file will be destroyed.

  \sa addLibrary()
*/

/*!
  \fn void QPluginManager::setDefaultPolicy( QLibrary::Policy pol )

  Sets the default policy for this plugin manager to \a pol. The default policy is
  propagated to all newly created QLibrary objects.

  \sa defaultPolicy()
*/

/*!
  \fn QLibrary::Policy QPluginManager::defaultPolicy() const

  Returns the current default policy.

  \sa setDefaultPolicy()
*/

/*!
  \fn QRESULT QPluginManager::queryInterface(const QString& feature, Type** iface) const

  Sets \a iface to point to the interface providing \a feature.

  \sa featureList(), library()
*/

/*!
  \fn const QLibrary* QPluginManager::library( const QString& feature ) const

  Returns a pointer to the QLibrary providing \a feature.

  \sa featureList()
*/

/*!
  \fn QStringList QPluginManager::featureList() const

  Returns a list of all features provided by the interfaces managed by this
  interface manager.

  \sa library(), queryInterface()
*/


QGPluginManager::QGPluginManager( const QUuid& id, QLibrary::Policy pol, bool cs )
    : interfaceId( id ), plugDict( 17, cs ), defPol( pol ), casesens( cs )
{
    // Every QLibrary object is destroyed on destruction of the manager
    libDict.setAutoDelete( TRUE );
}

QGPluginManager::~QGPluginManager()
{
}

void QGPluginManager::addLibraryPath( const QString& path )
{
    if ( !QDir( path ).exists( ".", TRUE ) )
	return;

#if defined(Q_OS_WIN32)
    QString filter = "dll";
#elif defined(Q_OS_MACX)
    QString filter = "dylib";
#elif defined(Q_OS_UNIX)
    QString filter = "so";
#endif

    QStringList plugins = QDir(path).entryList( "*." + filter );
    for ( QStringList::Iterator p = plugins.begin(); p != plugins.end(); ++p ) {
	QString lib = path + "/" + *p;
	if ( libList.contains( lib ) )
	    continue;

	libList.append( lib );

	if ( defPol == QLibrary::Immediately ) {
	    if ( !addLibrary( lib ) )
		libList.remove( lib );
	}
    }
}

void QGPluginManager::setDefaultPolicy( QLibrary::Policy pol )
{
    defPol = pol;
}

QLibrary::Policy QGPluginManager::defaultPolicy() const
{
    return defPol;
}

const QLibrary* QGPluginManager::library( const QString& feature ) const
{
    if ( feature.isEmpty() )
	return 0;

    // We already have a QLibrary object for this feature
    QLibrary *library = 0;
    if ( ( library = plugDict[feature] ) )
	return library;

    // Find the filename that matches the feature request best
    QMap<int, QStringList> map;
    QStringList::ConstIterator it = libList.begin();
    int best = 0;
    int worst = 15;
    while ( it != libList.end() ) {
	QString lib = *it;
	lib = lib.right( lib.length() - lib.findRev( "/" ) - 1 );
	lib = lib.left( lib.findRev( "." ) );
	int s = similarity( feature, lib );
	if ( s < worst )
	    worst = s;
	if ( s > best )
	    best = s;
	map[s].append( *it );
	++it;
    }

    // Start with the best match to get the library object
    QGPluginManager *that = (QGPluginManager*)this;
    for ( int s = best; s >= worst; --s ) {
	QStringList group = map[s];
	QStringList::Iterator git = group.begin();
	while ( git != group.end() ) {
	    QString lib = *git;
	    ++git;
	    if ( that->addLibrary( lib ) && ( library = plugDict[feature] ) )
		return library;
	}
    }

    return 0;
}

QStringList QGPluginManager::featureList() const
{
    // Make sure that all libraries have been loaded once.
    QGPluginManager *that = (QGPluginManager*)this;
    QStringList theLibs = libList;
    QStringList::Iterator it = theLibs.begin();
    while ( it != theLibs.end() ) {
	QString lib = *it;
	++it;
	that->addLibrary( lib );
    }

    QStringList list;
    QDictIterator<QLibrary> pit( plugDict );
    while( pit.current() ) {
	list << pit.currentKey();
	++pit;
    }

    return list;
}

#endif //QT_NO_COMPONENT
