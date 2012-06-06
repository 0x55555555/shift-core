#include "sprocessmanager.h"
#include "spropertyinformation.h"
#include "sproperty.h"
#include "QDebug"
#include "QThreadPool"
#include "QRunnable"
#include "QThread"
#include "QCoreApplication"

class WorkerThread : public QRunnable
  {
XProperties:
  XProperty(SProperty *, job, setJob);

public:
  WorkerThread();
  virtual void run();
  };

struct SProcessManagerData
  {
  XVector <WorkerThread> workers;
  XVector <WorkerThread *> spareWorkers;
  QThreadPool pool;
  QMutex criticalLock;
  };

SProcessManagerData *g_manager = 0;

WorkerThread::WorkerThread() : _job(0)
  {
  xAssert(g_manager);
  QMutexLocker l(&g_manager->criticalLock);
  setAutoDelete(false);
  g_manager->spareWorkers << this;
  }

void WorkerThread::run()
  {
  xAssert(g_manager);

  SProperty *prop = 0;

  // lock scope
    {
    QMutexLocker l(&g_manager->criticalLock);

    xAssert(job());
    prop = job();
    setJob(0);
    }

  prop->preGet();

  // lock scope
    {
    QMutexLocker l(&g_manager->criticalLock);
    g_manager->spareWorkers << this;
    }
  }

void SProcessManager::initiate(xsize processes)
  {
  xAssert(g_manager == 0);
  g_manager = new SProcessManagerData;

  g_manager->spareWorkers.reserve(processes - 1);
  g_manager->workers.resize(processes - 1);
  g_manager->pool.setMaxThreadCount(processes - 1);
  }

void SProcessManager::terminate()
  {
  xAssert(g_manager);
  delete g_manager;
  g_manager = 0;
  }

SProcessManager::SProcessManager()
  {
  }

bool SProcessManager::isMainThread()
  {
  return QThread::currentThread() == QCoreApplication::instance()->thread();
  }

void SProcessManager::preCompute(const SPropertyInstanceInformation *info, SPropertyContainer *ent)
  {
  return;
  xAssert(g_manager);


  return;

    {
    QMutexLocker l(&g_manager->criticalLock);
    if(g_manager->spareWorkers.size() == 0)
      {
      return;
      }
    }

  SProperty *jobs[32];
  xsize numJobs = 0;
  info->queueCompute()(info, ent, jobs, numJobs);

  for(xsize i=0; i<numJobs; /* nothing here deliberately */)
    {
    SProperty *mainJob = jobs[i++];

    // lock scope
      {
      QMutexLocker l(&g_manager->criticalLock);

      bool success = true;
      while(success && g_manager->spareWorkers.size())
        {
        WorkerThread *worker = g_manager->spareWorkers.back();
        g_manager->spareWorkers.pop_back();
        xAssert(worker);
        xAssert(worker->job() == 0);

        while(i<numJobs && worker->job() == 0)
          {
          // load extra jobs onto worker threads
          if(!jobs[i]->_flags.hasFlag(SProperty::PreGetting) &&
              jobs[i]->_flags.hasFlag(SProperty::Dirty))
            {
            xAssert(jobs[i]);

            jobs[i]->_flags.setFlag(SProperty::PreGetting);
            worker->setJob(jobs[i]);
            }
          ++i;
          }

        if(worker->job())
          {
          success = g_manager->pool.tryStart(worker);

          // failed bacause no spare threads, rewind on this job, and wait a bit by doing something in this thread.
          if(!success)
            {
            worker->setJob(0);
            --i;
            g_manager->spareWorkers << worker;
            }
          }
        else
          {
          success = false;
          g_manager->spareWorkers << worker;
          }
        }

      xAssert(mainJob);
      if(!mainJob->_flags.hasFlag(SProperty::PreGetting) &&
         mainJob->_flags.hasFlag(SProperty::Dirty))
        {
        mainJob->_flags.setFlag(SProperty::PreGetting);
        }
      else
        {
        mainJob = 0;
        }
      }

    if(mainJob)
      {
      mainJob->preGet();
      }
    }

  QMutexLocker l(&g_manager->criticalLock);
  bool neededToWait = true;
  while(neededToWait)
    {
    neededToWait = false;
    for(xsize i=0; i<numJobs; ++i)
      {
      if(jobs[i]->_flags.hasFlag(SProperty::Dirty) ||
         jobs[i]->_flags.hasFlag(SProperty::PreGetting))
        {
        l.unlock();
        QThread::yieldCurrentThread();

        l.relock();
        neededToWait = true;
        }
      }
    }
  }
