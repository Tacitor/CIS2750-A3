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

/**
 * Get the total number of treasures in all the rooms.
 * 
 * Returns:
 *  OK on success
 *  INVALID_ARGUMENT if eng is NULL
 *  NULL_POINTER if count_out is NULL
 *  INTERNAL_ERROR if there was an issue accessing the rooms interal to the GameEngine
 */
Status game_engine_get_world_treasure_count(const GameEngine *eng, int *count_out);

/**
 * Get the number of rooms that are completed. A complete room is one in which the player
 * has collected all the treasures.
 * 
 * Returns:
 *  OK on success
 *  INVALID_ARGUMENT if eng is NULL
 *  NULL_POINTER if count_out is NULL
 *  INTERNAL_ERROR if there was an issue accessing the rooms interal to the GameEngine
 */
Status game_engine_complete_room_count(const GameEngine *eng, int *count_out);

/**
 * Check all the portals in the current room to see if one of them is gated.
 * Keep in mind, "The generator only creates a switch if the room has
 * at least 1 portal and 1 pushable. At most one switch per room"
 * 
 * Returns:
 *  INVALID_ARGUMENT if eng or eng->player is NULL
 *  NULL_POINTER if has_gated, x_out, or y_out is NULL
 *  OK on success
 *  has_gated is false if none of the portals are gated, or the gated one is unlocked.
 *  has_gated is true if a gated portal exists and it is locked.
 *      The x and y positon of this portal is then put in x_out and y_out
 */
Status game_engine_query_gated_portal_current_room(const GameEngine *eng, bool *has_gated, int *x_out, int *y_out);

#endif /* MY_GAME_ENGINE_H */
