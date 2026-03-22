import ctypes
from ctypes import (
    c_int, c_double, c_char_p, c_size_t, c_void_p, POINTER, byref, c_bool
)

from ..bindings import Direction, lib, Status, Charset
from .exceptions import GameEngineError, status_to_exception
from .player import Player

class GameEngine:
    def __init__(self, config_path: str):
        temp_eng = c_void_p()
        stat = lib.game_engine_create(config_path.encode("utf-8"), byref(temp_eng))

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed to create the GameEngine. Status: " + str(stat))

        self._eng = temp_eng

        temp_plr = lib.game_engine_get_player(self._eng)

        if temp_plr is None:
            raise RuntimeError("ERROR: failed to create the player in GameEngine constructor")

        self._player = temp_plr
        self.player = Player(self._player)

    def destroy(self) -> None:
        # Defensive: free once, ignore errors.
        try:
            if getattr(self, "_eng", None):
                lib.game_engine_destroy(self._eng)
                self._eng = None
        except Exception:
            # Avoid raising from __del__
            pass

    def move_player(self, direction: Direction) -> None:
        stat = lib.game_engine_move_player(self._eng, direction)

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed to move the player. Status: " + str(stat))

    def render_current_room(self) -> str:
        c_string = c_char_p()
        stat = lib.game_engine_render_current_room(self._eng, byref(c_string))

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed to render the current room. Status: " + str(stat))

        py_string = c_string.value.decode("utf-8")
        lib.game_engine_free_string(c_string)
        return py_string

    def get_room_count(self) -> int:
        count_out = c_int()
        stat = lib.game_engine_get_room_count(self._eng, byref(count_out))

        if stat == Status.OK:
            return count_out.value

        raise status_to_exception(stat, "ERROR: failed to get the room count. Status: " + str(stat))

    def get_room_dimensions(self) -> tuple[int, int]:
        width = c_int()
        height = c_int()
        stat = lib.game_engine_get_room_dimensions(self._eng, byref(width), byref(height))

        if stat == Status.OK:
            return width.value, height.value

        raise status_to_exception(stat, "ERROR: failed to get the dimensions of the current room. Status: " + str(stat))

    def get_room_name(self) -> str:
        c_string_name = c_char_p()
        stat = lib.game_engine_get_room_name(self._eng, byref(c_string_name))

        if stat == Status.OK:
            return c_string_name.value.decode("utf-8")

        raise status_to_exception(stat, "ERROR: failed to get the name of the current room. Status: " + str(stat))

    def get_charset(self) -> dict[str, str]:
        c_char_set = lib.game_engine_get_charset(self._eng)

        if c_char_set is None:
            raise RuntimeError("ERROR: failed to get the charset from the GameEngine")

        py_char_set = {
            "wall": c_char_set.contents.wall.decode("utf-8"),
            "floor": c_char_set.contents.floor.decode("utf-8"),
            "player": c_char_set.contents.player.decode("utf-8"),
            "pushable": c_char_set.contents.pushable.decode("utf-8"),
            "treasure": c_char_set.contents.treasure.decode("utf-8"),
            "portal": c_char_set.contents.portal.decode("utf-8"),
            "switch_off": c_char_set.contents.switch_off.decode("utf-8"),
            "switch_on": c_char_set.contents.switch_on.decode("utf-8")
        }

        return py_char_set

    def get_room_ids(self) -> list[int]:
        all_ids = POINTER(c_int)()
        id_count = c_int()
        id_list = []

        stat = lib.game_engine_get_room_ids(self._eng, byref(all_ids), byref(id_count))

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed get the room IDs from the GameEngine. Status: " + str(stat))

        for i in range(id_count.value):
            id_list.append(all_ids[i])

        lib.game_engine_free_string(all_ids)
        return id_list

    def reset(self) -> None:
        stat = lib.game_engine_reset(self._eng)

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed to reset the GameEngine. Status: " + str(stat))

    def underfoot_portal(self) -> None:
        stat = lib.game_engine_underfoot_portal(self._eng)

        if stat != Status.OK:
            raise status_to_exception(stat, "ERROR: failed to use an underfoot portal by the GameEngine. Status: " + str(stat))

    def get_world_treasure_count(self) -> int:
        count_out = c_int()
        stat = lib.game_engine_get_world_treasure_count(self._eng, byref(count_out))

        if stat == Status.OK:
            return count_out.value

        raise status_to_exception(stat, "ERROR: failed to get the total number of treasures in the world. Status: " + str(stat))

    def get_complete_room_count(self) -> int:
        count_out = c_int()
        stat = lib.game_engine_complete_room_count(self._eng, byref(count_out))

        if stat == Status.OK:
            return count_out.value

        raise status_to_exception(stat, "ERROR: failed to get the count of complete rooms. Status: " + str(stat))

    def query_gated_portal(self) -> tuple[bool, int, int]:
        has_gated = c_bool()
        x_out = c_int()
        y_out = c_int()
        stat = lib.game_engine_query_gated_portal_current_room(self._eng, byref(has_gated), byref(x_out), byref(y_out))

        if stat == Status.OK:
            return has_gated.value, x_out.value, y_out.value

        raise status_to_exception(stat, "ERROR: failed to query the gated portal. Status: " + str(stat))
