/*	WARNING: COPYRIGHT (C) 2017 XSENS TECHNOLOGIES OR SUBSIDIARIES WORLDWIDE. ALL RIGHTS RESERVED.
	THIS FILE AND THE SOURCE CODE IT CONTAINS (AND/OR THE BINARY CODE FILES FOUND IN THE SAME
	FOLDER THAT CONTAINS THIS FILE) AND ALL RELATED SOFTWARE (COLLECTIVELY, "CODE") ARE SUBJECT
	TO A RESTRICTED LICENSE AGREEMENT ("AGREEMENT") BETWEEN XSENS AS LICENSOR AND THE AUTHORIZED
	LICENSEE UNDER THE AGREEMENT. THE CODE MUST BE USED SOLELY WITH XSENS PRODUCTS INCORPORATED
	INTO LICENSEE PRODUCTS IN ACCORDANCE WITH THE AGREEMENT. ANY USE, MODIFICATION, COPYING OR
	DISTRIBUTION OF THE CODE IS STRICTLY PROHIBITED UNLESS EXPRESSLY AUTHORIZED BY THE AGREEMENT.
	IF YOU ARE NOT AN AUTHORIZED USER OF THE CODE IN ACCORDANCE WITH THE AGREEMENT, YOU MUST STOP
	USING OR VIEWING THE CODE NOW, REMOVE ANY COPIES OF THE CODE FROM YOUR COMPUTER AND NOTIFY
	XSENS IMMEDIATELY BY EMAIL TO INFO@XSENS.COM. ANY COPIES OR DERIVATIVES OF THE CODE (IN WHOLE
	OR IN PART) IN SOURCE CODE FORM THAT ARE PERMITTED BY THE AGREEMENT MUST RETAIN THE ABOVE
	COPYRIGHT NOTICE AND THIS PARAGRAPH IN ITS ENTIRETY, AS REQUIRED BY THE AGREEMENT.
*/

#ifndef XSMUTEX_H
#define XSMUTEX_H

#include "xstypesconfig.h"

#ifdef XSENS_WINDOWS
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifndef __cplusplus
typedef struct XsMutex XsMutex;
#endif

struct XsMutex;

/*! \addtogroup cinterface C Interface
	@{
*/
/*! \brief Create a mutex */
static inline void XsMutex_construct(struct XsMutex* thisPtr);
/*! \brief Destroy a mutex */
static inline void XsMutex_destruct(struct XsMutex* thisPtr);
/*! \brief Lock the mutex
	\return non-zero on success, zero otherwise
*/
static inline int XsMutex_lock(struct XsMutex* thisPtr);
/*! \brief Try locking the mutex
	\return non-zero on success, zero otherwise
*/
static inline int XsMutex_tryLock(struct XsMutex* thisPtr);
/*! \brief Unlock a previously locked mutex
	\return non-zero on success, zero otherwise
*/
static inline int XsMutex_unlock(struct XsMutex* thisPtr);
/*! @} */

/*! \brief A platform independent mutex implementation

\details A mutex is used to protect data from being accessed by multiple threads at the same time.

This mutex is always recursive, which means that a single thread can
lock the same mutex multiple times. Of course it is important to unlock
the mutex just as many times before other threads can access the
protected data again.

XsMutex relies heavily on code inlining. The net result will be that any
dependent code will call platform code directly.

Typical usage in C++:
\code{.cpp}
class MyData
{
public:
	MyData() : m_value(0) {}

	void setValue(int newValue);
	int value() const;

private:
	int m_value;
	mutable XsMutex m_mutex;
};

void MyData::setValue(int newValue)
{
	m_mutex.lock();
	m_value = newValue;
	m_mutex.unlock();
}

int MyData::value() const
{
	m_mutex.lock();
	int v = m_value;
	m_mutex.unlock();
	return v;
}
\endcode


Typical usage in C:
\code{.c}
struct MyData
{
	int value;
	struct XsMutex mutex;
};

struct MyData d;
d.value = 0;
XsMutex_construct(&d.mutex);

XsMutex_lock(&d.mutex);
d.value = 3;
XsMutex_unlock(&d.mutex);

XsMutex_destruct(&d.mutex);
\endcode
*/
struct XsMutex
{
#ifdef __cplusplus
public:
	/*! \brief Create a mutex */
	inline XsMutex()
	{
		XsMutex_construct(this);
	}

