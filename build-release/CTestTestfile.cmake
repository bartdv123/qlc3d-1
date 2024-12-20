# CMake generated Testfile for 
# Source directory: C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode
# Build directory: C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(single-iteration_[integration-test] "C:/Users/badvries/AppData/Local/Programs/Python/Python313/python.exe" "python/e2e_single-newton-iteration.py" "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/build-release/qlc3d/qlc3d.exe")
set_tests_properties(single-iteration_[integration-test] PROPERTIES  WORKING_DIRECTORY "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/tests" _BACKTRACE_TRIPLES "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;22;add_test;C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;0;")
add_test(switch-1d_[integration-test] "C:/Users/badvries/AppData/Local/Programs/Python/Python313/python.exe" "python/e2e_1d-cell-switching.py" "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/build-release/qlc3d/qlc3d.exe")
set_tests_properties(switch-1d_[integration-test] PROPERTIES  WORKING_DIRECTORY "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/tests" _BACKTRACE_TRIPLES "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;27;add_test;C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;0;")
add_test(cpp-test "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/build-release/tests/cpp-test.exe")
set_tests_properties(cpp-test PROPERTIES  WORKING_DIRECTORY "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/build-release/tests" _BACKTRACE_TRIPLES "C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;32;add_test;C:/Users/badvries/OneDrive - UGent/Documents/QLC3Dsourcecode/CMakeLists.txt;0;")
subdirs("extern/fmt-8.1.1")
subdirs("extern/SpaMtrix")
subdirs("qlc3d")
subdirs("tests")
