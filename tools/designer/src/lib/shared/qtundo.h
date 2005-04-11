#ifndef QTUNDO_H
#define QTUNDO_H

#include "shared_global.h"

#include <QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QStringList>
#include <QListView>

class QWidget;
class QAction;
class QtUndoStack;

class QT_SHARED_EXPORT QtCommand : public QObject
{
    Q_OBJECT

    friend class QtUndoStack;

    public:
        enum Type { Command, MacroBegin, MacroEnd };

        QtCommand(Type type, const QString &description = QString(),
                        bool canMerge = false);
        QtCommand(const QString &description = QString(),
                        bool canMerge = true);

        virtual void redo() {};
        virtual void undo() {};

        QString description() const
            { return m_description; }
        void setDescription(const QString &s)
            { m_description = s; }
        bool canMerge() const
            { return m_can_merge; }
        void setCanMerge(bool b)
            { m_can_merge = b; }
        Type type() const
            { return m_type; }

        bool isMacroBegin() const
            { return m_type == MacroBegin; }
        bool isMacroEnd() const
            { return m_type == MacroEnd; }
        bool isCommand() const
            { return m_type == Command; }

    protected:
        virtual bool mergeMeWith(QtCommand *other);

    private:
        void shortenStack();

        bool m_can_merge;
        QString m_description;
        Type m_type;
};

class QtMultiCommand : public QtCommand
{
public:
    QtMultiCommand(const QString &description = QString());
    QtMultiCommand(const QList<QtCommand*> &command_list,
                    const QString &description = QString());
    ~QtMultiCommand();
    virtual void redo();
    virtual void undo();

    void append(QtCommand *command);
    int count() const;
    QtCommand *command(int i) const;
    
private:
    QList<QtCommand*> m_command_list;    
};

class QtUndoState;

class QT_SHARED_EXPORT QtUndoStack : public QObject, private QList<QtCommand*>
{
    Q_OBJECT

    friend class QtUndoManager;

    public:
        QtUndoStack(QObject *parent = 0);
        void push(QtCommand *command);
        bool canUndo() const;
        bool canRedo() const;
        QString undoDescription() const;
        QString redoDescription() const;
        QStringList undoList() const;
        QStringList redoList() const;
        bool isClean() const;

        void setCurrent();
        
        QAction *createUndoAction(QObject *parent) const;
        QAction *createRedoAction(QObject *parent) const;

        inline int currentIndex() const { return m_current_iter; }

    public slots:
        void undo(int count = 1);
        void redo(int count = 1);
        void clear();

        void setClean();

    signals:
            void cleanChanged(bool clean);
        void commandExecuted();

        void undoDescriptionChanged(const QString &newDescription);
        void redoDescriptionChanged(const QString &newDescription);
        void canUndoChanged(bool enabled);
        void canRedoChanged(bool enabled);

    private:
        typedef int CommandIter;

        void undoMacro();
        void redoMacro();
        CommandIter findMacroBegin(CommandIter it) const;
        CommandIter findMacroEnd(CommandIter it) const;

        void beforeChange(QtUndoState &state);
        void afterChange(const QtUndoState &state);

        // *m_current_iter == 0 means "one-before-first"
        CommandIter m_current_iter;
        uint m_num_commands;
        int m_macro_nest;

        bool m_have_clean_command;
        const QtCommand *m_clean_command;

        QtCommand *commandAt(CommandIter it) const;
};

class QT_SHARED_EXPORT QtUndoManager : public QObject
{
    Q_OBJECT

    public:
        QtUndoManager();

        QAction *createUndoAction(QObject *parent) const;
        QAction *createRedoAction(QObject *parent) const;

        void associateView(QObject *obj, QtUndoStack *stack);
        void disassociateView(QObject *obj);

        bool canUndo() const;
        bool canRedo() const;
        QString undoDescription() const;
        QString redoDescription() const;
        void setUndoLimit(uint i);
        uint undoLimit() const;
        QStringList undoList() const;
        QStringList redoList() const;
        QtUndoStack *currentStack() const;
        void setCurrentStack(QtUndoStack *stack);

        static QtUndoManager *manager();

    public slots:
        void undo(int count = 1);
        void redo(int count = 1);

        void updateActions();

    signals:
        void changed();

        void undoDescriptionChanged(const QString &newDescription);
        void redoDescriptionChanged(const QString &newDescription);
        void canUndoChanged(bool enabled);
        void canRedoChanged(bool enabled);

    private slots:
        void stackDestroyed(QObject *stack);
        void viewDestroyed(QObject *view);

    private:
        typedef QMap<QObject*, QtUndoStack*> StackMap;

        StackMap m_stack_map;
        QtUndoStack *m_current_stack;

        static QtUndoManager *m_manager; // singleton
        static uint m_undo_limit;

        bool m_can_undo, m_can_redo;
        QString m_undo_description, m_redo_description;
};

class QT_SHARED_EXPORT QtUndoListModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    QtUndoListModel(QObject *parent = 0);
    virtual ~QtUndoListModel();

    inline int undoIndex() const { return m_undoIndex; }

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private slots:
    void updateItems();

private:
    QStringList m_items;
    int m_undoIndex;
};

class QT_SHARED_EXPORT QtUndoListView: public QListView
{
    Q_OBJECT
public:
    QtUndoListView(QWidget *parent = 0);
    virtual ~QtUndoListView();
    virtual void reset();
private slots:
    void undoOrRedo();
};



#endif
