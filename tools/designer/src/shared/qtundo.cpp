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

#include "qtundo.h"

#include <QApplication>
#include <QAction>
#include <qalgorithms.h>
#include <qdebug.h>


Q_GLOBAL_STATIC(QtUndoManager, g_manager)


/*!
    \class QtCommand

    \brief The QtCommand class is the base class of all commands stored on a QtUndoStack.

    For an overview of the Qt Undo/Redo framework, see \l overview.html.

    A QtCommand represents a single editing action which an
    application can make, for example, inserting or deleting a block
    of text in a text editor. QtCommand knows how to apply a change to
    an object and how to undo the change.

    A change is applied by calling redo(). A change is undone by
    calling undo(). The default implementations of these virtual
    functions in QtCommand do nothing, they must be overriden in a
    derived class.

    \code
    CmdChangeImage::CmdChangeImage(Face *face, const QString &image)
    {
        m_face = face;
        m_old_image = face->image();
        m_new_image = image;
        setCanMerge(false);
        setDescription(tr("change %1 to %2").arg(m_old_image).arg(m_new_image));
    }

    void CmdChangeImage::redo()
    {
        m_face->setImage(m_new_image);
    }

    void CmdChangeImage::undo()
    {
        m_face->setImage(m_old_image);
    }
    \endcode

    A QtCommand object has a type(), which is used to differentiate
    between ordinary commands and macro delimiters. An ordinary
    command is of type() \c Command. A macro delimiter's type() is
    either \c MacroBegin or \c MacroEnd. Sequences of commands between
    macro delimiters are undone and redone in one go, rather than one
    at a time.

    Each QtCommand has a merge flag, returned by canMerge().
    QtUndoStack::push() will attempt to merge a new command with the
    current command in the stack only if they are both instances of
    the same class (ascertained using QObject::className()) \e and if
    the new command's merge flag is true.

    \warning All classes derived from QtCommand must contain the
    Q_OBJECT macro in their declaration.

    \sa QtUndoStack QtUndoManager
*/

/*!
    \enum QtCommand::Type

    This enumerated type is used by QtCommand to differentiate between
    ordinary commands and macro delimiters.

    \value Command The command is a normal command which applies a
    change to an object.
    \value MacroBegin The command marks the start of a macro sequence.
    \value MacroEnd The command marks the end of a macro sequence.
*/

/*!
    Constructs a QtCommand object with \a description and the merge
    flag set to \a canMerge. The command type is set to
    \c Command. This is the preferred constructor for commands
    which are not macro delimiters.

    \sa description() canMerge()
*/

QtCommand::QtCommand(const QString &description, bool canMerge)
{
    setDescription(description);
    setCanMerge(canMerge);
    m_type = Command;
}

/*!
    Constructs a QtCommand object of \a type, with \a description and
    the merge flag set to \a canMerge. This is the preferred
    constructor for macro delimiters.

    \sa QtCommand::Type description() canMerge()
*/

QtCommand::QtCommand(Type type, const QString &description, bool canMerge)
{
    setDescription(description);
    setCanMerge(canMerge);
    m_type = type;
}

/*!
    \fn QtCommand::Type QtCommand::type()

    Returns the type of this command.
*/
/*!
    \fn bool QtCommand::isMacroBegin()

    Returns true if the command is a macro start delimiter; otherwise
    returns false.

    \sa QtCommand::Type type()
*/
/*!
    \fn bool QtCommand::isMacroEnd()

    Returns true if the command is a macro end delimiter; otherwise
    returns false.

    \sa QtCommand::Type type()

*/
/*!
    \fn bool QtCommand::isCommand()

    Returns true if the command is an ordinary editing command;
    otherwise returns false.

    \sa QtCommand::Type type()
*/
/*!
    \fn QString QtCommand::description() const

    Returns a string which describes the effects of this command.

    The description of the current command is appended to the text
    property of the QAction returned by QUndoManager::undoAction(),
    whenever an object associated with the manager's undo stack has
    focus. The description of the command preceding the current
    command in the undo stack is appended to the text property of the
    QAction returned by QUndoManager::redoAction(), whenever an object
    associated with this undo stack has focus. Typical examples
    include "typing", "delete block", "change font", etc.

    \sa setDescription()
*/

