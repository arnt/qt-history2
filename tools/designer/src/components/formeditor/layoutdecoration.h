#ifndef LAYOUTDECORATION_H
#define LAYOUTDECORATION_H

#include <extension.h>
#include <QObject>

class QPoint;

struct ILayoutDecoration
{
    virtual ~ILayoutDecoration() {}
    
    virtual int findItemAt(const QPoint &pos) const = 0;
    virtual void adjustIndicator(const QPoint &pos, int index) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(ILayoutDecoration, "http://trolltech.com/Qt/IDE/LayoutDecoration")

#endif // LAYOUTDECORATION_H
