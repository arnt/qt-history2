#include <qaxobject.h>
#include <qaxfactory.h>

#include <qt_windows.h>

QAxBase *qax_create_object_wrapper(QObject *object)
{
    IDispatch *dispatch = 0;
    QAxBase *wrapper = 0;
    qAxFactory()->createObjectWrapper(object, &dispatch);
    if (dispatch) {
	wrapper = new QAxObject((IUnknown*)wrapper, object, object->name());
	dispatch->Release();
    }
    return wrapper;
}
