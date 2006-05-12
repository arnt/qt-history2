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

#include <QtCore/qdebug.h>
#include "qundostack.h"
#include "qundogroup.h"
#include "qundostack_p.h"

/*!
    \class QCommand
    \brief The QCommand class is the base class of all commands stored on a QUndoStack.

    For an overview of the Qt's undo framework, see the
    \link qundo.html overview\endlink.

    A QCommand represents a single editing action on a document, for example,
    inserting or deleting a block of text in a text editor. QCommand can apply
    a change to the document with redo() and undo the change with undo(). The
    implementations for these functions must be provided in a derived class.

    \code
    class AppendText : public QCommand
    {
    public:
        AppendText(QString *doc, const QString &text)
            : m_document(doc), m_text(text) { setText("append text"); }
        virtual void undo()
            { m_document->chop(m_text.length()); }
        virtual void redo()
            { m_document->append(m_text); }
    private:
        QString *m_document;
        QString m_text;
    };
    \endcode

    A QCommand has an associated text(). This is a short string
    describing what the command does. It is used to update the text
    properties of the stack's undo and redo actions, see QUndoStack::createUndoAction()
    and QUndoStack::createRedoAction().

    To support command compression, QCommand has an id() and the virtual function
    mergeWith(). These functions are used by QUndoStack::push().

    To support command macros, a QCommand object can have any number of child
    commands. Undoing or redoing the parent command will cause the child
    commands to be undone or redone. A command can be assigned
    to a parent explicitly in the constructor.

    \code
    QCommand *insertRed = new QCommand(); // an empty command
    insertRed->setText("insert red text");

    new InsertText(document, idx, text, insertRed); // becomes child of insertRed
    new SetColor(document, idx, text.length(), Qt::red, insertRed);

    stack.push(insertRed);
    \endcode

    Another way to create macros is to use the convenience functions
    QUndoStack::beginMacro() and QUndoStack::endMacro().
*/

/*!
    Constructs a QCommand object with parent \a parent and text \a text.

    If \a parent is not 0, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~QCommand()
*/

QCommand::QCommand(const QString &text, QCommand *parent)
{
    d = new QCommandPrivate;
    if (parent != 0)
        parent->d->child_list.append(this);
    d->text = text;
}

/*!
    Constructs a QCommand object with parent \a parent.

    If \a parent is not 0, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~QCommand()
*/

QCommand::QCommand(QCommand *parent)
{
    d = new QCommandPrivate;
    if (parent != 0)
        parent->d->child_list.append(this);
}

/*!
    Destroys the QCommand object and all child commands. If the command was in a QUndoGroup,
    removes it grom the group.

    \sa QCommand()
*/

QCommand::~QCommand()
{
    qDeleteAll(d->child_list);
    delete d;
}

/*!
    Returns the id of this command.

    A command id is used in command compression. It must be an integer unique to
    this command's class, or -1 if the command doesn't support compression.

    If the command supports compression this function must be overriden in the
    derived class to return the correct id. The base implementation returns -1.

    QUndoStack will only try to merge two commands if they have the same id, and
    the id is not -1.

    \sa mergeWith() QUndoStack::push()
*/

int QCommand::id() const
{
    return -1;
}

/*!
    Attempts to merge this command with \a cmd. Returns true on success; otherwise
    returns false. If this function returns true, calling this command's redo() must
    have the same effect as redoing this command and \a cmd.
    Simmilarly, calling this command's undo() must have the same effect as undoing
    \a cmd and this command.

    QUndoStack will only try to merge two commands if they have the same id, and the id
    is not -1.

    The default implementation returns false.

\code
    bool AppendText::mergeWith(const QCommand *other)
    {
        if (other->id() != id()) // make sure other is also an AppendText command
            return false;
        m_text += static_cast<const AppendText*>(other)->m_text;
        return true;
    }
\endcode

    \sa id() QUndoStack::push()
*/

bool QCommand::mergeWith(const QCommand *cmd)
{
    Q_UNUSED(cmd);
    return false;
}

/*!
    Applies changes to the document. This function must be implemented in
    the derived class.

    The default implementation calls redo() on all child commands.

    \sa undo()
*/

