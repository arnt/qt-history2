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

#include <qdecorationplugin_qws.h>
#include <qdecorationhydro_qws.h>

class DecorationHydro : public QDecorationPlugin
{
public:
    DecorationHydro();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationHydro::DecorationHydro()
: QDecorationPlugin()
{
}

QStringList DecorationHydro::keys() const
{
    QStringList list;
    list << "Hydro";
    return list;
}

QDecoration* DecorationHydro::create(const QString& s)
{
    if (s.toLower() == "hydro")
        return new QDecorationHydro();

    return 0;
}

Q_EXPORT_PLUGIN(DecorationHydro)
