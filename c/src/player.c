#include "player.h"
#include <stdlib.h>

//TESTME: Update tests with updated behaviour that is new from A2 API
Status player_create(int initial_room_id, int initial_x,
                     int initial_y, Player **player_out) {
    if (player_out == NULL) {
        return INVALID_ARGUMENT;
    }

    Player *p = calloc(1, sizeof(Player));
    if (p == NULL) {
        return NO_MEMORY;
    }

    p->room_id = initial_room_id;
    p->x = initial_x;
    p->y = initial_y;

    p->collected_count = 0;
    p->collected_treasures = NULL;

    *player_out = p;
    return OK;

}

//TESTME: Update tests with updated behaviour that is new from A2 API
void player_destroy(Player *p) {
    if (p == NULL) {
        return;
    }

    free(p->collected_treasures);
    p->collected_treasures = NULL;

    free(p);
}

int player_get_room(const Player *p) {
    if (p == NULL) {
        return -1;
    }

    return p->room_id;
}

Status player_get_position(const Player *p, int *x_out, int *y_out) {
    if (p == NULL || x_out == NULL || y_out == NULL) {
        return INVALID_ARGUMENT;
    }

    *x_out = p->x;
    *y_out = p->y;
    return OK;

}

Status player_set_position(Player *p, int x, int y) {
    if (p == NULL) {
        return INVALID_ARGUMENT;
    }

    p->x = x;
    p->y = y;
    return OK;
}

Status player_move_to_room(Player *p, int new_room_id) {
    if (p == NULL) {
        return INVALID_ARGUMENT;
    }

    p->room_id = new_room_id;
    return OK;
}

//TESTME: Update tests with updated behaviour that is new from A2 API
Status player_reset_to_start(Player *p,
                             int starting_room_id,
                             int start_x,
                             int start_y) {
    if (p == NULL) {
        return INVALID_ARGUMENT;
    }

    // No need to check the returned Status since the NULL player has early return.
    player_move_to_room(p, starting_room_id);
    player_set_position(p, start_x, start_y);

    free(p->collected_treasures);
    p->collected_treasures = NULL;
    p->collected_count = 0;
    return OK;
}

Status player_try_collect(Player *p, Treasure *treasure) {
    if (NULL == p || NULL == treasure) {
        return NULL_POINTER;
    }

    if (player_has_collected_treasure(p, treasure->id)) {
        return INVALID_ARGUMENT;
    }

    if (0 != p->collected_count && NULL == p->collected_treasures) {
        return INVALID_ARGUMENT;
    }

    if (NULL == p->collected_treasures) {
        // Start the array of treasure pointers. Just need 1 to start and collected_count should be 0.
        p->collected_treasures = calloc(p->collected_count + 1, sizeof(Treasure*));
    } else {
        p->collected_treasures = realloc(p->collected_treasures, (p->collected_count + 1) * sizeof(Treasure*));
    }

    if (NULL == p->collected_treasures) {
        return NO_MEMORY;
    }

    p->collected_treasures[p->collected_count] = treasure;
    treasure->collected = true;

    p->collected_count++;
    return OK;
}

bool player_has_collected_treasure(const Player *p, int treasure_id) {
    if (NULL == p) {
        return false;
    }

    if (treasure_id < 0 || 0 == p->collected_count || NULL == p->collected_treasures) {
        return false;
    }

    for (int i = 0; i < p->collected_count; i++) {
        if (p->collected_treasures[i]->id == treasure_id) {
            return true;
        }
    }

    return false;
}

int player_get_collected_count(const Player *p) {
    if (NULL == p) {
        return 0;
    }

    return p->collected_count;
}

const Treasure * const *player_get_collected_treasures(const Player *p, int *count_out) {
    if (NULL == p || NULL == count_out) {
        return NULL;
    }

    *count_out = p->collected_count;
    return (const Treasure * const *)(p->collected_treasures);
}
