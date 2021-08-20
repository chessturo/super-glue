/* Provides tests for `process_args.c`
   Copyright 2021 Mitchell Levy

This file is a part of super-glue

super-glue is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

super-glue is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with super-glue.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE
#include "test_process_args.h"

#include <arpa/inet.h>
#include <check.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "process_args.h"
#include "state.h"

// Helper to calculate argc based on argv
#define NUM_ELTS(x) (sizeof(x) / sizeof(*(x)))

// Helper constants that are passed as arguments.
static char *prog_name;
static char *basic_file;
static State *state;
static ConfigFiles *files;
static char *error;

static void common_setup() {
  prog_name = "super-glue";
  basic_file = "test/res/basic.sg";
  state = NULL;
  files = NULL;
  error = NULL;
}

static void common_teardown() {
  free_state(state);
  free_config_files(files);
  free(error);
}

// Core test case
static char *bogus_opt_name;
static char *bogus_opt_short;

static void core_setup() {
  common_setup();
  bogus_opt_name = "--bogus-arg";
  bogus_opt_short = "-Z";
}

static void core_teardown() {
  common_teardown();
}

START_TEST(no_args) {
  ArgsResult res = process_args(1, &prog_name, &state, &files, &error);
  ck_assert_msg(res == ARGS_NONE, "When passed no arguments, process_args "
      "should return ARGS_NONE");
} END_TEST

START_TEST(bogus_opt) {
  char *args[] = { prog_name, bogus_opt_name };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, "
      "process_args should return ARGS_UNKNOWN");
} END_TEST

START_TEST(bogus_opt_with_info_eq) {
  char *bogus_opt_with_info;
  ck_assert(asprintf(&bogus_opt_with_info, "%s=info", bogus_opt_name) > 0);

  char *args[] = { prog_name, bogus_opt_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");

  free(bogus_opt_with_info);
} END_TEST

START_TEST(bogus_opt_with_info_sp) {
  char *bogus_opt_with_info;
  ck_assert(asprintf(&bogus_opt_with_info, "%s info", bogus_opt_name) > 0);

  char *args[] = { prog_name, bogus_opt_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");

  free(bogus_opt_with_info);
} END_TEST

START_TEST(bogus_short_opt) {
  char *args[] = { prog_name, bogus_opt_short };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown short form "
      "option, process_args should return ARGS_UNKNOWN");
} END_TEST

START_TEST(bogus_short_opt_info_eq) {
  char *bogus_opt_with_info;
  ck_assert(asprintf(&bogus_opt_with_info, "%s=info", bogus_opt_short) > 0);

  char *args[] = { prog_name, bogus_opt_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");

  free(bogus_opt_with_info);
} END_TEST

START_TEST(bogus_short_opt_info_sp) {
  char *bogus_opt_with_info;
  ck_assert(asprintf(&bogus_opt_with_info, "%s info", bogus_opt_short) > 0);

  char *args[] = { prog_name, bogus_opt_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");

  free(bogus_opt_with_info);
} END_TEST

START_TEST(bogus_short_opt_info_adj) {
  char *bogus_opt_with_info;
  ck_assert(asprintf(&bogus_opt_with_info, "%sinfo", bogus_opt_short) > 0);

  char *args[] = { prog_name, bogus_opt_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");

  free(bogus_opt_with_info);
} END_TEST

// --version test case
static char *version;
static char *short_version;
static char *other;
static char *short_other;

static void version_setup() {
  common_setup();
  version = "--version";
  short_version = "-v";
  other = "--interactive";
  short_other = "-i";
}

static void version_teardown() {
  common_teardown();
}

START_TEST(version_no_files) {
  char *args[] = { prog_name, version };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);

  ck_assert_msg(res == ARGS_NO_FILES, "Using the version option by itself "
      "should result in ARGS_NO_FILES");
  ck_assert_msg(state->version_info_requested, "When requesting version, the "
      "global state should reflect that");
} END_TEST

START_TEST(short_version_no_files) {
  char *args[] = { prog_name, short_version };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);

  ck_assert_msg(res == ARGS_NO_FILES, "Using the version option by itself "
      "should result in ARGS_NO_FILES");
  ck_assert_msg(state->version_info_requested, "When requesting version, the "
      "global state should reflect that");
} END_TEST

START_TEST(version_with_files) {
  char *args[] = { prog_name, version, basic_file };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "When --version is given, "
      "process_args should fail if files are given.");
} END_TEST

START_TEST(short_version_with_files) {
  char *args[] = { prog_name, short_version, basic_file };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "When --version is given, "
      "process_args should fail if files are given.");
} END_TEST

START_TEST(version_with_info_eq) {
  char *version_with_info;
  ck_assert(asprintf(&version_with_info, "%s=info", version) > 0);

  char *args[] = { prog_name, version_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "When giving info to the version "
      "option, it should fail with ARGS_INVALID_USE");

  free(version_with_info);
} END_TEST

START_TEST(short_version_with_info_eq) {
  char *version_with_info;
  ck_assert(asprintf(&version_with_info, "%s=info", short_version) > 0);

  char *args[] = { prog_name, version_with_info };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "When giving info to the version "
      "option, it should fail with ARGS_INVALID_USE");

  free(version_with_info);
} END_TEST

START_TEST(version_with_other) {
  char *args[] = { prog_name, version, other };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_CONFLICT, "Combining --version with another "
      "option should fail with ARGS_CONFLICT");
} END_TEST

START_TEST(version_with_short_other) {
  char *args[] = { prog_name, version, short_other };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_CONFLICT, "Combining --version with another "
      "option should fail with ARGS_CONFLICT");
} END_TEST

START_TEST(short_version_with_other) {
  char *args[] = { prog_name, short_version, other };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_CONFLICT, "Combining -v with another "
      "option should fail with ARGS_CONFLICT");
} END_TEST

START_TEST(short_version_with_short_other) {
  char *args[] = { prog_name, short_version, short_other };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_CONFLICT, "Combining -v with another "
      "option should fail with ARGS_CONFLICT");
} END_TEST

// --port test case
static char *port;
static char *short_port;
static uint16_t default_port;
static uint16_t test_port;

static void port_setup() {
  common_setup();
  port = "--port";
  short_port = "-p";
  default_port = 80;
  test_port = 8080;
}

static void port_teardown() {
  common_teardown();
}

START_TEST(port_no_files) {
  char *args[] = { prog_name, port };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "Using --port without extra info "
      "should fail with ARGS_INVALID_USE");
} END_TEST

START_TEST(port_info_sp_no_files) {
  char *port_str;
  ck_assert(asprintf(&port_str, "%"PRIu16, test_port));
  char *args[] = { prog_name, port, port_str };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_NO_FILES, "Using --port without files should "
      "fail with ARGS_NO_FILES");

  free(port_str);
} END_TEST;

START_TEST(port_info_eq_no_files) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s=%"PRIu16, port, test_port) > 0);
  char *args[] = { prog_name, port_with_info };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_NO_FILES, "Using --port without files should "
      "fail with ARGS_NO_FILES");

  free(port_with_info);
} END_TEST

START_TEST(port_bare) {
  char *args[] = { prog_name, port, basic_file };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "Using --port without info should "
      "fail with ARGS_INVALID_USE");
} END_TEST

START_TEST(port_info_eq) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s=%"PRIu16, port, test_port) > 0);
  char *args[] = { prog_name, port_with_info, basic_file };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_OK, "Using --port with info when given files "
      "should succeed with ARGS_OK");
  ck_assert_msg(state->port == htons(test_port), "Using --port=port_num should "
      "properly set the port number");

  free(port_with_info);
} END_TEST

START_TEST(port_info_sp) {
  char *port_str;
  ck_assert(asprintf(&port_str, "%"PRIu16, test_port) > 0);
  char *args[] = { prog_name, port, port_str, basic_file };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_OK, "Using --port with info when given files "
      "should succeed with ARGS_OK");
  ck_assert_msg(state->port == htons(test_port), "Using --port=port_num should "
      "properly set the port number");

  free(port_str);
} END_TEST

START_TEST(short_port_no_files) {
  char *args[] = { prog_name, short_port };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "Using -p without extra info should "
      "fail with ARGS_INVALID_USE");
} END_TEST

START_TEST(short_port_info_sp_no_files) {
  char *port_str;
  ck_assert(asprintf(&port_str, "%"PRIu16, test_port));
  char *args[] = { prog_name, short_port, port_str };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_NO_FILES, "Using -p without files should fail with "
      "ARGS_NO_FILES");

  free(port_str);
} END_TEST;

START_TEST(short_port_info_eq_no_files) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s=%"PRIu16, short_port, test_port) > 0);
  char *args[] = { prog_name, port_with_info };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_NO_FILES, "Using -p without files should fail with "
      "ARGS_NO_FILES");

  free(port_with_info);
} END_TEST

START_TEST(short_port_info_adj_no_files) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s%"PRIu16, short_port, test_port) > 0);
  char *args[] = { prog_name, port_with_info };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_NO_FILES, "Using -p without files should fail with "
      "ARGS_NO_FILES");

  free(port_with_info);
} END_TEST

START_TEST(short_port_bare) {
  char *args[] = { prog_name, short_port, basic_file };
  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_INVALID_USE, "Using -p without info should fail "
      "with ARGS_INVALID_USE");
} END_TEST

START_TEST(short_port_info_sp) {
  char *port_str;
  ck_assert(asprintf(&port_str, "%"PRIu16, test_port) > 0);
  char *args[] = { prog_name, short_port, port_str, basic_file };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_OK, "Using -p with info when given files should "
      "succeed with ARGS_OK");
  ck_assert_msg(state->port == htons(test_port), "Using -p=port_num should "
      "properly set the port number");

  free(port_str);
} END_TEST

START_TEST(short_port_info_eq) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s=%"PRIu16, short_port, test_port) > 0);
  char *args[] = { prog_name, port_with_info, basic_file };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_OK, "Using -p with info when given files should "
      "succeed with ARGS_OK");
  ck_assert_msg(state->port == htons(test_port), "Using -p=port_num should "
      "properly set the port number");

  free(port_with_info);
} END_TEST

START_TEST(short_port_info_adj) {
  char *port_with_info;
  ck_assert(asprintf(&port_with_info, "%s%"PRIu16, short_port, test_port) > 0);
  char *args[] = { prog_name, port_with_info, basic_file };

  ArgsResult res = process_args(NUM_ELTS(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_OK, "Using -p with info when given files should "
      "succeed with ARGS_OK");
  ck_assert_msg(state->port == htons(test_port), "Using -p=port_num should "
      "properly set the port number");

  free(port_with_info);
} END_TEST

Suite *process_args_tests() {
  Suite *s = suite_create("process_args");

  TCase *tc_core = tcase_create("core");
  tcase_add_checked_fixture(tc_core, &core_setup, &core_teardown);
  tcase_add_test(tc_core, no_args);
  tcase_add_test(tc_core, bogus_opt);
  tcase_add_test(tc_core, bogus_opt_with_info_eq);
  tcase_add_test(tc_core, bogus_opt_with_info_sp);
  tcase_add_test(tc_core, bogus_short_opt);
  tcase_add_test(tc_core, bogus_short_opt_info_eq);
  tcase_add_test(tc_core, bogus_short_opt_info_sp);
  tcase_add_test(tc_core, bogus_short_opt_info_adj);
  suite_add_tcase(s, tc_core);

  TCase *tc_version = tcase_create("version");
  tcase_add_checked_fixture(tc_version, &version_setup, &version_teardown);
  tcase_add_test(tc_version, version_no_files);
  tcase_add_test(tc_version, version_with_info_eq);
  tcase_add_test(tc_version, version_with_files);
  tcase_add_test(tc_version, short_version_with_files);
  tcase_add_test(tc_version, short_version_no_files);
  tcase_add_test(tc_version, short_version_with_info_eq);
  tcase_add_test(tc_version, version_with_other);
  tcase_add_test(tc_version, version_with_short_other);
  tcase_add_test(tc_version, short_version_with_other);
  tcase_add_test(tc_version, short_version_with_short_other);
  suite_add_tcase(s, tc_version);

  TCase *tc_port = tcase_create("port");
  tcase_add_checked_fixture(tc_port, &port_setup, &port_teardown);
  tcase_add_test(tc_port, port_no_files);
  tcase_add_test(tc_port, port_info_sp_no_files);
  tcase_add_test(tc_port, port_info_eq_no_files);
  tcase_add_test(tc_port, port_bare);
  tcase_add_test(tc_port, port_info_eq);
  tcase_add_test(tc_port, port_info_sp);
  tcase_add_test(tc_port, short_port_no_files);
  tcase_add_test(tc_port, short_port_info_sp_no_files);
  tcase_add_test(tc_port, short_port_info_eq_no_files);
  tcase_add_test(tc_port, short_port_info_adj_no_files);
  tcase_add_test(tc_port, short_port_bare);
  tcase_add_test(tc_port, short_port_info_sp);
  tcase_add_test(tc_port, short_port_info_eq);
  tcase_add_test(tc_port, short_port_info_adj);
  suite_add_tcase(s, tc_port);

  return s;
}

