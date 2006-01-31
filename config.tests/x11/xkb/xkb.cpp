#include <X11/Xlib.h>
#include <X11/XKBlib.h>

int main(int, char **)
{
    Display *display = 0;
    unsigned int state = XkbPCF_GrabsUseXKBStateMask;
    (void) XkbSetPerClientControls(display, state, &state);
    return 0;
}
