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
#include <qmotifstyle.h>

class MotifStyle : public QStylePlugin
{
public:
    MotifStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

MotifStyle::MotifStyle()
: QStylePlugin()
{
}

QStringList MotifStyle::keys() const
{
    QStringList list;
    list << "Motif";
    return list;
}

QStyle* MotifStyle::create(const QString& s)
{
    if (s.toLower() == "motif")
        return new QMotifStyle();

    return 0;
}

Q_EXPORT_PLUGIN(MotifStyle)

