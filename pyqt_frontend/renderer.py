"""
PyQt renderer for acme digital circuits.

This module provides the visualization layer using PyQt6, including:
- Drawing wires with state-dependent colors (green = active, black = inactive)
- Drawing unit shapes from LUT metadata
- Wire endpoint circles for interactive toggling
- Pan and zoom controls

Classes:
    CircuitRenderer: Handles coordinate transformation and drawing logic
    CircuitCanvas: QWidget that displays the circuit and handles user input

The renderer supports the following interactions:
- Click on wire endpoints to toggle state
- Ctrl + scroll to zoom in/out around cursor
- Scroll to pan vertically
- Press '0' to reset zoom to default
"""

from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QPen, QBrush, QColor, QPainterPath
from PyQt6.QtCore import Qt, QPointF

from .parser import Parser, Lut, Wire, Unit, Point, PointNum, PointIdent, TablePath, TablePoint, Path
from .simulation import Simulation


class CircuitRenderer:
    """
    Renders the circuit simulation to a QPainter.
    
    Draws:
    - Wires with active (green) / inactive (black) colors
    - Wire endpoints as filled circles
    - Unit shapes from LUT metadata
    """
    
    ACTIVE_COLOR = QColor(0, 200, 0)     # Green for active wires
    INACTIVE_COLOR = QColor(0, 0, 0)      # Black for inactive wires
    BACKGROUND_COLOR = QColor(255, 255, 255)  # White background
    
    def __init__(self, simulation: Simulation):
        self.sim = simulation
        self.parser = simulation  # Simulation has access to parser data
        
        # View transformation
        self.offset_x = 0.0
        self.offset_y = 0.0
        self.scale = 10.0
    
    def scale_x(self, x: float) -> float:
        """Transform x coordinate to screen space."""
        return self.offset_x + x * self.scale
    
    def scale_y(self, y: float) -> float:
        """Transform y coordinate to screen space."""
        return self.offset_y + y * self.scale
    
    def screen_to_world(self, screen_x: float, screen_y: float) -> tuple[float, float]:
        """Transform screen coordinates to world coordinates."""
        world_x = (screen_x - self.offset_x) / self.scale
        world_y = (screen_y - self.offset_y) / self.scale
        return world_x, world_y
    
    def zoom(self, factor: float, center_x: float, center_y: float):
        """Zoom in/out around a center point."""
        self.offset_x -= center_x
        self.offset_x *= factor
        self.offset_x += center_x
        
        self.offset_y -= center_y
        self.offset_y *= factor
        self.offset_y += center_y
        
        self.scale *= factor
    
    def pan(self, dx: float, dy: float):
        """Pan the view by screen pixels."""
        self.offset_x += dx
        self.offset_y += dy
    
    def reset_view(self, center_x: float, center_y: float):
        """Reset view to default scale centered at given point."""
        self.offset_x -= center_x
        self.offset_x *= 10 / self.scale
        self.offset_x += center_x
        
        self.offset_y -= center_y
        self.offset_y *= 10 / self.scale
        self.offset_y += center_y
        
        self.scale = 10.0
    
    def draw(self, painter: QPainter):
        """Draw the entire circuit."""
        # Clear background
        painter.fillRect(painter.viewport(), self.BACKGROUND_COLOR)
        
        # Set up pen
        pen = QPen()
        pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        pen.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
        pen.setWidth(max(1, int((self.scale + 4) / 4)))
        
        # Draw wires
        for wire in self.sim.wires.values():
            color = self.ACTIVE_COLOR if wire.state else self.INACTIVE_COLOR
            pen.setColor(color)
            painter.setPen(pen)
            painter.setBrush(QBrush(color))
            self._draw_wire(painter, wire)
        
        # Draw units
        pen.setColor(self.INACTIVE_COLOR)
        painter.setPen(pen)
        for unit in self.sim.units.values():
            self._draw_unit(painter, unit)
    
    def _draw_wire(self, painter: QPainter, wire: Wire):
        """Draw a single wire."""
        path_val = wire.table.get(self.sim.lexer.get_ident_id('_path'))
        if not isinstance(path_val, TablePath):
            return
        
        for segment in path_val.path.segments:
            points = self._resolve_path_points(wire, segment)
            if len(points) >= 2:
                for i in range(len(points) - 1):
                    painter.drawLine(
                        QPointF(self.scale_x(points[i][0]), self.scale_y(points[i][1])),
                        QPointF(self.scale_x(points[i+1][0]), self.scale_y(points[i+1][1]))
                    )
            
            # Draw endpoint circles for numeric points at start/end
            if segment:
                for i, p in enumerate(segment):
                    if isinstance(p, PointNum) and (i == 0 or i == len(segment) - 1):
                        radius = self.scale / 2
                        painter.drawEllipse(
                            QPointF(self.scale_x(p.x), self.scale_y(p.y)),
                            radius, radius
                        )
    
    def _resolve_path_points(self, wire: Wire, segment: list[Point]) -> list[tuple[float, float]]:
        """Resolve a path segment to screen coordinates."""
        points: list[tuple[float, float]] = []
        
        for i, point in enumerate(segment):
            if isinstance(point, PointNum):
                points.append((point.x, point.y))
            elif isinstance(point, PointIdent):
                # Resolve unit port position
                unit_id = point.id
                if unit_id in self.sim.units:
                    unit = self.sim.units[unit_id]
                    pos = self._get_unit_position(unit)
                    if pos:
                        # Find which port this wire connects to
                        port_pos = self._get_wire_port_position(wire, unit)
                        if port_pos:
                            points.append((pos[0] + port_pos[0], pos[1] + port_pos[1]))
        
        return points
    
    def _get_unit_position(self, unit: Unit) -> tuple[float, float] | None:
        """Get unit position from _pos metadata."""
        pos_val = unit.table.get(self.sim.lexer.get_ident_id('_pos'))
        if isinstance(pos_val, TablePoint) and isinstance(pos_val.point, PointNum):
            return (pos_val.point.x, pos_val.point.y)
        return None
    
    def _get_wire_port_position(self, wire: Wire, unit: Unit) -> tuple[float, float] | None:
        """Get the port position where a wire connects to a unit."""
        if unit.lut_id not in self.sim.luts:
            return None
        
        lut = self.sim.luts[unit.lut_id]
        
        # Check input ports
        for i, input_wire in enumerate(unit.input_wires):
            if input_wire == wire.id:
                input_val = lut.table.get(self.sim.lexer.get_ident_id('_input'))
                if isinstance(input_val, TablePath) and input_val.path.segments:
                    seg = input_val.path.segments[0]
                    if i < len(seg) and isinstance(seg[i], PointNum):
                        return (seg[i].x, seg[i].y)
        
        # Check output ports
        for i, output_wire in enumerate(unit.output_wires):
            if output_wire == wire.id:
                output_val = lut.table.get(self.sim.lexer.get_ident_id('_output'))
                if isinstance(output_val, TablePath) and output_val.path.segments:
                    seg = output_val.path.segments[0]
                    if i < len(seg) and isinstance(seg[i], PointNum):
                        return (seg[i].x, seg[i].y)
        
        return None
    
    def _draw_unit(self, painter: QPainter, unit: Unit):
        """Draw a single unit."""
        if unit.lut_id not in self.sim.luts:
            return
        
        lut = self.sim.luts[unit.lut_id]
        pos = self._get_unit_position(unit)
        if not pos:
            return
        
        self._draw_lut_shape(painter, lut, pos[0], pos[1])
    
    def _draw_lut_shape(self, painter: QPainter, lut: Lut, x: float, y: float):
        """Draw a LUT shape at position (x, y)."""
        shape_val = lut.table.get(self.sim.lexer.get_ident_id('_shape'))
        if not isinstance(shape_val, TablePath):
            return
        
        for segment in shape_val.path.segments:
            if len(segment) >= 2:
                for i in range(len(segment) - 1):
                    p1 = segment[i]
                    p2 = segment[i + 1]
                    if isinstance(p1, PointNum) and isinstance(p2, PointNum):
                        painter.drawLine(
                            QPointF(self.scale_x(x + p1.x), self.scale_y(y + p1.y)),
                            QPointF(self.scale_x(x + p2.x), self.scale_y(y + p2.y))
                        )
    
    def find_wire_at(self, screen_x: float, screen_y: float) -> int | None:
        """Find wire ID near a screen position (for toggling)."""
        tolerance = self.scale
        
        for wire in self.sim.wires.values():
            path_val = wire.table.get(self.sim.lexer.get_ident_id('_path'))
            if not isinstance(path_val, TablePath):
                continue
            
            for segment in path_val.path.segments:
                if segment and isinstance(segment[0], PointNum):
                    tip = segment[0]
                    tip_x = self.scale_x(tip.x)
                    tip_y = self.scale_y(tip.y)
                    
                    if (abs(screen_x - tip_x) <= tolerance and 
                        abs(screen_y - tip_y) <= tolerance):
                        return wire.id
        
        return None


class CircuitCanvas(QWidget):
    """QWidget that displays the circuit and handles interaction."""
    
    def __init__(self, simulation: Simulation, parent=None):
        super().__init__(parent)
        self.simulation = simulation
        self.renderer = CircuitRenderer(simulation)
        
        self.setMinimumSize(512, 384)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.renderer.draw(painter)
    
    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            wire_id = self.renderer.find_wire_at(event.position().x(), event.position().y())
            if wire_id is not None:
                self.simulation.toggle_wire(wire_id)
                self.simulation.stabilize()
                self.update()
    
    def wheelEvent(self, event):
        modifiers = event.modifiers()
        delta = event.angleDelta().y()
        
        if modifiers & Qt.KeyboardModifier.ControlModifier:
            # Zoom
            factor = 1.25 if delta > 0 else 0.8
            self.renderer.zoom(factor, event.position().x(), event.position().y())
        else:
            # Pan
            scroll_amount = self.renderer.scale
            if delta > 0:
                self.renderer.pan(0, scroll_amount)
            else:
                self.renderer.pan(0, -scroll_amount)
        
        self.update()
    
    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_0:
            # Reset zoom to default
            self.renderer.reset_view(self.width() / 2, self.height() / 2)
            self.update()
