/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILEICONPROVIDER_H
#define QFILEICONPROVIDER_H

#include <QtGui/qicon.h>
#include <QtCore/qfileinfo.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_FILEICONPROVIDER

class QFileIconProviderPrivate;

class Q_GUI_EXPORT QFileIconProvider
{
public:
    QFileIconProvider();
    virtual ~QFileIconProvider();
    enum IconType { Computer, Desktop, Trashcan, Network, Drive, Folder, File };
    virtual QIcon icon(IconType type) const;
    virtual QIcon icon(const QFileInfo &info) const;
    virtual QString type(const QFileInfo &info) const;

private:
    Q_DECLARE_PRIVATE(QFileIconProvider)
    QFileIconProviderPrivate *d_ptr;
    Q_DISABLE_COPY(QFileIconProvider)
};

#endif

QT_END_HEADER

#endif

