#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QIcon>
#include <QPixmap>
#include <QString>

inline QIcon createIconSet(const QString &name)
{ return QPixmap(QString::fromUtf8(":/trolltech/formeditor/images/") + name); }

#endif
