/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLABEL_H
#define QLABEL_H

#include <QtGui/qframe.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QLabelPrivate;

class Q_GUI_EXPORT QLabel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
    Q_PROPERTY(bool scaledContents READ hasScaledContents WRITE setScaledContents)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(int indent READ indent WRITE setIndent)
    Q_PROPERTY(bool openExternalLinks READ openExternalLinks WRITE setOpenExternalLinks)
    Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)

public:
    explicit QLabel(QWidget *parent=0, Qt::WindowFlags f=0);
    explicit QLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);
    ~QLabel();

    QString text() const;
    const QPixmap *pixmap() const;
#ifndef QT_NO_PICTURE
    const QPicture *picture() const;
#endif
#ifndef QT_NO_MOVIE
    QMovie *movie() const;
#endif

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment);

    void setWordWrap(bool on);
    bool wordWrap() const;

    int indent() const;
    void setIndent(int);

    int margin() const;
    void setMargin(int);

    bool hasScaledContents() const;
    void setScaledContents(bool);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
#ifndef QT_NO_SHORTCUT
    void setBuddy(QWidget *);
    QWidget *buddy() const;
#endif
    int heightForWidth(int) const;

    bool openExternalLinks() const;
    void setOpenExternalLinks(bool open);

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

public Q_SLOTS:
    void setText(const QString &);
    void setPixmap(const QPixmap &);
#ifndef QT_NO_PICTURE
    void setPicture(const QPicture &);
#endif
#ifndef QT_NO_MOVIE
    void setMovie(QMovie *movie);
#endif
    void setNum(int);
    void setNum(double);
    void clear();

Q_SIGNALS:
    void linkActivated(const QString& link);
    void linkHovered(const QString& link);

protected:
    bool event(QEvent *e);
    void keyPressEvent(QKeyEvent *ev);
    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *ev);
    void focusInEvent(QFocusEvent *ev);
    void focusOutEvent(QFocusEvent *ev);
    bool focusNextPrevChild(bool next);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QLabel(QWidget *parent, const char* name, Qt::WindowFlags f=0);
    QT3_SUPPORT_CONSTRUCTOR QLabel(const QString &text, QWidget *parent, const char* name,
           Qt::WindowFlags f=0);
    QT3_SUPPORT_CONSTRUCTOR QLabel(QWidget *buddy, const QString &,
           QWidget *parent=0, const char* name=0, Qt::WindowFlags f=0);
    QT3_SUPPORT void setAlignment(int alignment);

    // don't mark the next function with QT3_SUPPORT
    inline void setAlignment(Qt::AlignmentFlag flag) { setAlignment((Qt::Alignment)flag); }
#endif

private:
    Q_DISABLE_COPY(QLabel)
    Q_DECLARE_PRIVATE(QLabel)
#ifndef QT_NO_MOVIE
    Q_PRIVATE_SLOT(d_func(), void _q_movieUpdated(const QRect&))
    Q_PRIVATE_SLOT(d_func(), void _q_movieResized(const QSize&))
#endif
    Q_PRIVATE_SLOT(d_func(), void _q_linkHovered(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_activateLink(const QString &))

    friend class QTipLabel;
    friend class QMessageBoxPrivate;
    friend class QBalloonTip;
};

QT_END_HEADER

#endif // QLABEL_H