void QCommand::redo()
{
    for (int i = 0; i < d->child_list.size(); ++i)
        d->child_list.at(i)->redo();
}

/*!
    Reverts changes to the document. After undo() is called, the state of
    the document should be the same as before redo() was called. This function must
    be implemented in the derived class.

    The default implementation calls undo() on all child commands in reverse order.

    \sa redo()
*/

void QCommand::undo()
{
    for (int i = d->child_list.size() - 1; i >= 0; --i)
        d->child_list.at(i)->undo();
}

/*!
    Returns a short text string describing what this command does, f.ex. "insert text".
    It is used to update the text properties of the stack's undo and redo actions.

    \sa setText() QUndoStack::createUndoAction() QUndoStack::createRedoAction()
*/

QString QCommand::text() const
{
    return d->text;
}

/*!
    Sets a short text string describing what this command does to \a text.

    \sa text() QUndoStack::createUndoAction() QUndoStack::createRedoAction()
*/

void QCommand::setText(const QString &text)
{
    d->text = text;
}

/*!
    \class QUndoStack
    \brief The QUndoStack class is a stack of QCommand objects.

    For an overview of the Qt's undo framework, see the
    \link qundo.html overview\endlink.

    New commands are pushed on the stack using push(). Commands can be
    undone and redone using undo() and redo(), or by triggering the
    actions returned by createUndoAction() and createRedoAction().

    QUndoStack supports the concept of a clean state. When the
    document is saved to disk, the stack can be marked as clean using
    setClean(). Whenever the stack returns to this state through undo/redo
    of commands, it emits the signal cleanChanged(). This signal is also
    emitted when the stack leaves the clean state. This signal is usually
    used to enable and disable the save actions in the application, and
    to update the document's title to reflect that it contains unsaved changes.

    QUndoStack supports command compression. This is useful when several
    commands can be compressed into a single command, which can be undone
    and redone in one go. When, f.ex., a user types a character in a text
    editor, a new command is created. This command inserts the
    character into the document at the cursor position. However, it is
    more convenient for the user to be able to undo or redo typing of
    whole words, sentences, or paragraphs. Command compression allows
    these single-character commands to be merged into a single command
    which inserts or deletes sections of text. For more information, see
    QCommand::mergeWith() and push().

    QUndoStack supports command macros. A command macro is a sequence of
    commands, all of which are undone and redone in one go. Command
    macros are created by giving a command a list of child commands.
    Undoing or redoing the parent command will cause the child commands to
    be undone or redone. Command macros may be created explicitly
    by specifying a parent in QCommand::QCommand(), or by using the convenience
    functions beginMacro() and endMacro().

    QUndoStack provides convenient undo and redo QAction objects, which
    can be inserted into a menu or a toolbar. When commands are undone or
    redone, QUndoStack updates the text properties of these actions
    to reflect what change they will trigger. The actions are also disabled
    when no command is available for undo or redo. These actions
    are returned by QUndoStack::createUndoAction() and QUndoStack::createRedoAction().

    QUndoStack keeps track of the \a current command. This is the command
    which will be executed by the next call to redo(). The index of this
    command is returned by index(). The state of the edited object can be
    rolled forward or back using setIndex(). If the top-most command on the
    stack has already been redone, index() is equal to count().
*/

QUndoAction::QUndoAction(const QString &prefix, QObject *parent)
    : QAction(parent)
{
    m_prefix = prefix;
}

void QUndoAction::setPrefixedText(const QString &text)
{
    QString s = m_prefix;
    if (!m_prefix.isEmpty() && !text.isEmpty())
        s.append(QLatin1Char(' '));
    s.append(text);
    setText(s);
}

/*! \internal
    Sets the current index to \a idx, emitting appropriate signals. If \a clean is true,
    makes \a idx the clean index as well.
*/

