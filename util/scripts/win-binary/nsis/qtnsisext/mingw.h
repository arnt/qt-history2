#define MINGW_BUFFER_SIZE 1024

void getMinGWVersion(char *path, int *major, int *minor, int *patch);
bool hasValidIncludeFiles(char *path);
bool shInEnvironment();