# E++ Language System

E++ is a small, beginner-friendly language for describing interfaces. The first compiler turns an `.epp` file into a standalone HTML page. That page runs in a browser on Windows, Linux, and the web without extra packages.

## Build it

E++ uses C++17 and CMake. From this folder, run:

```sh
cmake -S . -B build
cmake --build build
```

## Try it

```sh
./build/epp examples/hello.epp
```

This creates `examples/hello.html`. Open that file in a browser to see the interface.

To choose a different output path:

```sh
./build/epp examples/hello.epp --out hello.html
```

## Language preview

```epp
title "My first app"
heading "Hello, E++!"
text "I made this interface with a few simple lines."
button "Say hello" "Hello from E++!"
```

Supported statements are `title`, `heading`, `text`, `button`, and `link`. Read [the language reference](../E++%20Information/language-reference.md) for syntax and examples.

## Project status

The HTML compiler is the first working target. Its output works as a website and in desktop browsers on Windows and Linux. Native desktop app packaging is not implemented yet; it can be added later as another compiler target.

Run the compiler tests with:

```sh
python3 -m unittest discover -s tests
```