/*!
    \fn void QtCommand::setDescription(const QString &s)

    Sets the description of this command to \a s.

    \sa description()
*/

/*!
    \fn bool QtCommand::canMerge() const

    Returns the command's merge flag. QtUndoStack::push() will attempt
    to merge a new command with the command on top of the stack only
    if they are both instances of the same class (ascertained using
    QObject::className()) \e and if the  new command's merge flag is
    true.

    \sa setCanMerge() mergeMeWith()
*/

/*!
    \fn void QtCommand::setCanMerge(bool b)

    Sets the merge flag for this command to \a b.

    \sa canMerge() mergeMeWith()
*/

/*!
    \fn bool QtCommand::mergeMeWith(QtCommand *other)

    Attempts to merge \a other into this command. Returns true if it
    succeeds; otherwise returns false. If this function returns false,
    QtUndoStack::push() will push \a other on top of the stack. The
    default implementation does nothing and returns false. This function
    must be reimplemented in each derived command class which sets its
    merge flag to true.

    \code
    CmdChangeColor::CmdChangeColor(Face *face, const QString &color)
    {
        ...
        setCanMerge(true);
        ...
    }

    bool CmdChangeColor::mergeMeWith(QtCommand *c)
    {
        CmdChangeColor *other = (CmdChangeColor*) c;

        if (m_face != other->m_face)
            return false;

        m_new_color = other->m_new_color;
        setDescription("change " + m_old_color + " to " + m_new_color);
        setDescription(tr("change %1 to %2").arg(m_old_color).arg(m_new_color));
        return true;
    }
    \endcode

    \sa canMerge() setCanMerge()
*/

bool QtCommand::mergeMeWith(QtCommand *)
{
    return false;
}

/*!
    \fn void QtCommand::redo()

    This virtual function must be reimplemented by subclasses to apply
    changes.
*/

/*!
    \fn void QtCommand::undo()

    This virtual function must be reimplemented by subclasses to undo
    the changes applied by redo(). Calling redo() and then undo()
    should leave the edited object unmodified.
*/

/*!
    \class QtUndoStack

    \brief The QtUndoStack class is a stack of QtCommand objects.

    For an overview of the Qt Undo/Redo framework, see \l overview.html.

    New commands are added with push(). When a command is pushed on to
    the stack, QtUndoStack takes ownership of the command and applies
    it by calling QtCommand::redo(). Undo and redo are invoked with
    undo() and redo(). These functions may be called directly, or
    through the QtUndoManager::undoAction() and
    QtUndoManager::redoAction() QAction objects. In the latter case,
    QtUndoManager will choose a stack based on which object has the
    focus, and call the relevant undo() or redo() function.

    \code
    FaceEdit::FaceEdit(QWidget *parent, const char *name)
        : QWidget(parent, name, Qt::WDestructiveClose)
    {
        ...
        m_undo_stack = new QtUndoStack(this);
        ...
    }

    void FaceEdit::setImage(const QString &image_name)
    {
        Face *face = focusedFace();
        ...
        m_undo_stack->push(new CmdChangeImage(face, image_name));
        ...
    }
    \endcode

    QtUndoStack supports command compression. This is useful when
    several commands can be compressed into a single command, which
    can be undone and redone in one go. An example of this is a text
    editor. When the user types in a character, a new action is
    created, which inserts that character into the document at the
    cursor position. However, it is more convenient for the user to be
    able to undo or redo typing in whole words, sentences and
    paragraphs. Command compression allows these single-character
    commands to be merged into a single command which inserts or
    deletes chunks of text. See push() for more information on
    command compression.

    QtUndoStack supports command macros. A command macro is a sequence
    of commands which are undone and redone in one go. The sequence
    starts with a QtCommand object whose \link QtCommand::type()
    type()\endlink is \link QtCommand::MacroBegin MacroBegin\endlink.
    The \link QtCommand::description() description()\endlink of this
    command is taken to describe the effect of the entire macro. The
    sequence ends with a QtCommand object whose \link
    QtCommand::type() type()\endlink is
    \link QtCommand::MacroEnd MacroEnd\endlink. See undo() and redo()
    for more information on command macros.

    \code
    void FaceEdit::clearFaces()
    {
        ...
        m_undo_stack->push(new QtCommand(QtCommand::MacroBegin, "Clear faces"));
        for (; *child_it != 0; ++child_it) {
            Face *face = (Face*) *child_it;
            m_undo_stack->push(new CmdChangeImage(face, "none"));
            m_undo_stack->push(new CmdChangeColor(face, "green"));
        }
        m_undo_stack->push(new QtCommand(QtCommand::MacroEnd));
        ...
    }
    \endcode

    \sa QtCommand QtUndoManager

*/

