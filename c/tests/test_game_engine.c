#include <check.h>
#include <stdlib.h>
#include "game_engine.h"
#include "player.h"
#include "room.h"

#include <stdio.h>

#define CONFIG_PATH_SIMPLE "../assets/starter.ini"
#define CONFIG_PATH_SMALL "../assets/small_start.ini"
#define CONFIG_PATH_PUSH "../assets/pushables.ini"

#define ROOM_0 "###################\n#..........#......#\n#$..#......#......#\n#...#......#......X\n#.#........#...##.#\nX...##....##..#####\n#...#......#...#..#\n#...#......#...$..#\n#...#......#......#\nX..$.......#......#\n#..$#......#......#\n#.................X\n###################\n"
#define ROOM_0_WITH_PLAYER "###################\n#..........#......#\n#$..#......#......#\n#...#......#......X\n#.#........#...##.#\n@...##....##..#####\n#...#......#...#..#\n#...#......#...$..#\n#...#......#......#\nX..$.......#......#\n#..$#......#......#\n#.................X\n###################\n"
#define ROOM_1 "##############XX###\n#....#............#\n#....#............#\n##.#####.#..##.####\n#....#..........$.#\n#....#...#........#\n#....$...#........#\n#..#.#...###......#\n#....#...###......#\n#....#...#........#\n#..$.#...#........#\n#....#............#\n##############X####\n"
#define ROOM_2 "######################\n#....#...............#\n#$...#....#.....#....#\n#....#....#.....#....#\n#....#....$..........#\n#.###############..###\n#....#...............#\n#....###..#.....#....#\nX....###..#..........#\n#.#####..##.##########\n#....##.#.#.....#....X\n#....#....#.....#....#\n#....#....#.....#....#\n#....#....#..........#\n#.......$.#.....#....#\n#......$..#.....#....#\n######################\n"
#define ROOM_3 "################\nX..$.......#...#\nX..#..#....O...#\n#...@O......O..#\nX..#.......#...#\n#$.###.#.#.####X\n#..#........$..X\nX..#...........#\nX..#...........X\n#.....$......O.#\n################\n"
#define ROOM_4 "################\nX..$.......#...#\nX..#..#....O...#\n#...........O..#\nX..#.......#...#\n#$.###.#.#.####X\n#.O#........$..X\nX..#...........#\n@..#...........X\n#.....$......O.#\n################\n"

/* ============================================================
 * Setup and Teardown fixtures
 * ============================================================ */

static GameEngine *ge = NULL;

static void setup_game_engine(void)
{
    ck_assert_int_eq(game_engine_create(CONFIG_PATH_SIMPLE, &ge), OK);
    ck_assert_ptr_nonnull(ge);
}


static void teardown_game_engine(void)
{
    /* game_engine_destroy must free all memory related to the game engine */
    game_engine_destroy(ge);
    ge = NULL;
}

/* ============================================================
 * Test 1: Create a simple game engine from starter.ini
 * Tests that game_engine_create successfully returns OK on creation
 * This simple test just asserts nonnull on ge since the setup_game_engine() function asserts the OK code return.
 * ============================================================ */

START_TEST(test_game_engine_simple)
{
    ck_assert_ptr_nonnull(ge);
}
END_TEST

/* ============================================================
 * Test 2: Create GE from NULLPTR
 * Tests that game_engine_create successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_game_engine_null)
{
    ck_assert_ptr_null(ge);

    ck_assert_int_eq(game_engine_create(NULL, &ge), INVALID_ARGUMENT);
    ck_assert_ptr_null(ge);

    ck_assert_int_eq(game_engine_create(CONFIG_PATH_SIMPLE, NULL), INVALID_ARGUMENT);
    ck_assert_ptr_null(ge);
}
END_TEST

/* ============================================================
 * Test 3: Test getting the room count
 * Tests that game_engine_get_room_count successfully returns the right number of rooms from starter.ini
 * ============================================================ */

START_TEST(test_game_engine_get_room_count)
{
    int count = 0;
    ck_assert_int_eq(game_engine_get_room_count(ge, &count), OK);
    ck_assert_int_eq(count, 3);
}
END_TEST

