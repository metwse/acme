"""
Simulation engine for acme digital circuits.

This module manages the logic simulation, propagating wire state changes
through connected logic units until the circuit reaches a stable state.

The simulation uses a change-driven approach: only units whose inputs
have changed are re-evaluated, making it efficient for interactive use.

Classes:
    Simulation: Core simulation engine

Example:
    >>> from pyqt_frontend.parser import parse_file
    >>> from pyqt_frontend.simulation import Simulation
    >>> parser = parse_file("examples/gates.hdl")
    >>> sim = Simulation(parser)
    >>> sim.toggle_wire(wire_id)  # Toggle a wire
    >>> sim.stabilize()  # Propagate changes
"""

from typing import Optional
from .parser import Parser, Lut, Wire, Unit


class Simulation:
    """
    Core simulation engine.
    
    Manages wire states and propagates changes through logic units
    until the circuit reaches a stable state.
    """
    
    def __init__(self, parser: Parser):
        self.luts = parser.luts
        self.wires = parser.wires
        self.units = parser.units
        self.lexer = parser.lexer
        
        self.changed_wires: set[int] = set()
        
        # Initial stabilization
        self.stabilize()
    
    def set_wire_state(self, wire_id: int, state: bool):
        """Set a wire's state and mark it as changed."""
        if wire_id in self.wires:
            wire = self.wires[wire_id]
            if wire.state != state:
                wire.state = state
                self.changed_wires.add(wire_id)
    
    def toggle_wire(self, wire_id: int):
        """Toggle a wire's state."""
        if wire_id in self.wires:
            wire = self.wires[wire_id]
            self.set_wire_state(wire_id, not wire.state)
    
    def advance(self):
        """
        Advance simulation by one step.
        Process all units affected by changed wires.
        """
        if not self.changed_wires:
            return
        
        # Collect units affected by changed wires
        affected_units: set[int] = set()
        for wire_id in self.changed_wires:
            if wire_id in self.wires:
                affected_units.update(self.wires[wire_id].affects)
        
        self.changed_wires.clear()
        
        # Process each affected unit
        for unit_id in affected_units:
            if unit_id not in self.units:
                continue
            
            unit = self.units[unit_id]
            
            if unit.lut_id not in self.luts:
                continue
            
            lut = self.luts[unit.lut_id]
            
            # Gather input states
            inputs: list[bool] = []
            for wire_id in unit.input_wires:
                if wire_id in self.wires:
                    inputs.append(self.wires[wire_id].state)
                else:
                    inputs.append(False)
            
            # Compute outputs
            outputs = lut.lookup(inputs)
            
            # Update output wires
            for i, wire_id in enumerate(unit.output_wires):
                if i < len(outputs) and wire_id in self.wires:
                    new_state = outputs[i]
                    if self.wires[wire_id].state != new_state:
                        self.wires[wire_id].state = new_state
                        self.changed_wires.add(wire_id)
    
    def stabilize(self, max_iterations: int = 1000):
        """
        Run simulation until circuit stabilizes.
        
        Args:
            max_iterations: Maximum iterations to prevent infinite loops
        """
        # Initially mark all input wires of all units as potentially affecting outputs
        for unit in self.units.values():
            for wire_id in unit.input_wires:
                if wire_id in self.wires:
                    self.wires[wire_id].affects.add(unit.id)
                    self.changed_wires.add(wire_id)
        
        iterations = 0
        while self.changed_wires and iterations < max_iterations:
            self.advance()
            iterations += 1
