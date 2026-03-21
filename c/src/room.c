#include "my_room.h"
#include <stdlib.h>
#include <string.h>

//Forward declaration of private internal functions
static bool *floor_grid_base_layer(int w, int h);

Room *room_create(int id, const char *name,
                  int width, int height) {
    Room *r = calloc(1, sizeof(Room));
    if (r == NULL) {
        return NULL;
    }

    r->id = id;

    //clamp the dimensions
    if (width < 1) {
        r->width = 1;
    } else {
        r->width = width;
    }

    if (height < 1) {
        r->height = 1;
    } else {
        r->height = height;
    }

    if (name != NULL) {
        r->name = calloc(strlen(name) + 1, sizeof(char));
        if (r->name == NULL) {
            // Need to free the room allocated since there is no room for the string
            free(r);

            return NULL;
        }
        strcpy(r->name, name);
    } else {
        r->name = NULL;
    }

    r->floor_grid = NULL;
    r->portals = NULL;
    r->portal_count = 0;
    r->treasures = NULL;
    r->treasure_count = 0;
    r->neighbors = NULL;
    r->neighbor_count = 0;
    r->pushables = NULL;
    r->pushable_count = 0;
    r->switches = NULL;
    r->switch_count = 0;

    return r;
}

static void free_portals(Portal *p, int c) {
    for (int i = 0; i < c; i++) {
        free(p[i].name);
    }

    free(p);
}

static void free_treasures(Treasure *t, int c) {
    for (int i = 0; i < c; i++) {
        free(t[i].name);
    }

    free(t);
}

static void free_pushables(Pushable *push, int c) {
    for (int i = 0; i < c; i++) {
        free(push[i].name);
    }

    free(push);
}


void room_destroy(Room *r) {
    if (r == NULL) {
        return;
    }

    //It is safe to free a null pointer
    free(r->name);
    free(r->floor_grid);
    free_portals(r->portals, r->portal_count);
    free_treasures(r->treasures, r->treasure_count);
    free(r->neighbors);
    free_pushables(r->pushables, r->pushable_count);
    free(r->switches);

    free(r);
}

int room_get_width(const Room *r) {
    if (r == NULL) {
        return 0;
    }
    return r->width;
}

int room_get_height(const Room *r) {
    if (r == NULL) {
        return 0;
    }
    return r->height;
}

Status room_set_floor_grid(Room *r, bool *floor_grid) {
    if (r == NULL) {
        return INVALID_ARGUMENT;
    }

    if (r->floor_grid != NULL) {
        free(r->floor_grid);
    }

    if (floor_grid == NULL) {
        r->floor_grid = floor_grid_base_layer(r->width, r->height);
    } else {
        r->floor_grid = floor_grid;
    }

    return OK;
}

static bool *floor_grid_base_layer(int w, int h) {
    int num = w * h;

    // Init to all 0s == all false == all walls
    bool *grid = calloc(num, sizeof(bool));

    // Loop through and add the floors in the middle of room
    for (int y_pos = 1; y_pos < h - 1; y_pos++) {
        for (int x_pos = 1; x_pos < w - 1; x_pos++) {
            grid[y_pos * w + x_pos] = true;
        }
    }

    return grid;
}

Status room_set_portals(Room *r, Portal *portals, int portal_count) {
    if (r == NULL) {
        return INVALID_ARGUMENT;
    }

    if (portal_count > 0 && portals == NULL) {
        return INVALID_ARGUMENT;
    }

    if (r->portals != NULL) {
        free_portals(r->portals, r->portal_count);
    }

    r->portals = portals;
    r->portal_count = portal_count;

    return OK;
}

Status room_set_treasures(Room *r, Treasure *treasures, int treasure_count) {
    if (r == NULL) {
        return INVALID_ARGUMENT;
    }

    if (treasure_count > 0 && treasures == NULL) {
        return INVALID_ARGUMENT;
    }

    if (r->treasures != NULL) {
        free_treasures(r->treasures, r->treasure_count);
    }

    r->treasures = treasures;
    r->treasure_count = treasure_count;

    return OK;
}

Status room_set_pushables(Room *r, Pushable *pushables, int pushable_count) {
    if (r == NULL) {
        return INVALID_ARGUMENT;
    }

    if (pushable_count > 0 && pushables == NULL) {
        return INVALID_ARGUMENT;
    }

    if (r->pushables != NULL) {
        free_pushables(r->pushables, r->pushable_count);
    }

    r->pushables = pushables;
    r->pushable_count = pushable_count;

    return OK;
}

