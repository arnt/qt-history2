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

#ifndef LANGUAGEINTERFACE_H
#define LANGUAGEINTERFACE_H

#include <qcomponentinterface.h>
#include <qvaluelist.h>

// {f208499a-6f69-4883-9219-6e936e55a330}
#ifndef IID_LanguageInterface
#define IID_LanguageInterface QUuid( 0xf208499a, 0x6f69, 0x4883, 0x92, 0x19, 0x6e, 0x93, 0x6e, 0x55, 0xa3, 0x30 )
#endif

struct LanguageInterface : public QUnknownInterface
{
    struct Function
    {
	QString name;
	QString body;
    };

    virtual void functions( const QString &code, QValueList<Function> *funcs ) const = 0;
    virtual QString createFunctionStart( const QString &className, const QString &func ) = 0;
    virtual QStringList definitions() const = 0;
    virtual QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const = 0;
    virtual void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface ) = 0;

};

#endif
