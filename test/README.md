<!--
This document follows the Semantic Line Breaks standard.
<http://sembr.org/>
-->

# Testing `super-glue`
`super-glue` uses the `libcheck` testing framework.
This means that in order to build tests,
you must have `libcheck` installed.
Instructions for installing `libcheck` can be found on their
[website](https://libcheck.github.io/check/web/install.html).

Tests can be built and run using `make`;
the command to build and run all tests is `make test`.
Additionally, tests can be run under `valgrind` using `make`.
This target provides some options to make the output more friendly;
it can be run using `make test-mem BUILD=debug`.
You will recieve a warning if you omit the `BUILD=debug`.

