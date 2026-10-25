# E++ Language Reference

An E++ source file uses the `.epp` extension. Write one statement per line. Put text values in double quotes; use `#` to start a comment.

## Statements

| Statement | Example | Result |
| --- | --- | --- |
| `title` | `title "My page"` | Sets the browser tab title. Defaults to `E++ App`. |
| `heading` | `heading "Welcome"` | Adds a page heading. |
| `text` | `text "A short sentence."` | Adds a paragraph. |
| `button` | `button "Say hi" "Hello!"` | Adds a button that shows its message when clicked. |
| `link` | `link "Search" "https://example.com"` | Adds a link to the given address. |

Each statement takes exactly the values shown. Text can include spaces inside quotes. Unknown statements and invalid argument counts report the source line where the problem was found.

## Compile a page

Build the compiler from the `E++` folder, then use the native executable:

```sh
cmake -S . -B build
cmake --build build
./build/epp examples/hello.epp
```

By default, the compiler writes an HTML file beside the source file. Choose a different output path with `--out`:

```sh
./build/epp examples/hello.epp --out my-page.html
```

The generated HTML is standalone and can be opened directly or hosted as a website.