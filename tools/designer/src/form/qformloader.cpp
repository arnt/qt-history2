
#include "qformloader.h"
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

/*!
    \namespace QForm
    \inmodule QtForm

    \brief The QForm namespace contains classes that are used to
    handle forms created with Qt Designer.
*/

namespace QForm
{

typedef QMap<QString, bool> widget_map;
Q_GLOBAL_STATIC(widget_map, g_widgets)

class FormBuilderPrivate: public QFormInternal::QFormBuilder
{
    friend class Loader;
    friend class LoaderPrivate;
    typedef QFormInternal::QFormBuilder ParentClass;

public:
    Loader *loader;

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

class LoaderPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Loader)
public:
    FormBuilderPrivate builder;

    void setupWidgetMap() const;
};

void LoaderPrivate::setupWidgetMap() const
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
    \class QForm::Loader
    \inmodule QtForm
    \brief The Loader class allows standalone applicatons to create user
    interfaces dynamically at run-time using the information stored in .ui
    files, or using specified plugin paths.

    In addition, by subclassing the Loader class, you can intervene the
    process of creating the user interfaces.

    If you have a custom component or application that embed Qt Designer,
    you can also use the QFormBuilder class provided by the QtDesigner module.

    The Loader class provides functions that allows you to create \l {QWidget}s
    based on the information in \c .ui files created with Qt Designer, or
    available in the specified plugin paths. You can retrieve the contents of
    an \c .ui file using the load() function. For example:

    \code
    MyForm::MyForm(QWidget *parent)
        : QWidget(parent)
    {
        QForm::loader loader;
        QFile file(":/forms/myWidget.ui");
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

    The availableWidgets() function returns a QStringList with the classnames
    of the widgets available in the specified plugin paths (which can be
    retrieved using the pluginPaths() function). You can create
    any of these widgets using the createWidget() function. For example:

    \code
        QForm::Loader loader;
        MyCustomWidget *myWidget;
        Qwidget parent;
        QString name;

        QStringList availableWidgets = loader.availableWidgets();
        bool available = false;

        for each (QString name, availableWidgets) {
        if (name == "MyCustomWidget")
            available = true;
        }
        if (available)
            myWidget = qobject_cast<MyCustomWidget *>loader.createWidget(MyCustomWidget,
                                                                         parent, name)
    \endcode

    You can make a custom widget available to the loader using the
    addPluginPath() function, and you can remove all the available widgets
    by calling the clearPluginPaths() function.

    The createAction(), createActionGroup(), createLayout() and createWidget()
    functions are used internally by the Loader class whenever it has to create
    an action, action group, layout or widget respectively. For that reason, you can
    subclass the Loader class and reimplement these functions to intervene
    the process of constructing an user interface. For example, you might want to
    create a list of the actions created when loading a form or creating a custom widget.

    For a complete example using the Loader class, see the \l
    {designer/calculatorbuilder}{Calculator Builder} example.

    \sa QForm, QFormBuilder
*/

/*!
    \fn QForm::Loader::Loader(QObject *parent)

    Creates a form loader with the given \a parent.
*/
Loader::Loader(QObject *parent)
    : QObject(*new LoaderPrivate, parent)
{
    Q_D(Loader);

    d->builder.loader = this;

    QStringList paths;
    foreach (const QString &path, QApplication::libraryPaths()) {
        paths.append(path + QDir::separator() + QLatin1String("designer"));
    }

    d->builder.setPluginPath(paths);
}

/*!
    \fn QForm::Loader::~Loader()

    Destroys the loader.
*/
Loader::~Loader()
{
}

/*!
    \fn QWidget *QForm::Loader::load(QIODevice *device, QWidget *parent)

    Loads a form from the given \a device and creates a new widget with the given
    \a parent to hold its contents.

    \sa createWidget()
*/
QWidget *Loader::load(QIODevice *device, QWidget *parentWidget)
{
    Q_D(Loader);
    return d->builder.load(device, parentWidget);
}

/*!
    \fn QStringList QForm::Loader::pluginPaths() const

    Returns a list of paths the loader searches to locate custom widget plugins.

    \sa addPluginPath(), clearPluginPaths()
*/
QStringList Loader::pluginPaths() const
{
    Q_D(const Loader);
    return d->builder.pluginPaths();
}

/*!
    \fn void QForm::Loader::clearPluginPaths()

    Clears the list of paths the loader searches to locate plugins.

    \sa addPluginPath(), pluginPaths()
*/
void Loader::clearPluginPaths()
{
    Q_D(Loader);
    d->builder.clearPluginPaths();
}

/*!
    \fn void QForm::Loader::addPluginPath(const QString &path)

    Adds the given \a path to the list of paths the loader searches to locate plugins.

    \sa pluginPaths(), clearPluginPaths()
*/
void Loader::addPluginPath(const QString &path)
{
    Q_D(Loader);
    d->builder.addPluginPath(path);
}

/*!
    \fn QWidget *QForm::Loader::createWidget(const QString &className, QWidget *parent, const QString &name)

    Creates a new widget called \a name using the class specified by \a className with the given
    \a parent widget. You can create any of the widgets returned by the availableWidgets()
    function.

    The function is also used internally by the Loader class whenever it has to create
    a widget. For that reason, you can subclass the Loader class and reimplement
    this function to intervene the process of constructing an user interface or widget.

  \sa availableWidgets(), load()
*/
QWidget *Loader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    Q_D(Loader);
    return d->builder.defaultCreateWidget(className, parent, name);
}

