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

#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcom.h>

// {EA8CB381-59B5-44a8-BAE5-9BEA8295762A}
#ifndef IID_ImportFilterInterface
#define IID_ImportFilterInterface QUuid( 0xea8cb381, 0x59b5, 0x44a8, 0xba, 0xe5, 0x9b, 0xea, 0x82, 0x95, 0x76, 0x2a )
#endif

struct ImportFilterInterface : public QFeatureListInterface
{
    virtual QStringList import( const QString& filter, const QString& filename ) = 0;
};

// {C32A07E0-B006-471e-AFCA-D227457A1280}
#ifndef IID_ExportFilterInterface
#define IID_ExportFilterInterface QUuid( 0xc32a07e0, 0xb006, 0x471e, 0xaf, 0xca, 0xd2, 0x27, 0x45, 0x7a, 0x12, 0x80 )
#endif

struct ExportFilterInterface : public QFeatureListInterface
{
//    virtual QStringList export( const QString& filter, const QString& filename ) = 0;
};

#endif
