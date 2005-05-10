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

#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#include "QtGui/qwindowdefs.h"
#include "QtCore/qfactoryinterface.h"
#include "QtCore/qstringlist.h"
#include "QtGui/qicon.h"

struct QWidgetFactoryInterface : public QFactoryInterface
{
    /*! In the implementation create and return the widget \a widget
      here, use \a parent and \a name when creating the widget */
    virtual QWidget* create(const QString &widget, QWidget* parent = 0, const char* name = 0) = 0;

    /*! In the implementation return the name of the group of the
      widget \a widget */
    virtual QString group(const QString &widget) const = 0;

    /*! In the implementation return the icon, which should be used
      in the Qt Designer menubar and toolbar to represent the widget
      \a widget */
    virtual QIcon iconSet(const QString &widget) const = 0;

    /*! In the implementation return the include file which is needed
      for the widget \a widget in the generated code which uic
      generates. */
    virtual QString includeFile(const QString &widget) const = 0;

    /*! In the implementation return the text which should be
      displayed as tooltip for the widget \a widget */
    virtual QString toolTip(const QString &widget) const = 0;

    /*! In the implementation return the text which should be used for
      what's this help for the widget \a widget. */
    virtual QString whatsThis(const QString &widget) const = 0;

    /*! In the implementation return true here, of the \a widget
      should be able to contain other widget in the Qt Designer, else
      false. */
    virtual bool isContainer(const QString &widget) const = 0;
};

Q_DECLARE_INTERFACE(QWidgetFactoryInterface, "com.trolltech.Qt.QWidgetFactoryInterface")

struct QWidgetContainerInterfacePrivate
{
    virtual ~QWidgetContainerInterfacePrivate() {}
    virtual QWidget *containerOfWidget(const QString &f, QWidget *container) const = 0;
    virtual bool isPassiveInteractor(const QString &f, QWidget *container) const = 0;

    virtual bool supportsPages(const QString &f) const = 0;

    virtual QWidget *addPage(const QString &f, QWidget *container,
                              const QString &name, int index) const = 0;
    virtual void insertPage(const QString &f, QWidget *container,
                             const QString &name, int index, QWidget *page) const = 0;
    virtual void removePage(const QString &f, QWidget *container, int index) const = 0;
    virtual void movePage(const QString &f, QWidget *container, int fromIndex, int toIndex) const = 0;
    virtual int count(const QString &key, QWidget *container) const = 0;
    virtual int currentIndex(const QString &key, QWidget *container) const = 0;
    virtual QString pageLabel(const QString &key, QWidget *container, int index) const = 0;
    virtual QWidget *page(const QString &key, QWidget *container, int index) const = 0;
    virtual void renamePage(const QString &key, QWidget *container,
                             int index, const QString &newName) const = 0;
    virtual QWidgetList pages(const QString &f, QWidget *container) const = 0;
    virtual QString createCode(const QString &f, const QString &container,
                                const QString &page, const QString &pageName) const = 0;
};

Q_DECLARE_INTERFACE(QWidgetContainerInterfacePrivate, "com.trolltech.Qt.QWidgetContainerInterfacePrivate")

#ifndef QT_NO_WIDGETPLUGIN

#ifdef QT_PLUGIN
#define QT_WIDGET_PLUGIN_EXPORT Q_DECL_EXPORT
#else
#define QT_WIDGET_PLUGIN_EXPORT Q_DECL_IMPORT
#endif

class QWidget;

class Q_GUI_EXPORT QWidgetPlugin : public QObject, public QWidgetFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWidgetFactoryInterface:QFactoryInterface)
public:
    explicit QWidgetPlugin(QObject *parent = 0);
    ~QWidgetPlugin();

    virtual QStringList keys() const = 0;
    virtual QWidget *create(const QString &key, QWidget *parent = 0, const char *name = 0) = 0;

    virtual QString group(const QString &key) const;
    virtual QIcon iconSet(const QString &key) const;
    virtual QString includeFile(const QString &key) const;
    virtual QString toolTip(const QString &key) const;
    virtual QString whatsThis(const QString &key) const;
    virtual bool isContainer(const QString &key) const;
};

class QWidgetContainerPluginPrivate;

class Q_GUI_EXPORT QWidgetContainerPlugin : public QWidgetPlugin, public QWidgetContainerInterfacePrivate
{
    Q_OBJECT
    Q_INTERFACES(QWidgetContainerInterfacePrivate)
public:
    explicit QWidgetContainerPlugin(QObject *parent = 0);
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
