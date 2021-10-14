
#include <iostream>
#include <exception>

struct BufferOutOfBoundsException : public std::exception
{
private:
    const char *errorMessage;

public:
    BufferOutOfBoundsException(const char *errMessage) : errorMessage{errMessage}
    {
    }

    BufferOutOfBoundsException()
    {
        errorMessage = "BufferOutOfBoundsException";
    }

    const char *what() const throw()
    {
        return errorMessage;
    }
};