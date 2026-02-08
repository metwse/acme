# acme - Digital Circuit Simulator

A digital logic circuits simulation tool with a PyQt-based graphical interface.

## Features

- **HDL Parser**: Parse custom hardware description language files
- **Circuit Simulation**: Real-time logic propagation with LUT-based components
- **Interactive GUI**: Click to toggle wire states, zoom and pan the view
- **Visualization**: Dynamic display of wires (green=active, black=inactive) and logic gates

## Installation

### Requirements
- Python 3.10+
- PyQt6

### Install dependencies
```bash
pip install -r pyqt_frontend/requirements.txt
```

## Usage

```bash
python -m pyqt_frontend <simulation_file>
```

### Example
```bash
python -m pyqt_frontend examples/gates.hdl
```

## Controls

| Action | Control |
|--------|---------|
| Toggle wire state | Click on wire endpoint (circle) |
| Zoom in/out | Ctrl + scroll wheel |
| Pan view | Scroll wheel |
| Reset zoom | Press `0` |

## HDL Syntax

### Lookup Tables (LUTs)
```
lut<inputs, outputs> name = (truth_table) {
    _shape: [(x1, y1), (x2, y2), ...],
    _input: [(x, y), ...],
    _output: [(x, y), ...]
};
```

### Wires
```
wire name = initial_state { _path: [(x1, y1), unit_name, ...] };
```

### Units
```
unit<lut_name> name = (input_wires) -> (output_wires) { _pos: (x, y) };
```

### Example Circuit (AND gate with NOT and OR)
```
lut<2, 1> and2 = (0b1000) {
    _shape: [(0, 0), (6, 0), (8, 1), (9, 2), (10, 4), (10, 6), (9, 8),
             (8, 9), (6, 10), (0, 10), (0, 0)],
    _input: [(0, 2), (0, 8)],
    _output: [(10, 5)]
};

wire a = 1 { _path: [(2, 5), (2, 7), uut1] };
wire b = 0 { _path: [(2, 15), (2, 13), uut1] };
wire c = 0 { _path: [uut1, (20, 10)] };

unit<and2> uut1 = (a, b) -> (c) { _pos: (5, 5) };
```

## Project Structure

```
pyqt_frontend/
├── __init__.py      # Package initialization
├── __main__.py      # Entry point
├── lexer.py         # HDL tokenizer
├── parser.py        # HDL parser (builds circuit structures)
├── simulation.py    # Logic simulation engine
├── renderer.py      # PyQt canvas and drawing
├── main_window.py   # Application main window
└── requirements.txt # Python dependencies
```

## License

See [LICENSE](LICENSE) file.
