#ifndef EDITORINTERFACE_H
#define EDITORINTERFACE_H

#include <qcomponentinterface.h>
#include <qmap.h>

class QWidget;
class QObjectList;
class QObject;

class EditorInterface : public QUnknownInterface
{
public:
    EditorInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "EditorInterface" ); }

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
    virtual QMap<QString, QString> functions() const = 0;

};

#endif
