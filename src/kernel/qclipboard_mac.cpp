#include "qclipboard.h"
#include <stdio.h>

void QClipboard::ownerDestroyed()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QClipboard::connectNotify( const char * )
{
  printf("%s %d\n",__FILE__,__LINE__);
}

bool QClipboard::event( QEvent * e )
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

void QClipboard::clear()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QClipboard::setData( QMimeSource* src )
{
}

QMimeSource * QClipboard::data() const
{
    return 0;
}




