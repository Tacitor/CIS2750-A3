#include "world_loader.h"
#include <stdlib.h>
#include <stdio.h>
#include "datagen.h"
#include "graph.h"
#include "my_room.h"

/* ============================================================
 * Room compare/destroy callbacks for initializing the graph
 * ============================================================ */

static int compare_rooms(const void *a, const void *b) {
    const Room *ra = a;
    const Room *rb = b;
    if (ra->id < rb->id) return -1;
    if (ra->id > rb->id) return 1;
    return 0;
}

static void destroy_room(void *p) {
    room_destroy(p);
}

static Status check_config_file(const char *config_file) {
    if (config_file == NULL) {
        return INVALID_ARGUMENT;
    }
    
    //Create a file pointer
    FILE *conf_f = fopen(config_file, "r");

    // Guarding for opening the input file. Causes early return.
    if (conf_f == NULL) {
        return WL_ERR_CONFIG;
    }

    // This file path is verified good, and should be closed before passing to datagen
    if (fclose(conf_f) != 0) {
        return WL_ERR_CONFIG;
    }

    return OK;
}

Status convert_dg_err_to_wl(int dg_status) {
    if (dg_status == DG_ERR_CONFIG) {
        return WL_ERR_CONFIG;
    } 

    if (dg_status == DG_ERR_OOM) {
        return NO_MEMORY;
    } 

    if (dg_status == DG_ERR_INTERNAL) {
        return WL_ERR_DATAGEN;
    }

    return OK;
}

static void deep_copy_portals_and_treasures(DG_Room dg_room, Room *game_room) {
    Portal *ports = NULL;
    Treasure *treas = NULL;
    Status r_stat = OK;

    // Deep copy the portals
    ports = calloc(dg_room.portal_count, sizeof(Portal));

    for (int i = 0; i < dg_room.portal_count; i++) {
        ports[i].id = dg_room.portals[i].id;
        ports[i].name = NULL;
        ports[i].x = dg_room.portals[i].x;
        ports[i].y = dg_room.portals[i].y;
        ports[i].target_room_id = dg_room.portals[i].neighbor_id;
        // TODO: Deal with .gated, and .switch_id
    }

    r_stat = room_set_portals(game_room, ports, dg_room.portal_count);

    if (r_stat != OK) {
        printf("WL ERROR: Unexpected fault in portal assignment to room ID %d\n", dg_room.id);
    }

    // Deep copy the treasures
    treas = calloc(dg_room.treasure_count, sizeof(Treasure));

    for (int i = 0; i < dg_room.treasure_count; i++) {
        treas[i].id = dg_room.treasures[i].global_id;
        treas[i].name = NULL;
        treas[i].starting_room_id = dg_room.id;
        treas[i].x = dg_room.treasures[i].x;
        treas[i].y = dg_room.treasures[i].y;
        treas[i].initial_x = dg_room.treasures[i].x;
        treas[i].initial_y = dg_room.treasures[i].y;
        treas[i].collected = false;
    }

    r_stat = room_set_treasures(game_room, treas, dg_room.treasure_count);

    if (r_stat != OK) {
        printf("WL ERROR: Unexpected fault in treasure assignment to room ID %d\n", dg_room.id);
    }
}

static void deep_copy_pushables(DG_Room dg_room, Room *game_room) {
    Pushable *pushes = NULL;
    Status r_stat = OK;

    // Deep copy the pushables
    pushes = calloc(dg_room.pushable_count, sizeof(Pushable));

    for (int i = 0; i < dg_room.pushable_count; i++) {
        pushes[i].id = dg_room.pushables[i].id;
        pushes[i].name = NULL;
        pushes[i].initial_x = dg_room.pushables[i].x;
        pushes[i].x = dg_room.pushables[i].x;
        pushes[i].initial_y = dg_room.pushables[i].y;
        pushes[i].y = dg_room.pushables[i].y;
    }
    
    r_stat = room_set_pushables(game_room, pushes, dg_room.pushable_count);

    if (r_stat != OK) {
        printf("WL ERROR: Unexpected fault in pushables assignment to room ID %d\n", dg_room.id);
    }
}