	/*! \brief Destroy a mutex */
	inline ~XsMutex()
	{
		XsMutex_destruct(this);
	}

	/*! \brief Lock the mutex
		\return true on success
	*/
	inline bool lock()
	{
		return XsMutex_lock(this) != 0;
	}

	/*! \brief Try locking the mutex
		\return true on success
	*/
	inline bool tryLock()
	{
		return XsMutex_tryLock(this) != 0;
	}

	/*! \brief Unlock a previously locked mutex
		\return true on success
	*/
	inline bool unlock()
	{
		return XsMutex_unlock(this) != 0;
	}
private:
	friend inline void XsMutex_construct(struct XsMutex*);
	friend inline void XsMutex_destruct(struct XsMutex*);
	friend inline int XsMutex_lock(struct XsMutex*);
	friend inline int XsMutex_tryLock(struct XsMutex*);
	friend inline int XsMutex_unlock(struct XsMutex*);
#endif
#ifdef XSENS_WINDOWS
	CRITICAL_SECTION m_mutex;
#else
	pthread_mutexattr_t m_attr;
	pthread_mutex_t m_mutex;
#endif
};

#ifdef XSENS_WINDOWS
static inline void XsMutex_construct(struct XsMutex* thisPtr)
{
	InitializeCriticalSection(&thisPtr->m_mutex);
}

static inline void XsMutex_destruct(struct XsMutex* thisPtr)
{
	DeleteCriticalSection(&thisPtr->m_mutex);
}

static inline int XsMutex_lock(struct XsMutex* thisPtr)
{
	EnterCriticalSection(&thisPtr->m_mutex);
	return TRUE;
}

static inline int XsMutex_tryLock(struct XsMutex* thisPtr)
{
	return TryEnterCriticalSection(&thisPtr->m_mutex) == 1;
}

static inline int XsMutex_unlock(struct XsMutex* thisPtr)
{
	LeaveCriticalSection(&thisPtr->m_mutex);
	return TRUE;
}
#else
static inline void XsMutex_construct(struct XsMutex* thisPtr)
{
	pthread_mutexattr_init(&thisPtr->m_attr);
	pthread_mutexattr_settype(&thisPtr->m_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&thisPtr->m_mutex, &thisPtr->m_attr);
}

static inline void XsMutex_destruct(struct XsMutex* thisPtr)
{
	pthread_mutex_destroy(&thisPtr->m_mutex);
	pthread_mutexattr_destroy(&thisPtr->m_attr);
}

static inline int XsMutex_lock(struct XsMutex* thisPtr)
{
	return pthread_mutex_lock(&thisPtr->m_mutex) == 0;
}

static inline int XsMutex_tryLock(struct XsMutex* thisPtr)
{
	return pthread_mutex_trylock(&thisPtr->m_mutex) == 0;
}

static inline int XsMutex_unlock(struct XsMutex* thisPtr)
{
	return pthread_mutex_unlock(&thisPtr->m_mutex) == 0;
}
#endif

#ifdef __cplusplus
/*! \brief A RAII object that ensures mutexes are unlocked when leaving scope
 *
 * When using this object, it is advised not to touch the mutex we're working
 * on. The internal states may mismatch and result in undefined behavior.
 */
struct XsMutexLocker
{
public:
	/*! \brief Create a mutex locker and lock the mutex */
	explicit XsMutexLocker(XsMutex& m) :
		m_mutex(m),
		m_locked(false)
	{
		lock();
	}

	/*! \brief Destroy the mutex locker -- unlock the mutex */
	~XsMutexLocker()
	{
		unlock();
	}

	/*! \brief Lock the mutex if it isn't already locked */
	inline bool lock()
	{
		if (!m_locked)
			return (m_locked = m_mutex.lock());
		return true;
	}

	/*! \brief Unlock the mutex if it isn't already unlocked */
	inline bool unlock()
	{
		if (m_locked)
			return !(m_locked = !m_mutex.unlock());
		return true;
	}

	/*! \brief Try to lock the mutex */
	inline bool tryLock()
	{
		if (!m_locked)
			return (m_locked = m_mutex.tryLock());
		return true;
	}

private:
	XsMutex& m_mutex;
	bool m_locked;
};
#endif

#endif
