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
#include <qwindowsstyle.h>

class WindowsStyle : public QStylePlugin
{
public:
    WindowsStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

WindowsStyle::WindowsStyle()
: QStylePlugin()
{
}

QStringList WindowsStyle::keys() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* WindowsStyle::create(const QString& s)
{
    if (s.toLower() == "windows")
        return new QWindowsStyle();

    return 0;
}

Q_EXPORT_PLUGIN(WindowsStyle)

