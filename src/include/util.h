/* Declaration of various utility functions
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

#ifndef SUPER_GLUE_INCLUDE_UTIL_H_
#define SUPER_GLUE_INCLUDE_UTIL_H_

#include <stddef.h>

// Allocates a buffer large enough to hold the string produced by calling
// `snprintf` with the given `fmt` and `...`. This function uses a default
// starting size for the buffer. This buffer is allocated with `malloc` and must
// be freed with `free`.
//
// dest - An output parameter; `*dest` is set to the formatted string, or NULL
//        on error.
// fmt  - The `printf` style format string
// ...  - The arguments passed to `snprintf` in a `va_list`.
//
// Returns the number of bytes written to `*dest` on success, and a negative
// value on error. `errno` is set to indicate the error.
int alloc_sprintf(char **dest, const char *fmt, ...);

#endif  // SUPER_GLUE_INCLUDE_UTIL_H_