/*!
    \fn QLayout *QForm::Loader::createLayout(const QString &className, QObject *parent, const QString &name)

    Creates a new layout called \a name using the class specified by \a className with the given
    \a parent object.

    The function is used internally by the Loader class whenever it has to create
    a layout. For that reason, you can subclass the Loader class and reimplement
    this function to intervene the process of constructing an user interface or widget.

    \sa createWidget(), load()
*/
QLayout *Loader::createLayout(const QString &className, QObject *parent, const QString &name)
{
    Q_D(Loader);
    return d->builder.defaultCreateLayout(className, parent, name);
}

/*!
    \fn QActionGroup *QForm::Loader::createActionGroup(QObject *parent, const QString &name)

    Creates a new action group called \a name with the given \a parent object.

    The function is used internally by the Loader class whenever it has to create
    an action group. For that reason, you can subclass the Loader class and reimplement
    this function to intervene the process of constructing an user interface or widget.

    \sa createAction(), createWidget(), load()
 */
QActionGroup *Loader::createActionGroup(QObject *parent, const QString &name)
{
    Q_D(Loader);
    return d->builder.defaultCreateActionGroup(parent, name);
}

/*!
    \fn QAction *QForm::Loader::createAction(QObject *parent, const QString &name)

    Creates a new action called \a name with the given \a parent object.

    The function is used internally by the Loader class whenever it has to create
    an action. For that reason, you can subclass the Loader class and reimplement
    this function to intervene the process of constructing an user interface or widget.

    \sa createActionGroup(), createwidget(), load()
*/
QAction *Loader::createAction(QObject *parent, const QString &name)
{
    Q_D(Loader);
    return d->builder.defaultCreateAction(parent, name);
}

/*!
    \fn QStringList QForm::Loader::availableWidgets() const

    Returns a list of available widgets that can be built using the loader, i.e all the widgets
    specified within the given plugin paths.

    \sa pluginPaths()

*/
QStringList Loader::availableWidgets() const
{
    Q_D(const Loader);

    d->setupWidgetMap();
    widget_map available = *g_widgets();

    foreach (QDesignerCustomWidgetInterface *plugin, d->builder.customWidgets()) {
        available.insert(plugin->name(), true);
    }

    return available.keys();
}

} // namespace QForm

