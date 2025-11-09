# Boolean Script
Embryonic Boolean algebra interpreter.

See BNF in [utils](utils/README.md) for language definition.

```
expr half_adder(a, b) -> (sum, carry) {
    sum = (a + b) * (a * b)'
    carry = a * b
}
```

## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.

## Building
You can build Boolean Script and its utilities using GNUMake.
```bash
make               # Build the main 'bs' binary
make bin/bnf       # Build a specific utility
make tests         # Build all test binaries
make bin/bio.test  # Build a specific test binary
```

### Build Variants
- Default (optimized): \
  Uses `-O2` and all warnings enabled.
- Debug build: \
  Includes symbols and assertions.
  ```bash
  make DEBUG=1
  ```
- Test build (with coverage): \
  Enables debug flags plus `--coverage`. Building tests automatically enables
  this flags, i.e. `make bin/bio.test`, `make tests`.
  ```bash
  make TEST=1
  ```

### Documentation
To generate Doxygen documentation:
```bash
make docs
```

## BS Utilitiy
A collection of command-line tools for interacting with Boolean script is
available in the [utils](utils/README.md) directory.

## Testing
`runtest.sh` automatically builds and executes all test binaries under `bin/`.

```bash
./runtest.sh <mode>
```

### Modes
- (no argument) - Only builds the tests.
- `run` - Builds and runs all tests.
- `gcovr` - Runs all tests and generates a coverage report using `gcovr`.
- `valgrind` - Runs all tests under Valgrind for memory checks.

## Future Plans
HDL-like features for digital circuit simulation:
```
expr xor(a, b) -> (y) {
    y = (a + b) * (a * b)'
}

expr full_adder(a, b, cin) -> (sum, cout) {
    ab = xor(a, b)
    sum = xor(ab, cin)
    cout = (a * b) + (cin * (a + b))
}

sync {
    vec<4> cnt = [0, 0, 0, 0]

    @clk {
        cnt0, c0 = full_adder(cnt[0], 1, 0)
        cnt1, c1 = full_adder(cnt[1], 0, c0)
        cnt2, c2 = full_adder(cnt[2], 0, c1)
        cnt3, c3 = full_adder(cnt[3], 0, c2)

        cnt = [cnt0, cnt1, cnt2, cnt3]
    }
}
```
...toward full electronics simulation using Boolean Script