/*!
    Constructs a QtUndoStack object. \a parent and \a name are passed
    to the QObject constructor. Additionally, \a parent is registered
    as the target for the newly created stack. When \a parent has the
    focus the QtUndoManager will use this undo stack for its undo()
    and redo() actions.
*/

QtUndoStack::QtUndoStack(QObject *parent)
    : QObject(parent), m_current_iter(-1)
{
    QtUndoManager *manager = QtUndoManager::manager();
    connect(this, SIGNAL(destroyed(QObject*)),
                manager, SLOT(stackDestroyed(QObject*)));
    manager->associateView(parent, this);

    m_macro_nest = 0;
    m_num_commands = 0;
}

int QtUndoStack::findMacroBegin(int it)
{
    int nest = 1;

    QtCommand *command = at(it);
    Q_ASSERT(command != 0 && command->isMacroEnd());
    do {
        --it;

        command = at(it);
        Q_ASSERT(command != 0);

        if (command->isMacroBegin())
            --nest;
        else if (command->isMacroEnd())
            ++nest;
        Q_ASSERT(nest >= 0);
    } while (nest > 0 || !command->isMacroBegin());

    return it;
}

int QtUndoStack::findMacroEnd(int it)
{
    int nest = 1;

    QtCommand *command = at(it);
    Q_ASSERT(command != 0 && command->isMacroBegin());
    do {
        ++it;

        command = at(it);
        Q_ASSERT(command != 0);

        if (command->isMacroEnd())
            --nest;
        else if (command->isMacroBegin())
            ++nest;
        Q_ASSERT(nest >= 0);
    } while (nest > 0 || !command->isMacroEnd());

    return it;
}

/*!
    Pushes \a command onto this stack or merges it with the current
    command. If the current command is not topmost on the stack, all
    commands above it are deleted. Calls the \a command's
    QtCommand::redo() function to apply it.

    This function will attempt to merge a new command with the command
    on top of the stack only if they are both instances of the same
    class (ascertained using QObject::className()) \e and if the new
    command's merge flag is true.

    If there is no current command on this stack, or \a command and
    the current command are of different types, or \a command's merge
    flag is false, or QtCommand::mergeMeWith() returns false, push()
    will push \a command onto the stack.

    \sa undo() redo() clear()
*/