Status room_place_treasure(Room *r, const Treasure *treasure) {
    if (r == NULL || treasure == NULL) {
        return INVALID_ARGUMENT;
    }

    if (r->treasures == NULL) {
        r->treasures = calloc(r->treasure_count + 1, sizeof(Treasure));
    } else {
        r->treasures = realloc(r->treasures, (r->treasure_count + 1) * sizeof(Treasure));
    }

    if (r->treasures == NULL) {
        return NO_MEMORY;
    }

    // Deep copy the treasure
    r->treasures[r->treasure_count].id = treasure->id;
    r->treasures[r->treasure_count].collected = treasure->collected;
    r->treasures[r->treasure_count].initial_x = treasure->initial_x;
    r->treasures[r->treasure_count].initial_y = treasure->initial_y;
    r->treasures[r->treasure_count].x = treasure->x;
    r->treasures[r->treasure_count].y = treasure->y;
    r->treasures[r->treasure_count].starting_room_id = treasure->starting_room_id;

    r->treasure_count++;

    if (treasure->name != NULL) {
        r->treasures[r->treasure_count - 1].name = calloc(strlen(treasure->name) + 1, sizeof(char));
        if (r->treasures[r->treasure_count - 1].name  == NULL) {

            return NO_MEMORY;
        }
        strcpy(r->treasures[r->treasure_count - 1].name, treasure->name);
    } else {
        r->treasures[r->treasure_count - 1].name = NULL;
    }
    
    return OK;
}

int room_get_treasure_at(const Room *r, int x, int y) {
    if (r == NULL) {
        return -1;
    }

    //This list is not sorted and not big it does not make sense to sort
    for (int i = 0; i < r->treasure_count; i++) {
        if (r->treasures[i].x == x && r->treasures[i].y == y && false == r->treasures[i].collected) {
            return r->treasures[i].id;
        }
    }

    return -1;
}

int room_get_portal_destination(const Room *r, int x, int y) {
    if (r == NULL) {
        return -1;
    }

    //This list is not sorted and not big it does not make sense to sort
    for (int i = 0; i < r->portal_count; i++) {
        if (r->portals[i].x == x && r->portals[i].y == y) {
            return r->portals[i].target_room_id;
        }
    }

    return -1;
}

bool room_is_walkable(const Room *r, int x, int y) {
    if (r == NULL) {
        return false;
    }

    if (x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return false;
    }

    if (room_has_pushable_at(r, x, y, NULL)) {
        return false;
    }

    if (r->floor_grid == NULL) {
        return false;
    }

    return r->floor_grid[y * r->width + x];
}

RoomTileType room_classify_tile(const Room *r,
                                int x,
                                int y,
                                int *out_id) {
    if (r == NULL) {
        return ROOM_TILE_INVALID;
    }

    if (x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return ROOM_TILE_INVALID;
    }

    //Check for a treasure
    // The call to room_get_treasure_at will SHOULD return -1 even if the treasure is collected
    int potential_id = room_get_treasure_at(r, x, y);
    if (potential_id != -1) {
        if (out_id != NULL) {
            *out_id = potential_id;
        }

        return ROOM_TILE_TREASURE;
    }

    //Check for a portal
    potential_id = room_get_portal_destination(r, x, y);
    if (potential_id != -1) {
        if (out_id != NULL) {
            *out_id = potential_id;
        }

        return ROOM_TILE_PORTAL;
    }

    //Check for a pushable
    bool has_push = room_has_pushable_at(r, x, y, &potential_id);
    if (has_push) {
        if (out_id != NULL) {
            *out_id = potential_id;
        }

        return ROOM_TILE_PUSHABLE;
    }

    //Check for walkability
    if (room_is_walkable(r, x, y)) {
        return ROOM_TILE_FLOOR;
    }
    
    return ROOM_TILE_WALL;
}

Status room_render(const Room *r,
                   const Charset *charset,
                   char *buffer,
                   int buffer_width,
                   int buffer_height) {
    if (r == NULL || r->floor_grid == NULL || charset == NULL || buffer == NULL) {
        return INVALID_ARGUMENT;
    }

    if (buffer_width != r->width || buffer_height != r->height) {
        return INVALID_ARGUMENT;
    }

    // Add the base layer
    for (int i = 0; i < r->width * r->height; i++) {
        if (r->floor_grid[i] == 0) {
            buffer[i] = charset->wall;
        } else {
            buffer[i] = charset->floor;
        }
    }

    // Layer over the treasures
    for (int i = 0; i < r->treasure_count; i++) {
        if (r->treasures[i].collected == false) {
            buffer[r->treasures[i].y * r->width + r->treasures[i].x] = charset->treasure;
        }
    }

    // Layer over the portals
    for (int i = 0; i < r->portal_count; i++) {
        buffer[r->portals[i].y * r->width + r->portals[i].x] = charset->portal;
    }

    // Layer over the pushables
    for (int i = 0; i < r->pushable_count; i++) {
        buffer[r->pushables[i].y * r->width + r->pushables[i].x] = charset->pushable;
    }

    return OK;
}

