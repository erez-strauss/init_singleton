
#include <app_singletons.h>
#include <gtest/gtest.h>

TEST(AppSingletons, args)
{
    es::init::singleton<int>::instance();
    es::init::args.for_each(
        [](int index, auto& arg) { std::cout << "lambda arg[" << index << "]: '" << arg << "'" << std::endl; });

    EXPECT_TRUE(4 == es::init::details_static_instances_counting::global_static_instances_counter.load());
}
