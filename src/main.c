/* Provides an entry point and code for managing interactive sessions
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

#include "main.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "process_args.h"
#include "state.h"

// Prints usage information to stderr.
static void usage(const char *prog_name);

int main(int argc, char *argv[]) {
  printf("Hello world\n");

  // Process our command line arguments.
  State *state;
  ConfigFiles *files;
  char *error;
  ArgsResult res = process_args(argc, argv, &state, &files, &error);

  // Do any needed error handling based on the parsing of our arguments.
  if (res != ARGS_OK) {
    fprintf(stderr, "Error: %s\n", error);
    if (res == ARGS_NONE || res == ARGS_NO_FILES) {
      usage(argv[0]);
    }
    free_state(state);
    free_config_files(files);
    return EXIT_FAILURE;
  }
  
  free_state(state);
  free_config_files(files);
  return EXIT_SUCCESS;
}

static void usage(const char *prog_name) {
  fprintf(stderr, "Usage: \n");
  fprintf(stderr, "\t%s [-i] files ...\n", prog_name);
}

