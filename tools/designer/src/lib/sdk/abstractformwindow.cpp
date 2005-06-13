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

#include "abstractformwindow.h"

#include <QtGui/QTabBar>
#include <QtGui/QSizeGrip>
#include <QtGui/QAbstractButton>
#include <QtGui/QToolBox>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>

#include <QtCore/qdebug.h>

/*!
    \class QDesignerFormWindowInterface
    \brief The QDesignerFormWindowInterface class provides an interface that is used to control
    form windows provided by \QD's form editing component.
    \inmodule QtDesigner

    \sa QDesignerFormEditorInterface
*/

/*!
    \enum QDesignerFormWindowInterface::FeatureFlag

    This enum describes the features that are available and can be controlled by the
    form window interface. These values are used when querying the form window to determine
    which features it supports:

    \value EditFeature      Form editing
    \value GridFeature      Grid display and snap-to-grid facilities for editing
    \value TabOrderFeature  Tab order management
    \value DefaultFeature   Support for default features (form editing and grid)

    \sa hasFeature()
*/

/*!
    Constructs a form window interface with the given \a parent and specified window \a flags.*/
QDesignerFormWindowInterface::QDesignerFormWindowInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the form window interface.*/
QDesignerFormWindowInterface::~QDesignerFormWindowInterface()
{
}

/*!
    Returns the core form editor interface associated with this interface.*/
QDesignerFormEditorInterface *QDesignerFormWindowInterface::core() const
{
    return 0;
}

/*!
    \fn QDesignerFormWindowInterface *QDesignerFormWindowInterface::findFormWindow(QWidget *widget)

    Returns the form window interface for the given \a widget.*/
QDesignerFormWindowInterface *QDesignerFormWindowInterface::findFormWindow(QWidget *w)
{
    while (w != 0) {
        if (QDesignerFormWindowInterface *fw = qobject_cast<QDesignerFormWindowInterface*>(w)) {
            return fw;
        } else if (w->isWindow()) {
            break;
        }

        w = w->parentWidget();
    }

    return 0;
}

/*!
    \fn virtual QString QDesignerFormWindowInterface::fileName() const = 0

    Returns the file name of the .ui file that describes the form currently being shown.

    \sa setFileName()
*/

/*!
    \fn virtual QDir QDesignerFormWindowInterface::absoluteDir() const = 0

    Returns the absolute location of the directory containing the form shown in the form
    window.
*/

/*!
    \fn virtual QString QDesignerFormWindowInterface::contents() const = 0

    Returns details of the contents of the form currently being displayed in the window.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setContents(QIODevice *device) = 0

    Sets the form's contents using data obtained from the given \a device.

    Data can be read from QFile objects or any other subclass of QIODevice.
*/

/*!
    \fn virtual Feature QDesignerFormWindowInterface::features() const = 0

    Returns a combination of the features provided by the form window associated with the
    interface. The value returned can be tested against the \l Feature enum values to
    determine which features are supported by the window.

    \sa setFeatures()
*/

/*!
    \fn virtual bool QDesignerFormWindowInterface::hasFeature(Feature feature) const = 0

    Returns true if the form window offers the specified \a feature; otherwise returns false.
*/

/*!
    \fn virtual QString QDesignerFormWindowInterface::author() const = 0

    Returns details of the author or creator of the form currently being displayed in the window.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setAuthor(const QString &author) = 0

    Sets the details for the author or creator of the form to the \a author specified.
*/

/*!
    \fn virtual QString QDesignerFormWindowInterface::comment() const = 0

    Returns comments about the form currently being displayed in the window.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setComment(const QString &comment) = 0

    Sets the information about the form to the \a comment specified. This information should
    be a human-readable comment about the form.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::layoutDefault(int *margin, int *spacing) = 0

    Fills in the default margin and spacing for the form's default layout in the \a margin and
    \a spacing variables specified.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setLayoutDefault(int margin, int spacing) = 0

    Sets the default \a margin and \a spacing for the form's layout.

    \sa layoutDefault()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::layoutFunction(QString *margin, QString *spacing) = 0

    Fills in the current margin and spacing for the form's layout in the \a margin and
    \a spacing variables specified.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setLayoutFunction(const QString &margin, const QString &spacing) = 0

    Sets the \a margin and \a layout for the form's layout.

    \sa layoutFunction()
*/

