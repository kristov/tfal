#include <stdint.h>
typedef struct test_harness test_harness_t;
struct test_harness {
    uint32_t passes;
    uint32_t failures;
    uint8_t verbose;
};
test_harness_t* test_harness_create();
void test_harness_destroy(test_harness_t* test);
void test_harness_make_note(test_harness_t* test, const char* note);
uint32_t test_harness_nr_failed_tests(test_harness_t* test);
void test_harness_report(test_harness_t* test);
void test_harness_exit_with_status(test_harness_t* test);
void is_equal_string(test_harness_t* test, char* provided, const char* desired, const char* test_name);
void is_equal_uint32(test_harness_t* test, uint32_t provided, uint32_t desired, const char* test_name);
void is_equal_uint64(test_harness_t* test, uint64_t provided, uint64_t desired, const char* test_name);
void is_equal_uint8(test_harness_t* test, uint8_t provided, uint8_t desired, const char* test_name);
void is_equal_float(test_harness_t* test, float provided, float desired, const char* test_name);
