#ifndef QDIALOGPRIVATE_P_H
#define QDIALOGPRIVATE_P_H

#include <private/qwidget_p.h>
#include <qdialog.h>

class QPushButton;
class QSizeGrip;

// ### Don't export dialog private when printing has been ported into main...
class Q_GUI_EXPORT QDialogPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDialog)
public:

    QDialogPrivate()
        : mainDef(0), orientation(Horizontal),extension(0), doShowExtension(false),
#ifndef QT_NO_SIZEGRIP
          resizer(0),
#endif
          rescode(0), in_loop(0)
        {}

    QPushButton* mainDef;
    Orientation orientation;
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

#endif // QDIALOGPRIVATE_P_H