void QtUndoStack::push(QtCommand *command)
{
    command->redo();

    // If the current command on the stack is not last, we delete all
    // commands that follow it before adding the new command.

    while (m_current_iter != size() - 1)
        delete takeLast();

    switch (command->type()) {

        case QtCommand::Command: {
            // Either merge the new command with the current command, or append it to the
            // stack.
            QtCommand *current = m_current_iter == -1 ? 0 : at(m_current_iter);
            if (command->canMerge()
                    && current != 0
                    && current->metaObject() == command->metaObject()
                    && current->mergeMeWith(command))
                delete command;
            else
                append(command);

            if (m_macro_nest == 0)
                ++m_num_commands;

            break;
        }

        case QtCommand::MacroBegin:
            append(command);
            ++m_macro_nest;
            break;

        case QtCommand::MacroEnd:
            if (m_macro_nest == 0) {
                qWarning("QtUndoStack::push(): MacroEnd without MacroBegin");
                return;
            }

            append(command);
            --m_macro_nest;
            if (m_macro_nest == 0)
                ++m_num_commands;

            // Set the description to the corresponding MacroBegin's description
            int it = size() - 1;
            it = findMacroBegin(it); // I've just pushed the MacroEnd
            Q_ASSERT(at(it) != 0);
            command->setDescription(at(it)->description());

            break;
    }

    m_current_iter = size() - 1;
    QtUndoManager::manager()->updateActions();
}

/*!
    Returns true if a command is available for undo; otherwise returns
    false. Undo is not possible if the stack is empty or if the bottom
    command on the stack has already been undone.

    \sa undo() canRedo()
*/

bool QtUndoStack::canUndo()
{
    if (isEmpty())
        return false;

    if (m_macro_nest > 0)
        return false;

    if (m_current_iter < 0 || m_current_iter == size())
        return false;

    return true;
}

/*!
    Returns true if a command is available for redo; otherwise returns
    false. Redo is not possible if the stack is empty or if the top
    command on the stack has already been redone.

    \sa redo() canUndo()
*/

bool QtUndoStack::canRedo()
{
    if (isEmpty())
        return false;

    if (m_macro_nest > 0)
        return false;

    // We know the stack is not empty
    if (m_current_iter < 0)
        return true;

    if (m_current_iter == size() - 1)
        return false;

    return true;
}

void QtUndoStack::undoMacro()
{
    Q_ASSERT(m_macro_nest == 0);
    Q_ASSERT(at(m_current_iter) != 0);
    Q_ASSERT(at(m_current_iter)->type() == QtCommand::MacroEnd);

    int nest = 1;

    QtCommand *command = 0;
    do {
        --m_current_iter;

        command = at(m_current_iter);
        Q_ASSERT(command != 0);

        if (command->isMacroBegin())
            --nest;
        else if (command->isMacroEnd())
            ++nest;
        Q_ASSERT(nest >= 0);

        command->undo();
    } while (nest > 0 || !command->isMacroBegin());
}

void QtUndoStack::redoMacro()
{
    Q_ASSERT(m_macro_nest == 0);
    Q_ASSERT(at(m_current_iter) != 0);
    Q_ASSERT(at(m_current_iter)->type() == QtCommand::MacroBegin);

    int nest = 1;

    QtCommand *command = 0;
    do {
        ++m_current_iter;

        command = at(m_current_iter);
        Q_ASSERT(command != 0);

        if (command->isMacroBegin())
            ++nest;
        else if (command->isMacroEnd())
            --nest;
        Q_ASSERT(nest >= 0);

        command->redo();
    } while (!command->isMacroEnd() || nest > 0);
}

/*!
    If the current command's \link QtCommand::type() type()\endlink is
    \c Command, calls the current command's \link QtCommand::undo()
    undo()\endlink function and moves the current pointer one command
    down the stack.

    If the current command's \link QtCommand::type() type()\endlink is
    \c MacroEnd, traverses the stack downwards calling each command's
    \link QtCommand::undo() undo()\endlink, until a command of type \c
    MacroBegin is found. The current pointer is then set to one
    command below the macro begin marker.

    \sa push() redo() canUndo()
*/

void QtUndoStack::undo()
{
    if (!canUndo()) {
        qWarning("QtUndoStack::undo(): can't undo");
        return;
    }

    QtCommand *command = at(m_current_iter);
    Q_ASSERT(!command->isMacroBegin());

    if (command->isCommand())
        command->undo();
    else
        undoMacro();

    --m_current_iter;

    QtUndoManager::manager()->updateActions();
}


