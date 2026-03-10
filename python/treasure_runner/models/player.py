import ctypes

from ctypes import (
    c_int, c_double, c_char_p, c_size_t, c_void_p, POINTER, byref, c_bool
)

from ..bindings import lib, Status, Treasure
from .exceptions import status_to_status_exception

class Player:
    # set the instance variable _ptr to the opaque Player type defined in bindings.c
    def __init__(self, ptr):
        self._ptr = ptr

    # get the room using a function created in bindings.c
    def get_room(self) -> int:
        return lib.player_get_room(self._ptr)

    # get the player's position using a function created in bindings.c
    def get_position(self) -> tuple[int, int]:
        x = c_int()
        y = c_int()
        stat = lib.player_get_position(self._ptr, byref(x), byref(y))

        if stat == Status.OK:
            return x.value, y.value

        raise status_to_status_exception(stat, "ERROR: failed to get the player position. Status: " + str(stat))

    def get_collected_count(self) -> int:
        return lib.player_get_collected_count(self._ptr)

    def has_collected_treasure(self, treasure_id: int) -> bool:
        return lib.player_has_collected_treasure(self._ptr, treasure_id)

    def get_collected_treasures(self) -> list[dict]:
        count = c_int()

        treas = lib.player_get_collected_treasures(self._ptr, byref(count))

        if treas is None:
            raise RuntimeError("ERROR: failed to get collected treasures of the player")

        # Have this stupid hard to read list to please the linter. I had seperate vars for all of them, but that was "too many local vars" so enjoy reading this. Enjoy the "simplicity" of this.
        parsed_treasure = [c_int(), c_char_p(), c_int(), c_int(), c_int(), c_int(), c_int(), c_bool()]

        list_out = []

        for i in range(count.value):
            index = c_int(i)
            stat = lib.game_engine_parse_treasure(treas, index, byref(parsed_treasure[0]), byref(parsed_treasure[1]), byref(parsed_treasure[2]),
                                                byref(parsed_treasure[3]), byref(parsed_treasure[4]), byref(parsed_treasure[5]), byref(parsed_treasure[6]), byref(parsed_treasure[7]))

            if stat != Status.OK:
                raise status_to_status_exception(stat, "ERROR: failed to get treasure at index " + str(index) + " from the player. Status: " + str(stat))

            if parsed_treasure[1].value is None:
                name_str_py = None
            else:
                name_str_py = parsed_treasure[1].value.decode("utf-8")

            new_dict = {
                "id": parsed_treasure[0].value,
                "name": name_str_py,
                "starting_room_id": parsed_treasure[2].value,
                "initial_x": parsed_treasure[3].value,
                "initial_y": parsed_treasure[4].value,
                "x": parsed_treasure[5].value,
                "y": parsed_treasure[6].value,
                "collected": parsed_treasure[7].value,
                }

            list_out.append(new_dict)

        return list_out
