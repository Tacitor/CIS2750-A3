#ifndef MY_GAME_ENGINE_H
#define MY_GAME_ENGINE_H

#include "game_engine.h"

Status game_engine_parse_treasure(const Treasure * const *treasures, int treasure_index, int *id_out, char **name_out, int *starting_room_out,
    int *initial_x_out, int *initial_y_out, int *x_out, int *y_out, bool *collected_out);

Status game_engine_get_room_name(const GameEngine *eng, char **name_out);

const Charset *game_engine_get_charset(const GameEngine *eng);

/**
 * Teleport the player if they are standing on top of a portal.
 * This can only happen in portal_placement=interior is set in the config .ini file.
 * 
 * Returns:
 *  - OK on success
 *  - ROOM_NO_PORTAL if the player is not standing on a portal
 *  - INVALID_ARGUMENT, GE_NO_SUCH_ROOM, INTERNAL_ERROR, etc. as needed
 */
Status game_engine_underfoot_portal(GameEngine *eng);

#endif /* MY_GAME_ENGINE_H */
