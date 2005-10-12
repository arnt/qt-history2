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

#ifndef QFORMLOADER_H
#define QFORMLOADER_H

#include <QtCore/QObject>

class QWidget;
class QLayout;
class QAction;
class QActionGroup;
class QString;
class QIODevice;

namespace QForm
{

class LoaderPrivate;

class Loader: public QObject
{
public:
    Loader(QObject *parent = 0);
    virtual ~Loader();

    QStringList pluginPaths() const;
    void clearPluginPaths();
    void addPluginPath(const QString &path);

    QWidget *load(QIODevice *device, QWidget *parentWidget = 0);
    QStringList availableWidgets() const;

    virtual QWidget *createWidget(const QString &className, QWidget *parent = 0);
    virtual QLayout *createLayout(const QString &className, QObject *parent = 0);
    virtual QActionGroup *createActionGroup(QObject *parent = 0);
    virtual QAction *createAction(QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(Loader)
    Q_DISABLE_COPY(Loader)
};

} // namespace QForm

#endif // QFORMLOADER_H
