# CMake generated Testfile for 
# Source directory: C:/Users/Public/Public_desktop/qlc3d_new/qlc3d
# Build directory: C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(single-iteration_[integration-test] "C:/Users/badvries/AppData/Local/Programs/Python/Python313/python.exe" "python/e2e_single-newton-iteration.py" "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/qlc3d/qlc3d.exe")
set_tests_properties(single-iteration_[integration-test] PROPERTIES  WORKING_DIRECTORY "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/tests" _BACKTRACE_TRIPLES "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;22;add_test;C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;0;")
add_test(switch-1d_[integration-test] "C:/Users/badvries/AppData/Local/Programs/Python/Python313/python.exe" "python/e2e_1d-cell-switching.py" "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/qlc3d/qlc3d.exe")
set_tests_properties(switch-1d_[integration-test] PROPERTIES  WORKING_DIRECTORY "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/tests" _BACKTRACE_TRIPLES "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;27;add_test;C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;0;")
add_test(cpp-test "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/tests/cpp-test.exe")
set_tests_properties(cpp-test PROPERTIES  WORKING_DIRECTORY "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/tests" _BACKTRACE_TRIPLES "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;32;add_test;C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/CMakeLists.txt;0;")
subdirs("extern/fmt-8.1.1")
subdirs("extern/SpaMtrix")
subdirs("qlc3d")
subdirs("tests")
