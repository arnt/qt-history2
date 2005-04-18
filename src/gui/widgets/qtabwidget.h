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

#include "QtGui/qwidget.h"
#include "QtGui/qicon.h"

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

public:
    explicit QTabWidget(QWidget *parent = 0);
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
    QWidget *currentWidget() const;
    void setCurrentWidget(QWidget *widget);
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;
    int count() const;

    enum TabPosition { North, South, West, East
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
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

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QTabWidget(QWidget *parent, const char *name, Qt::WFlags f = 0);

    inline QT3_SUPPORT void insertTab(QWidget * w, const QString &s, int index = -1) { insertTab(index, w, s); }
    inline QT3_SUPPORT void insertTab(QWidget *child, const QIcon& icon,
                                    const QString &label, int index = -1) { insertTab(index, child, icon, label); }

    inline QT3_SUPPORT void changeTab(QWidget *w, const QString &s) {setTabText(indexOf(w), s); }
    inline QT3_SUPPORT void changeTab(QWidget *w, const QIcon& icon,
                                    const QString &label) { int idx = indexOf(w); setTabText(idx, label); setTabIcon(idx, icon); }

    inline QT3_SUPPORT bool isTabEnabled( QWidget *w) const {return isTabEnabled(indexOf(w)); }
    inline QT3_SUPPORT void setTabEnabled(QWidget *w, bool b) { setTabEnabled(indexOf(w), b); }

    inline QT3_SUPPORT QString tabLabel(QWidget *w) const  {return tabText(indexOf(w)); }
    inline QT3_SUPPORT void setTabLabel(QWidget *w, const QString &l) { setTabText(indexOf(w), l); }

    inline QT3_SUPPORT QIcon tabIconSet(QWidget * w) const  {return tabIcon(indexOf(w)); }
    inline QT3_SUPPORT void setTabIconSet(QWidget * w, const QIcon & icon) { setTabIcon(indexOf(w), icon); }

    inline QT3_SUPPORT void removeTabToolTip(QWidget * w) {setTabToolTip(indexOf(w), QString());}
    inline QT3_SUPPORT void setTabToolTip(QWidget * w, const QString & tip) {setTabToolTip(indexOf(w), tip);}
    inline QT3_SUPPORT QString tabToolTip(QWidget * w) const { return tabToolTip(indexOf(w)); }

    inline QT3_SUPPORT QWidget * currentPage() const { return currentWidget(); }
    inline QT3_SUPPORT QWidget *page(int index) const { return widget(index); }
    inline QT3_SUPPORT QString label(int index) const { return tabText(index); }
    inline QT3_SUPPORT int currentPageIndex() const { return currentIndex(); }

    inline QT3_SUPPORT int margin() const { return 0; }
    inline QT3_SUPPORT void setMargin(int) {}

public slots:
    inline QT_MOC_COMPAT void setCurrentPage(int index) { setCurrentIndex(index); }
    inline QT_MOC_COMPAT void showPage(QWidget *w) { setCurrentIndex(indexOf(w)); }
    inline QT_MOC_COMPAT void removePage(QWidget *w) { removeTab(indexOf(w)); }

signals:
    QT_MOC_COMPAT void currentChanged(QWidget *);
#endif // QT3_SUPPORT

private:
    Q_DECLARE_PRIVATE(QTabWidget)
    Q_DISABLE_COPY(QTabWidget)
    Q_PRIVATE_SLOT(d_func(), void showTab(int))
    Q_PRIVATE_SLOT(d_func(), void removeTab(int))

    void setUpLayout(bool = false);
    friend class Q3TabDialog;
};

#endif // QT_NO_TABWIDGET

#endif // QTABWIDGET_H
