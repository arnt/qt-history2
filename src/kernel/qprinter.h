/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.h#4 $
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
    QPrinter();
   ~QPrinter();

    enum Orientation { Portrait, Landscape };

    const char *printerName()	const;
    void	setPrinterName( const char * );
    const char *docName()	const;
    void	setDocName( const char * );
    const char *creator()	const;
    void	setCreator( const char * );
    Orientation orientation()	const;
    void	setOrientation( Orientation );
    int		fromPage()	const;
    int		toPage()	const;
    void	setFromTo( int fromPage, int toPage );
    int		minPage()	const;
    int		maxPage()	const;
    void	setMinMax( int minPage, int maxPage );
    int		numCopies()	const;
    void	setNumCopies( int );

    bool	newPage();
    bool	abort();
    bool	aborted()	const;

    bool	select( QWidget *parent );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );

private:
#if defined(_WS_X11_)
    QPaintDevice *pdrv;
#endif
    int		state;
    QString	printer_name;
    QString	doc_name;
    QString	creator_name;
    Orientation	orient;
    short	from_pg, to_pg;
    short	min_pg,  max_pg;
    short	ncopies;
};


inline const char *QPrinter::printerName() const
{ return printer_name; }

inline const char *QPrinter::docName() const
{ return doc_name; }

inline const char *QPrinter::creator() const
{ return creator_name; }

inline QPrinter::Orientation QPrinter::orientation() const
{ return orient; }

inline int QPrinter::fromPage() const
{ return from_pg; }

inline int QPrinter::toPage() const
{ return to_pg; }

inline int QPrinter::minPage() const
{ return min_pg; }

inline int QPrinter::maxPage() const
{ return max_pg; }

inline int QPrinter::numCopies() const
{ return ncopies; }


#endif // QPRINTER_H
