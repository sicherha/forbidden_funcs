This is a tiny GCC plugin that takes a list of forbidden functions and emits
`-Wdeprecated` warnings for every call to such a function.
It helps enforce bans on the use of certain 'dangerous' library functions such
as `sprintf()` or `system()`.

## Building the plugin
You need the following prerequisites:

* GCC >= 5, with C++ support and plugin-development headers
* Make
* Bash (for running the unit tests)

When all dependencies are in place, type:

```sh
make
```

## Installing the plugin
```sh
sudo make install
```
This will install `forbidden_funcs.so` into GCC's plugin directory, i.e.
`/usr/lib/gcc/<target>/<version>/plugin`.

## Using the plugin
The plugin takes a comma-separated list of symbols as a named argument called
`list`.
For example, to ban the use of `sprintf()` and `system()`, add the following
flags to GCC's command line (mind the underscores and dashes):

```
-fplugin=forbidden_funcs -fplugin-arg-forbidden_funcs-list=sprintf,system
```
To include a C++ function or method, use its mangled symbol name.
