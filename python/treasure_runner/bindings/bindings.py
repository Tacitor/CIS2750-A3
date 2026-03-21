"""
 Low-level ctypes bindings

This module provides direct ctypes access to the C library functions.
It handles:
  - Loading the shared library
  - Defining C enums and structures
  - Wrapping C function signatures
  - Managing error codes from the C layer

This is a thin layer - no error handling or convenience wrappers.
All error handling is done in the models layer.
"""

import ctypes
import os
from enum import IntEnum
from pathlib import Path
from ctypes import (
    c_int, c_double, c_char_p, c_size_t, c_void_p, POINTER, c_bool
)

# ============================================================
# Enums matching C definitions
# ============================================================

class Direction(IntEnum):
    """Movement directions (matches DIR_* in types.h)."""
    NORTH = 0
    SOUTH = 1
    EAST = 2
    WEST = 3


class Status(IntEnum):
    """Status codes for room and player operations."""
    OK = 0
    INVALID_ARGUMENT = 1
    NULL_POINTER = 2
    NO_MEMORY = 3
    BOUNDS_EXCEEDED = 4
    INTERNAL_ERROR = 5
    ROOM_IMPASSABLE = 6
    ROOM_NO_PORTAL = 7
    ROOM_NOT_FOUND = 8
    GE_NO_SUCH_ROOM = 9
    WL_ERR_CONFIG = 10
    WL_ERR_DATAGEN = 11

# Backwards compatibility for existing imports
GameEngineStatus = Status


# ============================================================
# C Structures - Opaque types only
# ============================================================

# Treasure is used by player_get_collected_treasures
class Treasure(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_int),
        ("name", ctypes.c_char_p),
        ("starting_room_id", ctypes.c_int),
        ("initial_x", ctypes.c_int),
        ("initial_y", ctypes.c_int),
        ("x", ctypes.c_int),
        ("y", ctypes.c_int),
        ("collected", ctypes.c_bool),
    ]

class Charset(ctypes.Structure):
    _fields_ = [
        ("wall", ctypes.c_char),
        ("floor", ctypes.c_char),
        ("player", ctypes.c_char),
        ("pushable", ctypes.c_char),
        ("treasure", ctypes.c_char),
        ("portal", ctypes.c_char),
        ("switch_off", ctypes.c_char),
        ("switch_on", ctypes.c_char)
    ]

# ============================================================
# Library Loading
# ============================================================

def _find_library():
    """Locate libbackend.so under the project dist directory."""
    # Optional override via env
    env_path = os.getenv("TREASURE_RUNNER_DIST")
    candidates = []

    if env_path:
        candidates.append(Path(env_path) / "libbackend.so")
        candidates.append(Path(env_path) / "libpuzzlegen.so")

    # Project-relative: ../../dist relative to this file
    here = Path(__file__).resolve()
    repo_root = here.parent.parent.parent.parent
    candidates.append(repo_root / "dist" / "libbackend.so")
    candidates.append(repo_root / "dist" / "libpuzzlegen.so")

    found = {}
    for path in candidates:
        if path.exists():
            found[path.name] = path

    if "libbackend.so" in found:
        # Ensure puzzlegen is loaded first if present to satisfy dependencies.
        puzzlegen = found.get("libpuzzlegen.so")
        if puzzlegen:
            ctypes.CDLL(str(puzzlegen))
        return str(found["libbackend.so"])

    tried = "\n".join(str(p) for p in candidates)
    raise RuntimeError(f"libbackend.so not found. Paths tried:\n{tried}")


# Load the library
_LIB_PATH = _find_library()
lib = ctypes.CDLL(_LIB_PATH)


# ============================================================
# C Function Signatures - GameEngine
# ============================================================

# Opaque pointer type for GameEngine
GameEngine = ctypes.c_void_p

# Opaque pointer type for Player
Player = ctypes.c_void_p

# Function Signatures (argtypes and restype)
# Status game_engine_create(const char *config_file_path, GameEngine **engine_out);
lib.game_engine_create.argtypes = [c_char_p, POINTER(GameEngine)]
lib.game_engine_create.restype = c_int

# void game_engine_destroy(GameEngine *eng);
lib.game_engine_destroy.argtypes = [GameEngine]
lib.game_engine_destroy.restype = None

# const Player *game_engine_get_player(const GameEngine *eng);
lib.game_engine_get_player.argtypes = [GameEngine]
lib.game_engine_get_player.restype = Player

