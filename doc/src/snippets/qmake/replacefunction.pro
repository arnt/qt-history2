defineReplace(headersAndSources) {
    variable = $$1
    files = $$eval($$variable)
    headers =
    sources =

    for(file, files) {
        header = $${file}.h
        exists($$header) {
            headers += $$header
        }
        source = $${file}.cpp
        exists($$source) {
            sources += $$source
        }
    }
    return($$headers $$sources)
}

names = delegate model view main
message($$names)
allFiles = $$headersAndSources(names)
message($$allFiles)
