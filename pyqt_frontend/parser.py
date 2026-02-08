"""
Parser for the acme HDL format.

This module parses tokenized HDL source into circuit data structures
that can be used for simulation and visualization.

Classes:
    Point, PointNum, PointIdent: Represent positions in the circuit
    Path: A series of connected points
    Table, TableValue, TableNum, TablePath, TablePoint: Metadata storage
    Lut: Lookup table defining a logic function
    Wire: A connection with state (active/inactive)
    Unit: An instance of a LUT with connected wires
    Parser: Main parser class

Functions:
    parse_file(filepath): Convenience function to parse an HDL file

Example:
    >>> from pyqt_frontend.parser import parse_file
    >>> parser = parse_file("examples/gates.hdl")
    >>> print(f"Found {len(parser.luts)} LUTs, {len(parser.wires)} wires")
"""

from dataclasses import dataclass, field
from typing import Optional, Any
from .lexer import Lexer, Token, TokenType, NumInfo


@dataclass
class Point:
    """A point in the circuit, either numeric (x, y) or identifier reference."""
    pass


@dataclass
class PointNum(Point):
    """Numeric point (x, y coordinates)."""
    x: float
    y: float


@dataclass
class PointIdent(Point):
    """Point referencing a unit identifier."""
    id: int
    name: str


@dataclass
class Path:
    """A path consisting of multiple point segments."""
    segments: list[list[Point]] = field(default_factory=list)


@dataclass  
class TableValue:
    """Base class for table values."""
    pass


@dataclass
class TableNum(TableValue):
    """Numeric table value."""
    value: int


@dataclass
class TablePath(TableValue):
    """Path table value."""
    path: Path


@dataclass
class TablePoint(TableValue):
    """Single point table value.""" 
    point: Point


@dataclass
class Table:
    """Metadata table for statements."""
    values: dict[int, TableValue] = field(default_factory=dict)
    
    def get(self, key: int) -> Optional[TableValue]:
        return self.values.get(key)


@dataclass
class Lut:
    """Lookup table component."""
    id: int
    name: str
    input_size: int
    output_size: int
    lut: list[bool]
    table: Table
    
    def input_variant_count(self) -> int:
        return 1 << self.input_size
    
    def lookup(self, inputs: list[bool]) -> list[bool]:
        """Look up output values for given inputs."""
        # Convert input bits to index
        index = 0
        for i, bit in enumerate(inputs):
            if bit:
                index |= (1 << i)
        
        # Get output bits
        outputs = []
        for i in range(self.output_size):
            bit_index = index * self.output_size + i
            if bit_index < len(self.lut):
                outputs.append(self.lut[bit_index])
            else:
                outputs.append(False)
        return outputs


@dataclass
class Wire:
    """Wire representing physical connections."""
    id: int
    name: str
    state: bool
    table: Table
    affects: set[int] = field(default_factory=set)


@dataclass
class Unit:
    """Physical logic element in the simulation."""
    id: int
    name: str
    lut_id: int
    input_wires: list[int]
    output_wires: list[int]
    table: Table


