#include <qapplication.h>

class AssistantApplication : public QApplication
{
public:
    AssistantApplication(int argc, char **argv, bool GUIEnabled);
    bool notify(QObject *reciever, QEvent *event);
    bool isShiftKeyPressed();
private:
    bool shiftKeyPressed;
};
