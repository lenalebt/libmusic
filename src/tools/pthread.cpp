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
    std::cout << "trying to start thread..." << std::endl;
    int rc;
    pthread_attr_init( &threadAttr );
    rc = pthread_create(&thread, /*&threadAttr*/ NULL, runPThread, this);
    if (rc==0)
        std::cout << "thread started..." << std::endl;
    else
        std::cout << "thread NOT started..." << std::endl;
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
