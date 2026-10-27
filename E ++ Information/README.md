# Learn E++ GUI

The optional `eplusplus-gui` package lets you describe a simple interface in `.epp` and build it as standalone HTML. It is separate from the core E++ programming language.

## First steps

1. Install CMake and a C++17 compiler.
2. Open a terminal in the `E++ GUI` folder.
3. Run `cmake -S . -B build`, then `cmake --build build`.
4. Run `./build/epp-gui examples/hello.epp` on Linux or macOS. On Windows, run `build\\Debug\\epp-gui.exe examples\\hello.epp`.
5. Open `examples/hello.html` in any modern browser.
6. Edit `examples/hello.epp`, run the compiler again, and refresh the page.

## Your first program

```epp
title "My page"
heading "Hello!"
text "This is my first E++ page."
button "Show a message" "It works!"
```

Each GUI statement starts with a word and has quoted text after it. A button's second value is the message it displays when clicked. Lines starting with `#` are comments.

See the [GUI package guide](../E++%20GUI/README.md) for the interface commands. The core language reference is separate: [E++ language reference](language-reference.md).

## Where E++ runs

The GUI package currently generates standalone HTML, so pages work in browsers on Windows, Linux, and macOS. Native desktop widgets are not implemented yet.