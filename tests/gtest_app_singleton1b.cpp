
#include <app_singletons.h>
#include <gtest/gtest.h>

TEST(AppSingletons, env)
{
    es::init::singleton<long>::instance();
    es::init::env.for_each(
        [](int index, auto& earg) { std::cout << "lambda env[" << index << "]: '" << earg << "'" << std::endl; });

    EXPECT_TRUE(4 == es::init::details_static_instances_counting::global_static_instances_counter.load());
}