static void deep_copy_switches(DG_Room dg_room, Room *game_room) {
    Switch *sws = NULL;
    Status r_stat = OK;

    // Deep copy the switches
    sws = calloc(dg_room.switch_count, sizeof(Switch));

    for (int i = 0; i < dg_room.switch_count; i++) {
        sws[i].id = dg_room.switches[i].id;
        sws[i].x = dg_room.switches[i].x;
        sws[i].y = dg_room.switches[i].y;
        sws[i].portal_id = dg_room.switches[i].portal_id;
    }
    
    r_stat = room_set_switches(game_room, sws, dg_room.switch_count);

    if (r_stat != OK) {
        printf("WL ERROR: Unexpected fault in switches assignment to room ID %d\n", dg_room.id);
    }
}

static char* room_id_to_name(int id) {
    char* name = NULL;

    switch (id) {
        case 0:
            name = "Start";
            break;
        case 1:
            name = "First";
            break;
        case 2:
            name = "Second";
            break;
        case 3:
            name = "Third";
            break;
        case 4:
            name = "Fourth";
            break;
        case 5:
            name = "Fifth";
            break;
        default:
            name = "Regular";
            break;
    }

    return name;
}

Status loader_load_world(const char *config_file,
                         Graph **graph_out,
                         Room **first_room_out,
                         int  *num_rooms_out,
                         Charset *charset_out) {
    // NOT SPECIFIED IN HEADER FILE. Check for bad input anyways.
    if (NULL == graph_out || NULL == first_room_out || NULL == num_rooms_out || NULL == charset_out) {
        return INVALID_ARGUMENT;
    }

    int conf_stat = check_config_file(config_file);
    if (OK != conf_stat) {
        return conf_stat;
    }

    GraphStatus g_stat = graph_create(compare_rooms, destroy_room, graph_out);
    
    if (GRAPH_STATUS_OK != g_stat) {
        return NO_MEMORY;
    }

    int dg_status = start_datagen(config_file);

    if (OK != convert_dg_err_to_wl(dg_status)) {
        graph_destroy(*graph_out);
        return convert_dg_err_to_wl(dg_status);
    }

    Room *game_room = NULL;
    Status r_stat = OK;
    bool *fl_grid = NULL;
    int fl_len = 0;

    *num_rooms_out = 0;
    while (has_more_rooms()) {
        DG_Room dg_room = get_next_room();

        game_room = room_create(dg_room.id, room_id_to_name(dg_room.id), dg_room.width, dg_room.height);

        // Assign first_room_out before the next iteration of the loop assigned a new mem address to game_room var.
        if (0 == *num_rooms_out) {
            *first_room_out = game_room;
        }

        // Deep copy floor grid
        fl_len = dg_room.width * dg_room.height;
        fl_grid = calloc(fl_len, sizeof(bool));

        for (int i = 0; i < fl_len; i++) {
            fl_grid[i] = dg_room.floor_grid[i];
        }

        r_stat = room_set_floor_grid(game_room, fl_grid);

        if (r_stat != OK) {
            printf("WL ERROR: Unexpected fault in floor_grid assignment to room ID %d\n", dg_room.id);
        }
        
        // Copy the rooms and portals in seperate function call to get under the Clang Tidy cognitive complexity limit threshold of 25
        deep_copy_portals_and_treasures(dg_room, game_room);
        deep_copy_pushables(dg_room, game_room);
        deep_copy_switches(dg_room, game_room);

        // Add this new Room * to the graph
        g_stat = graph_insert(*graph_out, game_room);

        if (GRAPH_STATUS_OK != g_stat) {
            printf("WL ERROR: Unexpected fault inserting room ID %d into directed graph. Graph status: %d\n", dg_room.id, g_stat);
        }

        (*num_rooms_out)++;
    }

    // Deep copy the char set
    const DG_Charset *dg_set = dg_get_charset();
    charset_out->floor = dg_set->floor;
    charset_out->player = dg_set->player;
    charset_out->portal = dg_set->portal;
    charset_out->pushable = dg_set->pushable;
    charset_out->treasure = dg_set->treasure;
    charset_out->wall = dg_set->wall;
    charset_out->switch_on = dg_set->switch_on;
    charset_out->switch_off = dg_set->switch_off;

    stop_datagen();

    return OK;
}