void QUndoStackPrivate::setIndex(int idx, bool clean)
{
    Q_Q(QUndoStack);

    bool was_clean = index == clean_index;

    if (idx != index) {
        index = idx;
        emit q->indexChanged(index);
        emit q->canUndoChanged(q->canUndo());
        emit q->undoTextChanged(q->undoText());
        emit q->canRedoChanged(q->canRedo());
        emit q->redoTextChanged(q->redoText());
    }

    if (clean)
        clean_index = index;

    bool is_clean = index == clean_index;
    if (is_clean != was_clean)
        emit q->cleanChanged(is_clean);
}

/*!
    Constructs an empty undo stack with the parent \a parent. The
    stack will initally be in the clean state. If \a parent is a
    QUndoGroup object, the stack is automatically added to the group.

    \sa ~QUndoStack() push()
*/

QUndoStack::QUndoStack(QObject *parent)
    : QObject(*(new QUndoStackPrivate), parent)
{
    if (QUndoGroup *group = qobject_cast<QUndoGroup*>(parent))
        group->addStack(this);
}

/*!
    Destroys the undo stack, deleting any commands that are on it. If the
    stack is in a QUndoGroup, the stack is automatically removed from the group.

    \sa QUndoStack()
*/

QUndoStack::~QUndoStack()
{
    Q_D(QUndoStack);
    if (d->group != 0)
        d->group->removeStack(this);
    clear();
}

/*!
    Deletes all commands and returns the stack to the clean state. Commands
    are not undone or redone, the state of the edited object remains unchanged.
    This function is usually used when the contents of the document are
    abandonned.

    \sa QUndoStack()
*/

void QUndoStack::clear()
{
    Q_D(QUndoStack);

    d->macro_stack.clear();
    qDeleteAll(d->command_list);
    d->command_list.clear();
    d->setIndex(0, true);
}

/*!
    Pushes \a cmd on the stack or merges it with the most recently executed command.
    In either case, executes \a cmd by calling its redo() function.

    If \a cmd's id is not -1, and if the id is the same as that of the
    most recently executed command, QUndoStack will attempt to merge the two
    commands by calling QCommand::mergeWith() on the most recently executed
    command. If QCommand::mergeWith() returns true, the \a cmd is deleted.

    In all other cases \a cmd is simply pushed on the stack.

    If the current command index does not point to the top of the stack - ie.
    if commands were undone before \a cmd was pushed - the current command and
    all commands above it are deleted. Hence \a cmd always ends up being the
    top-most command on the stack.

    Once a command is pushed, the stack takes ownership of it. There
    are no getters to return the command, since modifying it after it has
    been executed will almost always lead to corruption of the document's
    state.

    \sa QCommand::id() QCommand::mergeWith()
*/

void QUndoStack::push(QCommand *cmd)
{
    Q_D(QUndoStack);
    cmd->redo();

    bool macro = !d->macro_stack.isEmpty();

    QCommand *cur = 0;
    if (macro) {
        QCommand *macro_cmd = d->macro_stack.last();
        if (!macro_cmd->d->child_list.isEmpty())
            cur = macro_cmd->d->child_list.last();
    } else {
        if (d->index > 0)
            cur = d->command_list.at(d->index - 1);
        while (d->index < d->command_list.size())
            delete d->command_list.takeLast();
        if (d->clean_index > d->index)
            d->clean_index = -1; // we've deleted the clean state
    }

    bool try_merge = cur != 0
                        && cur->id() != -1
                        && cur->id() == cmd->id()
                        && (macro || d->index != d->clean_index);

    if (try_merge && cur->mergeWith(cmd)) {
        delete cmd;
        if (!macro) {
            emit indexChanged(d->index);
            emit canUndoChanged(canUndo());
            emit undoTextChanged(undoText());
            emit canRedoChanged(canRedo());
            emit redoTextChanged(redoText());
        }
    } else {
        if (macro) {
            d->macro_stack.last()->d->child_list.append(cmd);
        } else {
            d->command_list.append(cmd);
            d->setIndex(d->index + 1, false);
        }
    }
}

/*!
    Marks the stack as clean and emits cleanChanged() if the stack was
    not already clean.

    Whenever the stack returns to this state through undo/redo
    of commands, it emits the signal cleanChanged(). This signal is also
    emitted when the stack leaves the clean state.

    \sa isClean() cleanIndex()
*/

