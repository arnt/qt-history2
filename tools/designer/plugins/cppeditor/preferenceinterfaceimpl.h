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

#ifndef PREFERENCEINTERFACEIMPL_H
#define PREFERENCEINTERFACEIMPL_H

#include <preferenceinterface.h>
class QWidget;

class PreferenceInterfaceImpl : public PreferenceInterface
{
public:
    PreferenceInterfaceImpl( QUnknownInterface *outer = 0 );
    virtual ~PreferenceInterfaceImpl();

    ulong addRef();
    ulong release();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

    Preference *preference();
    void connectTo( QUnknownInterface * ) {}
    void deletePreferenceObject( Preference * );

private:
    QUnknownInterface *parent;
    int ref;
    QWidget *cppEditorSyntax;

};

#endif

