#ifndef MY_GAME_ENGINE_H
#define MY_GAME_ENGINE_H

#include "game_engine.h"

Status game_engine_parse_treasure(const Treasure * const *treasures, int treasure_index, int *id_out, char **name_out, int *starting_room_out,
    int *initial_x_out, int *initial_y_out, int *x_out, int *y_out, bool *collected_out);

Status game_engine_get_room_name(const GameEngine *eng, char **name_out);

#endif /* MY_GAME_ENGINE_H */
