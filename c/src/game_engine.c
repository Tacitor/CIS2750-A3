#include "my_game_engine.h"
#include <stdlib.h>
#include "world_loader.h"
#include "player.h"
#include "my_room.h"
#include "graph.h"

Status game_engine_create(const char *config_file_path, GameEngine **engine_out) {
    if (config_file_path == NULL || engine_out == NULL) {
        return INVALID_ARGUMENT;
    }

    *engine_out = calloc(1, sizeof(GameEngine));

    if (*engine_out == NULL) {
        return NO_MEMORY;
    }

    Room *rm_first = NULL;
    Status temp_stat = loader_load_world(config_file_path, &((*engine_out)->graph), &rm_first, &((*engine_out)->room_count), &((*engine_out)->charset));

    if (temp_stat != OK) {
        game_engine_destroy(*engine_out);
        *engine_out = NULL;
        return WL_ERR_DATAGEN;
    }

    // Init the initial room related variables
    (*engine_out)->initial_room_id = rm_first->id;
    temp_stat = room_get_start_position(rm_first, &((*engine_out)->initial_player_x), &((*engine_out)->initial_player_y));

    if (temp_stat != OK) {
        game_engine_destroy(*engine_out);
        *engine_out = NULL;
        return WL_ERR_DATAGEN;
    }

    // Init player
    temp_stat = player_create((*engine_out)->initial_room_id, (*engine_out)->initial_player_x, (*engine_out)->initial_player_y, &((*engine_out)->player));

    if (temp_stat != OK) {
        game_engine_destroy(*engine_out);
        *engine_out = NULL;
        return NO_MEMORY;
    }

    return OK;
}

void game_engine_destroy(GameEngine *eng) {
    if (eng == NULL) {
        return;
    }

    player_destroy(eng->player);
    graph_destroy(eng->graph);

    free(eng);
}

const Player *game_engine_get_player(const GameEngine *eng) {
    if (eng == NULL) {
        return NULL;
    }
    
    return eng->player;
}

static Status internal_get_room_from_id(const GameEngine *eng, int room_id, const Room **room_out) {
    Room* temp_rm = NULL;

    // Create a filler room with the same ID to match the compare func of the graph
    temp_rm = room_create(room_id, NULL, 1, 1);

    if (temp_rm == NULL) {
        return NO_MEMORY;
    }
    
    if (!graph_contains(eng->graph, temp_rm)) {
        room_destroy(temp_rm);
        return GE_NO_SUCH_ROOM;
    }

    *room_out = graph_get_payload(eng->graph, temp_rm);
    room_destroy(temp_rm);

    if ((*room_out) == NULL) {
        return GE_NO_SUCH_ROOM;
    }

    return OK;
}

Status internal_bump_query_in_dir(int *x_query, int *y_query, Direction dir) {
    if (DIR_NORTH == dir) {
        // Height 0 is most north position
        (*y_query)--;
    } else if (DIR_SOUTH == dir) {
        // Height 0 is most north position
        (*y_query)++;
    } else if (DIR_EAST == dir) {
        // Width 0 is most west position
        (*x_query)++;
    } else if (DIR_WEST == dir) {
        // Width 0 is most west position
        (*x_query)--;
    } else {
        return INVALID_ARGUMENT; //HERE
    }

    return OK;
}

Status internal_portal_move(GameEngine *eng, int portal_taget_room_id) {
    const Room *portal_targ_room = NULL;
    Status stat = internal_get_room_from_id(eng, portal_taget_room_id, &portal_targ_room);

    if (GE_NO_SUCH_ROOM == stat || NULL == portal_targ_room) {
        return GE_NO_SUCH_ROOM;
    }

    if (OK != stat) {
        return INTERNAL_ERROR;
    }

    int portal_targ_room_x = -1;
    int portal_targ_room_y = -1;
    stat = room_get_start_position(portal_targ_room, &portal_targ_room_x, &portal_targ_room_y);

    if (OK != stat || -1 == portal_targ_room_x || -1 == portal_targ_room_y) {
        return INTERNAL_ERROR;
    }

    // Since the destination room ID was found to be that of a real room and the start pos is valid we can move the player there
    eng->player->room_id = portal_taget_room_id;
    eng->player->x = portal_targ_room_x;
    eng->player->y = portal_targ_room_y;

    return OK;
}

