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

#include "qwidgetplugin.h"

#ifndef QT_NO_WIDGETPLUGIN
#include "qwidget.h"

/*!
    \class QWidgetPlugin
    \brief The QWidgetPlugin class provides an abstract base for custom QWidget plugins.

    \ingroup plugins

    The widget plugin is a simple plugin interface that makes it easy
    to create custom widgets that can be included in other Qt applications.

    Writing a widget plugin is achieved by subclassing this base
    class, reimplementing the pure virtual functions keys(), create(),
    group(), iconSet(), includeFile(), toolTip(), whatsThis() and
    isContainer(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro.

    \sa {plugins-howto.html}{Plugins Documentation}
*/

/*!
    \fn QStringList QWidgetPlugin::keys() const

    Returns the list of widget keys this plugin supports.

    These keys must be the class names of the custom widgets that are
    implemented in the plugin.

    \sa create()
*/

/*!
    \fn QWidget *QWidgetPlugin::create(const QString &, QWidget *, const char *)

    Creates and returns a QWidget object for the widget key \a key.
    The widget key is the class name of the required widget. The \a
    name and \a parent arguments are passed to the custom widget's
    constructor.

    \sa keys()
*/

/*!
    Constructs a widget plugin with the given \a parent. This is
    invoked automatically by the \c Q_EXPORT_PLUGIN macro.
*/
QWidgetPlugin::QWidgetPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the widget plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QWidgetPlugin::~QWidgetPlugin()
{
}

/*!
    Returns the group (toolbar name) that the custom widget of class
    \a key should be part of when \e{Qt Designer} loads it.

    The default implementation returns a null QString.
*/
QString QWidgetPlugin::group(const QString &) const
{
    return QString();
}

/*!
    Returns the icon that \e{Qt Designer} should use to represent
    the custom widget of class \a key in the toolbar.

    The default implementation returns an null icon.
*/
QIcon QWidgetPlugin::iconSet(const QString &) const
{
    return QIcon();
}

/*!
    Returns the name of the include file that \e{Qt Designer} and \c
    uic should use to include the custom widget of class \a key in
    generated code.

    The default implementation returns a null QString.
*/
QString QWidgetPlugin::includeFile(const QString &) const
{
    return QString();
}

/*!
    Returns the text of the tooltip that \e{Qt Designer} should use
    for the custom widget of class \a key's toolbar button.

    The default implementation returns a null QString.
*/
QString QWidgetPlugin::toolTip(const QString &) const
{
    return QString();
}

/*!
    Returns the text of the whatsThis text that \e{Qt Designer} should
    use when the user requests whatsThis help for the custom widget of
    class \a key.

    The default implementation returns a null QString.
*/
QString QWidgetPlugin::whatsThis(const QString &) const
{
    return QString();
}

/*!
    Returns true if the custom widget of class \a key can contain
    other widgets, e.g. like QFrame; otherwise returns false.

    The default implementation returns false.
*/
bool QWidgetPlugin::isContainer(const QString &) const
{
    return false;
}

/*!
    \class QWidgetContainerPlugin qwidgetplugin.h

    \brief The QWidgetContainerPlugin class provides an abstract base
    for complex custom container QWidget plugins.

    \ingroup plugins

    The widget container plugin is a subclass of QWidgetPlugin and
    extends the interface with functions necessary for supporting
    complex container widgets via plugins. These container widgets are
    widgets that have one or multiple sub widgets which act as the
    widget's containers. If the widget has multiple container
    subwidgets, they are referred to as "pages", and only one can be
    active at a time. Examples of complex container widgets include:
    QTabWidget, QWidgetStack and QToolBox.

    Writing a complex container widget plugin is achieved by
    subclassing this base class. First by reimplementing
    QWidgetPlugin's pure virtual functions keys(), create(), group(),
    iconSet(), includeFile(), toolTip(), whatsThis() and
    isContainer(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro. In addition containerOfWidget(), isPassiveInteractor() and
    supportsPages() must be reimplemented. If the widget
    supportsPages(), count(), currentIndex(), pageLabel(), page(),
    pages() and createCode() must be implemented. If the widget
    supportsPages() and you want to allow the containers pages to be
    modified, you must also reimplement addPage(), insertPage(),
    removePage(), movePage() and renamePage().

    \sa QWidgetPlugin
*/

/*!
    Constructs a complex container widget plugin with the given \a
    parent. This is invoked automatically by the \c Q_EXPORT_PLUGIN
    macro.
*/

