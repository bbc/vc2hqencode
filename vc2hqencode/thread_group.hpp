/*****************************************************************************
 * thread_group.hpp : Thread Group class
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

#ifndef __THREAD_GROUP_HPP__
#define __THREAD_GROUP_HPP__

#include <thread>
#include <algorithm>

class thread_group {
public:
  thread_group() : mThreads() {}
  ~thread_group() {
    std::for_each(mThreads.begin(), mThreads.end(), [](std::thread *t){ delete t; });
  }

  template<class F> std::thread *create_thread(F f) {
    std::thread *t = new std::thread(f);
    mThreads.push_back(t);
    return t;
  }

  void join_all() {
    std::for_each(mThreads.begin(), mThreads.end(), [](std::thread *t){ t->join(); });
  }

private:
  std::vector<std::thread *> mThreads;
};

#endif /* __THREAD_GROUP_HPP__ */
