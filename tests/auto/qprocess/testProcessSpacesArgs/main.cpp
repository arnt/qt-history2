/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>

int main(int argc, char ** argv)
{
    for (int i = 0; i < argc; ++i) {
        if (i)
            printf("|");
        printf(argv[i]);
    }

    return 0;
}