void QUndoStack::setClean()
{
    Q_D(QUndoStack);
    if (!d->macro_stack.isEmpty()) {
        qWarning() << "QUndoStack::setClean(): cannot set clean in the middle of a macro";
        return;
    }

    d->setIndex(d->index, true);
}

/*!
    If the stack is in the clean state, returns true; otherwise returns false.

    \sa setClean() cleanIndex()
*/

bool QUndoStack::isClean() const
{
    Q_D(const QUndoStack);
    if (!d->macro_stack.isEmpty())
        return false;
    return d->clean_index == d->index;
}

/*!
    Returns the clean index. This is the index at which setClean() was called.

    A stack may not have a clean index. This happens if a document is saved,
    some commands are undone, then a new command is pushed. Since
    push() deletes all the undone commands before pushing the new command, the stack
    can't return to the clean state again. In this case, this function returns -1.

    \sa isClean() setClean()
*/

int QUndoStack::cleanIndex() const
{
    Q_D(const QUndoStack);
    return d->clean_index;
}

/*!
    Undoes the command below the current command by calling QCommand::undo().
    Decrements the current command index.

    If the stack is empty, or if the bottom command on the stack has already been
    undone, this function does nothing.

    \sa redo() index()
*/

void QUndoStack::undo()
{
    Q_D(QUndoStack);
    if (d->index == 0)
        return;

    if (!d->macro_stack.isEmpty()) {
        qWarning() << "QUndoStack::undo(): cannot undo in the middle of a macro";
        return;
    }

    int idx = d->index - 1;
    d->command_list.at(idx)->undo();
    d->setIndex(idx, false);
}

/*!
    Redoes the current command by calling QCommand::redo(). Increments the current
    command index.

    If the stack is empty, or if the top command on the stack has already been
    redone, this function does nothing.

    \sa undo() index()
*/

void QUndoStack::redo()
{
    Q_D(QUndoStack);
    if (d->index == d->command_list.size())
        return;

    if (!d->macro_stack.isEmpty()) {
        qWarning() << "QUndoStack::redo(): cannot redo in the middle of a macro";
        return;
    }

    d->command_list.at(d->index)->redo();
    d->setIndex(d->index + 1, false);
}

/*!
    Returns the number of commands on the stack. Macro commands are counted as
    one command.

    \sa index() setIndex()
*/

int QUndoStack::count() const
{
    Q_D(const QUndoStack);
    return d->command_list.size();
}

/*!
    Returns the index of the current command. This is the command that will be
    executed on the next call to redo(). It is not always the top-most command
    on the stack, since a number of commands may have been undone.

    \sa undo() redo() count()
*/

int QUndoStack::index() const
{
    Q_D(const QUndoStack);
    return d->index;
}

/*!
    Repeatedly calls undo() or redo() until the the current command index reaches
    \a idx. This function can be used to roll the state of the document forwards
    of backwards. indexChanged() is emitted only once.

    \sa index() count() undo() redo()
*/

void QUndoStack::setIndex(int idx)
{
    Q_D(QUndoStack);
    if (!d->macro_stack.isEmpty()) {
        qWarning() << "QUndoStack::setIndex(): cannot set index in the middle of a macro";
        return;
    }

    if (idx < 0)
        idx = 0;
    else if (idx > d->command_list.size())
        idx = d->command_list.size();

    int i = d->index;
    while (i < idx)
        d->command_list.at(i++)->redo();
    while (i > idx)
        d->command_list.at(--i)->undo();

    d->setIndex(idx, false);
}

/*!
    Returns true if there is a command available for undo; otherwise returns false.

    This function returns false if the stack is empty, or if the bottom command
    on the stack has already been undone.

    Synonymous with index() == 0.

    \sa index() canRedo()
*/

bool QUndoStack::canUndo() const
{
    Q_D(const QUndoStack);
    if (!d->macro_stack.isEmpty())
        return false;
    return d->index > 0;
}

/*!
    Returns true if there is a command available for redo; otherwise returns false.

    This function returns false if the stack is empty or if the top command
    on the stack has already been redone.

    Synonymous with index() == count().

    \sa index() canUndo()
*/

