#include "qpicture.h"
#include <stdio.h>

QPicture::QPicture()
    : QPaintDevice( QInternal::Picture | QInternal::ExternalDevice )
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QPicture::~QPicture()
{
  printf("%s %d\n",__FILE__,__LINE__);
}
