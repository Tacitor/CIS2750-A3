#ifndef MY_ROOM_H
#define MY_ROOM_H

#include "room.h"

/*
 * Remove a treasure from the room by ID.
 *
 * Preconditions:
 *   r and treasure_out are not NULL 
 *   treasure_id is not negative
 *
 * Parameters:
 *   treasure_out: receives a pointer to the room-owned Treasure on success
 *
 * Returns:
 *   OK on success (treasure_out is set)
 *   INVALID_ARGUMENT if r or treasure_out is NULL, or treasure already collected
 *   ROOM_NOT_FOUND if treasure_id does not exist in the room
 *
 * Postconditions:
 *   If the treasure was found, its .collected status was set to true
 *   No other Room data has been modified in any way
 *
 * Ownership:
 *   The room retains ownership of the Treasure. Callers must NOT free it.
 */
Status custom_room_pick_up_treasure_const(const Room *r, int treasure_id, Treasure **treasure_out);

/*
 * Attempt to push a pushable in the given direction.
 *
 * Preconditions:
 *   r is not NULL 
 *   pushable_idx is not negative and is less than r->pushable_count
 *   dir is a valid member of the enum type Direction
 *
 * Returns:
 *   OK on success
 *   ROOM_IMPASSABLE if blocked
 *   INVALID_ARGUMENT if arguments are invalid
 *
 * Postconditions:
 *   If push was possible, the pushable's x and y coordinates in r->pushables
 *   have been correctly updated - i.e. the obstacle was pushed
 */
Status custom_room_try_push_const(const Room *r, int pushable_idx, Direction dir);

/*
 * Set pushables (ownership transfers to the room).
 *
 * Preconditions:
 *   r must not be NULL.
 *   If pushable_count > 0, pushables must not be NULL.
 *
 * Postconditions:
 *   On success, ownership of pushables transfers to the room.
 *   If the pushable array in the room was previously initialized:
 *   - It must be overwritten by the new pushable array.
 *   - The new pushable count must be correct.
 *   - The old pushable array must be freed, along with old pushable names.
 *
 * Returns:
 *   OK on success
 *   INVALID_ARGUMENT if room is NULL or parameters are inconsistent
 */
Status room_set_pushables(Room *r, Pushable *pushables, int pushable_count);

Status convert_dg_err_to_wl(int dg_status);

#endif /* MY_ROOM_H */
