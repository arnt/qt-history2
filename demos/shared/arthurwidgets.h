#ifndef ARTHUR_WIDGETS
#define ARTHUR_WIDGETS

#include <qbitmap.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include "arthurstyle.h"

class QTextDocument;
class QTextEdit;
class QVBoxLayout;

class ArthurFrame : public QWidget
{
    Q_OBJECT
public:
    ArthurFrame(QWidget *parent);
    virtual void paint(QPainter *) {}


    void paintDescription(QPainter *p);

    void loadDescription(const QString &filename);
    void setDescription(const QString &htmlDesc);

    void loadSourceFile(const QString &fileName);

public slots:
    void setDescriptionEnabled(bool enabled);
    void showSource();

signals:
    void descriptionEnabledChanged(bool);

protected:
    void paintEvent(QPaintEvent *);

    QPixmap m_tile;

    bool m_show_doc;
    QTextDocument *m_document;

    QString m_sourceFileName;
};

class ArthurGroupBox : public QGroupBox
{
public:
    ArthurGroupBox(QWidget *parent) : QGroupBox(parent) {}

protected:
    void paintEvent(QPaintEvent *) {
        QPainter painter(this);
        QRect frameRect = rect();
        ArthurGroupBoxStyleOption opt;
        opt.init(this);
        opt.rect = frameRect;
        opt.title = title();

        style()->drawPrimitive(QStyle::PE_FrameGroupBox, &opt, &painter, this);
    }
};

#endif