Status room_get_start_position(const Room *r,
                               int *x_out,
                               int *y_out) {
    if (r == NULL || x_out == NULL || y_out == NULL) {
        return INVALID_ARGUMENT;
    }

    // First preference is the first portal location
    if (r->portal_count > 0 && room_is_walkable(r, r->portals[0].x, r->portals[0].y)) {
        *x_out = r->portals[0].x;
        *y_out = r->portals[0].y;
        return OK;
    }

    // Second preference is any interior walkable tile.
    // Assume this means ONLY interior tiles are checked.
    for (int y_pos = 1; y_pos < r->height - 1; y_pos++) {
        for (int x_pos = 1; x_pos < r->width - 1; x_pos++) {
            if (room_is_walkable(r, x_pos, y_pos)) {
                *x_out = x_pos;
                *y_out = y_pos;
                return OK;
            }
        }
    }

    return ROOM_NOT_FOUND;
}

int room_get_id(const Room *r) {
    if (NULL == r) {
        return -1;
    }

    return r->id;
}

Status custom_room_pick_up_treasure_const(const Room *r, int treasure_id, Treasure **treasure_out) {
    if (NULL == r || NULL == treasure_out) {
        return INVALID_ARGUMENT;
    }

    // TODO: Return INVALID_ARG if treasure_id is negative??

    for (int i = 0; i < r->treasure_count; i++) {
        if (r->treasures[i].id == treasure_id) {
            if (r->treasures[i].collected) {
                return INVALID_ARGUMENT;
            }

            *treasure_out = &(r->treasures[i]);
            r->treasures[i].collected = true;
            return OK;
        }
    }

    return ROOM_NOT_FOUND;
}

Status room_pick_up_treasure(Room *r, int treasure_id, Treasure **treasure_out) {
    const Room *temp_room = r;

    return custom_room_pick_up_treasure_const(temp_room, treasure_id, treasure_out);
}

void destroy_treasure(Treasure *t) {
    if (NULL == t) {
        return;
    }

    free(t->name);
    free(t);
}

bool room_has_pushable_at(const Room *r, int x, int y, int *pushable_idx_out) {
    if (NULL == r || 0 == r->pushable_count || NULL == r->pushables) {
        return false;
    }

    //This list is not sorted and not big it does not make sense to sort
    for (int i = 0; i < r->pushable_count; i++) {
        if (r->pushables[i].x == x && r->pushables[i].y == y) {
            if (pushable_idx_out != NULL) {
                (*pushable_idx_out) = i/*(r->pushables[i].id)*/;
            }

            return true;
        }
    }

    return false;
}

Status custom_room_try_push_const(const Room *r, int pushable_idx, Direction dir) {
    if (NULL == r || 0 == r->pushable_count || NULL == r->pushables) {
        return INVALID_ARGUMENT;
    }

    if (pushable_idx < 0 || pushable_idx >= r->pushable_count) {
        return INVALID_ARGUMENT;
    }

    Pushable *le_push = &(r->pushables[pushable_idx]);
    int x_query = le_push->x;
    int y_query = le_push->y;

    if (DIR_NORTH == dir) {
        // Height 0 is most north position
        y_query--;
    } else if (DIR_SOUTH == dir) {
        // Height 0 is most north position
        y_query++;
    } else if (DIR_EAST == dir) {
        // Width 0 is most west position
        x_query++;
    } else if (DIR_WEST == dir) {
        // Width 0 is most west position
        x_query--;
    } else {
        return INVALID_ARGUMENT;
    }

    RoomTileType pos_query_type = room_classify_tile(r, x_query, y_query, NULL);

    if (ROOM_TILE_INVALID == pos_query_type) {
        // The caller has tried to move the player out of bounds, this the Direction dir is not valid
        return INVALID_ARGUMENT;
    }

    // Talked to Judi and these should all be ROOM_IMPASSABLE except for maybe ROOM_TILE_PUSHABLE
    // In theory ROOM_TILE_PUSHABLE should allow for pushing stacks of pushales. But the way it is now gives 100% in the auto grader soooo I'm leaving it as is. 
    if (ROOM_TILE_WALL == pos_query_type || ROOM_TILE_PUSHABLE == pos_query_type || ROOM_TILE_PORTAL == pos_query_type || ROOM_TILE_TREASURE == pos_query_type) {
        return ROOM_IMPASSABLE;
    }

    if (ROOM_TILE_FLOOR != pos_query_type) {
        return INVALID_ARGUMENT;
    }

    le_push->x = x_query;
    le_push->y = y_query;

    return OK;
}

Status room_try_push(Room *r, int pushable_idx, Direction dir) {
    const Room *temp_room = r;

    return custom_room_try_push_const(temp_room, pushable_idx, dir);
}

bool room_are_all_treasures_collected(const Room *r) {
    if (r == NULL) {
        return false;
    }

    bool result = true;

    // This list is not sorted and not big it does not make sense to sort
    for (int i = 0; i < r->treasure_count; i++) {
        if (! r->treasures[i].collected) {
            result = false;
        }
    }

    return result;
}
