# E++

<p align="center">
    <img src="assets/epp-logo.svg" alt="E++ logo" width="120">
</p>

E++ is split into two independent packages: a general-purpose language runtime and an optional GUI toolkit.

## Install without npm

From another Linux or macOS machine, clone the repository and run the installer:

```sh
git clone https://github.com/Dariutzu7942/E-Plus-Plus.git
cd E-Plus-Plus
./install.sh
export PATH="$HOME/.local/bin:$PATH"
```

This builds and installs both `epp` and `epp-gui` using CMake and a C++17 compiler. In Codespaces, it also installs the `.epp` file icon extension. To choose another install directory, set `EPP_INSTALL_PREFIX` when running the script.

## Documentation

- [Language runtime overview and build instructions](E++/README.md)
- [Optional GUI package and build instructions](E++%20GUI/README.md)
- [E++ language reference](E++%20Information/language-reference.md)
- [Learning guide](E++%20Information/README.md)

## Quick start

After the packages are published to npm, they can also be installed with npm:

```sh
npm install -g eplusplus-language
e++ run app.epp
```

For GUI features, install the separate package too:

```sh
npm install -g eplusplus-gui
e++-gui serve . --port 3000
```

Requirements:

- CMake 3.16 or newer
- A C++17 compiler

Build the compiler and compile the example:

```sh
cmake -S E++ -B E++/build
cmake --build E++/build
./E++/build/epp run E++/examples/runtime.epp
```

Run the tests:

```sh
ctest --test-dir E++/build --output-on-failure
```

Both packages require CMake 3.16+ and a C++17 compiler on first use. The language package does not depend on the GUI package.

Before publication, try the packages from this checkout by running `npm link` in each folder:

```sh
cd E++ && npm link
cd "../E++ GUI" && npm link
```

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

## GUI package example

```epp
title "My page"
heading "Hello!"
text "This is my first E++ page."
button "Show a message" "It works!"
```

GUI instructions and interface statements belong to the separate `eplusplus-gui` package.
