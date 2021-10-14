
#include <iostream>
#include <exception>

struct BufferBoundException : public std::exception
{
private:
    const char *errorMessage;

public:
    BufferBoundException(const char *errMessage) : errorMessage{errMessage}
    {
    }

    BufferBoundException() 
    {
        errorMessage = "BufferBoundException";
    }

    const char *what() const throw()
    {
        return errorMessage;
    }
};