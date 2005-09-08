#ifndef QFORMLOADER_H
#define QFORMLOADER_H

#include <QtCore/QObject>

class QWidget;
class QString;
class QIODevice;

namespace QForm
{

class LoaderPrivate;

class Loader: public QObject
{
public:
    Loader(QObject *parent = 0);
    virtual ~Loader();

    QWidget *load(QIODevice *device, QWidget *parentWidget = 0);

private:
    Q_DECLARE_PRIVATE(Loader)
    Q_DISABLE_COPY(Loader)
};

} // namespace QForm

#endif // QFORMLOADER_H
