/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include "creditform.h"


int main( int argc, char *argv[] ) 
{
    QApplication app( argc, argv );

    CreditForm creditForm;
    app.setMainWidget( &creditForm );
    creditForm.show();

    return app.exec();
}