/* ============================================================
 * Test 4: Test getting the room count from a NULL room or NULL count out
 * Tests that game_engine_get_room_count successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_game_engine_get_room_count_null)
{
    int count = 0;
    ck_assert_int_eq(game_engine_get_room_count(NULL, &count), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_count(ge, NULL), NULL_POINTER);
    ck_assert_int_eq(count, 0);
}
END_TEST

/* ============================================================
 * Test 5: Test redering a room by ID
 * Tests that game_engine_render_room successfully returns the string for printing
 * ============================================================ */

START_TEST(test_game_engine_render_room)
{
    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_room(ge, 0, &buff), OK);
    ck_assert_str_eq(buff, ROOM_0);
    free(buff);
    buff = NULL;

    ck_assert_int_eq(game_engine_render_room(ge, 1, &buff), OK);
    ck_assert_str_eq(buff, ROOM_1);
    free(buff);
    buff = NULL;

    ck_assert_int_eq(game_engine_render_room(ge, 2, &buff), OK);
    ck_assert_str_eq(buff, ROOM_2);
    free(buff);
    buff = NULL;
}
END_TEST

/* ============================================================
 * Test 6: Room IDs
 * Tests that game_engine_get_room_ids returns the expected room IDs in the expected order from starter.ini
 * ============================================================ */

START_TEST(test_game_engine_get_room_ids)
{
    int *ids_out;
    int count_out;
    ck_assert_int_eq(game_engine_get_room_ids(ge, &ids_out, &count_out), OK);

    ck_assert_int_eq(ids_out[0], 0);
    ck_assert_int_eq(ids_out[1], 1);
    ck_assert_int_eq(ids_out[2], 2);

    free(ids_out);
}
END_TEST

/* ============================================================
 * Test 7: Test moving the player on a null room
 * Tests that game_engine_move_player successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_game_engine_move_null)
{
    ck_assert_int_eq(game_engine_move_player(NULL, DIR_NORTH), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_move_player(ge, 4), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_move_player(ge, -1), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 8: Test moving the player each direction
 * Tests that game_engine_move_player successfully moves the player north, east, south, and west
 * ============================================================ */

START_TEST(test_game_engine_move_4_dir)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 2);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 2);
    ck_assert_int_eq(p->y, 6);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 2);
    ck_assert_int_eq(p->y, 7);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 2);
    ck_assert_int_eq(p->y, 6);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 6);
}
END_TEST

/* ============================================================
 * Test 9: Test moving the player out of bounds
 * Tests that game_engine_move_player successfully returns INVALID_ARGUMENT if the player tries to leave the room
 * ============================================================ */

START_TEST(test_game_engine_move_out_of_bounds)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), ROOM_IMPASSABLE);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);
}
END_TEST

/* ============================================================
 * Test 10: Test moving the player into a wall
 * Tests that game_engine_move_player successfully returns ROOM_IMPASSABLE if the player tries to walk into a wall
 * ============================================================ */

START_TEST(test_game_engine_move_into_wall)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 6);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), ROOM_IMPASSABLE);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 6);
}
END_TEST

/* ============================================================
 * Test 11: Test moving the player onto a treasure
 * Tests that game_engine_move_player successfully returns OK if the player tries to walk onto a treasure
 * ============================================================ */

START_TEST(test_game_engine_move_onto_treas)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 3);
    ck_assert_int_eq(p->y, 8);

    // Verify stepping onto treasure
    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    // Step a second time since the first is just for the picking up, the second moved the player on the now free spot
    ck_assert_int_eq(game_engine_move_player(ge, DIR_SOUTH), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 3);
    ck_assert_int_eq(p->y, 9);

    //Verify stepping off the treasure
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 2);
    ck_assert_int_eq(p->y, 9);
}
END_TEST

/* ============================================================
 * Test 12: Test moving the player into a portal
 * Tests that game_engine_move_player successfully returns OK if the player tries to walk into a wall
 * ============================================================ */

