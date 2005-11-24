
#include "quiloader.h"
#include "customwidget.h"

#include <formbuilder.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLayout>
#include <QWidget>
#include <QMap>
#include <private/qobject_p.h>

typedef QMap<QString, bool> widget_map;
Q_GLOBAL_STATIC(widget_map, g_widgets)

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class FormBuilderPrivate: public QFormBuilder
{
    friend class QUiLoader;
    friend class QUiLoaderPrivate;
    typedef QFormBuilder ParentClass;

public:
    QUiLoader *loader;

    FormBuilderPrivate(): loader(0) {}

    QWidget *defaultCreateWidget(const QString &className, QWidget *parent, const QString &name)
    {
        return ParentClass::createWidget(className, parent, name);
    }

    QLayout *defaultCreateLayout(const QString &className, QObject *parent, const QString &name)
    {
        return ParentClass::createLayout(className, parent, name);
    }

    QAction *defaultCreateAction(QObject *parent, const QString &name)
    {
        return ParentClass::createAction(parent, name);
    }

    QActionGroup *defaultCreateActionGroup(QObject *parent, const QString &name)
    {
        return ParentClass::createActionGroup(parent, name);
    }

    virtual QWidget *createWidget(const QString &className, QWidget *parent, const QString &name)
    {
        if (QWidget *widget = loader->createWidget(className, parent, name)) {
            widget->setObjectName(name);
            return widget;
        }

        return 0;
    }

    virtual QLayout *createLayout(const QString &className, QObject *parent, const QString &name)
    {
        if (QLayout *layout = loader->createLayout(className, parent, name)) {
            layout->setObjectName(name);
            return layout;
        }

        return 0;
    }

    virtual QActionGroup *createActionGroup(QObject *parent, const QString &name)
    {
        if (QActionGroup *actionGroup = loader->createActionGroup(parent, name)) {
            actionGroup->setObjectName(name);
            return actionGroup;
        }

        return 0;
    }

    virtual QAction *createAction(QObject *parent, const QString &name)
    {
        if (QAction *action = loader->createAction(parent, name)) {
            action->setObjectName(name);
            return action;
        }

        return 0;
    }
};

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

class QUiLoaderPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUiLoader)
public:
    FormBuilderPrivate builder;

    void setupWidgetMap() const;
};

void QUiLoaderPrivate::setupWidgetMap() const
{
    if (!g_widgets()->isEmpty())
        return;

#define DECLARE_WIDGET(a, b) g_widgets()->insert(QLatin1String(#a), true);
#define DECLARE_LAYOUT(a, b)

#include "widgets.table"

#undef DECLARE_WIDGET
#undef DECLARE_LAYOUT
}

/*!
    \class QUiLoader
    \inmodule QtUiTools

    \brief The QUiLoader class allows standalone applications dynamically
    create user interfaces at run-time using the information stored in
    .ui files or specified plugin paths.

    In addition, you can intervene the process of creating an user
    interface by deriving your own loader class.

    If you have a custom component or application that embed Qt
    Designer, you can also use the QFormBuilder class provided by the
    QtDesigner module to create user interfaces from .ui files.

    The QUiLoader class provides a collection of functions that allows
    you to create widgets based on the information stored in \c .ui
    files (created with Qt Designer) or available in the specified
    plugin paths. The specified plugin paths can be retrieved using
    the pluginPaths() function. You can retrieve the contents of an \c
    .ui file using the load() function. For example:

    \code
    MyForm::MyForm(QWidget *parent)
        : QWidget(parent)
    {
        QUiLoader loader;
        QFile file(":/forms/mywidget.ui");
        file.open(QFile::ReadOnly);
        QWidget *myWidget = loader.load(&file, this);
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(myWidget);
        setLayout(layout);
    }
    \endcode

    By including the user interface in the form's resources (\c
    myForm.grc), we ensure that it will be present at run-time:

    \code
        <!DOCTYPE RCC><RCC version="1.0">
        <qresource prefix="/forms">
        <file>mywidget.ui</file>
        </qresource>
        </RCC>
    \endcode

    The availableWidgets() function returns a QStringList with the
    class names of the widgets available in the specified plugin
    paths. You can create any of these widgets using the
    createWidget() function. For example:

    \code
        QUiLoader loader;
        MyCustomWidget *myWidget;
        QWidget parent;

        QStringList availableWidgets = loader.availableWidgets();

        if (availableWidgets.contains("MyCustomWidget"))
            myWidget = qobject_cast<MyCustomWidget *>loader.createWidget("MyCustomWidget",
                                                                          parent)
        }
    \endcode

    You can make a custom widget available to the loader using the
    addPluginPath() function, and you can remove all the available widgets
    by calling the clearPluginPaths() function.

    The createAction(), createActionGroup(), createLayout() and
    createWidget() functions are used internally by the QUiLoader class
    whenever it has to create an action, action group, layout or
    widget respectively. For that reason, you can subclass the QUiLoader
    class and reimplement these functions to intervene the process of
    constructing an user interface. For example, you might want to
    create a list of the actions created when loading a form or
    creating a custom widget.

    For a complete example using the QUiLoader class, see the \l
    {designer/calculatorbuilder}{Calculator Builder} example.

    \sa QtUiTools, QFormBuilder
*/

