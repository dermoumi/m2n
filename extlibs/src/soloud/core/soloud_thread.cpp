/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include "soloud.h"
#include "soloud_thread.h"

#include <thread>
#include <mutex>

namespace SoLoud
{
	namespace Thread
	{
        struct ThreadHandleData
        {
            std::thread thread;
        };

		void * createMutex()
		{
			return new std::mutex();
		}

		void destroyMutex(void *aHandle)
		{
			delete static_cast<std::mutex*>(aHandle);
		}

		void lockMutex(void *aHandle)
		{
			static_cast<std::mutex*>(aHandle)->lock();
		}

		void unlockMutex(void *aHandle)
		{
			static_cast<std::mutex*>(aHandle)->unlock();
		}

        ThreadHandle createThread(threadFunction aThreadFunction, void *aParameter)
		{
            return new ThreadHandleData {
            	std::thread(aThreadFunction, aParameter)
            };
		}

		void sleep(int aMSec)
		{
		    auto time = std::chrono::duration<double>(aMSec / 1000.0);
		    std::this_thread::sleep_for(time);
		}

        void wait(ThreadHandle aThreadHandle)
        {
        	if (aThreadHandle) aThreadHandle->thread.detach();
        }

        void release(ThreadHandle aThreadHandle)
        {
            delete aThreadHandle;
        }

		static void poolWorker(void *aParam)
		{
			Pool *myPool = (Pool*)aParam;
			while (myPool->mRunning)
			{
				PoolTask *t = myPool->getWork();
				if (!t)
				{
					sleep(1);
				}
				else
				{
					t->work();
				}
			}
		}

		Pool::Pool()
		{
			mRunning = 0;
			mThreadCount = 0;
			mThread = 0;
			mWorkMutex = 0;
			mRobin = 0;
			mMaxTask = 0;
		}

		Pool::~Pool()
		{
			mRunning = 0;
			int i;
			for (i = 0; i < mThreadCount; i++)
			{
				wait(mThread[i]);
				release(mThread[i]);
			}
			delete[] mThread;
			if (mWorkMutex)
				destroyMutex(mWorkMutex);
		}

		void Pool::init(int aThreadCount)
		{
			if (aThreadCount > 0)
			{
				mMaxTask = 0;
				mWorkMutex = createMutex();
				mRunning = 1;
				mThreadCount = aThreadCount;
				mThread = new ThreadHandle[aThreadCount];
				int i;
				for (i = 0; i < mThreadCount; i++)
				{
					mThread[i] = createThread(poolWorker, this);
				}
			}
		}

		void Pool::addWork(PoolTask *aTask)
		{
			if (mThreadCount == 0)
			{
				aTask->work();
			}
			else
			{
				if (mWorkMutex) lockMutex(mWorkMutex);
				if (mMaxTask == MAX_THREADPOOL_TASKS)
				{
					// If we're at max tasks, do the task on calling thread 
					// (we're in trouble anyway, might as well slow down adding more work)
					if (mWorkMutex) unlockMutex(mWorkMutex);
					aTask->work();
				}
				else
				{
					mTaskArray[mMaxTask] = aTask;
					mMaxTask++;
					if (mWorkMutex) unlockMutex(mWorkMutex);
				}
			}
		}

		PoolTask * Pool::getWork()
		{
			PoolTask *t = 0;
			if (mWorkMutex) lockMutex(mWorkMutex);
			if (mMaxTask > 0)
			{
				int r = mRobin % mMaxTask;
				mRobin++;
				t = mTaskArray[r];
				mTaskArray[r] = mTaskArray[mMaxTask - 1];
				mMaxTask--;
			}
			if (mWorkMutex) unlockMutex(mWorkMutex);
			return t;
		}
	}
}