static Status internal_parse_classified_tile(GameEngine *eng, RoomTileType pos_query_type, int query_tile_id, int x_query, int y_query, const Room *current_room, Direction dir) {
    if (ROOM_TILE_INVALID == pos_query_type || ROOM_TILE_WALL == pos_query_type) {
        // The caller has tried to move the player out of bounds (ROOM_TILE_INVALID), this the Direction dir is not valid
        // OR tried to move the player into a wall.
        return ROOM_IMPASSABLE;
    }

    if (ROOM_TILE_FLOOR == pos_query_type) {
        // Just a regular floor can update player position
        // A treasure tile can update player position (visual only in A1, will be collected in A2).
        eng->player->x = x_query;
        eng->player->y = y_query;
    } else if (ROOM_TILE_PUSHABLE == pos_query_type) {
        int stat = custom_room_try_push_const(current_room, query_tile_id, dir);

        if (INVALID_ARGUMENT == stat) {
            return INTERNAL_ERROR;
        }

        if (ROOM_IMPASSABLE == stat) {
            return ROOM_IMPASSABLE;
        }
        
        // The room has pushed the pushable in the direction specified. The player may now take this spot.
        if (OK == stat) {
            // TODO: Does there need to be another walkbale check? Is a pushable ever going to be on a non-floor tile?
            // They are able to be on top of portals so there is a chance I guess...
            eng->player->x = x_query;
            eng->player->y = y_query;
        }
    } else if (ROOM_TILE_TREASURE == pos_query_type) {
        Treasure *t_out = NULL;
        int stat = custom_room_pick_up_treasure_const(current_room, query_tile_id, &t_out);

        if (OK != stat || NULL == t_out) {
            return INTERNAL_ERROR;
        }

        stat = player_try_collect(eng->player, t_out);

        if (OK != stat) {
            return INTERNAL_ERROR;
        }
    } else if (ROOM_TILE_PORTAL == pos_query_type) {
        int stat = internal_portal_move(eng, query_tile_id);

        if (OK != stat) {
            return stat;
        }
    } else {
        return INTERNAL_ERROR;
    }

    return OK;
}

Status game_engine_move_player(GameEngine *eng, Direction dir) {
    if (NULL == eng || NULL == eng->player) {
        return INVALID_ARGUMENT;
    }

    const Room *current_room = NULL;
    Status stat = internal_get_room_from_id(eng, eng->player->room_id, &current_room);

    if (GE_NO_SUCH_ROOM == stat || NULL == current_room) {
        return GE_NO_SUCH_ROOM;
    }

    if (OK != stat) {
        return INTERNAL_ERROR;
    }

    int x_query = eng->player->x;
    int y_query = eng->player->y;
    stat = internal_bump_query_in_dir(&x_query, &y_query, dir);

    if (OK != stat) {
        return INVALID_ARGUMENT;
    }    

    int query_tile_id = -1;
    RoomTileType pos_query_type = room_classify_tile(current_room, x_query, y_query, &query_tile_id);

    stat = internal_parse_classified_tile(eng, pos_query_type, query_tile_id, x_query, y_query, current_room, dir);

    if (OK != stat) {
        return stat;
    }

    return OK;
}

Status game_engine_get_room_count(const GameEngine *eng, int *count_out) {
    if (eng == NULL) {
        return INVALID_ARGUMENT;
    }

    if (count_out == NULL) {
        return NULL_POINTER;
    }

    *count_out = eng->room_count;

    return OK;
}

Status game_engine_get_room_dimensions(const GameEngine *eng, int *width_out, int *height_out) {
    if (NULL == eng) {
        return INVALID_ARGUMENT;
    }

    if (NULL == width_out || NULL == height_out) {
        return NULL_POINTER;
    }

    if (NULL == eng->player) {
        return INTERNAL_ERROR;
    }

    const Room *current_room = NULL;
    Status stat = internal_get_room_from_id(eng, eng->player->room_id, &current_room);

    if (GE_NO_SUCH_ROOM == stat || NULL == current_room) {
        return GE_NO_SUCH_ROOM;
    }

    if (OK != stat) {
        return INTERNAL_ERROR;
    }

    *width_out = current_room->width;
    *height_out = current_room->height;

    return OK;
}

Status game_engine_get_room_name(const GameEngine *eng, char **name_out) {
    if (NULL == eng) {
        return INVALID_ARGUMENT;
    }

    if (NULL == name_out) {
        return NULL_POINTER;
    }

    if (NULL == eng->player) {
        return INTERNAL_ERROR;
    }

    const Room *current_room = NULL;
    Status stat = internal_get_room_from_id(eng, eng->player->room_id, &current_room);

    if (GE_NO_SUCH_ROOM == stat || NULL == current_room) {
        return GE_NO_SUCH_ROOM;
    }

    if (OK != stat) {
        return INTERNAL_ERROR;
    }

    *name_out = current_room->name;

    return OK;
}

Status game_engine_reset(GameEngine *eng) {
    if (NULL == eng) {
        return INVALID_ARGUMENT;
    }

    if (NULL == eng->player) {
        return INTERNAL_ERROR;
    }

    player_reset_to_start(eng->player, eng->initial_room_id, eng->initial_player_x, eng->initial_player_y);

    // Get all the Rooms to reset the treasures
    const void *const *rooms = NULL;
    int rm_count = 0;

    GraphStatus temp_stat = graph_get_all_payloads(eng->graph, &rooms, &rm_count);

    if (temp_stat != GRAPH_STATUS_OK || rm_count != eng->room_count) {
        return INTERNAL_ERROR;
    }

    const Room *temp_room = NULL;
    for (int i = 0; i < rm_count; i++) {
        temp_room = rooms[i];

        for (int j = 0; j < temp_room->treasure_count; j++) {
            temp_room->treasures[j].collected = false;
            temp_room->treasures[j].x = temp_room->treasures[j].initial_x;
            temp_room->treasures[j].y = temp_room->treasures[j].initial_y;
        }

        for (int j = 0; j < temp_room->pushable_count; j++) {
            temp_room->pushables[j].x = temp_room->pushables[j].initial_x;
            temp_room->pushables[j].y = temp_room->pushables[j].initial_y;
        }
    }

    return OK;
}

