#include <QApplication>
#include <QAction>
#include <qalgorithms.h>
#include <QtCore/qdebug.h>

#include "qtundo.h"

Q_GLOBAL_STATIC(QtUndoManager, g_manager)

class UndoRedoAction : public QAction
{
    Q_OBJECT

public:
    UndoRedoAction(QObject *parent)
        : QAction(parent) {}

public slots:
    // It's a pity QAction::setText() is not a slot...
    void setTextSlot(const QString &text)
    { setText(text); }
};

/*!
    \class QtCommand

    \brief The QtCommand class is the base class of all commands stored on a QtUndoStack.

    For an overview of the Qt Undo/Redo framework, see the
    \link overview.html overview\endlink.

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

    A QtCommand object has a description(), which describes its effect
    on the edited object. This description is used to give
    human-readable information to the widgets which trigger undo or
    redo in an application, such as the QAction objects returned by
    QtUndoManager::createUndoAction() and QtUndoManager::createRedoAction().

    \warning All classes derived from QtCommand must contain the
    \c Q_OBJECT macro in their declaration.

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
    Constructs a QtCommand object of the given \a type, with a \a
    description and the merge flag set to \a canMerge. This is the
    preferred constructor for macro delimiters.

    \sa QtCommand::Type description() canMerge()
*/

QtCommand::QtCommand(Type type, const QString &description, bool canMerge)
{
    setDescription(description);
    setCanMerge(canMerge);
    m_type = type;
}

/*!
    \fn QtCommand::Type QtCommand::type() const

    Returns the type of this command.
*/
/*!
    \fn bool QtCommand::isMacroBegin() const

    Returns true if the command is a macro start delimiter; otherwise
    returns false.

    \sa QtCommand::Type type()
*/
/*!
    \fn bool QtCommand::isMacroEnd() const

    Returns true if the command is a macro end delimiter; otherwise
    returns false.

    \sa QtCommand::Type type()

*/
/*!
    \fn bool QtCommand::isCommand() const

    Returns true if the command is an ordinary editing command;
    otherwise returns false.

    \sa QtCommand::Type type()
*/
/*!
    \fn QString QtCommand::description() const

    Returns a string which describes the effect of this command on
    the edited object.

    Typical examples include "typing", "delete block", "change font", etc.

    This description is used to assign human-readable information to
    the widgets which trigger undo or redo in an application, such as
    the QAction objects returned by QtUndoManager::createUndoAction()
    and QtUndoManager::createRedoAction().


    \sa setDescription() QtUndoStack::undoDescription() QtUndoStack::redoDescription() QtUndoManager::undoDescription() QtUndoManager::redoDescription()
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

    Attempts to merge the \a other command into this command. Returns
    true if it succeeds; otherwise returns false. If this function
    returns false, QtUndoStack::push() will push the \a other command
    on top of the stack. The default implementation does nothing and
    returns false. This function must be reimplemented in each derived
    command class which sets its merge flag to true.

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

QtMultiCommand::QtMultiCommand(const QString &description)
    : QtCommand(Command, description, false)
{
}

QtMultiCommand::QtMultiCommand(const QList<QtCommand*> &command_list,
                                const QString &description)
    : QtCommand(Command, description, false), m_command_list(command_list)
{
}

QtMultiCommand::~QtMultiCommand()
{
    while (!m_command_list.isEmpty())
        delete m_command_list.takeLast();
}

void QtMultiCommand::append(QtCommand *command)
{
    m_command_list.append(command);
}

QtCommand *QtMultiCommand::command(int i) const
{
    return m_command_list.at(i);
}

int QtMultiCommand::count() const
{
    return m_command_list.size();
}

void QtMultiCommand::redo()
{
    for (int i = 0; i < m_command_list.size(); ++i)
        m_command_list.at(i)->redo();
}

void QtMultiCommand::undo()
{
    for (int i = m_command_list.size() - 1; i >= 0; --i)
        m_command_list.at(i)->undo();
}

/*!
    \fn void QtCommand::redo()

    This virtual function must be reimplemented by subclasses to apply
    changes.

    \sa undo()
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

    For an overview of the Qt Undo/Redo framework, see the
    \link overview.html overview\endlink.

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
    can be undone and redone in one go. For example, in a text editor,
    when the user types in a character, a new action is created, which
    inserts that character into the document at the cursor position.
    However, it is more convenient for the user to be able to undo or
    redo typing in whole words, sentences, and paragraphs. Command
    compression allows these single-character commands to be merged
    into a single command which inserts or deletes chunks of text. See
    push() for more information on command compression.

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

    A certain state of the edited object may be marked as "clean",
    using setClean(). This function is usually called whenever the
    edited object is saved.  QtUndoStack emits the cleanChanged()
    signal whenever the edited object enters or leaves the clean
    state.

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

    m_clean_command = 0; // the initial empty stack is clean
    m_have_clean_command = true;
}

/*!
    \internal
*/

