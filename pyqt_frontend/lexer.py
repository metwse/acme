"""
Lexer for the acme HDL format.

This module provides tokenization for acme Hardware Description Language files.
It converts source text into a stream of tokens that can be consumed by the parser.

Classes:
    TokenType: Enum of all token types (keywords, literals, punctuation)
    Token: A single token with type, value, and position
    NumInfo: Semantic information for numeric tokens
    Lexer: Main tokenizer class

Example:
    >>> from pyqt_frontend.lexer import Lexer
    >>> lexer = Lexer("wire a = 1 { _path: [(0, 0)] };")
    >>> for token in lexer.tokenize():
    ...     print(token.type.name, token.value)
"""

import re
from dataclasses import dataclass
from enum import Enum, auto
from typing import Iterator, Optional


class TokenType(Enum):
    """Token types for the HDL lexer."""
    # Keywords
    LUT = auto()
    WIRE = auto()
    UNIT = auto()
    
    # Literals
    IDENT = auto()
    NUM_DEC = auto()
    NUM_BIN = auto()
    NUM_HEX = auto()
    
    # Punctuation
    LPAREN = auto()      # (
    RPAREN = auto()      # )
    LBRACE = auto()      # {
    RBRACE = auto()      # }
    LBRACKET = auto()    # [
    RBRACKET = auto()    # ]
    LANGLE = auto()      # <
    RANGLE = auto()      # >
    COMMA = auto()       # ,
    COLON = auto()       # :
    SEMICOLON = auto()   # ;
    EQUALS = auto()      # =
    ARROW = auto()       # ->
    
    # Special
    EOF = auto()
    NOTOKEN = auto()


@dataclass
class Token:
    """A single token from the lexer."""
    type: TokenType
    value: str
    line: int
    column: int


@dataclass
class NumInfo:
    """Semantic info for numeric tokens."""
    base: int
    value: str
    
    def decimal(self) -> int:
        """Convert to decimal value."""
        return int(self.value, self.base)


class Lexer:
    """
    Tokenizer for acme HDL files.
    
    Handles:
    - Keywords: lut, wire, unit
    - Identifiers: alphanumeric starting with letter or underscore
    - Numbers: decimal, binary (0b...), hexadecimal (0x...)
    - Punctuation: (), {}, [], <>, ,, :, ;, =, ->
    - Comments: /* ... */ and // ...
    """
    
    KEYWORDS = {
        'lut': TokenType.LUT,
        'wire': TokenType.WIRE,
        'unit': TokenType.UNIT,
    }
    
    PUNCTUATION = {
        '(': TokenType.LPAREN,
        ')': TokenType.RPAREN,
        '{': TokenType.LBRACE,
        '}': TokenType.RBRACE,
        '[': TokenType.LBRACKET,
        ']': TokenType.RBRACKET,
        '<': TokenType.LANGLE,
        '>': TokenType.RANGLE,
        ',': TokenType.COMMA,
        ':': TokenType.COLON,
        ';': TokenType.SEMICOLON,
        '=': TokenType.EQUALS,
    }
    
    def __init__(self, source: str):
        """Initialize lexer with source code."""
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1
        
        # Identifier tracking (matches C++ Lex behavior)
        self.idents: dict[str, int] = {}
        self.ident_names: list[str] = []
        self.last_ident_id = 0
    
    def get_ident_id(self, name: str) -> int:
        """Get or create an identifier ID."""
        if name not in self.idents:
            self.idents[name] = self.last_ident_id
            self.ident_names.append(name)
            self.last_ident_id += 1
        return self.idents[name]
    
    def ident_name(self, id: int) -> str:
        """Get identifier name by ID."""
        return self.ident_names[id]
    
    @property
    def current_char(self) -> Optional[str]:
        """Get current character or None if at end."""
        if self.pos >= len(self.source):
            return None
        return self.source[self.pos]
    
    def peek(self, offset: int = 1) -> Optional[str]:
        """Peek at character at offset from current position."""
        pos = self.pos + offset
        if pos >= len(self.source):
            return None
        return self.source[pos]
    
    def advance(self) -> Optional[str]:
        """Advance position and return current character."""
        char = self.current_char
        self.pos += 1
        if char == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return char
    
    def skip_whitespace(self):
        """Skip whitespace characters."""
        while self.current_char and self.current_char.isspace():
            self.advance()
    
    def skip_comment(self) -> bool:
        """Skip comments. Returns True if a comment was skipped."""
        if self.current_char == '/' and self.peek() == '*':
            # Block comment
            self.advance()  # skip /
            self.advance()  # skip *
            while self.current_char:
                if self.current_char == '*' and self.peek() == '/':
                    self.advance()  # skip *
                    self.advance()  # skip /
                    return True
                self.advance()
            return True
        elif self.current_char == '/' and self.peek() == '/':
            # Line comment
            while self.current_char and self.current_char != '\n':
                self.advance()
            return True
        return False
    
    def lex_number(self) -> Token:
        """Lex a numeric literal."""
        start_line = self.line
        start_col = self.column
        result = ''
        
        if self.current_char == '0' and self.peek() in ('b', 'B'):
            # Binary number
            self.advance()  # 0
            self.advance()  # b
            while self.current_char and self.current_char in '01':
                result += self.advance()
            return Token(TokenType.NUM_BIN, result, start_line, start_col)
        elif self.current_char == '0' and self.peek() in ('x', 'X'):
            # Hexadecimal number
            self.advance()  # 0
            self.advance()  # x
            while self.current_char and self.current_char in '0123456789abcdefABCDEF':
                result += self.advance()
            return Token(TokenType.NUM_HEX, result, start_line, start_col)
        else:
            # Decimal number
            while self.current_char and self.current_char.isdigit():
                result += self.advance()
            return Token(TokenType.NUM_DEC, result, start_line, start_col)
    
    def lex_ident(self) -> Token:
        """Lex an identifier or keyword."""
        start_line = self.line
        start_col = self.column
        result = ''
        
        while self.current_char and (self.current_char.isalnum() or self.current_char == '_'):
            result += self.advance()
        
        # Check if keyword
        if result in self.KEYWORDS:
            return Token(self.KEYWORDS[result], result, start_line, start_col)
        
        # Register identifier
        self.get_ident_id(result)
        return Token(TokenType.IDENT, result, start_line, start_col)
    
    def next_token(self) -> Token:
        """Get the next token from the source."""
        while True:
            self.skip_whitespace()
            
            if self.skip_comment():
                continue
            
            break
        
        if self.current_char is None:
            return Token(TokenType.EOF, '', self.line, self.column)
        
        start_line = self.line
        start_col = self.column
        
        # Check for arrow (->)
        if self.current_char == '-' and self.peek() == '>':
            self.advance()
            self.advance()
            return Token(TokenType.ARROW, '->', start_line, start_col)
        
        # Check single-char punctuation
        if self.current_char in self.PUNCTUATION:
            char = self.advance()
            return Token(self.PUNCTUATION[char], char, start_line, start_col)
        
        # Numbers
        if self.current_char.isdigit():
            return self.lex_number()
        
        # Identifiers/keywords
        if self.current_char.isalpha() or self.current_char == '_':
            return self.lex_ident()
        
        # Unknown character
        char = self.advance()
        return Token(TokenType.NOTOKEN, char, start_line, start_col)
    
    def tokenize(self) -> Iterator[Token]:
        """Tokenize the entire source, yielding tokens."""
        while True:
            token = self.next_token()
            yield token
            if token.type == TokenType.EOF:
                break
