# E++ Language Runtime

`eplusplus-language` is the standalone E++ language package. It runs general-purpose programs and does not include or require GUI tools.

## Install without npm

From the repository root, run `./install.sh` to build and install both tools under `~/.local/bin`. To build only this package manually:

```sh
cmake -S E++ -B build-language -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build-language
cmake --install build-language
```

## Build it

E++ uses C++17 and CMake. From this folder, run:

```sh
cmake -S . -B build
cmake --build build
```

## Install with npm after publication

```sh
npm install -g eplusplus-language
```

Before publication, run `npm link` from this package's folder to use it from the checkout.

Commands available after installation:

```sh
e++ run examples/runtime.epp
```

## Runtime preview

```epp
let name = "E++"
let count = 2 + 3 * 4

print name
print count

repeat 3
    print count
endrepeat

if count == 14
    print "done"
else
    print "wrong"
endif
```

Run it with the native binary or npm CLI:

```sh
./build/epp run examples/runtime.epp
e++ run examples/runtime.epp
```

The runtime supports `let`, `print`, `repeat`/`endrepeat`, `if`/`else`/`endif`, strings, numbers, booleans, variables, arithmetic, and string or numeric comparisons.

## Project status

The runtime supports variables, strings, numbers, booleans, arithmetic, comparisons, `if`/`else`, and `repeat` loops. Modules, functions, and bytecode compilation are planned. GUI features live in the separate [`eplusplus-gui`](../E++%20GUI/README.md) package.

Run the native compiler tests with:

```sh
ctest --test-dir build --output-on-failure
```