#include "capa.h"
PMutex::PMutex()
{
    pthread_mutexattr_t attr;
    int rc;

    rc = pthread_mutexattr_init(&attr);
    assert(rc == 0);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    rc = pthread_mutex_init(&mutex, &attr);
    assert(rc == 0);
    pthread_mutexattr_destroy(&attr);
}

PMutex::~PMutex()
{
    pthread_mutex_destroy(&mutex);
}

bool PMutex::lock()
{
    int rc = pthread_mutex_lock(&mutex);

    assert(rc == 0);
    return rc == 0;
}

bool PMutex::trylock()
{
    return 0 == pthread_mutex_trylock(&mutex);
}

void PMutex::unlock()
{
    int rc = pthread_mutex_unlock(&mutex);

    assert(rc == 0);
    (void)rc;
}

pthread_mutex_t* PMutex::getMutex()
{
    return &mutex;
}
