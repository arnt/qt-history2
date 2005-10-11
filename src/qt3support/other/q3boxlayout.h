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

#ifndef Q3BOXLAYOUT_H
#define Q3BOXLAYOUT_H

#include <QtGui/qboxlayout.h>

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3BoxLayout : public QBoxLayout
{
public:
    inline explicit Q3BoxLayout(Direction dir, QWidget *parent = 0)
        : QBoxLayout(dir, parent) { setMargin(0); setSpacing(0); }

    inline Q3BoxLayout(QWidget *parent, Direction dir, int margin = 0, int spacing = -1,
                       const char *name = 0)
        : QBoxLayout(parent, dir, margin, spacing, name) {}

    inline Q3BoxLayout(QLayout *parentLayout, Direction dir, int spacing = -1,
                       const char *name = 0)
        : QBoxLayout(parentLayout, dir, spacing, name) { setMargin(0); }

    inline Q3BoxLayout(Direction dir, int spacing, const char *name = 0)
        : QBoxLayout(dir, spacing, name) { setMargin(0); }

private:
    Q_DISABLE_COPY(Q3BoxLayout)
};

class Q_COMPAT_EXPORT Q3HBoxLayout : public Q3BoxLayout
{
public:
    inline QHBoxLayout() : Q3BoxLayout(LeftToRight) {}

    inline explicit QHBoxLayout(QWidget *parent) : Q3BoxLayout(LeftToRight, parent) {}

    inline QHBoxLayout(QWidget *parent, int margin,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parent, LeftToRight, margin, spacing, name) {}

    inline QHBoxLayout(QLayout *parentLayout,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parentLayout, LeftToRight, spacing, name) {}

    inline QHBoxLayout(int spacing, const char *name = 0)
        : Q3BoxLayout(LeftToRight, spacing, name) {}

private:
    Q_DISABLE_COPY(Q3HBoxLayout)
};

class Q_COMPAT_EXPORT Q3VBoxLayout : public Q3BoxLayout
{
public:
    inline QVBoxLayout() : Q3BoxLayout(TopToBottom) {}

    inline explicit QVBoxLayout(QWidget *parent) : Q3BoxLayout(TopToBottom, parent) {}

    inline QVBoxLayout(QWidget *parent, int margin,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parent, TopToBottom, margin, spacing, name) {}

    inline QVBoxLayout(QLayout *parentLayout,
                 int spacing = -1, const char *name = 0)
        : Q3BoxLayout(parentLayout, TopToBottom, spacing, name) {}

    inline QVBoxLayout(int spacing, const char *name = 0)
        : Q3BoxLayout(TopToBottom, spacing, name) {}

private:
    Q_DISABLE_COPY(Q3VBoxLayout)
};

#endif // Q3BOXLAYOUT_H