QtCommand *QtUndoStack::commandAt(CommandIter it) const
{
    if (it == -1)
        return 0;
    return at(it);
}

/*!
    Returns true if the edited object is in a clean state; otherwise
    returns false. The edited object is in a clean state if setClean()
    was previously called and the state of the edited object at the
    time of the call was the same as it is now.

    More precisely, the edited object is in a clean state if the
    current QtUndoStack command is the same one as it was at the time
    of the last call to setClean().

    \sa setClean() cleanChanged()
*/
bool QtUndoStack::isClean() const
{
    return m_have_clean_command
                && m_clean_command == commandAt(m_current_iter);
}

/*!
    \fn void QtUndoStack::cleanChanged(bool clean)

    This signal is emitted whenever the edited object enters or leaves the
    clean state. If \a clean is true, the edited object is currently clean;
    otherwise it is currently not clean.

    \sa isClean() setClean()
*/


/*!
    Marks the state of the edited object as clean. This function is
    usually called whenever the edited object is saved. The
    cleanChanged() signal is emited whenever the edited object enters
    or leaves the clean state.

    \sa isClean() cleanChanged()
*/

void QtUndoStack::setClean()
{
    bool old_clean = isClean();

    m_have_clean_command = true;
    m_clean_command = commandAt(m_current_iter);

    if (old_clean != isClean())
            emit cleanChanged(isClean());
}

class QtUndoState
{
public:
    bool can_undo, can_redo, clean;
    QString undo_description, redo_description;
};

/*!
    \internal
*/
void QtUndoStack::beforeChange(QtUndoState &state)
{
    state.clean = isClean();
    state.can_undo = canUndo();
    state.can_redo = canRedo();
    state.undo_description = undoDescription();
    state.redo_description = redoDescription();
}

/*!
    \internal
*/
void QtUndoStack::afterChange(const QtUndoState &state)
{
    if (state.can_undo != canUndo())
        emit canUndoChanged(canUndo());
    if (state.can_redo != canRedo())
        emit canRedoChanged(canRedo());
    if (state.undo_description != undoDescription())
        emit undoDescriptionChanged(undoDescription());
    if (state.redo_description != redoDescription())
        emit redoDescriptionChanged(redoDescription());
    if (state.clean != isClean())
        emit cleanChanged(isClean());
    QtUndoManager::manager()->updateActions();
}

/*!
    \internal
*/
QtUndoStack::CommandIter QtUndoStack::findMacroBegin(CommandIter it) const
{
    int nest = 1;

    QtCommand *command = commandAt(it);
    Q_ASSERT(command != 0 && command->isMacroEnd());
    do {
        --it;

        command = commandAt(it);
        Q_ASSERT(command != 0);

        if (command->isMacroBegin())
            --nest;
        else if (command->isMacroEnd())
            ++nest;
        Q_ASSERT(nest >= 0);
    } while (nest > 0 || !command->isMacroBegin());

    return it;
}