START_TEST(test_game_engine_move_into_portal)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 1);
    ck_assert_int_eq(p->y, 5);

    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);
    ck_assert_int_eq(p->room_id, 2);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 8);
}
END_TEST

/* ============================================================
 * Test 13: Test getting the dimensions of a null engine
 * Tests that game_engine_get_room_dimensions successfully returns INVALID_ARGUMENT or NULL_POINTER if the inputs are null
 * ============================================================ */

START_TEST(test_game_engine_dim_null)
{
    int w_out = -1;
    int h_out = -1;
    ck_assert_int_eq(game_engine_get_room_dimensions(NULL, &w_out, &h_out), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_dimensions(ge, NULL, &h_out), NULL_POINTER);
    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &w_out, NULL), NULL_POINTER);

    ck_assert_int_eq(w_out, -1);
    ck_assert_int_eq(h_out, -1);
}
END_TEST

/* ============================================================
 * Test 14: Test getting the dimensions of the starter room
 * Tests that game_engine_get_room_dimensions successfully returns 19 by 13
 * ============================================================ */

START_TEST(test_game_engine_dim_start)
{
    int w_out = -1;
    int h_out = -1;
    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &w_out, &h_out), OK);

    ck_assert_int_eq(w_out, 19);
    ck_assert_int_eq(h_out, 13);
}
END_TEST

/* ============================================================
 * Test 15: Test getting the dimensions of room ID 1 & 2
 * Tests that game_engine_get_room_dimensions successfully returns 19 by 13, and 22 by 17
 * ============================================================ */

START_TEST(test_game_engine_dim_room_2)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);

    int w_out = -1;
    int h_out = -1;
    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &w_out, &h_out), OK);

    ck_assert_int_eq(w_out, 22);
    ck_assert_int_eq(h_out, 17);
    ck_assert_int_eq(p->room_id, 2);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);

    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &w_out, &h_out), OK);

    ck_assert_int_eq(w_out, 19);
    ck_assert_int_eq(h_out, 13);
    ck_assert_int_eq(p->room_id, 1);
}
END_TEST

/* ============================================================
 * Test 16: Test resetting to the starter room but enging is null
 * Tests that game_engine_reset successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_game_engine_reset_null)
{
    ck_assert_int_eq(game_engine_reset(NULL), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 17: Test resetting to the starter room from romm 2
 * Tests that game_engine_reset successfully returns the player to start
 * ============================================================ */

START_TEST(test_game_engine_reset_room_1)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);

    ck_assert_int_eq(p->room_id, 2);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 8);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);

    ck_assert_int_eq(p->room_id, 1);
    ck_assert_int_eq(p->x, 14);
    ck_assert_int_eq(p->y, 0);

    ck_assert_int_eq(game_engine_reset(ge), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);
}
END_TEST

/* ============================================================
 * Test 18: Test resetting to the starter room from room 1
 * Tests that game_engine_reset successfully returns the player to start
 * ============================================================ */

START_TEST(test_game_engine_reset_room_2)
{
    const Player *p = game_engine_get_player(ge);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    // Step into portal and verify new room and start position
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);

    ck_assert_int_eq(p->room_id, 2);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 8);

    ck_assert_int_eq(game_engine_reset(ge), OK);
    ck_assert_int_eq(p->room_id, 0);
    ck_assert_int_eq(p->x, 0);
    ck_assert_int_eq(p->y, 5);
}
END_TEST

/* ============================================================
 * Test 19: Test redering a room by ID on NULL input
 * Tests that game_engine_render_room successfully returns INVALID_ARGUMENT or NULL_POINTER
 * ============================================================ */

START_TEST(test_game_engine_render_room_null)
{
    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_room(NULL, 0, &buff), INVALID_ARGUMENT);
    ck_assert_ptr_null(buff);

    ck_assert_int_eq(game_engine_render_room(ge, 0, NULL), NULL_POINTER);
    ck_assert_ptr_null(buff);
}
END_TEST

/* ============================================================
 * Test 20: Test redering a room by ID on bad room ID
 * Tests that game_engine_render_room successfully returns GE_NO_SUCH_ROOM
 * ============================================================ */