/*!
    \fn virtual QString QDesignerFormWindowInterface::pixmapFunction() const = 0

    Returns the name of the function used to generate a pixmap for the form window.

    \omit
    ### Explain
    \endomit

    \sa setPixmapFunction()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setPixmapFunction(const QString &pixmapFunction) = 0

    Sets the function used to generate the pixmap for the form window to the given
    \a pixmapFunction.

    \sa pixmapFunction()
*/

/*!
    \fn virtual QString QDesignerFormWindowInterface::exportMacro() const = 0

    Returns the export macro associated with the form currently being displayed in the window.
    The export macro is used when the form is compiled to create a widget plugin.

    \sa \link Creating Custom Widgets for Qt Designer \endlink
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setExportMacro(const QString &exportMacro) = 0

    Sets the form window's export macro to \a exportMacro. The export macro is used when building
    a widget plugin to export the form's interface to other components.
*/

/*!
    \fn virtual QStringList QDesignerFormWindowInterface::includeHints() const = 0

    Returns a list of hints.

    \omit
    ### Explain
    \endomit

    \sa setIncludeHints()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setIncludeHints(const QStringList &includeHints) = 0

    Sets the include hints used by the form to the list specified by \a includeHints.

    \omit
    ### Explain
    \endomit

    \sa includeHints()
*/

/*!
    \fn virtual QDesignerFormWindowCursorInterface *QDesignerFormWindowInterface::cursor() const = 0

    Returns the cursor interface used by the form window.
*/

/*!
    \fn virtual int QDesignerFormWindowInterface::toolCount() const = 0

    Returns the number of tools available.
*/

/*!
    \fn virtual int QDesignerFormWindowInterface::currentTool() const = 0

    Returns the index of the current tool in use.

    \sa setCurrentTool()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setCurrentTool(int index) = 0

    Sets the current tool to be the one with the given \a index.

    \sa currentTool()
*/

/*!
    \fn virtual QDesignerFormWindowToolInterface *QDesignerFormWindowInterface::tool(int index) const = 0

    Returns the tool interface used by the form window.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::registerTool(QDesignerFormWindowToolInterface *tool) = 0

    Registers the given \a tool with the form window.
*/

/*!
    \fn virtual QPoint QDesignerFormWindowInterface::grid() const = 0

    Returns the grid spacing used by the form window.

    \sa setGrid()
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowInterface::mainContainer() const = 0

    Returns the main container widget for the form window.

    \sa setMainContainer()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setMainContainer(QWidget *mainContainer) = 0

    Sets the main container widget on the form to the specified \a mainContainer.

    \sa mainContainer()
*/

/*!
    \fn virtual bool QDesignerFormWindowInterface::isManaged(QWidget *widget) const = 0

    Returns true if the specified \a widget is managed by the form window; otherwise returns
    false.

    \sa setManaged()
*/

/*!
    \fn virtual bool QDesignerFormWindowInterface::isDirty() const = 0

    Returns true if the form window is "dirty" (is modified but not saved); otherwise returns
    false.

    \sa setDirty()
*/

/*!
    \fn virtual QtUndoStack *QDesignerFormWindowInterface::commandHistory() const = 0

    Returns an object that can be used to obtain the commands used so far in the construction
    of the form.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::beginCommand(const QString &description) = 0

    Begins execution of a command. Commands are executed between beginCommand() and endCommand()
    function calls to ensure that the undo stack records them.

    \sa endCommand()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::endCommand() = 0

    Ends execution of the current command.

    \sa beginCommand()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::simplifySelection(QList<QWidget*> *widgets) const = 0

    Simplifies the selection of widgets specified by \a widgets.

    \sa selectionChanged()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::emitSelectionChanged() = 0
    \internal

    Emits the selectionChanged() signal.
*/

/*!
    \fn virtual QStringList QDesignerFormWindowInterface::resourceFiles() const = 0

    Returns a list of paths to resource files that are currently being used by the form window.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::addResourceFile(const QString &path) = 0

    Adds the resource file at the given \a path to those used by the form.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::removeResourceFile(const QString &path) = 0

    Removes the resource file at the specified \a path from the list of those used by the form.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::ensureUniqueObjectName(QObject *object) = 0

    Ensures that the specified \a object has a unique name amongst the other objects on the form.
*/

