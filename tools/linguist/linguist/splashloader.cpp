/**********************************************************************
**   Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "splashloader.h"
#include "trwindow.h"

QPixmap splashScreen()
{
    return TrWindow::splash();
}
