TEMPLATE        = subdirs

no-png {
    message("Some graphics-related tools are unavailable without PNG support")
} else {
     contains(QT_CONFIG, qdbus):SUBDIRS += qdbus
     SUBDIRS		+= assistant/lib \
 			assistant \
			pixeltool \
 			porting \
                         qtestlib
     contains(QT_EDITION, Console) {
         SUBDIRS += designer/src/uitools     # Linguist depends on this
     } else {
         SUBDIRS += designer
     }
     SUBDIRS     += linguist
     unix:!mac:!embedded:contains(QT_CONFIG, qt3support):SUBDIRS += qtconfig
     win32:!contains(QT_EDITION, OpenSource|Console):SUBDIRS += activeqt
}

# Patternist use member templates
!win32-msvc: SUBDIRS += patternist

CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"