START_TEST(test_game_engine_render_room_bad_id)
{
    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_room(ge, 4, &buff), GE_NO_SUCH_ROOM);
    ck_assert_ptr_null(buff);
}
END_TEST

/* ============================================================
 * Test 21: Test redering current room on NULL input
 * Tests that game_engine_render_current_room successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_game_engine_render_current_room_null)
{
    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_current_room(NULL, &buff), INVALID_ARGUMENT);
    ck_assert_ptr_null(buff);

    ck_assert_int_eq(game_engine_render_current_room(ge, NULL), INVALID_ARGUMENT);
    ck_assert_ptr_null(buff);
}
END_TEST

/* ============================================================
 * Test 22: Test redering current room
 * Tests that game_engine_render_current_room successfully returns the right buffer
 * ============================================================ */

START_TEST(test_game_engine_render_current_room)
{
    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_current_room(ge, &buff), OK);
    ck_assert_str_eq(buff, ROOM_0_WITH_PLAYER);
    free(buff);
    buff = NULL;
}
END_TEST

/* ============================================================
 * Test 23: Create GE from bad path
 * Tests that game_engine_create successfully returns WL_ERR_DATAGEN
 * and does not leak any memory.
 * ============================================================ */

START_TEST(test_game_engine_bad_path)
{
    ck_assert_ptr_null(ge);

    ck_assert_int_eq(game_engine_create("/bad/path/structure.ini", &ge), WL_ERR_DATAGEN);
    ck_assert_ptr_null(ge);
}
END_TEST

/* ============================================================
 * Test 24: Create GE with no start room
 * Tests that game_engine_create successfully returns WL_ERR_DATAGEN
 * and does not leak any memory.
 * ============================================================ */

START_TEST(test_game_engine_bad_start)
{
    ck_assert_ptr_null(ge);

    ck_assert_int_eq(game_engine_create(CONFIG_PATH_SMALL, &ge), WL_ERR_DATAGEN);
    ck_assert_ptr_null(ge);
}
END_TEST

/* ============================================================
 * Test 25: Test getting the player from the GE with NULL input
 * Tests that game_engine_get_player handle the negative null input successfully
 * ============================================================ */

START_TEST(test_game_engine_player_null)
{
    ck_assert_ptr_nonnull(ge);
    ck_assert_ptr_null(game_engine_get_player(NULL));
}
END_TEST

/* ============================================================
 * Test 26: Test bad rooms
 * Tests that the GE handels bad room ID successfully
 * ============================================================ */

START_TEST(test_game_engine_bad_rooms)
{
    ck_assert_ptr_nonnull(ge);

    // The starter.ini has 3 rooms. Valid IDs are: 0, 1, 2
    // Inject a bad current room ID
    ge->player->room_id = 4;

    int width = -1;
    int height = -1;
    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &width, &height), GE_NO_SUCH_ROOM);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), GE_NO_SUCH_ROOM);
}
END_TEST

/* ============================================================
 * Test 27: Move a player into a pushable
 * Tests that game_engine_move_player successfully returns OK and updates the world
 * ============================================================ */

START_TEST(test_game_engine_move_pushable)
{
    ck_assert_ptr_null(ge);
    ck_assert_int_eq(game_engine_create(CONFIG_PATH_PUSH, &ge), OK);
    ck_assert_ptr_nonnull(ge);

    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_WEST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_NORTH), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);
    ck_assert_int_eq(game_engine_move_player(ge, DIR_EAST), OK);

    char *buff = NULL;
    ck_assert_int_eq(game_engine_render_current_room(ge, &buff), OK);
    ck_assert_str_eq(buff, ROOM_3);
    game_engine_free_string(buff);
    game_engine_free_string(NULL); // Test this path too
    buff = NULL;

    // Now reset the room
    ck_assert_int_eq(game_engine_reset(ge), OK);

    ck_assert_int_eq(game_engine_render_current_room(ge, &buff), OK);
    ck_assert_str_eq(buff, ROOM_4);
    game_engine_free_string(buff);
    buff = NULL;
}
END_TEST

