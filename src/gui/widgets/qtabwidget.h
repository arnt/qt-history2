/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#include "qwidget.h"
#include "qicon.h"

#ifndef QT_NO_TABWIDGET

class QTabBar;
class QTabWidgetPrivate;

class Q_GUI_EXPORT QTabWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(TabPosition TabShape)
    Q_PROPERTY(TabPosition tabPosition READ tabPosition WRITE setTabPosition)
    Q_PROPERTY(TabShape tabShape READ tabShape WRITE setTabShape)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int count READ count)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QTabWidget(QWidget *parent = 0);
    ~QTabWidget();

    int addTab(QWidget *widget, const QString &);
    int addTab(QWidget *widget, const QIcon& icon, const QString &label);

    int insertTab(int index, QWidget *widget, const QString &);
    int insertTab(int index, QWidget *widget, const QIcon& icon, const QString &label);

    void removeTab(int index);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool);

    QString tabText(int index) const;
    void setTabText(int index, const QString &);

    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon & icon);

    void setTabToolTip(int index, const QString & tip);
    QString tabToolTip(int index) const;

    int currentIndex() const;
    QWidget *currentWidget() const; // no setter on purpose
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;
    int count() const;

    enum TabPosition { North, South, West, East
#ifdef QT_COMPAT
        , Top = North, Bottom = South
#endif
    };
    TabPosition tabPosition() const;
    void setTabPosition(TabPosition);

    enum TabShape { Rounded, Triangular };
    TabShape tabShape() const;
    void setTabShape(TabShape s);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setCornerWidget(QWidget * w, Qt::Corner corner = Qt::TopRightCorner);
    QWidget * cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

public slots:
    void setCurrentIndex(int index);

signals:
    void currentChanged(int index);

protected:
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void setTabBar(QTabBar *);
    QTabBar* tabBar() const;
    void changeEvent(QEvent *);
    void updateMask();

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QTabWidget(QWidget *parent, const char *name, Qt::WFlags f = 0);

    inline QT_COMPAT void insertTab(QWidget * w, const QString &s, int index = -1) { insertTab(index, w, s); }
    inline QT_COMPAT void insertTab(QWidget *child, const QIcon& icon,
                                    const QString &label, int index = -1) { insertTab(index, child, icon, label); }

    inline QT_COMPAT void changeTab(QWidget *w, const QString &s) {setTabText(indexOf(w), s); }
    inline QT_COMPAT void changeTab(QWidget *w, const QIcon& icon,
                                    const QString &label) { int idx = indexOf(w); setTabText(idx, label); setTabIcon(idx, icon); }

    inline QT_COMPAT bool isTabEnabled( QWidget *w) const {return isTabEnabled(indexOf(w)); }
    inline QT_COMPAT void setTabEnabled(QWidget *w, bool b) { setTabEnabled(indexOf(w), b); }

    inline QT_COMPAT QString tabLabel(QWidget *w) const  {return tabText(indexOf(w)); }
    inline QT_COMPAT void setTabLabel(QWidget *w, const QString &l) { setTabText(indexOf(w), l); }

    inline QT_COMPAT QIcon tabIconSet(QWidget * w) const  {return tabIcon(indexOf(w)); }
    inline QT_COMPAT void setTabIconSet(QWidget * w, const QIcon & icon) { setTabIcon(indexOf(w), icon); }

    inline QT_COMPAT void removeTabToolTip(QWidget * w) {setTabToolTip(indexOf(w), QString());}
    inline QT_COMPAT void setTabToolTip(QWidget * w, const QString & tip) {setTabToolTip(indexOf(w), tip);}
    inline QT_COMPAT QString tabToolTip(QWidget * w) const { return tabToolTip(indexOf(w)); }

    inline QT_COMPAT QWidget * currentPage() const { return currentWidget(); }
    inline QT_COMPAT QWidget *page(int index) const { return widget(index); }
    inline QT_COMPAT QString label(int index) const { return tabText(index); }
    inline QT_COMPAT int currentPageIndex() const { return currentIndex(); }

    inline QT_COMPAT int margin() const { return 0; }
    inline QT_COMPAT void setMargin(int) {}

public slots:
    inline QT_MOC_COMPAT void setCurrentPage(int index) { setCurrentIndex(index); }
    inline QT_MOC_COMPAT void showPage(QWidget *w) { setCurrentIndex(indexOf(w)); }
    inline QT_MOC_COMPAT void removePage(QWidget *w) { removeTab(indexOf(w)); }

signals:
    QT_MOC_COMPAT void currentChanged(QWidget *);
#endif // QT_COMPAT

private:
    Q_DECLARE_PRIVATE(QTabWidget)
    Q_DISABLE_COPY(QTabWidget)
    Q_PRIVATE_SLOT(d, void showTab(int))
    Q_PRIVATE_SLOT(d, void removeTab(int))

    void setUpLayout(bool = false);
    friend class Q3TabDialog;
};

#endif // QT_NO_TABWIDGET

#endif // QTABWIDGET_H
