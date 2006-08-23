TEMPLATE      = subdirs
unset(EXAMPLES_THREADS_SUBDIRS)
EXAMPLES_THREADS_SUBDIRS = examples_threads_mandelbrot \
                           examples_threads_semaphores \
                           examples_threads_waitconditions

# install
target.path = $$[QT_INSTALL_EXAMPLES]/threads
EXAMPLES_THREADS_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS threads.pro README
EXAMPLES_THREADS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/threads
INSTALLS += target EXAMPLES_THREADS_install_sources

#subdirs
examples_threads_mandelbrot.subdir = $$QT_BUILD_TREE/examples/threads/mandelbrot
examples_threads_mandelbrot.depends =  src_corelib src_gui
examples_threads_semaphores.subdir = $$QT_BUILD_TREE/examples/threads/semaphores
examples_threads_semaphores.depends =  src_corelib
examples_threads_waitconditions.subdir = $$QT_BUILD_TREE/examples/threads/waitconditions
examples_threads_waitconditions.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_THREADS_SUBDIRS
SUBDIRS += $$EXAMPLES_THREADS_SUBDIRS
