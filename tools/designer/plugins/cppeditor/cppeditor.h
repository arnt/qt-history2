#ifndef CPPEDITOR_H
#define CPPEDITOR_H

#include <editor.h>

class EditorCompletion;

class  CppEditor : public Editor
{
    Q_OBJECT

public:
    CppEditor( const QString &fn, QWidget *parent, const char *name );

    virtual EditorCompletion *completionManager() { return completion; }

protected:
    EditorCompletion *completion;

};

#endif
