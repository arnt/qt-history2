
#include "qformloader.h"
#include <formbuilder.h>
#include <private/qobject_p.h>

namespace QForm
{

class LoaderPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Loader)
public:
    QFormInternal::QFormBuilder builder;
};

Loader::Loader(QObject *parent)
    : QObject(*new LoaderPrivate, parent)
{
}

Loader::~Loader()
{
}

QWidget *Loader::load(QIODevice *device, QWidget *parentWidget)
{
    Q_D(Loader);
    return d->builder.load(device, parentWidget);
}

} // namespace QForm