/*!
    \internal
*/
QtUndoStack::CommandIter QtUndoStack::findMacroEnd(CommandIter it) const
{
    int nest = 1;

    QtCommand *command = commandAt(it);
    Q_ASSERT(command != 0 && command->isMacroBegin());
    do {
        ++it;

        command = commandAt(it);
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
    Pushes the \a command onto this stack or merges it with the
    current command, and calls the \a{command}'s QtCommand::redo()
    function to apply it.

    If the current command is not topmost on the stack, all commands
    above it are deleted.

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
    QtUndoState state;
    beforeChange(state);

    command->redo();

    // If the current command on the stack is not last, we delete all
    // commands that follow it before adding the new command.
    while (m_current_iter != size() - 1) {
        if (m_have_clean_command
                    && commandAt(m_current_iter) == m_clean_command) {
            m_have_clean_command = false;
            m_clean_command = 0;
        }
        delete takeLast();
    }

    switch (command->type()) {

        case QtCommand::Command: {
            // Either merge the new command with the current command, or append it to the
            // stack.
            QtCommand *current = commandAt(m_current_iter);
            if (command->canMerge()
                    && current != 0
                    && current->metaObject() == command->metaObject()
                    && (!m_have_clean_command || m_clean_command != current)
                    && current->mergeMeWith(command))
                delete command;
            else
                append(command);

            break;
        }

        case QtCommand::MacroBegin:
            append(command);
            ++m_macro_nest;
            break;

        case QtCommand::MacroEnd:
            if (m_macro_nest == 0) {
                qWarning("QtUndoStack::push(): MacroEnd without MacroBegin");
                        break;
            }

            append(command);
            --m_macro_nest;

            // Set the description to the corresponding MacroBegin's description
            CommandIter it = size() - 1;
            it = findMacroBegin(it); // I've just pushed the MacroEnd
            Q_ASSERT(it != -1);
            command->setDescription(commandAt(it)->description());

            break;
    }

    m_current_iter = size() - 1;

    if (command->type() != QtCommand::MacroBegin && m_macro_nest == 0) {
        ++m_num_commands;
        emit commandExecuted();
    }

    afterChange(state);
}

/*!
    \fn bool QtUndoStack::canUndo() const

    Returns true if a command is available for undo; otherwise returns
    false. Undo is not possible if the stack is empty or if the bottom
    command on the stack has already been undone.

    \sa undo() canRedo()
*/

bool QtUndoStack::canUndo() const
{
    if (isEmpty())
        return false;

    if (m_macro_nest > 0)
        return false;

    if (m_current_iter == -1)
        return false;

    return true;
}

/*!
    \fn bool QtUndoStack::canRedo() const

    Returns true if a command is available for redo; otherwise returns
    false. Redo is not possible if the stack is empty or if the top
    command on the stack has already been redone.

    \sa redo() canUndo()
*/

bool QtUndoStack::canRedo() const
{
    if (m_macro_nest > 0)
        return false;

    if (m_current_iter == size() - 1)
        return false;

    return true;
}


/*!
    \internal
*/
void QtUndoStack::undoMacro()
{
    Q_ASSERT(m_macro_nest == 0);
    Q_ASSERT(m_current_iter != -1);
    Q_ASSERT(commandAt(m_current_iter)->type() == QtCommand::MacroEnd);

    int nest = 1;

    QtCommand *command = 0;
    do {
        --m_current_iter;

        command = commandAt(m_current_iter);
        Q_ASSERT(command != 0);

        if (command->isMacroBegin())
            --nest;
        else if (command->isMacroEnd())
            ++nest;
        Q_ASSERT(nest >= 0);

        command->undo();
    } while (nest > 0 || !command->isMacroBegin());
}

/*!
    \internal
*/
void QtUndoStack::redoMacro()
{
    Q_ASSERT(m_macro_nest == 0);
    Q_ASSERT(m_current_iter != -1);
    Q_ASSERT(commandAt(m_current_iter)->type() == QtCommand::MacroBegin);

    int nest = 1;

    QtCommand *command = 0;
    do {
        ++m_current_iter;

        command = commandAt(m_current_iter);
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
    undo()\endlink function and moves the current position one command
    down the stack.

    If the current command's \link QtCommand::type() type()\endlink is
    \c MacroEnd, traverses the stack downwards calling each command's
    \link QtCommand::undo() undo()\endlink, until a command of type \c
    MacroBegin is found. The current position is then set to one
    command below the macro begin marker.

    This process is repeated \a count times. \a count defaults to 1.

    \sa push() redo() canUndo()
*/

void QtUndoStack::undo(int count)
{
    QtUndoState state;
    beforeChange(state);

    int i = 0;
    for (; i < count; ++i) {
        if (!canUndo()) {
            qWarning("QtUndoStack::undo(): can't undo");
            break;
        }

        QtCommand *command = commandAt(m_current_iter);
        Q_ASSERT(!command->isMacroBegin());

        if (command->isCommand())
            command->undo();
        else
            undoMacro();

        --m_current_iter;
    }

    if (i > 0) {
        afterChange(state);
        emit commandExecuted();
    }
}


/*!
    If the current command's \link QtCommand::type() type()\endlink is
    \c Command, moves the current position one command up in the stack
    and calls the new current command's \link QtCommand::redo()
    redo()\endlink.

    If the current command's \link QtCommand::type() type()\endlink is
    \c MacroBegin, traverses the stack upwards calling each command's
    \link QtCommand::redo() redo()\endlink, until a command of type \c
    MacroEnd is found. The current position remains at this command.

    This process is repeated \a count times. \a count defaults to 1.

    \sa push() undo() canRedo()
*/

void QtUndoStack::redo(int count)
{
    QtUndoState state;
    beforeChange(state);

    int i = 0;
    for (; i < count; ++i) {
        if (!canRedo()) {
            qWarning("QtUndoStack::redo(): can't redo");
            break;
        }

        ++m_current_iter;

        QtCommand *command = commandAt(m_current_iter);
        Q_ASSERT(!command->isMacroEnd());

        if (command->isCommand())
            command->redo();
        else
            redoMacro();
    }

    if (i > 0) {
        afterChange(state);
        emit commandExecuted();
    }
}

/*!
    Returns the \link QtCommand::description() description()\endlink
    of the current command on the stack, or an empty string if there is
    no current command.

    \sa QtUndoManager::undoDescription() QtCommand::description() redoDescription()
*/

QString QtUndoStack::undoDescription() const
{
    if (canUndo())
        return commandAt(m_current_iter)->description();
    else
        return QString();
}

/*!
    Returns the \link QtCommand::description() description()\endlink for
    the command preceding the current command on the stack, or an
    empty string if the current command is at the top.

    \sa QtUndoManager::redoDescription() QtCommand::description() undoDescription()
*/

QString QtUndoStack::redoDescription() const
{
    if (canRedo())
        return commandAt(m_current_iter + 1)->description();
    else
        return QString();
}

void QtUndoStack::setCurrent()
{
    QtUndoManager::manager()->setCurrentStack(this);
}

/*!
    Clears all the commands on this undo stack.

    \sa push()
*/

void QtUndoStack::clear()
{
    QtUndoState state;
    beforeChange(state);

    while (!isEmpty())
        delete takeLast();

    m_macro_nest = 0;
    m_num_commands = 0;
    m_have_clean_command = true;
    m_clean_command = 0;

    afterChange(state);
}

/*!
    Returns a list of descriptions of all the commands up to the
    current command in this stack.

    \sa redoList()
*/

QStringList QtUndoStack::undoList() const
{
    QStringList result;

    if (m_macro_nest > 0)
        return result;

    if (m_current_iter == -1)
        return result;

    for (int it = 0; it <= m_current_iter; ++it) {
        QtCommand *command = commandAt(it);
        result.append(command->description());

        Q_ASSERT(!command->isMacroEnd());
        if (command->isMacroBegin())
            it = findMacroEnd(it);
    }

    return result;
}

/*!
    Returns a list of descriptions of all the commands preceding the
    current command in this stack.

    \sa undoList()
*/

QStringList QtUndoStack::redoList() const
{
    QStringList result;

    if (m_macro_nest > 0)
        return result;

    for (int it = m_current_iter + 1; it < size(); ++it) {
        QtCommand *command = commandAt(it);
        result.append(command->description());

        Q_ASSERT(!command->isMacroEnd());
        if (command->isMacroBegin())
            it = findMacroEnd(it);
    }

    return result;
}

/*!
    Creates a QAction object connected to the QtUndoStack's undo()
    slot. The \a parent becomes the owner and parent of the QAction.
    This is significant, since any accelerators that are assigned to
    the QAction will only work within the \a parent.

    Unlike QtUndoManager::createUndoAction(), the returned QAction is
    connected directly to this stack's undo() slot. This is useful if
    you want each of your target windows to have it's own undo button.

    The returned QAction will keep its text property in sync with
    undoDescription() and disable itself whenever no commands are
    available for undo.

    If the application's default QMimeSourceFactory contains a pixmap
    called "undo" or "undo.png", this pixmap is assigned to the QAction.

    \sa undo() undoDescription() createRedoAction()
*/

QAction *QtUndoStack::createUndoAction(QObject *parent) const
{
    UndoRedoAction *undo_action = new UndoRedoAction(parent);
    connect(undo_action, SIGNAL(triggered()), this, SLOT(undo()));
    connect(this, SIGNAL(undoDescriptionChanged(QString)),
                        undo_action, SLOT(setTextSlot(QString)));
    connect(this, SIGNAL(canUndoChanged(bool)),
                        undo_action, SLOT(setEnabled(bool)));

    undo_action->setEnabled(canUndo());
    undo_action->setText(undoDescription());

    return undo_action;
}

/*!
    Creates a QAction object connected to the QtUndoStack's redo()
    slot. The \a parent becomes the owner and parent of the QAction.
    This is significant, since any accelerators that are assigned to
    the QAction will only work within the \a parent.

    Unlike QtUndoManager::createRedoAction(), the returned QAction is
    connected directly to this stack's redo() slot. This is useful if
    you want each of your target windows to have it's own redo button.

    The returned QAction will keep its text property in sync with
    redoDescription() and disable itself whenever no commands are
    available for redo.

    If the application's default QMimeSourceFactory contains a pixmap
    called "redo" or "redo.png", this pixmap is assigned to the QAction.

    \sa redo() redoDescription() createUndoAction()
*/

QAction *QtUndoStack::createRedoAction(QObject *parent) const
{
    UndoRedoAction *redo_action = new UndoRedoAction(parent);
    connect(redo_action, SIGNAL(triggered()), this, SLOT(redo()));
    connect(this, SIGNAL(redoDescriptionChanged(QString)),
                        redo_action, SLOT(setTextSlot(QString)));
    connect(this, SIGNAL(canRedoChanged(bool)),
                        redo_action, SLOT(setEnabled(bool)));

    redo_action->setEnabled(canRedo());
    redo_action->setText(redoDescription());

    return redo_action;
}

/*!
    \fn void QtUndoStack::commandExecuted()

    This signal is emitted whenever a QtCommand on the stack is undone
    or redone. When macro commands are undone or redone, this signal is
    emitted only once, even though the macro may contain more than one
    command.

    \sa redo() undo()
*/

/*!
    \fn void QtUndoStack::undoDescriptionChanged(const QString &newDescription)

    This signal is emitted whenever the undo description for this QtUndoStack changes.
    \a newDescription is the new undo description. It is useful when
    you want to trigger undo using a custom widget, rather than
    using the QAction returned by createUndoAction().

    \sa undoDescription() canUndoChanged() redoDescriptionChanged()
*/

/*!
    \fn void QtUndoStack::redoDescriptionChanged(const QString &newDescription)

    This signal is emitted whenever the redo description for this QtUndoStack changes.
    \a newDescription is the new redo description. It is useful when
    you want to trigger undo using a custom widget, rather than
    using the QAction returned by createRedoAction().

    \sa redoDescription() canRedoChanged() undoDescriptionChanged()
*/

/*!
    \fn void QtUndoStack::canUndoChanged(bool enabled)

    This signal is emitted whenever the state reported by canUndo()
    changes. \a enabled is the new state.

    This function is useful if you want to trigger undo with a custom
    widget, rather than the QAction returned by createUndoAction().

    \sa canUndo() undoDescriptionChanged() canRedoChanged()
*/

/*!
    \fn void QtUndoStack::canRedoChanged(bool enabled)

    This signal is emitted whenever the state reported by canRedo()
    changes. \a enabled is the new state.

    This function is useful if you want to trigger redo with a custom
    widget, rather than the QAction returned by createRedoAction().

    \sa canRedo() redoDescriptionChanged() canUndoChanged()
*/


/*!
    \class QtUndoManager

    \brief The QtUndoManager class manages command stacks in an
    undo/redo framework based on the Command design pattern.

    For an overview of the Qt Undo/Redo framework, see the
    \link overview.html overview\endlink.

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
    QtUndoManager also provides the functions createUndoAction() and
    createRedoAction() for creating QAction objects that trigger undo
    and redo. These QActions have the additional benefit of keeping
    their text properties in sync with undoDescription() and
    redoDescription(), as well as disabling themselves whenever no
    commands are available for undo or redo.

    \code
    MainWindow::MainWindow(QWidget *parent, const char *name)
        : QMainWindow(parent, name)
    {
        ...
        QtUndoManager *manager = QtUndoManager::manager();
        QAction *undo_action = manager->createUndoAction(this);
        QAction *redo_action = manager->createRedoAction(this);
        undo_action->setAccel(QKeySequence("Ctrl+Z"));
        redo_action->setAccel(QKeySequence("Shift+Ctrl+Z"));

        QToolBar *toolbar = new QToolBar(this);
        undo_action->addTo(toolbar);
        redo_action->addTo(toolbar);

        QPopupMenu *editmenu = new QPopupMenu(this);
        undo_action->addTo(editmenu);
        redo_action->addTo(editmenu);
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
    parent chain, the QAction objects created using createUndoAction() and
    createRedoAction() are disabled.

    \img qtundo-menu.png
    <p>
    \img qtundo-toolbar.png

    \sa QtCommand QtUndoStack
*/

/*! \internal */

QtUndoManager::QtUndoManager()
{
    m_current_stack = 0;
    m_can_undo = false;
    m_can_redo = false;

    updateActions();
}

/*!
    Creates a QAction object connected to the QtUndoManager's undo()
    slot. The \a parent becomes the owner and parent of the QAction.
    This is significant, since any accelerators that are assigned to
    the QAction will only work within the \a parent.

    The returned QAction will keep its text property in sync with
    undoDescription() and disable itself whenever no commands are
    available for undo.

    If the application's default QMimeSourceFactory contains a pixmap
    called "undo" or "undo.png", this pixmap is assigned to the QAction.

    \sa undo() undoDescription() createRedoAction()
*/

QAction *QtUndoManager::createUndoAction(QObject *parent) const
{
    UndoRedoAction *undo_action = new UndoRedoAction(parent);
    connect(undo_action, SIGNAL(triggered()), this, SLOT(undo()));
    connect(this, SIGNAL(undoDescriptionChanged(QString)),
                        undo_action, SLOT(setTextSlot(QString)));
    connect(this, SIGNAL(canUndoChanged(bool)),
                        undo_action, SLOT(setEnabled(bool)));

    undo_action->setEnabled(m_can_undo);
    undo_action->setText(m_undo_description);

    return undo_action;
}

/*!
    Creates a QAction object connected to the QtUndoManager's redo()
    slot. The \a parent becomes the owner and parent of the QAction.
    This is significant, since any accelerators that are assigned to
    the QAction will only work within the \a parent.

    The returned QAction will keep its text property in sync with
    redoDescription() and disable itself whenever no commands are
    available for redo.

    If the application's default QMimeSourceFactory contains a pixmap
    called "redo" or "redo.png", this pixmap is assigned to the QAction.

    \sa redo() redoDescription() createUndoAction()
*/

QAction *QtUndoManager::createRedoAction(QObject *parent) const
{
    UndoRedoAction *redo_action = new UndoRedoAction(parent);
    connect(redo_action, SIGNAL(triggered()), this, SLOT(redo()));
    connect(this, SIGNAL(redoDescriptionChanged(QString)),
                        redo_action, SLOT(setTextSlot(QString)));
    connect(this, SIGNAL(canRedoChanged(bool)),
                        redo_action, SLOT(setEnabled(bool)));

    redo_action->setEnabled(m_can_redo);
    redo_action->setText(m_redo_description);

    return redo_action;
}

/*! \internal */

void QtUndoManager::updateActions()
{
    QtUndoStack *stack = currentStack();

    bool undo_enabled = stack != 0 && stack->canUndo();
    QString undo_description = tr("Undo");
    if (undo_enabled)
        undo_description += " " + stack->undoDescription();

    if (undo_enabled != m_can_undo) {
        m_can_undo = undo_enabled;
        emit canUndoChanged(undo_enabled);
    }

    if (undo_description != m_undo_description) {
        m_undo_description = undo_description;
        emit undoDescriptionChanged(undo_description);
    }

    bool redo_enabled = stack != 0 && stack->canRedo();
    QString redo_description = tr("Redo");
    if (redo_enabled)
        redo_description += " " + stack->redoDescription();

    if (redo_enabled != m_can_redo) {
        m_can_redo = redo_enabled;
        emit canRedoChanged(redo_enabled);
    }
    if (redo_description != m_redo_description) {
        m_redo_description = redo_description;
        emit redoDescriptionChanged(redo_description);
    }
}

/*!
    Returns true if a command is available for redo; otherwise returns
    false. Redo is not possible if the widget with the keyboard focus
    has no targets in its parent chain, or if a target is found but
    the associated stack is empty, or if the last command on the stack
    has already been undone.

    The QAction returned by createUndoAction() disables itself
    whenever canUndo() is false.

    \sa undo() canRedo()
*/

bool QtUndoManager::canUndo() const
{
    return m_can_undo;
}

/*!
    Returns true if a command is available for undo; otherwise returns
    false. Undo is not possible if the widget with the keyboard focus
    has no targets in its parent chain, or if a target is found but
    the associated stack is empty, or if the first command on the
    stack has already been redone.

    A QAction returned by createRedoAction() disables itself
    whenever canRedo() is false.

    \sa redo() canUndo()
*/

bool QtUndoManager::canRedo() const
{
    return m_can_redo;
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
        else
            ++it;
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

    \a count is the number of commands that should be undone. It defaults to 1.

    \sa redo() canUndo()
*/

void QtUndoManager::undo(int count)
{
    QtUndoStack *stack = currentStack();

    if (stack == 0 || !stack->canUndo()) {
        qWarning("QtUndoManager::undo(): can't undo");
        return;
    }

    stack->undo(count);
}

/*!
    Directs the redo request to the appropriate QtUndoStack. The stack
    is chosen by finding the widget with the keyboard focus and
    searching its parent chain for a target. If a target is found,
    QtUndoStack::redo() is called on the associated stack. If no such
    target is found, this function does nothing.

    \a count is the number of commands that should be redone. It defaults to 1.

    \sa undo() canRedo()
*/

void QtUndoManager::redo(int count)
{
    QtUndoStack *stack = currentStack();

    if (stack == 0 || !stack->canRedo()) {
        qWarning("QtUndoManager::redo(): can't redo");
        return;
    }

    stack->redo(count);
}

QtUndoManager *QtUndoManager::m_manager = 0;
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

void QtUndoManager::setCurrentStack(QtUndoStack *stack)
{
    m_current_stack = stack;
    updateActions();
}

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
    disconnect(*it, 0, this, 0);
    m_stack_map.erase(it);
}

/*!
    Associates the object \a obj with the given \a stack, adding \a
    obj the \a stack's targets. undo() and redo() requests will be
    directed to \a stack, whenever \a obj or one of its children has
    the keyboard focus.

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
    connect(stack, SIGNAL(commandExecuted()), this, SIGNAL(changed()));

    updateActions();
}

/*!
    Returns the maximum size that any undo stack can grow to. A size
    of 0 means that the stacks can grow indefinitely.

    \sa setUndoLimit()
*/

uint QtUndoManager::undoLimit() const
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

QtUndoStack *QtUndoManager::currentStack() const
{
    return m_current_stack;
}

/*!
    \fn void QtUndoManager::changed()
    \internal
*/

/*!
    Returns the current undo description.

    The undo description is a string that describes what effects
    calling QtUndoManager::undo() will have on the edited object.

    It contains the text returned by QtCommand::description() for the
    current command on the QtUndoStack associated with the target
    widget that contains the keyboard focus.

    The QAction returned by createUndoAction() keeps its text property
    in sync with the undo description. This function is useful if you
    want to trigger undo with a custom widget, rather than with this
    QAction.

    \sa undoDescriptionChanged() createUndoAction() QtCommand::description() QtUndoStack::undoDescription()
*/

QString QtUndoManager::undoDescription() const
{
    return m_undo_description;
}

/*!
    Returns the current redo description.

    The redo description is a string that describes what effects
    calling QtUndoManager::redo() will have on the edited object.

    It contains the text returned by QtCommand::description() for the
    command preceding the current command on the QtUndoStack
    associated with the target widget that contains the keyboard
    focus.

    The QAction returned by createRedoAction() keeps its text property
    in sync with the redo description. This function is useful if you
    want to trigger redo with a custom widget, rather than with this
    QAction.

    \sa redoDescriptionChanged() createRedoAction() QtCommand::description() QtUndoStack::redoDescription()
*/

QString QtUndoManager::redoDescription() const
{
    return m_redo_description;
}

/*!
    \fn void QtUndoManager::redoDescriptionChanged(const QString &newDescription)

    This signal is emitted whenever the redo description for the QtUndoStack associated
    with the target widget that contains the keyboard focus changes.
    \a newDescription is the new redo description. It is useful when
    you want to trigger redo using a custom widget, rather than
    using the QAction returned by createRedoAction().

    \sa redoDescription() canRedoChanged() undoDescriptionChanged()
*/

/*!
    \fn void QtUndoManager::undoDescriptionChanged(const QString &newDescription)

    This signal is emitted whenever the undo description for the QtUndoStack associated
    with the target widget that contains the keyboard focus changes.
    \a newDescription is the new undo description. It is useful when
    you want to trigger undo using a custom widget, rather than
    using the QAction returned by createUndoAction().

    \sa undoDescription() canUndoChanged() redoDescriptionChanged()
*/

/*!
    \fn void QtUndoManager::canUndoChanged(bool enabled)

    This signal is emitted whenever the state reported by canUndo()
    changes. \a enabled is the new state.

    This function is useful if you want to trigger undo with a custom
    widget, rather than the QAction returned by createUndoAction().

    \sa canUndo() undoDescriptionChanged() canRedoChanged()
*/

/*!
    \fn void QtUndoManager::canRedoChanged(bool enabled)

    This signal is emitted whenever the state reported by canRedo()
    changes. \a enabled is the new state.

    This function is useful if you want to trigger redo with a custom
    widget, rather than the QAction returned by createRedoAction().

    \sa canRedo() redoDescriptionChanged() canUndoChanged()
*/

/*!
    Returns a list of descriptions of all the commands up to the
    current command in the stack associated with the currently focused
    target. If no target has focus, returns an empty list.

    \sa redoList()
*/

QStringList QtUndoManager::undoList() const
{
    QtUndoStack *stack = currentStack();
    if (stack == 0)
        return QStringList();

    return stack->undoList();
}

/*!
    Returns a list of descriptions of all the commands preceding the
    current command in the stack associated with the currently focused
    target. If no target has focus, returns an empty list.

    \sa undoList()
*/

QStringList QtUndoManager::redoList() const
{
    QtUndoStack *stack = currentStack();
    if (stack == 0)
        return QStringList();

    return stack->redoList();
}

/*!
    \class QtUndoListBox

    \brief The QtUndoListBox class is a QListBox which displays the
    commands on the QtUndoStack associated with the focused target.

    QtUndoListBox keeps track of changes in the stack and focus in the
    application, and updates itself accordingly. Selecting a command
    causes undo or redo until the selected command is current. Hence
    the history of changes can be undone or redone by traversing the
    list.

    \img qtundo-list.png
*/

/*!
    Constructs a QtUndoListBox. The \a parent and \a name are passed
    on to the QListBox constructor.
*/

QtUndoListModel::QtUndoListModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_undoIndex(0)
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

    return m_items.count() + 1;
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
    case Qt::DisplayRole:
        if (index.row() < m_items.size())
            return m_items.at(index.row());
        else if (index.row() == m_items.size())
            return QString();
        else
            return QVariant();
    default:
        return QVariant();
    }
}

QtUndoListView::QtUndoListView(QWidget *parent)
    : QListView(parent)
{
    QtUndoListModel *m = new QtUndoListModel(this);
    setModel(m);
    setSelectionMode(SingleSelection);
    setCurrentIndex(m->index(0, 0, QModelIndex()));
    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(undoOrRedo()));
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

void QtUndoListView::reset()
{
    QListView::reset();
    QtUndoListModel *m = qobject_cast<QtUndoListModel*>(model());
    setCurrentIndex(m->index(m->undoIndex(), 0, QModelIndex()));
}

#include "qtundo.moc"

