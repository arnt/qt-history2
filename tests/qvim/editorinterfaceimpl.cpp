#include "editorinterfaceimpl.h"
#include <viewmanager.h>
#include "gui_qt_widget.h"
#include "qapplication.h"

extern "C" {
int vim_main(int argc, char** argv);
}


EditorInterfaceImpl::EditorInterfaceImpl()
    : EditorInterface(), vimEditor(0),ref( 0 )
{
}

EditorInterfaceImpl::~EditorInterfaceImpl()
{
}

QUnknownInterface *EditorInterfaceImpl::queryInterface( const QUuid &uuid )
{
    qDebug("EditorInterfaceImpl::queryInterface");
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_EditorInterface ) {
	qDebug("..returning IID_EditorInterface");
	iface = (EditorInterface*)this;
    }

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long EditorInterfaceImpl::addRef()
{
    qDebug("EditorInterfaceImpl::addRef");
    return ref++;
}

unsigned long EditorInterfaceImpl::release()
{
    qDebug("EditorInterfaceImpl::release");
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QWidget *EditorInterfaceImpl::editor( QWidget *parent ) const
{
    qDebug("EditorInterfaceImpl::editor");
    qDebug("setting qvim parent");
    qvim_set_parent( parent );
    qDebug("starting qvim_main()...");
#if 0
    vim_main(qApp->argc(),qApp->argv()); // automatically creates editor
    qDebug("...done");
    ((EditorInterfaceImpl*)this)->vimEditor = qvim_get_editor();
#endif
    ((EditorInterfaceImpl*)this)->vimEditor = new VimMainWindow( parent );
    return vimEditor;
}

void EditorInterfaceImpl::setText( const QString &txt )
{
    qDebug("EditorInterfaceImpl::setText");
    if ( !vimEditor )
	return;
    vimEditor->setText( txt );
}

QString EditorInterfaceImpl::text() const
{
    qDebug("EditorInterfaceImpl::text()");
    if ( !vimEditor )
	return QString::null;
    return vimEditor->text();
}

void EditorInterfaceImpl::undo()
{
}

void EditorInterfaceImpl::redo()
{
}

void EditorInterfaceImpl::cut()
{
}

void EditorInterfaceImpl::copy()
{
}

void EditorInterfaceImpl::paste()
{
}

void EditorInterfaceImpl::selectAll()
{
}

bool EditorInterfaceImpl::find( const QString &expr, bool cs, bool wo, bool forward )
{
}

void EditorInterfaceImpl::indent()
{
}

void EditorInterfaceImpl::splitView()
{
}

void EditorInterfaceImpl::scrollTo( const QString &txt )
{
}

QStringList EditorInterfaceImpl::featureList() const
{
    QStringList list;
    list << "C++";
    return list;
}

void EditorInterfaceImpl::setContext( QObjectList *toplevels, QObject *this_ )
{
}

void EditorInterfaceImpl::setError( int line )
{
}
