#include "qclipboard.h"
#include <stdio.h>

void QClipboard::ownerDestroyed()
{
}

void QClipboard::connectNotify( const char * )
{
}

bool QClipboard::event( QEvent * e )
{
    return false;
}

void QClipboard::clear()
{
}

void QClipboard::setData( QMimeSource* src )
{
}

QMimeSource * QClipboard::data() const
{
    return 0;
}




