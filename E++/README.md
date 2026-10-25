# E++ Language System

E++ is a small, beginner-friendly language with its own native runtime. It can execute general programs and compile `.epp` files into standalone HTML pages.

## Build it

E++ uses C++17 and CMake. From this folder, run:

```sh
cmake -S . -B build
cmake --build build
```

## Try it with npm

For an npm package install, publish or link this repository first, then install the package by its package name:

```sh
npm install eplusplus-language
```

Commands available after installation:

```sh
e++ run examples/runtime.epp
e++ build app.epp --out app.html
e++ serve . --port 3000
```

The development server compiles `.epp` files and serves them as HTML.

## Try the native binary

```sh
./build/epp examples/hello.epp
```

This creates `examples/hello.html`. Open that file in a browser to see the interface.

To choose a different output path:

```sh
./build/epp examples/hello.epp --out hello.html
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

## Interface preview

```epp
title "My first app"
heading "Hello, E++!"
text "I made this interface with a few simple lines."
button "Say hello" "Hello from E++!"
```

Supported statements are `title`, `heading`, `text`, `button`, and `link`. Read [the language reference](../E++%20Information/language-reference.md) for syntax and examples.

## Project status

E++ now has a native interpreter and a standalone HTML compiler. Native desktop UI packaging, modules, functions, and bytecode compilation are planned future targets.

Run the native compiler tests with:

```sh
ctest --test-dir build --output-on-failure
```