/*!
    If the current command's \link QtCommand::type() type()\endlink is
    \c Command, moves the current pointer one command up in the stack
    and calls the new current command's \link QtCommand::redo()
    redo()\endlink.

    If the current command's \link QtCommand::type() type()\endlink is
    \c MacroBegin, traverses the stack upwards calling each command's
    \link QtCommand::redo() redo()\endlink, until a command of type
    \c MacroEnd is found. The current pointer is left pointing
    to this command.

    \sa push() undo() canRedo()
*/

void QtUndoStack::redo()
{
    if (!canRedo()) {
        qWarning("QtUndoStack::redo(): can't redo");
        return;
    }

    if (m_current_iter < 0)
        m_current_iter = 0;
    else
        ++m_current_iter;

    QtCommand *command = at(m_current_iter);
    Q_ASSERT(!command->isMacroEnd());

    if (command->isCommand())
        command->redo();
    else
        redoMacro();

    QtUndoManager::manager()->updateActions();
}

/*!
    Returns \link QtCommand::description() description()\endlink for
    the current command on the stack, or a null string if there is no
    current command.

    \sa redoDescription()
*/

QString QtUndoStack::undoDescription()
{
    if (canUndo() )
        return at(m_current_iter)->description();
    else
        return QString::null;
}

/*!
    Returns \link QtCommand::description() description()\endlink for
    the command preceding the current command on the stack, or a null
    string if the current command is at the top.

    \sa undoDescription()
*/

QString QtUndoStack::redoDescription()
{
    if (canRedo()) {
        int it = m_current_iter;
        if (it < m_current_iter)
            it = 0;
        else
            ++it;
        return at(it)->description();
    }
    else
        return QString::null;
}

/*!
    Clears all the commands on this undo stack.

    \sa push()
*/

void QtUndoStack::clear()
{
    while( !isEmpty() )
        delete takeFirst();

    QList<QtCommand*>::clear();
    m_macro_nest = 0;
    m_num_commands = 0;
}

/*!
    Returns a list of descriptions of all the commands up to the
    current command in this stack.

    \sa redoList()
*/

QStringList QtUndoStack::undoList()
{
    QStringList result;

    if (m_macro_nest > 0)
        return result;

    if (m_current_iter<0 || m_current_iter >= size())
        return result;

    int it = 0;
    for (; it<size(); ++it) {
        QtCommand *command = at(it);

        result.append(command->description());

        Q_ASSERT(!command->isMacroEnd());
        if (command->isMacroBegin())
            it = findMacroEnd(it);

        if (it == m_current_iter)
            break;
    }

    return result;
}

/*!
    Returns a list of descriptions of all the commands preceding the
    current command in this stack.

    \sa undoList()
*/

QStringList QtUndoStack::redoList()
{
    QStringList result;

    if (m_macro_nest > 0)
        return result;

    int it = m_current_iter;
    if (m_current_iter < 0)
        it = 0;
    else
        ++it;

    for (; it<size(); ++it) {
        QtCommand *command = at(it);

        result.append(command->description());

        Q_ASSERT(!command->isMacroEnd());
        if (command->isMacroBegin())
            it = findMacroEnd(it);
    }

    return result;
}


