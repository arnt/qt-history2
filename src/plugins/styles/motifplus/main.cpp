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
#include <qmotifplusstyle.h>

class MotifPlusStyle : public QStylePlugin
{
public:
    MotifPlusStyle();

    QStringList keys() const;
    QStyle *create(const QString&);

};

MotifPlusStyle::MotifPlusStyle()
: QStylePlugin()
{
}

QStringList MotifPlusStyle::keys() const
{
    QStringList list;
    list << "MotifPlus";
    return list;
}

QStyle* MotifPlusStyle::create(const QString& s)
{
    if (s.toLower() == "motifplus")
        return new QMotifPlusStyle();

    return 0;
}

Q_EXPORT_PLUGIN(MotifPlusStyle)

