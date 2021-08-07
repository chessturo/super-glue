/* Provides an entry point and code for running tests
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
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_process_args.h"

int main(int argc, char *argv[]) {
  if (argc != 1) {
    (void)argv;
    fprintf(stderr, "Command-line options are ignored for tests.");
  }

  SRunner *runner = srunner_create(process_args_tests());
  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
