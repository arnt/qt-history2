#include "qclipboard.h"
#include <stdio.h>

void QClipboard::ownerDestroyed()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QClipboard::connectNotify(const char *)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

bool QClipboard::event(QEvent * e)
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

QString QClipboard::text() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return QString();
}

void QClipboard::setText(const QString &text)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QClipboard::clear()
{
  printf("%s %d\n",__FILE__,__LINE__);
}






