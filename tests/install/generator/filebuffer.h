
class FileBuffer
{
public:
    FileBuffer( const char* data = 0, unsigned int length = 0 );
    ~FileBuffer();

    char* data();
    unsigned int size();
private:
    char* buffer;
    unsigned int buffSize;
};

