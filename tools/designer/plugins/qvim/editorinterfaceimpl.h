#ifndef EDITORINTERFACEIMPL_H
#define EDITORINTERFACEIMPL_H

#include <editorinterface.h>

class VimMainWindow;

class EditorInterfaceImpl : public EditorInterface
{
public:
    EditorInterfaceImpl();
    virtual ~EditorInterfaceImpl();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QWidget *editor( QWidget *parent ) const;

    void setText( const QString &txt );
    QString text() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    bool find( const QString &expr, bool cs, bool wo, bool forward );
    void indent();
    void scrollTo( const QString &txt );
    void splitView();
    void setContext( QObjectList *toplevels, QObject *this_ );

    void setError( int line );

private:
    VimMainWindow* vimEditor;
    unsigned long ref;

};

#endif
