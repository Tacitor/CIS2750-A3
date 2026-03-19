"""
bindings package - Direct ctypes interface to C library

This package provides low-level ctypes bindings to the C libraries.
Normal usage should go through the models package instead.
"""

from .bindings import (
    lib,
    GameEngineStatus,
    Direction,
    Status,
    Treasure,
    Charset
)

# For compatibility
GameEngineBinding = lib

__all__ = [
    'lib',
    'GameEngineStatus',
    'Direction',
    'Status',
    'Treasure',
    'GameEngineBinding',
    'Charset',
]
