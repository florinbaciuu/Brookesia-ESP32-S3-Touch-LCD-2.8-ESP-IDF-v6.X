#include "esp_pthread.h"
#include "common.hpp"
#include <boost/detail/lightweight_test.hpp>

void common_init()
{
  common_set_pthread_config(4 * 1024, true);
  boost::detail::test_results().errors() = 0;
}

void common_set_pthread_config(int stack, bool is_inherit)
{
    auto cfg = esp_pthread_get_default_config();
    cfg.stack_size = stack;
    cfg.inherit_cfg = is_inherit;
    esp_pthread_set_cfg(&cfg);
}

void common_delay(uint32_t seconds)
{
  if (seconds == 0) {
    seconds = 2;
  }
  boost::this_thread::sleep_for(boost::chrono::seconds(seconds));
}
