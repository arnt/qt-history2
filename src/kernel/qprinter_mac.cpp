#include "qprinter.h"
#include <stdio.h>

QPrinter::QPrinter()
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QPrinter::~QPrinter()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

bool QPrinter::newPage()
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

bool QPrinter::abort()
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

bool QPrinter::aborted() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

bool QPrinter::setup( QWidget * parent )
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

int QPrinter::metric( int m ) const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}



