#ifndef QDIALOGPRIVATE_P_H
#define QDIALOGPRIVATE_P_H

#include <private/qwidget_p.h>

class QPushButton;
class QSizeGrip;

class QDialogPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QWidget);
public:

    QDialogPrivate()
        : mainDef(0), orientation(Horizontal),extension(0), doShowExtension(false)
#ifndef QT_NO_SIZEGRIP
        ,resizer(0)
#endif
        {
    }

    QPushButton* mainDef;
    Orientation orientation;
    QWidget* extension;
    bool doShowExtension;
    QSize size, min, max;
#ifndef QT_NO_SIZEGRIP
    QSizeGrip* resizer;
#endif
    QPoint lastRMBPress;
};

#endif // QDIALOGPRIVATE_P_H
