#ifndef QDIALOG_P_H
#define QDIALOG_P_H

#include <private/qwidget_p.h>
#include <qdialog.h>

class QPushButton;
class QSizeGrip;

class QDialogPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDialog)
public:

    QDialogPrivate()
        : mainDef(0), orientation(Qt::Horizontal),extension(0), doShowExtension(false),
#ifndef QT_NO_SIZEGRIP
          resizer(0),
#endif
          rescode(0), in_loop(0)
        {}

    QPushButton* mainDef;
    Qt::Orientation orientation;
    QWidget* extension;
    bool doShowExtension;
    QSize size, min, max;
#ifndef QT_NO_SIZEGRIP
    QSizeGrip* resizer;
#endif
    QPoint lastRMBPress;

    void        setDefault(QPushButton *);
    void        setMainDefault(QPushButton *);
    void        hideDefault();
#ifdef Q_OS_TEMP
    void        hideSpecial();
#endif

    int                rescode;
    uint        in_loop: 1;
};

#endif // QDIALOG_P_H
