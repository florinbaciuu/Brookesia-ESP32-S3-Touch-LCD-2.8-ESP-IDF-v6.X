#pragma once

#include <thread>
#include "boost/thread.hpp"
#include "common_components.hpp"

#undef BOOST_AUTO_TEST_CASE
#define BOOST_AUTO_TEST_CASE(name) TEST_CASE("thread : example : " #name, "[thread][example][" #name "]")

void common_init();
void common_set_pthread_config(int stack, bool is_inherit);
void common_delay(uint32_t seconds = 0);
