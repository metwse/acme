"""
acme - Digital Circuit Simulator (PyQt Frontend)

A Python/PyQt reimplementation of the acme circuit simulator frontend.

Modules:
    lexer: Tokenizes HDL source files into tokens
    parser: Parses tokens into circuit structures (Lut, Wire, Unit)
    simulation: Propagates wire state changes through logic units
    renderer: Draws the circuit using PyQt graphics
    main_window: Application main window

Usage:
    python -m pyqt_frontend <hdl_file>

Example:
    python -m pyqt_frontend examples/gates.hdl
"""

__version__ = "1.0.0"
__author__ = "acme contributors"
