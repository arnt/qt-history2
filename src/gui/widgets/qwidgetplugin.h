/****************************************************************************
**
** Definition of QWidgetPlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qgplugin.h"
#include "qstringlist.h"
#include "qiconset.h"
#endif // QT_H
#ifndef QT_NO_WIDGETPLUGIN

#ifdef Q_WS_WIN
#ifdef QT_PLUGIN
#define QT_WIDGET_PLUGIN_EXPORT __declspec(dllexport)
#else
#define QT_WIDGET_PLUGIN_EXPORT __declspec(dllimport)
#endif
#else
#define QT_WIDGET_PLUGIN_EXPORT
#endif

class QWidgetPluginPrivate;
class QWidget;

class Q_GUI_EXPORT QWidgetPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QWidgetPlugin();
    ~QWidgetPlugin();

    virtual QStringList keys() const = 0;
    virtual QWidget *create(const QString &key, QWidget *parent = 0, const char *name = 0) = 0;

    virtual QString group(const QString &key) const;
    virtual QIconSet iconSet(const QString &key) const;
    virtual QString includeFile(const QString &key) const;
    virtual QString toolTip(const QString &key) const;
    virtual QString whatsThis(const QString &key) const;
    virtual bool isContainer(const QString &key) const;

private:
    QWidgetPluginPrivate *d;
};

class QWidgetContainerPluginPrivate;

class Q_GUI_EXPORT QWidgetContainerPlugin : public QWidgetPlugin
{
     Q_OBJECT
public:
    QWidgetContainerPlugin();
    ~QWidgetContainerPlugin();

    virtual QWidget* containerOfWidget(const QString &key, QWidget *container) const;
    virtual bool isPassiveInteractor(const QString &key, QWidget *container) const;

    virtual bool supportsPages(const QString &key) const;

    virtual QWidget *addPage(const QString &key, QWidget *container,
                              const QString &name, int index) const;
    virtual void insertPage(const QString &key, QWidget *container,
                             const QString &name, int index, QWidget *page) const;
    virtual void removePage(const QString &key, QWidget *container, int index) const;
    virtual void movePage(const QString &key, QWidget *container, int fromIndex, int toIndex) const;
    virtual int count(const QString &key, QWidget *container) const;
    virtual int currentIndex(const QString &key, QWidget *container) const;
    virtual QString pageLabel(const QString &key, QWidget *container, int index) const;
    virtual QWidget *page(const QString &key, QWidget *container, int index) const;
    virtual void renamePage(const QString &key, QWidget *container,
                             int index, const QString &newName) const;
    virtual QWidgetList pages(const QString &key, QWidget *container) const;
    virtual QString createCode(const QString &key, const QString &container,
                                const QString &page, const QString &pageName) const;
};

#endif // QT_NO_WIDGETPLUGIN
#endif // QWIDGETPLUGIN_H
