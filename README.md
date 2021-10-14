# Report | Assignment 2: Synchronization - Operating Systems

| Student name | Student number |
| ------------ | -------------- | 
| Jelmer van Vugt | s1081716 |

The goal of this assignment was to write an implementation of a simple (possible bounded) buffer in C++. The buffer of this kind can be used for intercommunication between different threads. For this reason, the buffer should also be free of any concurrency problems.  

In this document the correctness in terms of functionality and concurrency is discussed.

# Design
In this chapter the implementation of the buffer is discussed.

## Classification of operations
The operations of the buffer that are available to users can be divided into two groups. In the first group only the read operation is classified. The operation in the first group are able to work concurrently without interfering with each other. All operations in the second group execute a modifying operation, making them unable to work concurrently with other operations in their own group as in group one. In the second group the write, remove, bind and unbind operations are classified. 

## Critical sections & pseudo-code
When writing the buffer an eye had the be kept on the critical sections of the code. A critcal section is a section of code that has to be executed without interference from the outside, other threads for example. When these critical sections are not properly maintained concurrency issues will ensue.

To maintain the readability of the code I've chosen to not mark the critical sections of the buffer in the source files themselves. Instead I marked the lines in the critical sections with a '|' in the pseudo-code section below. These code blocks contain all components to be able to reason about the concurrency handling in a clear way. 

### Pseudo-code

<table style="width: 100%">
<tr>
<th>Shared Variables</th>
<th>Group 1</th>
<th>Group 2</th>
</tr>
<tr>
<td>

```c++
long unsigned int bufferBound;
int nReaders;

mutex m_nReaders;
mutex m_currentlyReading;
mutex m_mod_queued;
```

</td>
<td>

```c++

//read

read()
{            
    m_mod_queued.lock();
    m_mode_queued.unlock(); 

    m_nReaders.lock();                    
    if (nReaders == 0)                      |      
    {                                       |
        m_currentlyReading.lock();          |
    }                                       |
    nReaders++;                             |
    m_nReaders.unlock();                    |
                                            |
    // reading operation                    |
                                            |
    m_nReaders.lock();                      |
    if (nReaders == 1)                      |
    {                                       |
        m_currentlyReading.unlock();        |
    }                                       |
    nReaders--;                             |
    m_nReaders.unlock();                         

    return;
}
```
</td>
<td>

```c++

// write
// remove
// bind
// unbind

mod()
{            
    m_mod_queued.lock();
    m_currentlyReading.lock();
                                            |
    // modifying operation                  |
                                            |
    m_currentlyReading.unlock();
    m_mod_queued.unlock();
}
```
</td>
</tr>
</table>

## Buffer - General
The buffer utilizes a set of variables to ensure its functionality which will be explained here. The `bool: isBounded` variable is pretty self-explanatory; it keeps track on if the buffer is or is not bounded. Depending on the value of the variable some functions differ on implementation. The `long unsigned int: bufferBound` stores the max. bound of the buffer. If there is no max. bound on the buffer it is set to 0. A long unsigned int is used because when calling the size function on a vector it returns the same datatype. The last two general variables are two vectors. `vector<T>: buffer` is a generic vector which is used to store the values in the buffer, `vector<string>: logger` is used to keep logs on the execution of operations. The logger does not have to be generic since logs are always stored in the form of strings.

## Buffer - Concurrency
The buffer utilizes another set of variables to prevent concurrency problems. The `int: nReaders` keeps track of the amount of threads that are currently executing a reading operation. The buffer also uses three mutex (mutual exclusion) locks. The `mutex: m_nReaders` is used to guarantee mutual exclusion between readers, this lock is only necessary when reading or updating the `nReaders` variable. `mutex: currentlyReading` is used to check whether there are any threads that are currently reading and `mutex: m_mod_queued` to check if there is any thread that wants to execute a modifying operation.

## Group 1 - Read
In this buffer modifying operations take a higher priority than reading operations. This means that threads are only able to read if and only if there are no other threads queued or are currently executing a modifying operation, which is checked on `m_mod_queued`. If the reader is able to acquire the lock this is not the case, and the thread can execute the reader operation. If it is the thread waits till the modifying operation(s) is/are finished and is able to acquire the lock. 

The second thing a thread needs to know is if there are currently other reading threads. This can be done by checking `nReaders`. When the variable is set to 0 there are no thread currently reading, meaning the current thread has to lock `m_currentlyReading` and increment `nReaders`. If it is set to 1 there are already other threads reading and the current one only has to increment `nReaders`. The mutex `m_nReaders` is used to ensure different reading threads aren't altering `nReaders` concurrently.

After checking if the index value that has been given as an argument is valid the thread can begin its reading operation. If not, an error is thrown. 

After finishing it's reading operation the thread checks the value of `nReaders` again. If it's equal to 1, the current thread is the only thread executing a reading operation. `m_currentlyReading` can be unlocked and `nReaders` decremented. If it is not equal to 1 there are still other threads reading, so the current thread only has to decrement. The mutex `m_nReaders` is used to ensure different reading threads aren't altering `nReaders` concurrently.

## Group 2 - Modification
Every operation classified in group 2 (write, remove, bind, unbind) follow the same mutex pattern when maintaining mutual exclusion. 

Before a thread can execute a modifying operation on the buffer it has to make sure there are no other threads currently reading and/or executing other modifying operations. This is done by `m_mod_queued.lock()`. When the thread receives the lock other threads that want to read of modify the buffer have to wait till the current thread has finished its modification and released the lock. However, it is still possible that there are threads in the middle of a reading operation. This is why the current thread also calls `m_currentlyReading.lock()`. When the thread has both locks it can execute its modifying operation without external interference. After finishing it releases both locks.

