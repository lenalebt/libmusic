#ifndef PTHREAD_HPP_
#define PTHREAD_HPP_

#include <pthread.h>
#include <queue>

class PThreadWaitCondition;

/**
 * @brief Implements a mutex.
 *
 * Call lock() to lock it, call unlock() to unlock it.
 *
 * @remarks This lock is not recursive. You cannot lock the mutex again
 * if you already hold it, doing so will result in a deadlock.
 * 
 * @ingroup tools
 */
class PThreadMutex
{
private:
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexAttr;
public:
    PThreadMutex();
    virtual ~PThreadMutex();

    /**
     * Locks this mutex.
     */
    void lock();
    /**
     * Unlocks this mutex.
     */
    void unlock();
    
    friend class PThreadWaitCondition;
};

/**
 * @brief This is similar to a QMutexLocker.
 *
 * It locks a mutex upon creation,
 * and unlocks it on deletion. You may use it to unlock a mutex after
 * having called <code>return</code>, emulating the Java style of unlocking
 * a mutex after exiting a function via try/finally.
 * 
 * @ingroup tools
 */
class PThreadMutexLocker
{
private:
    PThreadMutex* mutex;
public:
    PThreadMutexLocker(PThreadMutex* mutex);
    ~PThreadMutexLocker();
};

class PThreadWaitCondition
{
private:
    pthread_cond_t _cond;
protected:
    
public:
    PThreadWaitCondition();
    void wakeAll();
    void wakeOne();
    void wait(PThreadMutex* mutex);
};



void* runPThread(void*);
/**
 * @brief Implements a thread somehow similar to the Java Thread API.
 *
 * @remarks Uses the pthreads API.
 * 
 * @ingroup tools
 */
class PThread
{
private:
    pthread_t thread;
    pthread_attr_t threadAttr;
    PThreadMutex runThreadMutex;
    bool runThread;
protected:
    /**
     * @brief Sets the runThread variable (uses a mutex)
     * @todo go and use a spinlock instead!
     */
    void setRunThread(bool runThread);
    /**
     * @brief Gets the runThread variable (uses a mutex)
     * @todo go and use a spinlock instead!
     */
    bool isRunThread();
public:
    PThread();
    virtual ~PThread();

    /**
     * @brief This function will be called as the thread's payload and be executed
     * in the new thread's context.
     *
     * @remarks Don't call it yourself, use start().
     */
    virtual void run()=0;
    /**
     * @brief Use this function to start this thread.
     *
     * Automatically calls run().
     */
    void start();

    /**
     * @brief Wait at maximum <code>msecs</code> milliseconds for a thread to complete.
     * If <code>msecs < 0</code>, this function waits for an unlimited
     * amount of time.
     * @bug Waiting for a limited time only works on QNX for now, on other
     *   POSIX systems, this call will wait until the thread completes.
     */
    bool join(long msecs=-1l);

    /**
     * @brief Shuts this thread down by requesting to end its event loop.
     *
     * This
     * is different to the approach Java follows via its deprecated <code>stop()</code>
     * function, as it does not force the thread to end.
     */
    void shutdown();

    friend void* runPThread(void*);
};

/**
* @brief Implementation of a thread-safe blocking queue with limited capacity.
*
* @author Lena Brüder
* @date 2011-11-07
* @ingroup tools
*/

template <typename T>
class BlockingQueue
{
private:
    unsigned int _size;
    unsigned int _elementCount;
    std::queue<T> _queue;
    PThreadMutex _mutex;
    PThreadWaitCondition _notFull;
    PThreadWaitCondition _notEmpty;
    bool _queueDestroyed;
    
    BlockingQueue(const BlockingQueue<T>& queue) :
        _size(queue._size), _elementCount(queue._elementCount),
        _queue(queue._queue), _mutex(), _notFull(),
         _notEmpty(), _queueDestroyed(queue._queueDestroyed)
    {
        
    }
    
public:
    /**
    * @brief Initializes the queue with capacity <code>size</code>.
    * @param size Determines the number of elements that may be stored in the queue.
    */
    BlockingQueue(int size) : _size(size), _elementCount(0),
        _queue(), _mutex(), _notFull(),
         _notEmpty(), _queueDestroyed(false)
    {
        
    }
    
    /**
    * @brief Removes an element from the queue.
    *
    * Blocks, if no element is in the queue. Returns <code>false</code>
    * if the queue was destroyed and thus will never contain an element again.
    *
    * To remove an element, use it this way:
    * @code
    * BlockingQueue<int> queue;
    * int a = 5;
    * queue.enqueue(3);
    * queue.enqueue(5);
    * queue.enqueue(7);
    *
    * while(queue.dequeue(a))
    * {
    * std::cout << a << " ";
    * }
    * //Output: 3 5 7
    * @endcode
    *
    * @param[out] t Here, the element from the queue will be saved, if any.
    * @return <code>true</code>, if removing the element from the queue was successful,
    * <code>false</code> sonst.
    * @see destroyQueue()
    */
    bool dequeue (T& t);
    /**
    * @brief Adds an element to the queue.
    * 
    * Blocks, if the queue is full. If the queue was destroyed and thus
    * no element should be added to the queue, it returns <code>false</code>.
    *
    * @param t The element that should be added to the queue.
    * @return If adding the element was successful, or not.
    * @see destroyQueue()
    */
    bool enqueue ( const T & t );
    /**
    * @brief Destroys the queue and will wake up all blocking operations.
    */
    void destroyQueue();
    /**
    * @brief Tells if the queue was destroyed, or not.
    * @return if the queue was destroyed.
    */
    bool isDestroyed();

    BlockingQueue<T>& operator<<(const T& element);
    T& operator>>(T& element);
};

#endif /* PTHREAD_HPP_ */