# Status game_engine_move_player(GameEngine *eng, Direction dir);
lib.game_engine_move_player.argtypes = [GameEngine, c_int]
lib.game_engine_move_player.restype = c_int

# Status game_engine_render_current_room(const GameEngine *eng, char **str_out);
lib.game_engine_render_current_room.argtypes = [GameEngine, POINTER(ctypes.c_char_p)]
lib.game_engine_render_current_room.restype = c_int

# Status game_engine_get_room_count(const GameEngine *eng, int *count_out);
lib.game_engine_get_room_count.argtypes = [GameEngine, POINTER(c_int)]
lib.game_engine_get_room_count.restype = c_int

# Status game_engine_get_room_dimensions(const GameEngine *eng, int *width_out, int *height_out);
lib.game_engine_get_room_dimensions.argtypes = [GameEngine, POINTER(c_int), POINTER(c_int)]
lib.game_engine_get_room_dimensions.restype = c_int

# Status game_engine_get_room_name(const GameEngine *eng, char **name_out)
lib.game_engine_get_room_name.argtypes = [GameEngine, POINTER(ctypes.c_char_p)]
lib.game_engine_get_room_name.restype = c_int

# Status game_engine_get_room_ids(const GameEngine *eng, int **ids_out, int *count_out);
lib.game_engine_get_room_ids.argtypes = [GameEngine, POINTER(POINTER(c_int)), POINTER(c_int)]
lib.game_engine_get_room_ids.restype = c_int

# Status game_engine_reset(GameEngine *eng);
lib.game_engine_reset.argtypes = [GameEngine]
lib.game_engine_reset.restype = c_int

# const Charset *game_engine_get_charset(const GameEngine *eng)
lib.game_engine_get_charset.argtypes = [GameEngine]
lib.game_engine_get_charset.restype = POINTER(Charset)

# Status game_engine_underfoot_portal(GameEngine *eng)
lib.game_engine_underfoot_portal.argtypes = [GameEngine]
lib.game_engine_underfoot_portal.restype = c_int

# Status game_engine_get_world_treasure_count(const GameEngine *eng, int *count_out)
lib.game_engine_get_world_treasure_count.argtypes = [GameEngine, POINTER(c_int)]
lib.game_engine_get_world_treasure_count.restype = c_int

# Status game_engine_complete_room_count(const GameEngine *eng, int *count_out)
lib.game_engine_complete_room_count.argtypes = [GameEngine, POINTER(c_int)]
lib.game_engine_complete_room_count.restype = c_int

# ============================================================
# C Function Signatures - Player
# ============================================================

# int player_get_room(const Player *p);
lib.player_get_room.argtypes = [Player]
lib.player_get_room.restype = c_int

# Status player_get_position(const Player *p, int *x_out, int *y_out);
lib.player_get_position.argtypes = [Player, POINTER(c_int), POINTER(c_int)]
lib.player_get_position.restype = c_int

# int player_get_collected_count(const Player *p);
lib.player_get_collected_count.argtypes = [Player]
lib.player_get_collected_count.restype = c_int

# bool player_has_collected_treasure(const Player *p, int treasure_id);
lib.player_has_collected_treasure.argtypes = [Player, c_int]
lib.player_has_collected_treasure.restype = c_bool

# const Treasure * const *player_get_collected_treasures(const Player *p, int *count_out);
lib.player_get_collected_treasures.argtypes = [Player, POINTER(c_int)]
lib.player_get_collected_treasures.restype = POINTER(Treasure)

# Status player_reset_to_start(Player *p, int starting_room_id, int start_x, int start_y);
lib.player_reset_to_start.argtypes = [Player, c_int, c_int, c_int]
lib.player_reset_to_start.restype = c_int

lib.game_engine_parse_treasure.argtypes = [POINTER(Treasure), c_int, POINTER(c_int), POINTER(c_char_p), POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_bool)]
lib.game_engine_parse_treasure.restype = c_int

# ============================================================
# C Function Signatures - Room
# ============================================================

# Room is opaque - no direct room accessors exposed to Python
Room = ctypes.c_void_p


# ============================================================
# Memory Management
# ============================================================

# void game_engine_free_string(void *ptr);
lib.game_engine_free_string.argtypes = [c_void_p]
lib.game_engine_free_string.restype = None

# void destroy_treasure(Treasure *t);
lib.destroy_treasure.argtypes = [Treasure]
lib.destroy_treasure.restype = None
