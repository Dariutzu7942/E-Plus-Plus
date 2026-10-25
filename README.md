# E++

E++ is a beginner-friendly interface language that compiles `.epp` source files into standalone HTML pages.

## Documentation

- [Language overview and build instructions](E++/README.md)
- [E++ language reference](E++%20Information/language-reference.md)
- [Learning guide](E++%20Information/README.md)

## Quick start

Install as an npm package locally for testing:

```sh
cd E++
npm link
e++ run examples/runtime.epp
e++ build examples/hello.epp --out hello.html
e++ serve examples --port 3000
```

Requirements:

- CMake 3.16 or newer
- A C++17 compiler

Build the compiler and compile the example:

```sh
cmake -S E++ -B E++/build
cmake --build E++/build
./E++/build/epp E++/examples/hello.epp
```

Run the tests:

```sh
ctest --test-dir E++/build --output-on-failure
```

The generated HTML can be opened directly in a browser on Windows, Linux, or macOS.

## Runtime example

```epp
let name = "E++"
let count = 2 + 3 * 4
print name
print count
repeat 3
    print "repeat"
endrepeat
if count == 14
    print "done"
endif
```

Run it with `e++ run app.epp`.

## HTML interface example

```epp
title "My page"
heading "Hello!"
text "This is my first E++ page."
button "Show a message" "It works!"
```

The compiler currently targets standalone HTML. Native Windows and Linux interface targets are planned future extensions.
