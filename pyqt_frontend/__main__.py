"""
Entry point for the acme PyQt frontend.

This module provides the command-line interface for launching the
circuit simulator application.

Usage:
    python -m pyqt_frontend <simulation_file>

Arguments:
    simulation_file: Path to an HDL file describing the circuit

Example:
    python -m pyqt_frontend examples/gates.hdl

Exit Codes:
    0: Success
    1: Error (file not found, syntax error, etc.)
"""

import sys
from PyQt6.QtWidgets import QApplication

from .parser import parse_file
from .simulation import Simulation
from .main_window import MainWindow


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <simulation_file>", file=sys.stderr)
        sys.exit(1)
    
    filepath = sys.argv[1]
    
    try:
        # Parse HDL file
        parser = parse_file(filepath)
        
        # Create simulation
        simulation = Simulation(parser)
        
        # Create Qt application
        app = QApplication(sys.argv)
        
        # Create and show main window
        window = MainWindow(simulation)
        window.show()
        
        # Run event loop
        sys.exit(app.exec())
        
    except FileNotFoundError:
        print(f"Error: File not found: {filepath}", file=sys.stderr)
        sys.exit(1)
    except SyntaxError as e:
        print(f"Syntax error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