class Parser:
    """
    Parser for acme HDL files.
    
    Parses:
    - lut<N, M> name = (bits) { table };
    - wire name = state { table };
    - unit<lut> name = (inputs) -> (outputs) { table };
    """
    
    def __init__(self, lexer: Lexer):
        self.lexer = lexer
        self.tokens: list[Token] = []
        self.pos = 0
        
        # Parsed elements
        self.luts: dict[int, Lut] = {}
        self.wires: dict[int, Wire] = {}
        self.units: dict[int, Unit] = {}
        
        # Pre-register metadata keys
        self.k_shape = lexer.get_ident_id('_shape')
        self.k_input = lexer.get_ident_id('_input')
        self.k_output = lexer.get_ident_id('_output')
        self.k_path = lexer.get_ident_id('_path')
        self.k_pos = lexer.get_ident_id('_pos')
    
    @property
    def current(self) -> Token:
        if self.pos >= len(self.tokens):
            return Token(TokenType.EOF, '', 0, 0)
        return self.tokens[self.pos]
    
    def peek(self, offset: int = 1) -> Token:
        pos = self.pos + offset
        if pos >= len(self.tokens):
            return Token(TokenType.EOF, '', 0, 0)
        return self.tokens[pos]
    
    def advance(self) -> Token:
        token = self.current
        self.pos += 1
        return token
    
    def expect(self, type: TokenType) -> Token:
        if self.current.type != type:
            raise SyntaxError(
                f"Expected {type.name}, got {self.current.type.name} "
                f"at line {self.current.line}, column {self.current.column}"
            )
        return self.advance()
    
    def parse(self):
        """Parse the entire source."""
        # Tokenize first
        self.tokens = list(self.lexer.tokenize())
        
        while self.current.type != TokenType.EOF:
            if self.current.type == TokenType.LUT:
                self.parse_lut()
            elif self.current.type == TokenType.WIRE:
                self.parse_wire()
            elif self.current.type == TokenType.UNIT:
                self.parse_unit()
            else:
                raise SyntaxError(
                    f"Unexpected token {self.current.type.name} "
                    f"at line {self.current.line}"
                )
    
    def parse_number(self) -> int:
        """Parse a number (decimal, binary, or hex)."""
        token = self.current
        if token.type == TokenType.NUM_DEC:
            self.advance()
            return int(token.value)
        elif token.type == TokenType.NUM_BIN:
            self.advance()
            return int(token.value, 2)
        elif token.type == TokenType.NUM_HEX:
            self.advance()
            return int(token.value, 16)
        else:
            raise SyntaxError(f"Expected number at line {token.line}")
    
    def parse_point(self) -> Point:
        """Parse a point: either (x, y) or identifier."""
        if self.current.type == TokenType.LPAREN:
            self.advance()  # (
            x = self.parse_number()
            self.expect(TokenType.COMMA)
            y = self.parse_number()
            self.expect(TokenType.RPAREN)
            return PointNum(x, y)
        elif self.current.type == TokenType.IDENT:
            token = self.advance()
            id = self.lexer.get_ident_id(token.value)
            return PointIdent(id, token.value)
        else:
            raise SyntaxError(f"Expected point at line {self.current.line}")
    
    def parse_path(self) -> Path:
        """Parse a path: [point, point, ...; point, ...]"""
        self.expect(TokenType.LBRACKET)
        
        path = Path()
        current_segment: list[Point] = []
        
        while self.current.type != TokenType.RBRACKET:
            point = self.parse_point()
            current_segment.append(point)
            
            if self.current.type == TokenType.COMMA:
                self.advance()
            elif self.current.type == TokenType.SEMICOLON:
                self.advance()
                path.segments.append(current_segment)
                current_segment = []
        
        if current_segment:
            path.segments.append(current_segment)
        
        self.expect(TokenType.RBRACKET)
        return path
    
    def parse_table(self) -> Table:
        """Parse a table: { key: value, ... }"""
        self.expect(TokenType.LBRACE)
        
        table = Table()
        
        while self.current.type != TokenType.RBRACE:
            # Key is an identifier
            key_token = self.expect(TokenType.IDENT)
            key_id = self.lexer.get_ident_id(key_token.value)
            
            self.expect(TokenType.COLON)
            
            # Value can be number, point, or path
            if self.current.type == TokenType.LBRACKET:
                value = TablePath(self.parse_path())
            elif self.current.type == TokenType.LPAREN:
                value = TablePoint(self.parse_point())
            elif self.current.type in (TokenType.NUM_DEC, TokenType.NUM_BIN, TokenType.NUM_HEX):
                value = TableNum(self.parse_number())
            else:
                raise SyntaxError(f"Unexpected table value at line {self.current.line}")
            
            table.values[key_id] = value
            
            if self.current.type == TokenType.COMMA:
                self.advance()
        
        self.expect(TokenType.RBRACE)
        return table
    
    def parse_lut(self):
        """Parse: lut<input, output> name = (bits) { table };"""
        self.expect(TokenType.LUT)
        self.expect(TokenType.LANGLE)
        input_size = self.parse_number()
        self.expect(TokenType.COMMA)
        output_size = self.parse_number()
        self.expect(TokenType.RANGLE)
        
        name_token = self.expect(TokenType.IDENT)
        lut_id = self.lexer.get_ident_id(name_token.value)
        
        self.expect(TokenType.EQUALS)
        self.expect(TokenType.LPAREN)
        
        bits_value = self.parse_number()
        
        self.expect(TokenType.RPAREN)
        
        # Convert bits to bool list
        lut_bits: list[bool] = []
        num_bits = (1 << input_size) * output_size
        for i in range(num_bits):
            lut_bits.append(bool((bits_value >> i) & 1))
        
        table = self.parse_table()
        self.expect(TokenType.SEMICOLON)
        
        self.luts[lut_id] = Lut(
            id=lut_id,
            name=name_token.value,
            input_size=input_size,
            output_size=output_size,
            lut=lut_bits,
            table=table
        )
    
    def parse_wire(self):
        """Parse: wire name = state { table };"""
        self.expect(TokenType.WIRE)
        
        name_token = self.expect(TokenType.IDENT)
        wire_id = self.lexer.get_ident_id(name_token.value)
        
        self.expect(TokenType.EQUALS)
        state = self.parse_number()
        
        table = self.parse_table()
        self.expect(TokenType.SEMICOLON)
        
        self.wires[wire_id] = Wire(
            id=wire_id,
            name=name_token.value,
            state=bool(state),
            table=table
        )
    
    def parse_wire_list(self) -> list[int]:
        """Parse a list of wire identifiers: (a, b, c)"""
        self.expect(TokenType.LPAREN)
        wires: list[int] = []
        
        while self.current.type != TokenType.RPAREN:
            token = self.expect(TokenType.IDENT)
            wire_id = self.lexer.get_ident_id(token.value)
            wires.append(wire_id)
            
            if self.current.type == TokenType.COMMA:
                self.advance()
        
        self.expect(TokenType.RPAREN)
        return wires
    
    def parse_unit(self):
        """Parse: unit<lut> name = (inputs) -> (outputs) { table };"""
        self.expect(TokenType.UNIT)
        self.expect(TokenType.LANGLE)
        
        lut_token = self.expect(TokenType.IDENT)
        lut_id = self.lexer.get_ident_id(lut_token.value)
        
        self.expect(TokenType.RANGLE)
        
        name_token = self.expect(TokenType.IDENT)
        unit_id = self.lexer.get_ident_id(name_token.value)
        
        self.expect(TokenType.EQUALS)
        
        input_wires = self.parse_wire_list()
        self.expect(TokenType.ARROW)
        output_wires = self.parse_wire_list()
        
        table = self.parse_table()
        self.expect(TokenType.SEMICOLON)
        
        self.units[unit_id] = Unit(
            id=unit_id,
            name=name_token.value,
            lut_id=lut_id,
            input_wires=input_wires,
            output_wires=output_wires,
            table=table
        )
        
        # Register which wires affect which units
        for wire_id in input_wires:
            if wire_id in self.wires:
                self.wires[wire_id].affects.add(unit_id)


def parse_file(filepath: str) -> Parser:
    """Parse an HDL file and return the parser with parsed elements."""
    with open(filepath, 'r') as f:
        source = f.read()
    
    lexer = Lexer(source)
    parser = Parser(lexer)
    parser.parse()
    
    return parser
