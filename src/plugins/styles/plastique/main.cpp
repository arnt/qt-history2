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

#include <qstyleplugin.h>
#include "qplastiquestyle.h"

class PlastiqueStyle : public QStylePlugin
{
public:
    PlastiqueStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

PlastiqueStyle::PlastiqueStyle()
    : QStylePlugin()
{
}

QStringList PlastiqueStyle::keys() const
{
    QStringList list;
    list << "Plastique";
    return list;
}

QStyle* PlastiqueStyle::create(const QString& s)
{
    if (s.toLower() == "plastique")
        return new QPlastiqueStyle();
    
    return 0;
}

Q_EXPORT_PLUGIN(PlastiqueStyle)

