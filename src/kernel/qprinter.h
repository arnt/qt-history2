/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.h#18 $
**
** Definition of QPrinter class
**
** Created : 940927
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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
    enum PageSize    { A4, B5, Letter, Legal, Executive };

    const char *printerName()	const;
    void	setPrinterName( const char * );
    bool	outputToFile()	const;
    void	setOutputToFile( bool );
    const char *outputFileName()const;
    void	setOutputFileName( const char * );
    const char *printProgram()	const;
    void	setPrintProgram( const char * );

    const char *docName()	const;
    void	setDocName( const char * );
    const char *creator()	const;
    void	setCreator( const char * );

    Orientation orientation()	const;
    void	setOrientation( Orientation );
    PageSize	pageSize()	const;
    void	setPageSize( PageSize );

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

    bool	setup( QWidget *parent = 0 );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;

#if defined(_WS_WIN_)
    void	setActive();
    void	setIdle();
#endif

private:
#if defined(_WS_X11_)
    QPaintDevice *pdrv;
#endif
    int		state;
    QString	printer_name;
    QString	output_filename;
    bool	output_file;
    QString	print_prog;
    QString	doc_name;
    QString	creator_name;
    Orientation orient;
    PageSize	page_size;
    short	from_pg, to_pg;
    short	min_pg,	 max_pg;
    short	ncopies;

private:	// Disabled copy constructor and operator=
    QPrinter( const QPrinter & );
    QPrinter &operator=( const QPrinter & );
};


inline const char *QPrinter::printerName() const
{ return printer_name; }

inline bool QPrinter::outputToFile() const
{ return output_file; }

inline const char *QPrinter::outputFileName() const
{ return output_filename; }

inline const char *QPrinter::printProgram() const
{ return print_prog; }

inline const char *QPrinter::docName() const
{ return doc_name; }

inline const char *QPrinter::creator() const
{ return creator_name; }

inline QPrinter::PageSize QPrinter::pageSize() const
{ return page_size; }

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
