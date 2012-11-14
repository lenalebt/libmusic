#include "pthread.hpp"

#include <iostream>

PThread::PThread() :
    runThread(true)
{
    // TODO Auto-generated constructor stub
    
}

void* runPThread(void* object)
{
    //((PThread*)object)->setRunThread(true);
    ((PThread*)object)->run();
    return 0;
}

PThread::~PThread()
{
    // TODO: Destroy pthread in a correct manner
}

void PThread::setRunThread(bool runThread)
{
    PThreadMutexLocker locker(&runThreadMutex);
    this->runThread = runThread;
}

bool PThread::isRunThread()
{
    PThreadMutexLocker locker(&runThreadMutex);
    return runThread;
}

void PThread::start()
{
    int rc;
    pthread_attr_init( &threadAttr );
    rc = pthread_create(&thread, /*&threadAttr*/ NULL, runPThread, this);
}
bool PThread::join(long msecs)
{
    void* retVal;
#ifdef __QNX__
    //this is only valid on qnx targets.
    if (msecs>0)
    {
        timespec then;
        clock_gettime(CLOCK_REALTIME, &then);
        then.tv_nsec += msecs * 1000000l;
        then.tv_sec += then.tv_nsec / 1000000000l;
        then.tv_nsec = then.tv_nsec % 1000000000l;
        return pthread_timedjoin(thread, &retVal, &then)/* == ETIMEDOUT*/;
    }
    else
    {
        pthread_join(thread, &retVal);
        return true;
    }
#else
    //this is valid on all platforms which support pthreads, but
    //blocks in every case until the thread ends.
    pthread_join(thread, &retVal);
    return true;
#endif
}

void PThread::shutdown()
{
    setRunThread(false);
}

PThreadMutex::PThreadMutex()
{
    pthread_mutex_init(&mutex , /*&mutexAttr*/ NULL);
}
PThreadMutex::~PThreadMutex()
{
    pthread_mutex_destroy(&mutex);
}
void PThreadMutex::lock()
{
    pthread_mutex_lock(&mutex);
}
void PThreadMutex::unlock()
{
    pthread_mutex_unlock(&mutex);
}

PThreadMutexLocker::PThreadMutexLocker(PThreadMutex* mutex)
{
    this->mutex = mutex;
    mutex->lock();
}
PThreadMutexLocker::~PThreadMutexLocker()
{
    mutex->unlock();
}

PThreadWaitCondition::PThreadWaitCondition()
{
    pthread_cond_init(&_cond, NULL);
}
void PThreadWaitCondition::wakeAll()
{
    pthread_cond_broadcast(&_cond);
}
void PThreadWaitCondition::wakeOne()
{
    pthread_cond_signal(&_cond);
}
void PThreadWaitCondition::wait(PThreadMutex* mutex)
{
    pthread_cond_wait(&_cond, &(mutex->mutex));
}

template <typename T>
bool BlockingQueue<T>::dequeue(T& t, bool block)
{
    PThreadMutexLocker locker(&_mutex);
    
    bool waitForElement=true;
    while (waitForElement)
    {
        if (_elementCount <= 0)
        {
            if (!_queueDestroyed)
            {
                if (block)
                    _notEmpty.wait(&_mutex);
                else
                    return false;
            }
            else
                return false;
        }
        else
            waitForElement = false;
    }
    
    _elementCount--;
    t = _queue.front();
    _queue.pop();
    _notFull.wakeAll();
    return true;
}

template <typename T>
bool BlockingQueue<T>::enqueue(const T& t)
{
    PThreadMutexLocker locker(&_mutex);
    
    if (_queueDestroyed)
        return false;
    
    bool waitForSpace=true;
    while (waitForSpace)
    {
        if (_elementCount >= _size)
        {
            if (!_queueDestroyed)
                _notFull.wait(&_mutex);
            else
                return false;
        }
        else
            waitForSpace = false;
    }
    
    _elementCount++;
    _queue.push(t);
    _notEmpty.wakeAll();
    return true;
}

template <typename T>
void BlockingQueue<T>::destroyQueue()
{
    PThreadMutexLocker locker(&_mutex);
    _queueDestroyed = true;
    _notFull.wakeAll();
    _notEmpty.wakeAll();
}

template <typename T>
bool BlockingQueue<T>::isDestroyed()
{
    PThreadMutexLocker locker(&_mutex);
    return _queueDestroyed;
}

template <typename T>
BlockingQueue<T>& BlockingQueue<T>::operator<<(const T& element)
{
    this->enqueue(element);
    return *this;
}
template <typename T>
T& BlockingQueue<T>::operator>>(T& element)
{
    this->dequeue(element);
    return element;
}

namespace music {namespace databaseentities {class Recording;} }

template class BlockingQueue<int>;
template class BlockingQueue<std::string>;
template class BlockingQueue<music::databaseentities::Recording*>;