QWidgetContainerPlugin::QWidgetContainerPlugin(QObject *parent)
    : QWidgetPlugin(parent)
{
}

/*!
    Destroys the complex container widget plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/

QWidgetContainerPlugin::~QWidgetContainerPlugin()
{
}

/*!
    Operates on the plugin's \a key class.

    Returns the current \a container's custom widget. If the custom
    widget is a tab widget, this function takes the \a container as
    input and returns the widget's current page.

    The default implementation returns \a container.
*/

QWidget* QWidgetContainerPlugin::containerOfWidget(const QString &,
                                                    QWidget *container) const
{
    return container;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's number of pages. If the
    custom widget is a tab widget, this function returns the number of
    tabs.

    The default implementation returns 0.
*/

int QWidgetContainerPlugin::count(const QString &, QWidget *) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's current page index. If
    the custom widget is a tab widget, this function returns the
    current tab's index.

    The default implementation returns -1.
*/

int QWidgetContainerPlugin::currentIndex(const QString &, QWidget *) const
{
    return -1;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's label at position \a
    index. If the custom widget is a tab widget, this function returns
    the current tab's label.

    The default implementation returns an empty string.
*/

QString QWidgetContainerPlugin::pageLabel(const QString &, QWidget *, int) const
{
    return QString();
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's page at position \a
    index. If the custom widget is a tab widget, this function returns
    the tab at index position \e index.


    The default implementation returns 0.
*/

QWidget *QWidgetContainerPlugin::page(const QString &, QWidget *, int) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    Returns true if the \a container custom widget is a passive
    interactor for class \e key; otherwise returns false. The \a
    container is a child widget of the actual custom widget.

    Usually, when a custom widget is used in \e{Qt Designer}'s design
    mode, no widget receives any mouse or key events, since \e{Qt
    Designer} filters and processes them itself. If one or more
    widgets of a custom widget still need to receive such events, for
    example, because the widget needs to change pages, this function
    must return true for the widget. In such cases \e{Qt Designer}
    will not filter out key and mouse events destined for the widget.

    If the custom widget is a tab widget, the tab bar is the passive
    interactor, since that's what the user will use to change pages.

    The default implementation returns false.
*/

bool QWidgetContainerPlugin::isPassiveInteractor(const QString &,
                                                 QWidget *container) const
{
    Q_UNUSED(container)
    return false;
}

/*!
    Operates on the plugin's \a key class.

    Returns true if the widget supports pages; otherwise returns
    false. If the custom widget is a tab widget this function should
    return true.

    The default implementation returns false.
*/

bool QWidgetContainerPlugin::supportsPages(const QString &) const
{
    return false;
}

/*!
    Operates on the plugin's \a key class.

    This function is called when a new page with the given \a name
    should be added to the \a container custom widget at position \a
    index.

    The default implementation does nothing.
*/

QWidget* QWidgetContainerPlugin::addPage(const QString &, QWidget *,
                                          const QString &, int) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    This function is called when a new page, \a page, with the given
    \a name should be added to the \a container custom widget at
    position \a index.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::insertPage(const QString &, QWidget *,
                                         const QString &, int, QWidget *) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a index should
    be removed from the \a container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::removePage(const QString &, QWidget *, int) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a fromIndex should
    be moved to position \a toIndex in the \a container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::movePage(const QString &, QWidget *, int, int) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a index should
    be renamed (have its label changed) to \a newName in the \a
    container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::renamePage(const QString &, QWidget *,
                                         int, const QString &) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function should return a list of the \a container custom
    widget's pages.
*/

QWidgetList QWidgetContainerPlugin::pages(const QString &, QWidget *) const
{
    return QWidgetList();
}

/*!
    Operates on the plugin's \a key class.

    This function is called from \e{Qt Designer}'s User Interface
    Compiler \c uic, when generating C++ code for inserting a page in
    the \a container custom widget. The name of the page widget which
    should be inserted at the end of the container is \a page, and the
    label of the page should be \a pageName.

    If the custom widget was a QTabWidget, the implementation of this
    function should return:

    \code
    return widget + "->addTab(" + page + ", \"" + pageName + "\")";
    \endcode

    Warning: If the code returned by this function contains invalid
    C++ syntax, the generated \c uic code will not compile.
*/

QString QWidgetContainerPlugin::createCode(const QString &, const QString &,
                                            const QString &, const QString &) const
{
    return QString();
}

#endif //QT_NO_WIDGETPLUGIN