// Slots

/*!
    \fn virtual void QDesignerFormWindowInterface::manageWidget(QWidget *widget) = 0

    Instructs the form window to manage the specified \a widget.

    \sa isManaged()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::unmanageWidget(QWidget *widget) = 0

    Instructs the form window not to manage the specified \a widget.

    \sa aboutToUnmanageWidget(), widgetUnmanaged()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setFeatures(Feature features) = 0

    Enables the specified \a features for the form window.

    \sa features()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setDirty(bool dirty) = 0

    If \a dirty is true, the form window is marked as dirty, meaning that it has been modified
    but not saved. If \a dirty is false, the form window is considered to be unmodified.

    \sa isDirty()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::clearSelection(bool changePropertyDisplay) = 0

    Clears the current selection in the form window.

    \omit
     If \a changePropertyDisplay is true, the
    property editor will be updated to indicate that 
    \endomit
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::selectWidget(QWidget *widget, bool select) = 0

    If \a select is true, the given \a widget is selected; otherwise the \a widget is deselected.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setGrid(const QPoint &grid) = 0

    Sets the grid size for the form window to the point specified by \a grid. In this function,
    the coordinates in the QPoint are used to specify the dimensions of a rectangle in the grid.

    \sa grid()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setFileName(const QString &fileName) = 0

    Sets the file name for the form to the given \a fileName.

    \sa fileName()
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::setContents(const QString &contents) = 0

    Sets the contents of the form using data read from the specified \a contents string.
*/

/*!
    \fn virtual void QDesignerFormWindowInterface::editWidgets() = 0

    Switches the form window into editing mode.

    \sa \link Qt Designer's Form Editing Mode \endlink
*/

// Signals

/*!
    \fn void QDesignerFormWindowInterface::mainContainerChanged(QWidget *mainContainer)

    This signal is emitted whenever the main container changes.
    The new container is specified by \a mainContainer.
*/

/*!
    \fn void QDesignerFormWindowInterface::toolChanged(int toolIndex)

    This signal is emitted whenever the current tool changes.
    The specified \a toolIndex is the index of the new tool in the list of
    tools in the widget box.
*/

/*!
    \fn void QDesignerFormWindowInterface::fileNameChanged(const QString &fileName)

    This signal is emitted whenever the file name of the form changes.
    The new file name is specified by \a fileName.
*/

/*!
    \fn void QDesignerFormWindowInterface::featureChanged(Feature feature)

    This signal is emitted whenever a feature changes in the form.
    The new feature is specified by \a feature.
*/

/*!
    \fn void QDesignerFormWindowInterface::selectionChanged()

    This signal is emitted whenever the selection in the form changes.

    \sa geometryChanged()
*/

/*!
    \fn void QDesignerFormWindowInterface::geometryChanged()

    This signal is emitted whenever the form's geometry changes.

    \sa selectionChanged()
*/

/*!
    \fn void QDesignerFormWindowInterface::resourceFilesChanged()

    This signal is emitted whenever the list of resource files used by the
    form changes.
*/

/*!
    \fn void QDesignerFormWindowInterface::widgetManaged(QWidget *widget)

    This signal is emitted whenever a widget on the form becomes managed.
    The newly managed widget is specified by \a widget.
*/

/*!
    \fn void QDesignerFormWindowInterface::widgetUnmanaged(QWidget *widget)

    This signal is emitted whenever a widget on the form becomes unmanaged.
    The newly released widget is specified by \a widget.
*/

/*!
    \fn void QDesignerFormWindowInterface::aboutToUnmanageWidget(QWidget *widget)

    This signal is emitted whenever a widget on the form is about to become unmanaged.
    When this signal is emitted, the specified \a widget is still managed, and a
    widgetUnmanaged() signal will follow, indicating when it is no longer managed.
*/

/*!
    \fn void QDesignerFormWindowInterface::activated(QWidget *widget)

    This signal is emitted whenever a widget is activated on the form.
    The activated widget is specified by \a widget.
*/

/*!
    \fn void QDesignerFormWindowInterface::changed()

    This signal is emitted whenever a form is changed.
*/

/*!
    \fn void QDesignerFormWindowInterface::widgetRemoved(QWidget *widget)

    This signal is emitted whenever a widget is removed from the form.
    The widget that was removed is specified by \a widget.
*/
