static struct {
    const char* label;
    const char* file;
} command[] = {
    { "Desktop-in-an-application", "( cd ../multidoc; exec ./multidoc )" },
    { "Info Kiosk - MPEGs", "( cd ../kiosk; exec ./kiosk)" },
    { "Help Text Browser", "exec ../helpviewer/helpviewer" },
    { "Canvas - alpha-blending", "( cd ../canvas; exec ./canvas )" },
    { "Text Editor", "exec ../qwerty/qwerty ../qwerty/unicode.txt" },
    { "Scribble Editor", "exec ../scribble/scribble" },
    { "Internationalization", "( cd ../i18n; exec ./i18n all )" },
    { "Magnifier", "exec ../qmag/qmag" },
    /*{ "Dumb Terminal", "exec ../dumbterm/dumbterm" },*/
    { 0, 0 }
};

static struct {
    const char* label;
    const char* file;
} other_command[] = {
    { "aclock", "( cd ../aclock; exec ./aclock; )" },
    { "addressbook", "( cd ../addressbook; exec ./addressbook; )" },
    { "buttongroups", "( cd ../buttongroups; exec ./buttongroups; )" },
    { "checklists", "( cd ../checklists; exec ./checklists; )" },
    { "cursor", "( cd ../cursor; exec ./cursor; )" },
    { "customlayout", "( cd ../customlayout; exec ./customlayout; )" },
    { "dclock", "( cd ../dclock; exec ./dclock; )" },
    { "dirview", "( cd ../dirview; exec ./dirview; )" },
    //{ "drawdemo", "( cd ../drawdemo; exec ./drawdemo; )" },
    { "drawlines", "( cd ../drawlines; exec ./drawlines; )" },
    //{ "forever", "( cd ../forever; exec ./forever; )" },
    { "hello", "( cd ../hello; exec ./hello; )" },
    { "layout", "( cd ../layout; exec ./layout; )" },
    { "life", "( cd ../life; exec ./life; )" },
    { "lineedits", "( cd ../lineedits; exec ./lineedits; )" },
    { "listbox", "( cd ../listbox; exec ./listbox; )" },
    { "listboxcombo", "( cd ../listboxcombo; exec ./listboxcombo; )" },
    { "menu", "( cd ../menu; exec ./menu; )" },
    { "movies", "( cd ../movies; exec ./movies; )" },
    //{ "picture", "( cd ../picture; exec ./picture; )" },
    { "popup", "( cd ../popup; exec ./popup; )" },
    { "progress", "( cd ../progress; exec ./progress; )" },
    { "progressbar", "( cd ../progressbar; exec ./progressbar; )" },
    { "qfd", "( cd ../qfd; exec ./qfd; )" },
    { "rangecontrols", "( cd ../rangecontrols; exec ./rangecontrols; )" },
    { "richtext", "( cd ../richtext; exec ./richtext; )" },
    { "scrollview", "( cd ../scrollview; exec ./scrollview; )" },
    { "showimg", "( cd ../showimg; exec ./showimg; )" },
    //{ "sound", "( cd ../sound; exec ./sounds; )" },
    { "splitter", "( cd ../splitter; exec ./splitter; )" },
    { "tabdialog", "( cd ../tabdialog; exec ./tabdialog; )" },
    { "table", "( cd ../table; exec ./table; )" },
    { "tetrix", "( cd ../tetrix; exec ./tetrix; )" },
    { "tictac", "( cd ../tictac; exec ./tictac; )" },
    { "tooltip", "( cd ../tooltip; exec ./tooltip; )" },
    { "validator", "( cd ../validator; exec ./validator; )" },
    { "widgets", "( cd ../widgets; exec ./widgets; )" },
    { "wizard", "( cd ../wizard; exec ./wizard; )" },
    //{ "xform", "( cd ../xform; exec ./xform; )" },
    { 0, 0 }
};
