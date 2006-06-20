/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>

int main()
{
    for (int i=0; i<10240; i++) {
        fprintf(stdout, "%d -this is a number\n", i);
        fflush(stderr);
    }
    

    return 0;
}