/*!
    \class QtUndoManager

    \brief The QtUndoManager class manages command stacks in an
    undo/redo framework based on the Command design pattern.

    For an overview of the Qt Undo/Redo framework, see \l overview.html.

    QtUndoManager keeps a list of QtUndoStack objects. Each is a list
    of QtCommand objects and a pointer to the last executed command
    (the undo stack's \e current command). Undo is invoked by calling
    the current command's QtCommand::undo() function and making the
    previous command in the stack the current command. Redo is invoked
    by making the next command in the stack the current command and
    calling its QtCommand::redo() function.

    An application has one global QtUndoManager, accessed through the
    static function QtUndoManager::manager() (which creates the
    QtUndoManager when it is called for the first time).

    Undo and redo are requested through the undo() and redo() slots.
    QtUndoManager also provides two QAction objects connected to these
    slots, returned by undoAction() and redoAction(). They have the
    additional benefit that QtUndoManager appends
    QtCommand::description() to their text properties to reflect the
    command they will undo or redo. They are disabled whenever undo or
    redo are not possible.

    \code
    MainWindow::MainWindow(QWidget *parent, const char *name)
        : QMainWindow(parent, name)
    {
        ...
        QtUndoManager *manager = QtUndoManager::manager();
        QToolBar *toolbar = new QToolBar(this);
        manager->undoAction()->addTo(toolbar);
        manager->redoAction()->addTo(toolbar);

        QPopupMenu *editmenu = new QPopupMenu(this);
        manager->undoAction()->addTo(editmenu);
        manager->redoAction()->addTo(editmenu);
        ...
    }
    \endcode

    A single application can have multiple undo stacks, typically one
    for each editor window in an MDI application. Each stack is
    associated with a widget, called the stack's \e target. undo() and
    redo() requests are directed to a stack whenever its target, or a
    child of its target, has the keyboard focus. A target is associated
    with a stack in QtUndoStack's constructor. Additional targets may be
    associated with a stack using associateView(). This is useful when
    two or more editor windows edit the same underlying object. An SDI
    aplication typically has a single stack associated with the
    application's main window.

    Whenever the widget with the keyboard focus has no targets in its
    parent chain, the QAction objects returned by undoAction() and
    redoAction() are disabled.

    \img qtundo-menu.png
    <p>
    \img qtundo-toolbar.png

    \sa QtCommand QtUndoStack
*/

/*! \internal */

QtUndoManager::QtUndoManager()
{
    m_undo_action = new QAction(); /// ### these two actions needs a parent!!
    m_redo_action = new QAction();
    m_current_stack = 0;

    connect(m_undo_action, SIGNAL(triggered()), this, SLOT(undo()));
    connect(m_redo_action, SIGNAL(triggered()), this, SLOT(redo()));

    updateActions();

    qApp->installEventFilter(this);
}

/*!
    Returns a QAction object connected to the QtUndoManager's undo()
    slot. QtUndoManager appends the QtCommand::description() from the
    current command on the focused target's stack to the QAction's
    text property to reflect what the QAction will undo. If no undo
    command is available, QtUndoManager disables this QAction.

    If the application's default QMimeSourceFactory contains a pixmap
    called "undo", this pixmap is assigned to the QAction.

    \sa undo() redoAction()
*/

QAction *QtUndoManager::undoAction() const
{
    return m_undo_action;
}

/*!
    Returns a QAction object connected to the QtUndoManager's redo()
    slot. QtUndoManager appends the QtCommand::description() from the
    command preceding the current command on the focused target's
    stack to the QAction's text property to reflect what the QAction
    will redo. If no redo command is available, QtUndoManager disables
    this QAction.

    If the application's default QMimeSourceFactory contains a pixmap
    called "redo", this pixmap is assigned to the QAction.

    \sa redo() undoAction()
*/

QAction *QtUndoManager::redoAction() const
{
    return m_redo_action;
}

/*! \internal */

void QtUndoManager::updateActions()
{
    QtUndoStack *stack = currentStack();

    if (stack != 0 && stack->canUndo()) {
        m_undo_action->setText("Undo " + stack->undoDescription());
        m_undo_action->setEnabled(true);
    }
    else {
        m_undo_action->setText("Undo");
        m_undo_action->setEnabled(false);
    }

    if (stack != 0 && stack->canRedo()) {
        m_redo_action->setText("Redo " + stack->redoDescription());
        m_redo_action->setEnabled(true);
    }
    else {
        m_redo_action->setText("Redo");
        m_redo_action->setEnabled(false);
    }

    emit changed();
}

/*!
    Returns true if a command is available for redo; otherwise returns
    false. Redo is not possible if: the widget with the keyboard focus
    has no targets in its parent chain, or a target is found but the
    associated stack is empty, or if the last command on the stack
    has already been undone. The QAction returned by
    QtUndoManager::undoAction() is disabled whenever canUndo() is
    false.

    \sa undo() canRedo()
*/