Status game_engine_render_current_room(const GameEngine *eng, char **str_out) {
    if (eng == NULL || str_out == NULL) {
        return INVALID_ARGUMENT;
    }

    if (NULL == eng->player) {
        return INTERNAL_ERROR;
    }

    const Room *gr_out_room = NULL;

    if (OK != internal_get_room_from_id(eng, eng->player->room_id, &gr_out_room) || gr_out_room == NULL) {
        return INTERNAL_ERROR;
    }

    int buff_num = gr_out_room->width * gr_out_room->height;
    char buff[buff_num];

    if (OK != room_render(gr_out_room, &(eng->charset), buff, gr_out_room->width, gr_out_room->height)) {
        return INTERNAL_ERROR;
    }

    buff[eng->player->y * gr_out_room->width + eng->player->x] = eng->charset.player;

    // Make room for 1D buffer, add room for a CR char at end of each row, add one more for a NULL terminator
    int out_num = buff_num + gr_out_room->height + 1;
    *str_out = calloc(out_num, sizeof(char));

    int x = 0;
    int y = 0;

    for (int i = 0; i < buff_num; i++) {
        (*str_out)[i + y] = buff[i];
        x++;

        if (x >= gr_out_room->width) {
            x = 0;
            
            (*str_out)[i + y + 1] = '\n';
            y++;
        }
    }

    (*str_out)[out_num - 1] = '\0';

    return OK;
}

Status game_engine_render_room(const GameEngine *eng, int room_id, char **str_out) {
    if (eng == NULL) {
        return INVALID_ARGUMENT;
    }

    if (str_out == NULL) {
        return NULL_POINTER;
    }

    const Room *gr_out_room = NULL;

    if (OK != internal_get_room_from_id(eng, room_id, &gr_out_room) || gr_out_room == NULL) {
        return GE_NO_SUCH_ROOM;
    }

    int buff_num = gr_out_room->width * gr_out_room->height;
    char buff[buff_num];

    if (OK != room_render(gr_out_room, &(eng->charset), buff, gr_out_room->width, gr_out_room->height)) {
        return GE_NO_SUCH_ROOM;
    }

    // Make room for 1D buffer, add room for a CR char at end of each row, add one more for a NULL terminator
    int out_num = buff_num + gr_out_room->height + 1;
    *str_out = calloc(out_num, sizeof(char));

    int x = 0;
    int y = 0;

    for (int i = 0; i < buff_num; i++) {
        (*str_out)[i + y] = buff[i];
        x++;

        if (x >= gr_out_room->width) {
            x = 0;
            
            (*str_out)[i + y + 1] = '\n';
            y++;
        }
    }

    (*str_out)[out_num - 1] = '\0';

    return OK;
}

Status game_engine_get_room_ids(const GameEngine *eng, int **ids_out, int *count_out) {
    if (eng == NULL) {
        return INVALID_ARGUMENT;
    }

    if (ids_out == NULL || count_out == NULL) {
        return NULL_POINTER;
    }

    const void *const *rooms = NULL;
    int rm_count = 0;

    GraphStatus temp_stat = graph_get_all_payloads(eng->graph, &rooms, &rm_count);

    if (temp_stat != GRAPH_STATUS_OK || rm_count != eng->room_count) {
        return INTERNAL_ERROR;
    }

    *ids_out = calloc(rm_count, sizeof(int));

    if (*ids_out == NULL) {
        return NO_MEMORY;
    }

    *count_out = rm_count;
    const Room *temp_room = NULL;

    for (int i = 0; i < *count_out; i++) {
        temp_room = rooms[i];
        (*ids_out)[i] = temp_room->id;
    }

    return OK;
}

void game_engine_free_string(void *ptr) {
    if (NULL == ptr) {
        return;
    }

    free(ptr);
}

Status game_engine_parse_treasure(const Treasure * const *treasures, int treasure_index, int *id_out, char **name_out, int *starting_room_out,
    int *initial_x_out, int *initial_y_out, int *x_out, int *y_out, bool *collected_out) {

    if (NULL == treasures || NULL == id_out || NULL == name_out || NULL == starting_room_out || NULL == initial_x_out || NULL == initial_y_out || NULL == x_out || NULL == y_out || NULL == collected_out) {
        return NULL_POINTER;
    }

    *id_out = treasures[treasure_index]->id;
    *name_out = treasures[treasure_index]->name;
    *starting_room_out = treasures[treasure_index]->starting_room_id;
    *initial_x_out = treasures[treasure_index]->initial_x;
    *initial_y_out = treasures[treasure_index]->initial_y;
    *x_out = treasures[treasure_index]->x;
    *y_out = treasures[treasure_index]->y;
    *collected_out = treasures[treasure_index]->collected;

    return OK;
}
