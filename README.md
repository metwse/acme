# acme
Logic circuits simulation.

## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.

## Building
Use `make` for building. You can set `DEBUG` environment variable to 1 for
building in debug mode.

### Requirements
- `libx11`, `libx11-dev`
- `libxft2`, `libxft2-dev`
- `libfreetype6`, `libfreetype6-dev`
- `libglib2.0`, `libglib2.0-dev`
- `libresc` with `stack`, `dump_dot`, and `dump_bnf` features (check out its
  [repo](https://github.com/metwse/rdesc) for building documentation)

## Future Plans
```rs
lut<2, 1> nand = (0b0111)
{
    prop_delay: 1,
    _shape: [(0, 0), (3, 0), (4, 1)...],
    _input_port_pos: [(0, 2), (0, 4)],
    _out_port_pos: [(5, 3)]
}; /* properties starting with '_' are metadata fields and ignored by the
      simulation engine */


wire a = 1 { _pos: [(5, 12), uut1] }; /* wires require initial state */
wire b = 0 { _pos: [(5, 14), uut1] };
wire c = 1 { _pos: [uut1, (10, 13)] };


unit<nand> uut1 = (a, b) -> (c) { _pos: (10, 10) };
```
