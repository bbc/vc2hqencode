/*****************************************************************************
 * ThreadPool.hpp : Thread Pool
 *****************************************************************************
 * Copyright (C) 2014-2015 BBC
 *
 * Authors: James P. Weaver <james.barrett@bbc.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at ipstudio@bbc.co.uk.
 *****************************************************************************/

#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#if !defined _WIN32 && !defined __CYGWIN__
  #include "config.h"
#endif

#ifdef VC2_USE_BOOST
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>

#include <boost/asio.hpp>

using boost::asio::io_service;
using boost::thread;
using boost::thread_group;
using boost::barrier;
using boost::bind;
#else
#include <thread>
#include "threadbarrier.hpp"
#include "thread_group.hpp"
#define ASIO_STANDALONE

#include "asio/io_service.hpp"
#include <functional>

using asio::io_service;
using std::thread;
using std::bind;
#endif

#ifdef VC2_USE_NUMA
#include <sched.h>
#include <pthread.h>
#include <numa.h>
#endif

#include <atomic>

class ThreadPool {
public:
  typedef io_service::work Work;

  ThreadPool()
    : N(1)
    , start_barrier(1 + 1)
    , stop_barrier(1 + 1)
    , _stop(false)
    , exceptions(0) {
    for (int i = 0; i < N; i++) {
      threads.create_thread(bind(&ThreadPool::_run, this));
    }
  }

  ThreadPool(int n)
    : N(n)
    , start_barrier(n + 1)
    , stop_barrier(n + 1)
    , _stop(false)
    , exceptions(0) {
    for (int i = 0; i < N; i++) {
      threads.create_thread(bind(&ThreadPool::_run, this));
    }
  }

  ThreadPool(int n, int numa_first_node)
    : N(n)
    , start_barrier(n + 1)
    , stop_barrier(n + 1)
    , _stop(false)
    , exceptions(0) {
#ifdef VC2_USE_NUMA
    int n_cpus = numa_num_task_cpus();
    cpu_set_t cpus;
#endif
    for (int i = 0; i < N; i++) {
      thread * t = threads.create_thread(bind(&ThreadPool::_run, this));
#ifdef VC2_USE_NUMA
      pthread_t pt = t->native_handle();
      CPU_ZERO(&cpus);
      CPU_SET((numa_first_node + i)%n_cpus, &cpus);
      pthread_setaffinity_np(pt, sizeof(cpu_set_t), &cpus);
#else
      (void)numa_first_node;
      (void)t;
#endif
    }
  }

  void ready() {
    exceptions = 0;
    m_io_service.reset();
    work = new Work(m_io_service);
    start_barrier.wait();
  }

  template <typename F> void post(F f) { m_io_service.post(f); }

  void _run() {
    while(true) {
      start_barrier.wait();
      if (_stop)
        return;

      try {
        m_io_service.run();
      } catch(...) {
        ++exceptions;
      }
      if (_stop)
        return;

      stop_barrier.wait();
      if (_stop)
        return;
    }
  }

  int execute() {
    if (work)
      delete work;
    work = NULL;

    stop_barrier.wait();
    return exceptions;
  }

  void stop() {
    _stop = true;
#ifndef VC2_USE_BOOST
    start_barrier.unblock();
    stop_barrier.unblock();
#endif
    if (work)
      delete work;
    m_io_service.stop();
#ifdef VC2_USE_BOOST
    threads.interrupt_all();
#endif
    threads.join_all();
  }

  ~ThreadPool() {
  }

protected:
  int N;

  io_service m_io_service;
  thread_group threads;
  Work *work;

  barrier start_barrier;
  barrier stop_barrier;
  bool _stop;

  std::atomic<int> exceptions;
};

#endif /* __THREAD_POOL_HPP__ */
