#include "test_harness.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_harness_report(test_harness_t* test) {
    fprintf(stderr, "Ran %d tests:\n", test->failures + test->passes);
    fprintf(stderr, "  %d Failures\n", test->failures);
    fprintf(stderr, "  %d Passes\n", test->passes);
    if (0 == test->failures) {
        fprintf(stderr, "-- ALL TESTS PASS --\n");
    }
    else {
        fprintf(stderr, "-- SOME TESTS FAILED --\n");
        // Store the failed tests on an array and report them here
    }
}

void test_harness_exit_with_status(test_harness_t* test) {
    int exit_code = 0;

    if (test->verbose) {
        test_harness_report(test);
    }

    if (test->failures) {
        exit_code = test->failures;
    }

    test_harness_destroy(test);
    exit(exit_code);
}

uint32_t test_harness_nr_failed_tests(test_harness_t* test) {
    return test->failures;
}

void test_harness_destroy(test_harness_t* test) {
    free(test);
}

void test_harness_make_note(test_harness_t* test, const char* note) {
    if (test->verbose) fprintf(stderr, "-- %s --\n", note);
}

void is_equal_string(test_harness_t* test, char* provided, const char* desired, const char* test_name) {
    if (0 != strcmp(provided, desired)) {
        test->failures++;
        if (test->verbose) {
            fprintf(stderr, "FAIL: %s\n", test_name);
            fprintf(stderr, "    provided: '%s', expected: '%s'\n", provided, desired);
        }
    }
    else {
        test->passes++;
        if (test->verbose) fprintf(stderr, "pass: %s\n", test_name);
    }
}

void is_equal_uint32(test_harness_t* test, uint32_t provided, uint32_t desired, const char* test_name) {
    if (provided != desired) {
        test->failures++;
        if (test->verbose) {
            fprintf(stderr, "FAIL: %s\n", test_name);
            fprintf(stderr, "    provided: %d, desired: %d\n", provided, desired);
        }
    }
    else {
        test->passes++;
        if (test->verbose) fprintf(stderr, "pass: %s\n", test_name);
    }
}

void is_equal_uint64(test_harness_t* test, uint64_t provided, uint64_t desired, const char* test_name) {
    if (provided != desired) {
        test->failures++;
        if (test->verbose) {
            fprintf(stderr, "FAIL: %s\n", test_name);
            fprintf(stderr, "    provided: %lu, desired: %lu\n", provided, desired);
        }
    }
    else {
        test->passes++;
        if (test->verbose) fprintf(stderr, "pass: %s\n", test_name);
    }
}

void is_equal_uint8(test_harness_t* test, uint8_t provided, uint8_t desired, const char* test_name) {
    if (provided != desired) {
        test->failures++;
        if (test->verbose) {
            fprintf(stderr, "FAIL: %s\n", test_name);
            fprintf(stderr, "    provided: %d, desired: %d\n", provided, desired);
        }
    }
    else {
        test->passes++;
        if (test->verbose) fprintf(stderr, "pass: %s\n", test_name);
    }
}

void is_equal_float(test_harness_t* test, float provided, float desired, const char* test_name) {
    if (provided != desired) {
        test->failures++;
        if (test->verbose) {
            fprintf(stderr, "FAIL: %s\n", test_name);
            fprintf(stderr, "    provided: %0.4f, desired: %0.4f\n", provided, desired);
        }
    }
    else {
        test->passes++;
        if (test->verbose) fprintf(stderr, "pass: %s\n", test_name);
    }
}

void test_harness_init(test_harness_t* test) {
    memset(test, 0, sizeof(test_harness_t));
}

test_harness_t* test_harness_create() {
    test_harness_t* test;
    test = malloc(sizeof(test_harness_t));
    return test;
}