bool QUndoStack::canRedo() const
{
    Q_D(const QUndoStack);
    if (!d->macro_stack.isEmpty())
        return false;
    return d->index < d->command_list.size();
}

/*!
    Returns the text of the command which will be undone in the next call to undo().

    \sa  QCommand::text() redoText()
*/

QString QUndoStack::undoText() const
{
    Q_D(const QUndoStack);
    if (!d->macro_stack.isEmpty())
        return QString();
    if (d->index > 0)
        return d->command_list.at(d->index - 1)->text();
    return QString();
}

/*!
    Returns the text of the command which will be redone in the next call to redo().

    \sa QCommand::text() undoText()
*/

QString QUndoStack::redoText() const
{
    Q_D(const QUndoStack);
    if (!d->macro_stack.isEmpty())
        return QString();
    if (d->index < d->command_list.size())
        return d->command_list.at(d->index)->text();
    return QString();
}

/*!
    Creates an undo QAction object with parent \a parent.

    Triggering this action will cause a call to undo(). The text of this action
    will always be the text of the command which will be undone in the next call to undo(),
    prefixed by \a prefix. If there is no command available for undo, this action will
    be disabled.

    If \a prefix is empty, the default prefix "Undo" is used.

    \sa createRedoAction() canUndo() QCommand::text()
*/

QAction *QUndoStack::createUndoAction(QObject *parent, const QString &prefix) const
{
    QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
    QUndoAction *result = new QUndoAction(pref, parent);
    result->setEnabled(canUndo());
    result->setPrefixedText(undoText());
    connect(this, SIGNAL(canUndoChanged(bool)),
            result, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(undoTextChanged(QString)),
            result, SLOT(setPrefixedText(QString)));
    connect(result, SIGNAL(triggered()), this, SLOT(undo()));
    return result;
}

/*!
    Creates an redo QAction object with parent \a parent.

    Triggering this action will cause a call to redo(). The text of this action
    will always be text of the command which will be redone in the next call to redo(),
    prefixed by \a prefix. If there is no command available for redo, this action will
    be disabled.

    If \a prefix is empty, the default prefix "Redo" is used.

    \sa createUndoAction() canRedo() QCommand::text()
*/

QAction *QUndoStack::createRedoAction(QObject *parent, const QString &prefix) const
{
    QString pref = prefix.isEmpty() ? tr("Redo") : prefix;
    QUndoAction *result = new QUndoAction(pref, parent);
    result->setEnabled(canRedo());
    result->setPrefixedText(redoText());
    connect(this, SIGNAL(canRedoChanged(bool)),
            result, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoTextChanged(QString)),
            result, SLOT(setPrefixedText(QString)));
    connect(result, SIGNAL(triggered()), this, SLOT(redo()));
    return result;
}

/*!
    Begins composition of a macro command with text \a text.

    An empty command with text \a text is pushed on the stack. Any subsequent
    commands pushed on the stack will be appended to the empty command's children,
    until endMacro() is called.

    Calls to beginMacro() and endMacro() mey be nested, but every call to beginMacro()
    must have a matching call to endMacro().

    While a macro is composed, the stack is disabled. This means that:
    \list
    \i indexChanged() and cleanChanged() are not emitted,
    \i canUndo() and canRedo() return false,
    \i calling undo() or redo() has no effect,
    \i the undo/redo actions are disabled.
    \endlist
    The stack becomes enabled and appropriate signals are emitted when endMacro() is called
    for the outermost macro.

    \code
    stack.beginMacro("insert red text");
    stack.push(new InsertText(document, idx, text));
    stack.push(new SetColor(document, idx, text.length(), Qt::red));
    stack.endMacro(); // indexChanged() is emitted
    \endcode

    This code is equivalent to:

    \code
    QCommand *insertRed = new QCommand(); // an empty command
    insertRed->setText("insert red text");

    new InsertText(document, idx, text, insertRed); // becomes child of insertRed
    new SetColor(document, idx, text.length(), Qt::red, insertRed);

    stack.push(insertRed);
    \endcode

    \sa endMacro()
*/

