#ifndef EDITORINTERFACE_H
#define EDITORINTERFACE_H

#include <qcomponentinterface.h>
#include <qmap.h>
#include <qstringlist.h>

class QWidget;
class QObjectList;
class QObject;

// {8668161A-6037-4220-86B6-CCAA20127DF8}
#ifndef IID_EditorInterface
#define IID_EditorInterface QUuid( 0x8668161a, 0x6037, 0x4220, 0x86, 0xb6, 0xcc, 0xaa, 0x20, 0x12, 0x7d, 0xf8 )
#endif

class EditorInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual QWidget *editor( QWidget *parent ) const = 0;

    virtual void setText( const QString &txt ) = 0;
    virtual QString text() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void cut() = 0;
    virtual void copy() = 0;
    virtual void paste() = 0;
    virtual void selectAll() = 0;
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward ) = 0;
    virtual void indent() = 0;
    virtual void scrollTo( const QString &txt ) = 0;
    virtual void splitView() = 0;
    virtual void setContext( QObjectList *toplevels, QObject *this_ ) = 0;
    virtual void setError( int line ) = 0;

};

#endif
