#include "qprinter.h"

#if QT_FEATURE_PRINTER

#include <stdio.h>

QPrinter::QPrinter()
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
}

QPrinter::~QPrinter()
{
}

bool QPrinter::newPage()
{
    return false;
}

bool QPrinter::abort()
{
    return false;
}

bool QPrinter::aborted() const
{
    return false;
}

bool QPrinter::setup( QWidget * parent )
{
    return false;
}

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    return false;
}

int QPrinter::metric( int m ) const
{
    return 0;
}

#endif // QT_FEATURE_PRINTER
