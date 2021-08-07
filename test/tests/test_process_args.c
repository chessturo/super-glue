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
#include "test_process_args.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "process_args.h"
#include "state.h"

// Helper constants that are passed as arguments.
static char *prog_name;
static State *state;
static ConfigFiles *files;
static char *error;

static void checked_setup() {
  prog_name = "super-glue";
  state = NULL;
  files = NULL;
  error = NULL;
}

static void checked_teardown() {
  free_state(state);
  free_config_files(files);
  free(error);
}

// Core test case
START_TEST(no_args) {
  ArgsResult res = process_args(1, &prog_name, &state, &files, &error);
  ck_assert_msg(res == ARGS_NONE, "When passed no arguments, process_args "
      "should return ARGS_NONE");
} END_TEST

START_TEST(bogus_opt) {
  char *args[] = { prog_name, "--bogus-arg" };
  ArgsResult res = process_args(sizeof(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, "
      "process_args should return ARGS_UNKNOWN");
} END_TEST

START_TEST(bogus_opt_with_info) {
  char *args[] = { prog_name, "--bogus-arg=info" };
  ArgsResult res = process_args(sizeof(args), args, &state, &files, &error);
  ck_assert_msg(res == ARGS_UNKNOWN, "When passed an unknown option, even "
      "with extra info, process_args should return ARGS_UNKNOWN");
} END_TEST

Suite *process_args_tests() {
  Suite *s = suite_create("process_args");

  TCase *tc_core = tcase_create("core");
  tcase_add_checked_fixture(tc_core, &checked_setup, &checked_teardown);
  tcase_add_test(tc_core, no_args);
  tcase_add_test(tc_core, bogus_opt);
  tcase_add_test(tc_core, bogus_opt_with_info);
  suite_add_tcase(s, tc_core);

  return s;
}

