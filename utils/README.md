# Boolean Script Utilities
This module provides command-line utilities for working with the Boolean Script
language.

Utilities in this module can be built using the following syntax:

```bash
make bin/<utility_name>
```

i.e.
```bash
make bin/dnf
```

### `bnf`
Outputs the current Boolean Script grammar in BNF format to `stdout`.

Usage:
```bash
bin/bnf
```

### `parse-tree`
Generates a parse tree in DOT format from the given source code read from
`stdin`.

Example:
```bash
echo "vec<3> cnt = [a, b([0, 1]), c * (d + 1)];" | bin/parse-tree
```

The output can be visualized using Graphviz:
```bash
echo "<your_code>" | bin/parse-tree | dot -Tpng | display
```
