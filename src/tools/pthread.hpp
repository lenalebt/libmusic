#ifndef PTHREAD_HPP_
#define PTHREAD_HPP_

#include <pthread.h>

/**
 * @brief Implements a mutex.
 *
 * Call lock() to lock it, call unlock() to unlock it.
 *
 * @remarks This lock is not recursive. You cannot lock the mutex again
 * if you already hold it, doing so will result in a deadlock.
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
};

/**
 * @brief This is similar to a QMutexLocker.
 *
 * It locks a mutex upon creation,
 * and unlocks it on deletion. You may use it to unlock a mutex after
 * having called <code>return</code>, emulating the Java style of unlocking
 * a mutex after exiting a function via try/finally.
 */
class PThreadMutexLocker
{
private:
    PThreadMutex* mutex;
public:
    PThreadMutexLocker(PThreadMutex* mutex);
    ~PThreadMutexLocker();
};

static void* runPThread(void*);

/**
 * @brief Implements a thread somehow similar to the Java Thread API.
 *
 * @remarks Uses the pthreads API.
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

#endif /* PTHREAD_HPP_ */
