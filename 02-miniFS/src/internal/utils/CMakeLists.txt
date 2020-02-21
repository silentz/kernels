cmake_minimum_required(VERSION 3.0)

# ========== [ PARENT PROJECT ] ==========

set(SRC_LIST ${SRC_LIST} src/internal/utils/utils.c PARENT_SCOPE)

# ========== [ LOCAL ] ==========

add_executable(utils-test utils-test.c utils.c)

enable_testing()

add_test(UtilsTest utils-test)
set_tests_properties(UtilsTest PROPERTIES
	PASS_REGULAR_EXPRESSION "[GLOBAL OK]"
	FAIL_REGULAR_EXPRESSION "[BAD]")
