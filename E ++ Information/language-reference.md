# E++ Language Reference

An E++ source file uses the `.epp` extension. Write one statement per line. Put text values in double quotes; use `#` to start a comment.

## Runtime programs

Run a general E++ program with the native binary or npm CLI:

```sh
./build/epp run app.epp
e++ run app.epp
```

Runtime syntax:

```epp
let name = "E++"
let count = 2 + 3
print name
print count

if count == 5
    print "equal"
else
    print "different"
endif

repeat 3
    print "hello"
endrepeat
```

Supported features include strings, numbers, booleans, variables, arithmetic, comparisons, `if`/`else`/`endif`, and `repeat`/`endrepeat`.