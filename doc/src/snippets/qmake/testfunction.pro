defineTest(allFiles) {
    files = $$ARGS

    for(file, files) {
        !exists($$file) {
            return(false)
        }
    }
    return(true)
}

files = delegate.h model.h view.h

allFiles($$files) {
    message(All files are present: $$files)
} else {
    message(Not all files are present: $$files)
}
