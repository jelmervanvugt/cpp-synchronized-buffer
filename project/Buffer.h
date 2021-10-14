
#include <mutex>
#include <vector>
#include "exceptions/BufferOutOfBoundsException.cpp"
#include "exceptions/BufferBoundException.cpp"

template <typename T>
class Buffer
{
private:
    bool isBounded;
    long unsigned int bufferBound;
    int nReaders;

    std::vector<T> buffer;
    std::vector<std::string> logger;

    std::mutex m_nReaders;
    std::mutex m_currentlyReading;
    std::mutex m_mod_queued;

    void openMod()
    {
        m_mod_queued.lock();
        m_currentlyReading.lock();
    }

    void closeMod()
    {
        m_currentlyReading.unlock();
        m_mod_queued.unlock();
    }

public:
    Buffer(long unsigned int bs) : isBounded{true}, bufferBound{bs}, nReaders{0}, buffer{}, logger{}
    {
        buffer.reserve(bufferBound);
    }

    Buffer() : isBounded{false}, bufferBound{0}, nReaders{0}, buffer{}, logger{}
    {
    }

    /*
    A thread is only able to read if:
      - There are no modifications queued
      - There are no current readers
      - There are no modifictions currently executing
  */
    T read(unsigned long int i, int threadId)
    {
        m_mod_queued.lock();
        m_mod_queued.unlock();

        m_nReaders.lock();
        if (nReaders == 0)
        {
            m_currentlyReading.lock();
        }
        nReaders++;
        m_nReaders.unlock();

        if (i < 0 || i > buffer.size() || (buffer.size() == 0 && i == 0))
        {
            std::string errmsg = "Error while reading element " + std::to_string(i) + " from buffer. Index cannot be negative of greater than bufferSize.";
            logger.push_back(errmsg);
            throw BufferOutOfBoundsException(errmsg.c_str());
        }

        T val = buffer[i];
        logger.push_back("Reading operation has been executed for index " + std::to_string(i) + " by thread with ID: " + std::to_string(threadId) + ".");

        m_nReaders.lock();
        if (nReaders == 1)
        {
            m_currentlyReading.unlock();
        }
        nReaders--;
        m_nReaders.unlock();

        return val;
    }

    /*
    A thread is only able to write if:
      - There are no current readers
      - There are no current modifications executing
  */
    void write(T content, int threadId)
    {
        openMod();

        if (isBounded && buffer.size() == bufferBound)
        {
            std::string errmsg = "Error while trying to write element " + std::to_string(content) + " to buffer. Buffer cannot exceed bufferbound.";
            logger.push_back(errmsg);
            throw BufferBoundException(errmsg.c_str());
        }
        else
        {
            buffer.push_back(content);
            logger.push_back("Writing operation has been executed for element " + std::to_string(content) + " by thread with ID: " + std::to_string(threadId) + ".");
        }

        closeMod();
    }

    /*
    An thread can only be removed if:
      - There are no current readers
      - There are no current modifications executing
  */
    void remove(unsigned long int i, int threadId)
    {
        openMod();

        if (i < 0 || i > buffer.size() || (buffer.size() == 0 && i == 0))
        {
            std::string errmsg = "Error occured while removing element " + std::to_string(i) + " from buffer. Index cannot be negative or greater than bufferSize.";
            logger.push_back(errmsg);
            throw BufferOutOfBoundsException(errmsg.c_str());
        }
        else
        {
            buffer.erase(buffer.begin + 1);
            logger.push_back("Removal operation has been executed for index " + std::to_string(i) + " by thread with ID: " + std::to_string(threadId) + ".");
        }

        closeMod();
    }

    /*
      A thread can only unbind the buffer if:
        - There are no current modifications executing
        - Buffer had a bound
  */
    void unbind(int threadId)
    {
        openMod();

        if (isBounded == false)
        {
            std::string errmsg = "Cannot unbind an unbounded buffer.";
            logger.push_back(errmsg);
            throw BufferBoundException(errmsg.c_str());
        }
        else
        {
            bufferBound = 0;
            isBounded = false;
            logger.push_back("Unbind operation has been executed for by thread with ID: " + std::to_string(threadId) + ".");
        }

        closeMod();
    }

    /*
  A thread can only bind the buffer if:
    - There are no current modifications executing
    - Buffer has no bound
*/
    void bind(unsigned long int bound, int threadId)
    {
        openMod();

        if (bound < buffer.size() || bound < 0)
        {
            std::string errmsg = "Error occured while trying to set bound " + std::to_string(bound) + ". Bound cannot be negative or lesser than bufferSize.";
            logger.push_back(errmsg);
            throw BufferBoundException(errmsg.c_str());
        }
        else
        {
            buffer.reserve(bound);
            isBounded = true;
            bufferBound = bound;
            logger.push_back("Removal operation has been executed for bound " + std::to_string(bound) + " by thread with ID: " + std::to_string(threadId) + ".");
        }

        closeMod();
    }

    /*
    A thread can only get buffer size if:
    - There are no current modifictions executing
    */
    unsigned long int getBufferSize(int threadId)
    {
        openMod();

        unsigned long int size = buffer.size();
        logger.push_back("Buffer size has been requested by thread with ID: " + std::to_string(threadId) + ".");

        closeMod();

        return size;
    }
};
