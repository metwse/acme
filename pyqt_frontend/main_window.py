"""
Main window for the acme PyQt frontend.

This module provides the application main window, which hosts the
circuit canvas widget and manages the window lifecycle.

Classes:
    MainWindow: QMainWindow subclass that displays the circuit simulation
"""

from PyQt6.QtWidgets import QMainWindow, QApplication
from PyQt6.QtCore import Qt

from .renderer import CircuitCanvas
from .simulation import Simulation


class MainWindow(QMainWindow):
    """Main application window for the circuit simulator."""
    
    def __init__(self, simulation: Simulation):
        super().__init__()
        
        self.simulation = simulation
        self.canvas = CircuitCanvas(simulation, self)
        
        self.setCentralWidget(self.canvas)
        self.setWindowTitle("acme-sim")
        self.setMinimumSize(512, 384)
        self.resize(800, 600)
        
        # Center on screen
        screen = QApplication.primaryScreen().geometry()
        x = (screen.width() - self.width()) // 2
        y = (screen.height() - self.height()) // 2
        self.move(x, y)
