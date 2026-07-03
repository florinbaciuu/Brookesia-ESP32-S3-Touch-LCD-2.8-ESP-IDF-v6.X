//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Copyright (C) 2011 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <boost/thread/locks.hpp>

// template <class Mutex> class shared_lock;

// template <class Rep, class Period>
//   bool try_lock_for(const chrono::duration<Rep, Period>& rel_time);

#include <boost/thread/lock_types.hpp>
//#include <boost/thread/shared_mutex.hpp>
#include <boost/detail/lightweight_test.hpp>

static bool shared_lock_unlock_pass_unlock_called = false;

struct SharedLockUnlockPassMutex
{
  void lock_shared()
  {
  }
  void unlock_shared()
  {
    shared_lock_unlock_pass_unlock_called = true;
  }
};

static SharedLockUnlockPassMutex m;

static int test_main()
{
  shared_lock_unlock_pass_unlock_called = false;
  boost::shared_lock<SharedLockUnlockPassMutex> lk(m);
  lk.unlock();
  BOOST_TEST(shared_lock_unlock_pass_unlock_called == true);
  BOOST_TEST(lk.owns_lock() == false);
  try
  {
    lk.unlock();
    BOOST_TEST(false);
  }
  catch (boost::system::system_error& e)
  {
    BOOST_TEST(e.code().value() == boost::system::errc::operation_not_permitted);
  }
  lk.release();
  try
  {
    lk.unlock();
    BOOST_TEST(false);
  }
  catch (boost::system::system_error& e)
  {
    BOOST_TEST(e.code().value() == boost::system::errc::operation_not_permitted);
  }

  return boost::report_errors();
}

#include "common.hpp"

BOOST_AUTO_TEST_CASE(sync/mutual_exclusion/locks/shared_lock, unlock_pass)
{
    common_init();
    std::thread([&]() {
        TEST_ASSERT(test_main() == 0);
    }).join();
    common_delay();
}
