#ifndef QTMD5_H
#define QTMD5_H

#include <qstring.h>
#include <qbytearray.h>

void qtMD5(const QByteArray &src, unsigned char *digest);
QString qtMD5(const QByteArray &src);

#endif // QTMD5_H
