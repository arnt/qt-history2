#ifndef BROWSER_H
#define BROWSER_H

#include <qobject.h>
#include "dlldefs.h"

class Editor;

class EDITOR_EXPORT EditorBrowser : public QObject
{
    Q_OBJECT

public:
    EditorBrowser( Editor *e );

    bool eventFilter( QObject *o, QEvent *e );
    virtual void setCurrentEdior( Editor *e );
    virtual void addEditor( Editor *e );

protected:
    Editor *curEditor;

};

#endif
