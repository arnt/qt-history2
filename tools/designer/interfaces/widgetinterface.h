 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef WIDGETINTERFACE_H
#define WIDGETINTERFACE_H

#include <qcom.h>
#include <qiconset.h>

class QWidget;

// {55184143-F18F-42c0-A8EB-71C01516019A}
#ifndef IID_WidgetInterface
#define IID_WidgetInterface QUuid( 0x55184143, 0xf18f, 0x42c0, 0xa8, 0xeb, 0x71, 0xc0, 0x15, 0x16, 0x1, 0x9a )
#endif

struct WidgetInterface : public QFeatureListInterface
{
public:
    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;

    virtual QString group( const QString& ) const = 0;
    virtual QString iconSet( const QString& ) const = 0;
    virtual QIconSet iconset( const QString& ) const = 0;
    virtual QString includeFile( const QString& ) const = 0;
    virtual QString toolTip( const QString& ) const = 0;
    virtual QString whatsThis( const QString& ) const = 0;
    virtual bool isContainer( const QString& ) const = 0;
};

#endif
