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

#ifndef QUNDOSTACK_H
#define QUNDOSTACK_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QAction;
class QUndoCommandPrivate;
class QUndoStackPrivate;

class Q_GUI_EXPORT QUndoCommand
{
    QUndoCommandPrivate *d;

public:
    explicit QUndoCommand(QUndoCommand *parent = 0);
    explicit QUndoCommand(const QString &text, QUndoCommand *parent = 0);
    virtual ~QUndoCommand();

    virtual void undo();
    virtual void redo();

    QString text() const;
    void setText(const QString &text);

    virtual int id() const;
    virtual bool mergeWith(const QUndoCommand *other);

private:
    Q_DISABLE_COPY(QUndoCommand)
    friend class QUndoStack;
};

class Q_GUI_EXPORT QUndoStack : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUndoStack)
    Q_PROPERTY(bool active READ isActive WRITE setActive)

public:
    explicit QUndoStack(QObject *parent = 0);
    ~QUndoStack();
    void clear();

    void push(QUndoCommand *cmd);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    int count() const;
    int index() const;
    QString text(int idx) const;

    QAction *createUndoAction(QObject *parent,
                                const QString &prefix = QString()) const;
    QAction *createRedoAction(QObject *parent,
                                const QString &prefix = QString()) const;

    bool isActive() const;
    bool isClean() const;
    int cleanIndex() const;

    void beginMacro(const QString &text);
    void endMacro();

public Q_SLOTS:
    void setClean();
    void setIndex(int idx);
    void undo();
    void redo();
    void setActive(bool active = true);

Q_SIGNALS:
    void indexChanged(int idx);
    void cleanChanged(bool clean);
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void undoTextChanged(const QString &undoText);
    void redoTextChanged(const QString &redoText);

private:
    Q_DISABLE_COPY(QUndoStack)
    friend class QUndoGroup;
};

QT_END_HEADER

#endif // QUNDOSTACK_H