/* ============================================================
 * Test 28: Test the GE when the player is null
 * ============================================================ */

START_TEST(test_game_engine_null_player)
{
    ck_assert_ptr_nonnull(ge);

    Player *player_backup = ge->player;
    ge->player = NULL;
    int width = -1;
    int height = -1;
    char *buff = NULL;

    ck_assert_int_eq(game_engine_get_room_dimensions(ge, &width, &height), INTERNAL_ERROR);
    ck_assert_int_eq(game_engine_reset(ge), INTERNAL_ERROR);
    ck_assert_int_eq(game_engine_render_current_room(ge, &buff), INTERNAL_ERROR);

    int *ids_out = NULL;
    int count_out = -1;
    ck_assert_int_eq(game_engine_get_room_ids(NULL, &ids_out, &count_out), INVALID_ARGUMENT);
    ck_assert_int_eq(game_engine_get_room_ids(ge, NULL, &count_out), NULL_POINTER);
    ck_assert_int_eq(game_engine_get_room_ids(ge, &ids_out, NULL), NULL_POINTER);

    player_destroy(player_backup);
}
END_TEST

/* ============================================================
 * Suite Creation Function
 * 
 * This function builds and returns a test suite for the Check framework.
 * A Suite is a collection of test cases that are run together.
 * ============================================================ */

Suite *game_engine_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("Game Engine");
    
    // Create a test case to group related tests
    TCase *tc_create = tcase_create("Create");
    TCase *tc_basic = tcase_create("Basic");

    // Attach setup and teardown functions to run before/after each test
    // setup_player() runs before each test to create a fresh graph
    // teardown_player() runs after each test to clean up the graph
    tcase_add_checked_fixture(tc_create, NULL, teardown_game_engine);
    tcase_add_checked_fixture(tc_basic, setup_game_engine, teardown_game_engine);

    // Add individual test functions to the test case
    tcase_add_test(tc_create, test_game_engine_null);

    tcase_add_test(tc_basic, test_game_engine_simple);
    tcase_add_test(tc_basic, test_game_engine_get_room_count);
    tcase_add_test(tc_basic, test_game_engine_get_room_count_null);
    tcase_add_test(tc_basic, test_game_engine_render_room);
    tcase_add_test(tc_basic, test_game_engine_get_room_ids);

    tcase_add_test(tc_basic, test_game_engine_move_null);
    tcase_add_test(tc_basic, test_game_engine_move_4_dir);
    tcase_add_test(tc_basic, test_game_engine_move_out_of_bounds);
    tcase_add_test(tc_basic, test_game_engine_move_into_wall);
    tcase_add_test(tc_basic, test_game_engine_move_onto_treas);
    tcase_add_test(tc_basic, test_game_engine_move_into_portal);

    tcase_add_test(tc_basic, test_game_engine_dim_null);
    tcase_add_test(tc_basic, test_game_engine_dim_start);
    tcase_add_test(tc_basic, test_game_engine_dim_room_2);

    tcase_add_test(tc_basic, test_game_engine_reset_null);
    tcase_add_test(tc_basic, test_game_engine_reset_room_1);
    tcase_add_test(tc_basic, test_game_engine_reset_room_2);

    tcase_add_test(tc_basic, test_game_engine_render_room_null);
    tcase_add_test(tc_basic, test_game_engine_render_room_bad_id);

    tcase_add_test(tc_basic, test_game_engine_render_current_room_null);
    tcase_add_test(tc_basic, test_game_engine_render_current_room);

    tcase_add_test(tc_create, test_game_engine_bad_path);
    tcase_add_test(tc_create, test_game_engine_bad_start);
    tcase_add_test(tc_basic, test_game_engine_player_null);
    tcase_add_test(tc_basic, test_game_engine_bad_rooms);

    tcase_add_test(tc_create, test_game_engine_move_pushable);

    tcase_add_test(tc_basic, test_game_engine_null_player);

    // Add the test case to the suite
    suite_add_tcase(s, tc_create);
    suite_add_tcase(s, tc_basic);
    
    // Return the complete suite so main() can run it
    return s;
}