void QUndoStack::beginMacro(const QString &text)
{
    Q_D(QUndoStack);
    QCommand *cmd = new QCommand();
    cmd->setText(text);

    if (d->macro_stack.isEmpty()) {
        while (d->index < d->command_list.size())
            delete d->command_list.takeLast();
        if (d->clean_index > d->index)
            d->clean_index = -1; // we've deleted the clean state
        d->command_list.append(cmd);
    } else {
        d->macro_stack.last()->d->child_list.append(cmd);
    }
    d->macro_stack.append(cmd);

    if (d->macro_stack.count() == 1) {
        emit canUndoChanged(false);
        emit undoTextChanged(QString());
        emit canRedoChanged(false);
        emit redoTextChanged(QString());
    }
}

/*!
    Ends composition of a macro command.

    If this is the outermost macro in a set nested macros, this function emits
    indexChanged() once for the entire macro command.

    \sa beginMacro()
*/

void QUndoStack::endMacro()
{
    Q_D(QUndoStack);
    if (d->macro_stack.isEmpty()) {
        qWarning() << "QUndoStack::endMacro(): no matching beginMacro()";
        return;
    }

    d->macro_stack.removeLast();

    if (d->macro_stack.isEmpty())
        d->setIndex(d->index + 1, false);
}

/*!
    Returns the text of the command at index \a idx.

    \sa beginMacro()
*/

QString QUndoStack::text(int idx) const
{
    Q_D(const QUndoStack);

    if (idx < 0 || idx >= d->command_list.size())
        return QString();
    return d->command_list.at(idx)->text();
}

/*!
    \property QUndoStack::active
    \brief the active status of this stack.

    An application often has multiple undo stacks, one for each opened document. The active
    stack is the one associated with the currently active document. If the stack belongs
    to a QUndoGroup, calls to QUndoGroup::undo() or QUndoGroup::redo() will be forwarded
    to this stack when it is active. If the QUndoGroup is watched by a QUndoView, the view
    will display the contents of this stack when it is active. If the stack does not belong to
    a QUndoGroup, making it active has no effect.

    It is the programmer's responsibility to specify which stack is active by
    calling setActive(), usually when the associated document window receives focus.

    \sa QUndoGroup
*/

void QUndoStack::setActive(bool active)
{
    Q_D(QUndoStack);

    if (d->group != 0) {
        if (active)
            d->group->setActiveStack(this);
        else if (d->group->activeStack() == this)
            d->group->setActiveStack(0);
    }
}

bool QUndoStack::isActive() const
{
    Q_D(const QUndoStack);
    return d->group == 0 || d->group->activeStack() == this;
}

/*!
    \fn void QUndoStack::indexChanged(int idx)

    This signal is emitted whenever a command modifies the state of the document.
    This happens when a command is undone or redone. When a macro
    command is undone or redone, or setIndex() is called, this signal
    is emitted only once.

    \a idx specifies the index of the current command, ie. the command which will be
    executed on the next call to redo().

    \sa index() setIndex()
*/

/*!
    \fn void QUndoStack::cleanChanged(bool clean)

    This signal is emitted whenever the stack enters or leaves the clean state. \a clean
    specifies the new state.

    \sa isClean() setClean()
*/

/*!
    \fn void QUndoStack::undoTextChanged(const QString &undoText)

    This signal is emitted whenever the value of undoText() changes. It is
    used to update the text property of the undo action returned by createUndoAction().
    \a undoText specifies the new text.
*/

/*!
    \fn void QUndoStack::canUndoChanged(bool canUndo)

    This signal is emitted whenever the value of canUndo() changes. It is
    used to enable or disable the undo action returned by createUndoAction().
    \a canUndo specifies the new value.
*/

/*!
    \fn void QUndoStack::redoTextChanged(const QString &redoText)

    This signal is emitted whenever the value of redoText() changes. It is
    used to update the text property of the redo action returned by createRedoAction().
    \a redoText specifies the new text.
*/

/*!
    \fn void QUndoStack::canRedoChanged(bool canUndo)

    This signal is emitted whenever the value of canRedo() changes. It is
    used to enable or disable the redo action returned by createRedoAction().
    \a canUndo specifies the new value.
*/

