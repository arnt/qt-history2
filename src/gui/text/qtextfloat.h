#ifndef QTEXTFLOAT_H
#define QTEXTFLOAT_H

#ifndef QT_H
#include <qshareddata.h>
#include <qtextformat.h>
#endif // QT_H

class QTextFloatPrivate;

class Q_GUI_EXPORT QTextFloat : public QTextFormatGroup
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextFloat);
public:

    void setFormat(const QTextFloatFormat &format) { setCommonFormat(format); }
    QTextFloatFormat format() const { return commonFormat().toFloatFormat(); }

private:
    QTextFloat(QObject *parent);
    ~QTextFloat();

    friend class QTextFormatCollection;

#if defined(Q_DISABLE_COPY)
    QTextFloat(const QTextFloat &rhs);
    QTextFloat &operator=(const QTextFloat &rhs);
#endif
};

#endif // QTEXTLIST_H
