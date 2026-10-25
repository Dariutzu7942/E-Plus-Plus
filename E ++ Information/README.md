# Learn E++

E++ is designed to make a first interface feel approachable. You write a few readable statements, compile them, and open the result in a browser.

## First steps

1. Install CMake and a C++17 compiler.
2. Open a terminal in the `E++` folder.
3. Run `cmake -S . -B build`, then `cmake --build build`.
4. Run `./build/epp examples/hello.epp` on Linux or macOS. On Windows, run `build\\Debug\\epp.exe examples\\hello.epp`.
5. Open `examples/hello.html` in any modern browser.
6. Edit `examples/hello.epp`, run the compiler again, and refresh the page.

## Your first program

```epp
title "My page"
heading "Hello!"
text "This is my first E++ page."
button "Show a message" "It works!"
```

Each statement starts with a word and has quoted text after it. A button's second value is the message it displays when clicked. Lines starting with `#` are comments.

See the [language reference](language-reference.md) for all statements and their rules. The compiler and example live in the [E++ system folder](../E++/README.md).

## Where E++ runs

The current compiler creates standalone HTML, so E++ pages work on the web and in browsers on Windows and Linux. A native Windows/Linux application target is a future extension; the first version does not package native desktop apps.