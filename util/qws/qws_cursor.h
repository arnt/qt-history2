
#include <qimage.h>

class QWSCursor
{
public:
    QWSCursor() {}
    QWSCursor(const uchar *data, const uchar *mask,
                int width, int height, int hotX, int hotY)
	{ set(data, mask, width, height, hotX, hotY); }

    void set(const uchar *data, const uchar *mask,
		int width, int height, int hotX, int hotY);

    QPoint hotSpot() const { return hot; }
    QImage &image() { return cursor; }
    const QRegion region() const { return rgn; }

    static QWSCursor *systemCursor(int id);

private:
    QPoint hot;
    QImage cursor;
    QRegion rgn;
};