/*!
    Creates a form loader with the given \a parent.
*/
QUiLoader::QUiLoader(QObject *parent)
    : QObject(*new QUiLoaderPrivate, parent)
{
    Q_D(QUiLoader);

    d->builder.loader = this;

    QStringList paths;
    foreach (const QString &path, QApplication::libraryPaths()) {
        paths.append(path + QDir::separator() + QLatin1String("designer"));
    }

    d->builder.setPluginPath(paths);
}

/*!
    Destroys the loader.
*/
QUiLoader::~QUiLoader()
{
}

/*!
    \fn QWidget *QUiLoader::load(QIODevice *device, QWidget *parent)

    Loads a form from the given \a device and creates a new widget with the given
    \a parent to hold its contents.

    \sa createWidget()
*/
QWidget *QUiLoader::load(QIODevice *device, QWidget *parentWidget)
{
    Q_D(QUiLoader);
    return d->builder.load(device, parentWidget);
}

/*!
    Returns a list naming the paths the loader searches when locating
    custom widget plugins.

    \sa addPluginPath(), clearPluginPaths()
*/
QStringList QUiLoader::pluginPaths() const
{
    Q_D(const QUiLoader);
    return d->builder.pluginPaths();
}

/*!
    Clears the list of paths the loader searches when locating
    plugins.

    \sa addPluginPath(), pluginPaths()
*/
void QUiLoader::clearPluginPaths()
{
    Q_D(QUiLoader);
    d->builder.clearPluginPaths();
}

/*!
    Adds the given \a path to the list of paths the loader searches
    when locating plugins.

    \sa pluginPaths(), clearPluginPaths()
*/
void QUiLoader::addPluginPath(const QString &path)
{
    Q_D(QUiLoader);
    d->builder.addPluginPath(path);
}

/*!
    Creates a new widget with the given \a parent and \a name
    using the class specified by \a className. You can use this
    function to create any of the widgets returned by the
    availableWidgets() function.

    The function is also used internally by the QUiLoader class whenever
    it has to create a widget. For that reason, you can subclass the
    QUiLoader class and reimplement this function to intervene the
    process of constructing an user interface or widget.

  \sa availableWidgets(), load()
*/
QWidget *QUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    Q_D(QUiLoader);
    return d->builder.defaultCreateWidget(className, parent, name);
}

/*!
    Creates a new layout with the given \a parent and \a name
    using the class specified by \a className.

    The function is used internally by the QUiLoader class whenever it
    has to create a layout. For that reason, you can subclass the
    QUiLoader class and reimplement this function to intervene the
    process of constructing an user interface or widget.

    \sa createWidget(), load()
*/
QLayout *QUiLoader::createLayout(const QString &className, QObject *parent, const QString &name)
{
    Q_D(QUiLoader);
    return d->builder.defaultCreateLayout(className, parent, name);
}

/*!
    Creates a new action group with the given \a parent and \a name.

    The function is used internally by the QUiLoader class whenever it
    has to create an action group. For that reason, you can subclass
    the QUiLoader class and reimplement this function to intervene the
    process of constructing an user interface or widget.

    \sa createAction(), createWidget(), load()
 */
QActionGroup *QUiLoader::createActionGroup(QObject *parent, const QString &name)
{
    Q_D(QUiLoader);
    return d->builder.defaultCreateActionGroup(parent, name);
}

/*!
    Creates a new action with the given \a parent and \a name.

    The function is used internally by the QUiLoader class whenever it
    has to create an action. For that reason, you can subclass the
    QUiLoader class and reimplement this function to intervene the
    process of constructing an user interface or widget.

    \sa createActionGroup(), createWidget(), load()
*/
QAction *QUiLoader::createAction(QObject *parent, const QString &name)
{
    Q_D(QUiLoader);
    return d->builder.defaultCreateAction(parent, name);
}

/*!
    Returns a list naming the available widgets that can be built
    using the createWidget() function, i.e all the widgets specified
    within the given plugin paths.

    \sa pluginPaths(), createWidget()

*/
QStringList QUiLoader::availableWidgets() const
{
    Q_D(const QUiLoader);

    d->setupWidgetMap();
    widget_map available = *g_widgets();

    foreach (QDesignerCustomWidgetInterface *plugin, d->builder.customWidgets()) {
        available.insert(plugin->name(), true);
    }

    return available.keys();
}
