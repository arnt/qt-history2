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

#ifndef TEMPLATEWIZARDIFACE_H
#define TEMPLATEWIZARDIFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>

class QWidget;
struct DesignerFormWindow;

// {983d3eab-fea3-49cc-97ad-d8cc89b7c17b}
#ifndef IID_TemplateWizardInterface
#define IID_TemplateWizardInterface QUuid( 0x983d3eab, 0xfea3, 0x49cc, 0x97, 0xad, 0xd8, 0xcc, 0x89, 0xb7, 0xc1, 0x7b )
#endif

class TemplateWizardInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual void setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *appIface ) = 0;

};

#endif
