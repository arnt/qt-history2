/****************************************************************************
** $Id: //depot/qt/main/src/tools/qres.h#1 $
**
** Definition of QResource class
**
** Author  : Haavard Nord
** Created : 920527
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRES_H
#define QRES_H

#include "qstrvec.h"


const Res_SDictSize = 11;			// section dict size
const Res_TDictSize = 13;			// tag dict size
const Res_MaxLineLength = 240;			// max length of one line

class QListM_QResEntry;				// internal classes
class QDictM_QResEntry;
#define QResList QListM_QResEntry
#define QResDict QDictM_QResEntry


class QResource					// resource database utility
{
public:
    QResource( bool caseSensitivity=TRUE, bool multipleValues=FALSE,
	       char separator='=',
	       int  sdsize=Res_SDictSize, int tdsize=Res_TDictSize);
    virtual ~QResource();

    int	      load( const char *filename );	// load/save resource file
    bool      save( const char *filename, const char *header = 0 ) const;

    QStrVec  *getAllSections() const;
    QStrVec  *getAllTags( const char *section ) const;
    QStrVec  *getAllValues( const char *section, const char *tag ) const;
    char     *get( const char *section, const char *tag ) const;
    void      insert( const char *section, const char *tag, const char *value);
    bool      remove( const char *section );
    bool      remove( const char *section, const char *tag );
    void      clear();				// remove everything

private:
    QResList *list;				// list of sections
    QResDict *dict;				// dictionary of sections
    int	      tsize;				// tag dictionary size
    bool      cases;				// case sensitivity
    bool      multi;				// multiple values
    char      sep;				// separator
};


#endif // QRES_H