bool QtUndoManager::canUndo()
{
    QtUndoStack *stack = currentStack();

    return stack != 0 && stack->canUndo();
}

/*!
    Returns true if a command is available for undo; otherwise returns
    false. Undo is not possible if: the widget with the keyboard focus
    has no targets in its parent chain, or a target is found but the
    associated stack is empty, or if the first command on the stack
    has already been redone. The QAction returned by
    QtUndoManager::redoAction() is disabled whenever canRedo() is
    false.

    \sa redo() canUndo()
*/

bool QtUndoManager::canRedo()
{
    QtUndoStack *stack = currentStack();

    return stack != 0 && stack->canRedo();
}

/*! \internal */

void QtUndoManager::stackDestroyed(QObject *stack)
{
    if (m_current_stack == stack)
        m_current_stack = 0;

    // remove all views associated with that stack from the map
    StackMap::iterator it = m_stack_map.begin();
    while (it != m_stack_map.end()) {
        if (*it == stack) {
            disconnect(it.key(), 0, this, 0);
            StackMap::iterator tmp = it; // iterator invalidation
            ++tmp;
            m_stack_map.erase(it);
            it = tmp;
        }
        else ++it;
    }

    updateActions();
}

/*! \internal */

void QtUndoManager::viewDestroyed(QObject *view)
{
    StackMap::iterator it = m_stack_map.find(view);
    if (it == m_stack_map.end()) {
        qWarning("QtUndoManager::viewDestroyed(): no such view");
        return;
    }

    if (*it == m_current_stack)
        m_current_stack = 0;

    m_stack_map.erase(it);

    updateActions();
}


/*!
    Directs an undo request to the appropriate QtUndoStack. The stack
    is chosen by finding the widget with the keyboard focus and
    searching its parent chain for a target. If a target is found,
    QtUndoStack::undo() is called on the associated stack. If no such
    target is found, this function does nothing.

    \sa redo() canUndo()
*/

void QtUndoManager::undo()
{
    QtUndoStack *stack = currentStack();

    if (stack == 0 || !stack->canUndo()) {
        qWarning("QtUndoManager::undo(): can't undo");
        return;
    }

    stack->undo();

    updateActions();
}

/*!
    Directs the redo request to the appropriate QtUndoStack. The stack
    is chosen by finding the widget with the keyboard focus and
    searching its parent chain for a target. If a target is found,
    QtUndoStack::redo() is called on the associated stack. If no such
    target is found, this function does nothing.

    \sa undo() canRedo()
*/

void QtUndoManager::redo()
{
    QtUndoStack *stack = currentStack();

    if (stack == 0 || !stack->canRedo()) {
        qWarning("QtUndoManager::redo(): can't redo");
        return;
    }

    stack->redo();

    updateActions();
}

uint QtUndoManager::m_undo_limit = 0;

/*!
    Returns the application-global instance of QtUndoManager, creating it if
    it does not yet exist.
*/

QtUndoManager *QtUndoManager::manager()
{
    return g_manager();
}

/*!
    Disassociates \a obj from any stack.

    \sa associateView()
*/

void QtUndoManager::disassociateView(QObject *obj)
{
    if (obj == 0) {
        qWarning("QtUndoManager::disassociateView(): canot disassociate null object");
        return;
    }

    StackMap::iterator it = m_stack_map.find(obj);
    if (it == m_stack_map.end()) {
        qWarning("QtUndoManager::disassociateView(): object has no associated stack");
        return;
    }

    disconnect(obj, 0, this, 0);
    m_stack_map.erase(it);
}

/*!
    Associates \a obj with \a stack, making \a obj the \a stack's
    target. undo() and redo() requests will be directed to \a stack,
    whenever \a obj or one of its children has the keyboard focus.

    \sa disassociateView()
*/

