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

#ifndef STYLEBUTTON_H
#define STYLEBUTTON_H

#include <qpushbutton.h>

class StyleButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_ENUMS(ButtonType)

public:
    enum ButtonType {ColorButton, PixmapButton};

	StyleButton (QWidget *parent = 0, ButtonType type = ColorButton);
	~StyleButton () {}

	void setButtonType(ButtonType type);
    const QBrush &brush();
	void setBrush(const QBrush &b);

	QString pixmapFileName() const;

protected:
	void paintEvent (QPaintEvent *event);

private:
	QString buildImageFormatList() const;
	bool openPixmap();

	ButtonType btype;
	QString pixFile;
	QBrush mBrush;

public slots:
    virtual void onEditor();

signals:
	void changed();
};

#endif // STYLEBUTTON_H
