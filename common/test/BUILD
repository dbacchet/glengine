cc_test(
    name = 'common_core_unit_tests',
    srcs = ['test_core.cpp',
           ],
    deps = ['//common:core',
            '@googletest//:gtest_main']
)
cc_test(
    name = 'common_math_unit_tests',
    srcs = ['test_polyline.cpp',
            'test_math_utils.cpp',
            'test_trajectory.cpp',
           ],
    deps = ['//common:core',
            '//common:math',
            '@googletest//:gtest_main']
)