void QtUndoManager::associateView(QObject *obj, QtUndoStack *stack)
{
    if (obj == 0) {
        qWarning("QtUndoManager::associateView(): cannot associate a null object");
        return;
    }

    if (stack == 0) {
        qWarning("QtUndoManager::associateView(): cannot associate a null stack");
        return;
    }

    if (m_stack_map.contains(obj)) {
        qWarning("QtUndoManager::associateView(): view already associated with a stack");
        return;
    }

    m_stack_map[obj] = stack;
    connect(obj, SIGNAL(destroyed(QObject*)), this,
                SLOT(viewDestroyed(QObject*)));

    updateActions();
}

/*!
    Returns the maximum size that any undo stack can grow to. A size
    of 0 means that the stacks can grow indefinitely.

    \sa setUndoLimit()
*/

uint QtUndoManager::undoLimit()
{
    return m_undo_limit;
}

/*!
    Sets the maximum size that any stack can grow to, to \a i. A
    size of 0 means that the stacks can grow indefinitely.

    \sa undoLimit()
*/


void QtUndoManager::setUndoLimit(uint i)
{
    m_undo_limit = i;
}

/*! \internal */

bool QtUndoManager::eventFilter(QObject*, QEvent *e)
{
    if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut)
        updateActions();

    return false;
}

/*! \internal */

QtUndoStack *QtUndoManager::currentStack()
{
    QWidget *w = qApp->focusWidget();
    while (w != 0) {
        StackMap::iterator it = m_stack_map.find(w);
        if (it != m_stack_map.end()) {
            m_current_stack = *it;
            break;
        }
        w = w->parentWidget();
    }

    return m_current_stack;
}

/*!
    \fn void QtUndoManager::changed()
    \internal
*/

/*!
    Returns a list of descriptions of all commands up to the the
    current command in the stack associated with the currently focused
    target. If no target has focus, returns an empty list.

    \sa redoList()
*/

QStringList QtUndoManager::undoList()
{
    QtUndoStack *stack = currentStack();
    if (stack == 0)
        return QStringList();

    return stack->undoList();
}

/*!
    Returns a list of descriptions of all commands preceding the
    current command in the stack associated with the currently focused
    target. If no target has focus, returns an empty list.

    \sa undoList()
*/

QStringList QtUndoManager::redoList()
{
    QtUndoStack *stack = currentStack();
    if (stack == 0)
        return QStringList();

    return stack->redoList();
}

QtUndoListModel::QtUndoListModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_undoIndex(-1)
{
    connect(QtUndoManager::manager(), SIGNAL(changed()),
        this, SLOT(updateItems()));
}

QtUndoListModel::~QtUndoListModel()
{
}

void QtUndoListModel::updateItems()
{
    m_items = QtUndoManager::manager()->undoList();
    m_undoIndex = m_items.count();
    m_items += QtUndoManager::manager()->redoList();
    emit reset();
}

int QtUndoListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

int QtUndoListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex QtUndoListModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QModelIndex QtUndoListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, 0);
}

QVariant QtUndoListModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case DisplayRole:
        return m_items.at(index.row());
    default:
        return QVariant();
    }
}


QtUndoListView::QtUndoListView(QWidget *parent)
    : QListView(parent)
{
    setModel(new QtUndoListModel(this));
    setSelectionMode(SingleSelection);
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(undoOrRedo()));
}

QtUndoListView::~QtUndoListView()
{
}

void QtUndoListView::undoOrRedo()
{
    int idx = currentIndex().row();
    bool block = model()->blockSignals(true);

    int undoIndex = static_cast<QtUndoListModel*>(model())->undoIndex();
    QtUndoManager *manager = QtUndoManager::manager();

    if (idx < undoIndex) {
        for (int i = idx; i < undoIndex; ++i) {
            Q_ASSERT(manager->canUndo());
            manager->undo();
        }
    } else {
        for (int i = undoIndex; i < idx; ++i) {
            Q_ASSERT(manager->canRedo());
            manager->redo();
        }
    }

    model()->blockSignals(block);
}
