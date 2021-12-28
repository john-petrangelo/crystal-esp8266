# Create the build files
**$ cmake -S . -B build**
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
...
-- Build files have been written to: .../my_project/build

# Build the tests
**$ cmake --build build**
Scanning dependencies of target gtest
...
[100%] Built target gmock_main

# Run the tests
**$ ctest --test-dir build**
Test project .../my_project/build
    Start 1: HelloTest.BasicAssertions
1/1 Test #1: HelloTest.BasicAssertions ........   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