### Bind / Unbind
When a vector in C++ is initialized an integer can be given which binds the capacity of the vector at a certain size. When no argument is given it does not have a bound. The generic vector that is used to store the buffer values also does not have a bound. When the buffer is bound the vector stays infinite but the buffer manually keeps track of the size of the vector and checks if a write operation will exceeds this bound. When the `unbind()` function is called the bound is removed and the buffer will always allow write operations. This because the vector that stores the values has no bound.

When binding the buffer the buffer will reinstate the checks on the write operations. Unbinding is only allowed when the bind is equal of greater than the current buffer size. Otherwise elements in the buffer will be deleted.

## Buffer - Exceptions
To add a little extra functionality the buffer features a couple of custom exceptions. These are used by operations throughout the buffer when for example an illegal argument is given. The operation can set its own custom error message and throws the corresponding exception.

# Concurrency
In this chapter the resilience of the buffer against starvation and deadlocks is discussed.

## Deadlocks
To prove this buffer does not suffer from deadlocks proof will be provided that there is no circular waiting. 

When threads execute a modifying operation both locks that are acquired in the process will always be released when the operation finishes. Threads that execute reading operations however, are able to execute concurrently. Making the process of attaining and releasing locks different per thread. Though, the last concurrent reader will always make sure all the locks are released. Because in both cases all locks will be released eventually circular waiting will not occur. Thus, deadlocks are not possible.

## Starvation
To prevent starvation from occuring while using the buffer a couple of measures have been taken. The first one being the fact that readers are able to execute concurrently. Minimizing the time reading threads have to wait for each other. To ensure modifying threads are also able to execute without suffering from starvation this type of thread is given priority over reading threads. This way both types of threads are able to execute their tasks without suffering from starvation. 

# Testing
To ensure the correct functionality of the implemented buffer below a few test cases are defined. These test cases could be implemented by future developers.

The test cases below all assume the buffer is executed in a multi-threaded environment. Tests for a single-threaded environment are not defined. This is because when the buffer works correctly when used by multiple threads the same goes for a single thread.

<table>
    <tr>
        <th>n</th>
        <th>Test description</th>
        <th>Expected output</th>
    </tr>
    <tr>
        <td>1</td>
        <td>Five threads try to simultaneously increment the same element (assuming the element is a numeric) in the buffer by 10.</td>
        <td>The original value of the element should be increased by 50 and the logging should be maintained correctly.</td>
    </tr>
     <tr>
        <td>2</td>
        <td>Two threads try to simultaneously remove the same element on the buffer.</td>
        <td>After execution of the first thread the element should be removed. After the execution of the second thread an `BufferOutOfBounds` exception should be thrown and there should also be a new entry in the logger.</td>
    </tr>
     <tr>
        <td>3</td>
        <td>Two threads try to simultaneously set a bound of 10 on an unbounded buffer.</td>
        <td>After execution of the first thread a bound of 10 is set on the buffer. After execution of the second thread an `BufferBoundException` should be thrown, since the buffer already has a bound. There should also be a new entry in the logger.</td>
    </tr>
     <tr>
      <td>4</td>
        <td>Two threads try to simultaneously remove the bound on a bounded buffer.</td>
        <td>After execution of the first thread the bound on the buffer should be removed. After execution of the second thread an `BufferBoundException` should be thrown, since the buffer has no bound. There should also be a new entry in the logger.</td>
    </tr>
     <tr>
        <td>5</td>
        <td>A thread tries to read an item with an index that is either negative or greater than the buffersize.</td>
        <td>An `BufferOutOfBoundsException` is thrown since the buffer has no element associated with this index. There should also be an entry in the logger.</td>
    </tr>
     <tr>
        <td>6</td>
        <td>A thread tries to write an item to an index that is either negative or greater than the buffersize.</td>
        <td>An `BufferOutOfBoundsException` is thrown since the buffer has no such index. There should also be a new entry in the logger.</td>
    </tr>
     <tr>
        <td>7</td>
        <td>A thread tries to remove an item from an index that is either negative or greater than the buffersize.</td>
        <td>An `BufferOutOfBoundsException` is thrown since the buffer has no such index. There should also be a new entry in the logger.</td>
    </tr>
     <tr>
        <td>8</td>
        <td>A thread tries to add a bound to the buffer that is negative either negative or lesser than the current buffersize.</td>
        <td>An `BufferOutOfBoundsException` is thrown since the buffer does not allow such bounds. There should also be a new entry in the logger.</td>
    </tr>
     <tr>
        <td>9</td>
        <td>A thread tries to add a bound to the buffer that is negative either negative or lesser than the current buffersize.</td>
        <td>An `BufferOutOfBoundsException` is thrown since the buffer does not allow such bounds. There should also be a new entry in the logger.</td>
    </tr>
    <tr>
        <td>10</td>
        <td>A thread tries to alter the bound with either `bind()` or `unbind()` while there are modification threads active.</td>
        <td>Given the argument for the `bind()` and other threads function is valid, all actions complete successfully. The logging has also been maintained correctly.</td>
    </tr>
    <tr>
        <td>11</td>
        <td>Multiple threads try to simultaneously perform either a reading or writing operation on the buffer.</td>
        <td>Given all arguments given to the functions are valid; all threads should complete successfully. The logging should also have been correctly maintained.</td>
    </tr>
</table>



















