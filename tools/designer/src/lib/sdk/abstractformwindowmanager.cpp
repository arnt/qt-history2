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

#include "abstractformwindowmanager.h"

/*!
    \class QDesignerFormWindowManagerInterface

    \brief The QDesignerFormWindowManagerInterface class allows you to
    manipulate the collection of form windows in Qt Designer, and
    control Qt Designer's form editing actions.

    \inmodule QtDesigner

    QDesignerFormWindowManagerInterface is not intended to be
    instantiated directly. \QD uses the form window manager to
    control the various form windows in its workspace. You can
    retrieve an interface to \QD's form window manager using
    the QDesignerFormEditorInterface::formWindowManager()
    function. For example:

    \code
        QDesignerFormWindowManagerInterface *manager = 0;
        QDesignerFormWindowInterface *formWindow = 0;

        manager = formEditor->formWindowManager();
        formWindow = manager->formWindow(0);

        manager->setActiveFormWindow(formWindow);
    \endcode

    When implementing a custom widget plugin, a pointer to \QD's
    current QDesignerFormEditorInterface object (\c formEditor) is
    provided by the QDesignerCustomWidgetInterface::initialize()
    function's parameter.  You must subclass the
    QDesignerCustomWidgetInterface to expose your plugin to Qt
    Designer.

    The form window manager interface provides the createFormWindow()
    function that enables you to create a new form window which you
    can add to the collection of form windows that the manager
    maintains, using the addFormWindow() slot. It also provides the
    formWindowCount() function returning the current number of form
    windows under the managers control, the formWindow() function
    returning the form window associated with a given index and the
    activeFormWindow() function returning the currently selected form
    window. The removeFormWindow() slot allows you to reduce the
    number of form windows the manager must maintain, and the
    setActiveFormWindow() slot allows you to change the form window
    focus in \QD's workspace.

    In addition, QDesignerFormWindowManagerInterface contains a
    collection of functions that enables you to intervene and control
    \QD's form editing actions. All these functions return the
    original action, making it possible to propagate the function
    further after intervention.

    Finally, the interface provides three signals which are emitted
    when a form window is added, when the currently selected form
    window changes, or when a form window is removed, respectively. All
    the signals carry the form window in question as their parameter.

    \sa QDesignerFormEditorInterface, QDesignerFormWindowInterface
*/

/*!
    Constructs an interface with the given \a parent for the form window
    manager.
*/
QDesignerFormWindowManagerInterface::QDesignerFormWindowManagerInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the interface for the form window manager.
*/
QDesignerFormWindowManagerInterface::~QDesignerFormWindowManagerInterface()
{
}

/*!
    Allows you to intervene and control \QD's "cut" action. The function
    returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionCut() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "copy" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionCopy() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "paste" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionPaste() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "delete" action. The function
    returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionDelete() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "select all" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionSelectAll() const
{
    return 0;
}

/*!
    Allows you to intervene and control the action of lowering a form
    window in \QD's workspace. The function returns the original
    action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionLower() const
{
    return 0;
}

/*!
    Allows you to intervene and control the action of raising of a
    form window in \QD's workspace. The function returns the original
    action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionRaise() const
{
    return 0;
}

/*!
    Allows you to intervene and control a request for horizontal
    layout for a form window in \QD's workspace. The function returns
    the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionHorizontalLayout() const
{
    return 0;
}

/*!
    Allows you to intervene and control a request for vertical layout
    for a form window in \QD's workspace. The function returns the
    original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionVerticalLayout() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "split horizontal"
    action. The function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionSplitHorizontal() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "split vertical"
    action. The function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionSplitVertical() const
{
    return 0;
}

/*!
    Allows you to intervene and control a request for grid layout for
    a form window in \QD's workspace. The function returns the
    original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionGridLayout() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "break layout" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionBreakLayout() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "adjust size" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionAdjustSize() const
{
    return 0;
}

/*!
   Returns the currently active form window in \QD's workspace.

   \sa setActiveFormWindow(), removeFormWindow()
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::activeFormWindow() const
{
    return 0;
}

/*!
    Returns a pointer to \QD's current QDesignerFormEditorInterface
    object.
*/
QDesignerFormEditorInterface *QDesignerFormWindowManagerInterface::core() const
{
    return 0;
}

/*!
   Adds the given \a formWindow to the collection of windows \QD's
   form window manager maintains.

   \sa formWindowAdded()
*/
void QDesignerFormWindowManagerInterface::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
   Removes the given \a formWindow from the collection of windows
   \QD's form window manager maintains.

   \sa formWindow(), formWindowRemoved()
*/
void QDesignerFormWindowManagerInterface::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
   Sets the given \a formWindow to be the currently active form window in
   \QD's workspace.

   \sa activeFormWindow(), activeFormWindowChanged()
*/
void QDesignerFormWindowManagerInterface::setActiveFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
}

/*!
   Returns the number of form windows maintained by \QD's form window
   manager.
*/
int QDesignerFormWindowManagerInterface::formWindowCount() const
{
    return 0;
}

/*!
   Returns the form window at the given \a index.

   \sa setActiveFormWindow(), removeFormWindow()
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::formWindow(int index) const
{
    Q_UNUSED(index);
    return 0;
}

/*!
  \fn QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::createFormWindow(QWidget *parent, Qt::WindowFlags flags)

   Creates a form window with the given \a parent and the given window
   \a flags.

   \sa addFormWindow()
*/
QDesignerFormWindowInterface *QDesignerFormWindowManagerInterface::createFormWindow(QWidget *parentWidget, Qt::WindowFlags flags)
{
    Q_UNUSED(parentWidget);
    Q_UNUSED(flags);
    return 0;
}

/*!
    Allows you to intervene and control \QD's "undo" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionUndo() const
{
    return 0;
}

/*!
    Allows you to intervene and control \QD's "redo" action. The
    function returns the original action.

    \sa QAction
*/
QAction *QDesignerFormWindowManagerInterface::actionRedo() const
{
    return 0;
}

/*!
    \fn void QDesignerFormWindowManagerInterface::formWindowAdded(QDesignerFormWindowInterface *formWindow)

    This signal is emitted when a new form window is added to the
    collection of windows \QD's form window manager maintains. A
    pointer to the new \a formWindow is passed as an argument.

    \sa addFormWindow(), setActiveFormWindow()
*/

/*!
    \fn void QDesignerFormWindowManagerInterface::formWindowRemoved(QDesignerFormWindowInterface *formWindow)

    This signal is emitted when a form window is removed from the
    collection of windows \QD's form window manager maintains. A
    pointer to the removed \a formWindow is passed as an argument.

    \sa removeFormWindow()
*/

/*!
    \fn void QDesignerFormWindowManagerInterface::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)

    This signal is emitted when the contents of the currently active
    form window in \QD's workspace changed. A pointer to the currently
    active \a formWindow is passed as an argument.

    \sa activeFormWindow()
*/

/*!
    \fn void QDesignerFormWindowManagerInterface::dragItems(const QList<QDesignerDnDItemInterface*> &item_list)

    \internal
*/
