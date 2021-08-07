/* Definitions of various utility functions.
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

#include "util.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


int alloc_sprintf(char **dest, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int bytes_needed = vsnprintf(NULL, 0, fmt, args);

  // For the '\0'
  bytes_needed++;
  
  va_start(args, fmt);

  *dest = malloc(bytes_needed);
  if (*dest == NULL) return -1;
  int res = vsnprintf(*dest, bytes_needed, fmt, args);
  if (res < 0) {
    *dest = NULL;
    return -1;
  }

  va_end(args);
  return bytes_needed;
}

