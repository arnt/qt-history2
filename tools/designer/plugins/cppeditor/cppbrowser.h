/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CPPBROWSER_H
#define CPPBROWSER_H

#include <browser.h>

class CppEditorBrowser : public EditorBrowser
{
    Q_OBJECT

public:
    CppEditorBrowser( Editor *e );
    void showHelp( const QString &word );

};

#endif
