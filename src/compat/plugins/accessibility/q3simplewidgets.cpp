#include "q3simplewidgets.h"

#include <q3groupbox.h>
#include <qlabel.h>

QString Q_GUI_EXPORT qacc_stripAmp(const QString &text);

Q3AccessibleDisplay::Q3AccessibleDisplay(QWidget *w, Role role)
: QAccessibleWidget(w, role)
{
}

/*! \reimp */
QAccessible::Role Q3AccessibleDisplay::role(int child) const
{
    QLabel *l = qt_cast<QLabel*>(object());
    if (l) {
        if (l->pixmap() || l->picture())
            return Graphic;
#ifndef QT_NO_PICTURE
        if (l->picture())
            return Graphic;
#endif
#ifndef QT_NO_MOVIE
        if (l->movie())
            return Animation;
#endif
    }
    return QAccessibleWidget::role(child);
}

/*! \reimp */
QString Q3AccessibleDisplay::text(Text t, int child) const
{
    QString str;
    switch (t) {
    case Name:
        if (qt_cast<QLabel*>(object())) {
            str = qt_cast<QLabel*>(object())->text();
        } else if (qt_cast<Q3GroupBox*>(object())) {
            str = qt_cast<Q3GroupBox*>(object())->title();
        }
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);;
    return qacc_stripAmp(str);
}

/*! \reimp */
int Q3AccessibleDisplay::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    int relation = QAccessibleWidget::relationTo(child, other, otherChild);
    if (child || otherChild)
        return relation;

    QObject *o = other->object();
    QLabel *label = qt_cast<QLabel*>(object());
    Q3GroupBox *groupbox = qt_cast<Q3GroupBox*>(object());
    if (label) {
        if (o == label->buddy())
            relation |= Label;
    } else if (groupbox && !groupbox->title().isEmpty()) {
        if (groupbox->children().contains(o))
            relation |= Label;
    }
    return relation;
}

/*! \reimp */
int Q3AccessibleDisplay::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (rel == Labelled) {
        QObject *targetObject = 0;
        QLabel *label = qt_cast<QLabel*>(object());
        Q3GroupBox *groupbox = qt_cast<Q3GroupBox*>(object());
        if (label) {
            if (entry == 1)
                targetObject = label->buddy();
        } else if (groupbox && !groupbox->title().isEmpty()) {
            rel = Child;
        }
        *target = QAccessible::queryAccessibleInterface(targetObject);
        if (*target)
            return 0;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
}

