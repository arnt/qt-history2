/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.h#2 $
**
** Definition of QPrinter class
**
** Authors : Eirik Eng and Haavard Nord
** Created : 940927
**
** Copyright (c) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QPRINTER_H
#define QPRINTER_H

#include "qpaintd.h"
#include "qstring.h"


class QPrinter : public QPaintDevice
{
public:
    QPrinter( const char *name=0 );
   ~QPrinter();

    const char *printerName()	const;
    void	setPrinterName( const char * );
    const char *docName()	const;
    void	setDocName( const char * );
    int		fromPage()	const;
    int		toPage()	const;
    void	setFromTo( int fromPage, int toPage );
    int		minPages()	const;
    int		maxPages()	const;
    void	setMinMax( int minPages, int maxPages );
    int		numCopies()	const;
    void	setNumCopies( int );
    const char *creator()	const;
    void	setCreator( const char * );

    bool	abort();
    bool	aborted()	const;
    bool	page();

    static bool select( QPrinter *, QWidget *parent );

protected:
    bool	cmd( int, QPDevCmdParam * );

private:
    QString	pname;
    QString	dname;
    QString	cname;
    int		state;
    short	fromp, top;
    short	minp, maxp;
    short	ncopies;
};


inline const char *QPrinter::printerName() const
{ return pname; }

inline const char *QPrinter::docName() const
{ return dname; }

inline int QPrinter::fromPage() const
{ return fromp; }

inline int QPrinter::toPage() const
{ return top; }

inline int QPrinter::minPages() const
{ return minp; }

inline int QPrinter::maxPages() const
{ return maxp; }

inline int QPrinter::numCopies() const
{ return ncopies; }

inline const char *QPrinter::creator() const
{ return cname; }


#endif // QPRINTER_H
