const char* baseDir = "../../examples";
#if defined(Q_OS_UNIX)
const char* suffixDir = "";
#else
const char* suffixDir = "Debug";
#endif

static struct {
    const char* label;
    const char* path;
    const char* file;
} command[] = {
    { "Text Editor",        "qwerty",	    "qwerty unicode.txt" },
    { "Scribble Editor",    "scribble",	    "scribble" },
    { "Helpviewer",	    "helpviewer",   "helpviewer" },
    { "I18n",		    "i18n",	    "i18n" },
    { "Demo",		    "demo",	    "demo" },
    { "Magnifier",          "qmag",	    "qmag" },
    { "Richtext",           "richtext",	    "richtext" },
    { "Widgets",            "widgets",	    "widgets" },
    { 0, 0, 0 }
};

static struct {
    const char* label;
    const char* path;
    const char* file;
} other_command[] = {
    { "aclock",        "aclock",        "aclock" },
    { "addressbook",   "addressbook",   "addressbook" },
    { "buttongroups",  "buttongroups",  "buttongroups" },
    { "checklists",    "checklists",    "checklists" },
    { "cursor",        "cursor",        "cursor" },
    { "customlayout",  "customlayout",  "customlayout" },
    { "dclock",        "dclock",        "dclock" },
    { "dirview",       "dirview",       "dirview" },
    { "drawlines",     "drawlines",     "drawlines" },
    { "fileiconview",       "fileiconview",       "fileiconview" },
    { "hello",         "hello",         "hello" },
    { "helpviewer",         "helpviewer",         "helpviewer" },
    { "layout",        "layout",        "layout" },
    { "life",          "life",          "life" },
    { "lineedits",     "lineedits",     "lineedits" },
    { "listbox",       "listbox",       "listbox" },
    { "listviews",       "listviews",       "listviews" },
    { "listboxcombo",  "listboxcombo",  "listboxcombo" },
    { "menu",          "menu",          "menu" },
    { "movies",        "movies",        "movies" },
    { "popup",         "popup",         "popup" },
    { "progress",      "progress",      "progress" },
    { "progressbar",   "progressbar",   "progressbar" },
    { "qfd",           "qfd",           "qfd" },
    { "rangecontrols", "rangecontrols", "rangecontrols" },
    { "richtext",      "richtext",      "richtext" },
    { "scrollview",    "scrollview",    "scrollview" },
    { "showimg",       "showimg",       "showimg" },
    { "splitter",      "splitter",      "splitter" },
    { "tabdialog",     "tabdialog",     "tabdialog" },
    { "table",         "table",         "table" },
    { "themes",         "themes",         "themes" },
    { "tetrix",        "tetrix",        "tetrix" },
    { "tictac",        "tictac",        "tictac" },
    { "tooltip",       "tooltip",       "tooltip" },
    { "validator",     "validator",     "validator" },
    { "widgets",       "widgets",       "widgets" },
    { "wizard",        "wizard",        "wizard" },
    { 0, 0, 0 }
};
