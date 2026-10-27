# E++ GUI

`eplusplus-gui` is an optional interface package for E++. It is separate from the `eplusplus-language` runtime; plain E++ programs do not need this package.

## Install without npm

Clone the repository and run `./install.sh` from its root to install both `epp` and `epp-gui` under `~/.local/bin`. For a GUI-only build, run:

```sh
cmake -S "E++ GUI" -B build-gui -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build-gui
cmake --install build-gui
```

## Install after publication

```sh
npm install -g eplusplus-gui
```

Before publication, run `npm link` from this package's folder to use it from the checkout.

Requires CMake 3.16+ and a C++17 compiler on first use.

## Build and preview

```sh
e++-gui build examples/hello.epp --out hello.html
e++-gui serve examples --port 3000
```

Open `http://localhost:3000/hello.epp` to preview the interface. The server compiles `.epp` pages and serves the generated HTML.

## Interface syntax

```epp
title "My app"
heading "Hello!"
text "A simple interface."
button "Say hello" "Hello from E++ GUI!"
link "Guide" "https://example.com"
```

The GUI package supports `title`, `heading`, `text`, `button`, and `link`. It currently generates browser-based HTML; native desktop widgets are not implemented yet.
