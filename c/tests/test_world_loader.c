#include <check.h>
#include <stdlib.h>
#include "world_loader.h"
#include "graph.h"
#include "my_room.h"
#include "datagen.h"

#define ROOM_0 "####################..........#......##$..#......#......##...#......#......X#.#........#...##.#X...##....##..######...#......#...#..##...#......#...$..##...#......#......#X..$.......#......##..$#......#......##.................X###################"
#define ROOM_1 "##############XX####....#............##....#............###.#####.#..##.#####....#..........$.##....#...#........##....$...#........##..#.#...###......##....#...###......##....#...#........##..$.#...#........##....#............###############X####"
#define ROOM_2 "#######################....#...............##$...#....#.....#....##....#....#.....#....##....#....$..........##.###############..####....#...............##....###..#.....#....#X....###..#..........##.#####..##.###########....##.#.#.....#....X#....#....#.....#....##....#....#.....#....##....#....#..........##.......$.#.....#....##......$..#.....#....#######################"

/* ============================================================
 * Test 1: Call loader_load_world on bad file path
 * Tests that loader_load_world successfully returns WL_ERR_CONFIG on a fake files path
 * ============================================================ */
START_TEST(test_world_loader_bad_path)
{
    Graph *gr = NULL;
    Room *r_first = NULL;
    int room_num = 0;
    Charset set = {0};

    ck_assert_int_eq(loader_load_world("/dir1/dir2/file.ini", &gr, &r_first, &room_num, &set), WL_ERR_CONFIG);
}
END_TEST

/* ============================================================
 * Test 2: Call loader_load_world on NULL file path
 * Tests that loader_load_world successfully returns WL_ERR_CONFIG on a NULL path
 * ============================================================ */
START_TEST(test_world_loader_null_path)
{
    Graph *gr = NULL;
    Room *r_first = NULL;
    int room_num = 0;
    Charset set = {0};

    ck_assert_int_eq(loader_load_world(NULL, &gr, &r_first, &room_num, &set), INVALID_ARGUMENT);
    ck_assert_int_eq(loader_load_world("../assets/starter.ini", NULL, &r_first, &room_num, &set), INVALID_ARGUMENT);
    ck_assert_int_eq(loader_load_world("../assets/starter.ini", &gr, NULL, &room_num, &set), INVALID_ARGUMENT);
    ck_assert_int_eq(loader_load_world("../assets/starter.ini", &gr, &r_first, NULL, &set), INVALID_ARGUMENT);
    ck_assert_int_eq(loader_load_world("../assets/starter.ini", &gr, &r_first, &room_num, NULL), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 3: Call loader_load_world on starter.ini file path
 * Tests that loader_load_world successfully returns OK on a real path.
 * Using relative path ../assets/starter.ini in the hopes it works on the CI pipeline too.
 * Assertations come from the output of puzzlegen_demo and the unaltered starter.ini
 * ============================================================ */
START_TEST(test_world_loader_full)
{
    Graph *gr = NULL;
    Room *r_first = NULL;
    Room *temp_room = NULL;
    const Room *gr_out_room = NULL;
    int room_num = 0;
    Charset set = {0};
    
    ck_assert_int_eq(loader_load_world("../assets/starter.ini", &gr, &r_first, &room_num, &set), OK);

    // starter.ini specified 3 rooms
    ck_assert_int_eq(room_num, 3);
    ck_assert_int_eq(graph_size(gr), 3);

    ck_assert_ptr_nonnull(r_first);
    ck_assert_int_eq(r_first->id, 0);
    ck_assert_int_eq(r_first->width, 19);
    ck_assert_int_eq(r_first->height, 13);
    ck_assert_int_eq(r_first->portal_count, 4);    
    ck_assert_int_eq(r_first->treasure_count, 4);

    // Create a filler room with the same ID to match the compare func of the graph
    temp_room = room_create(0, NULL, 1, 1);
    ck_assert_int_eq(graph_contains(gr, temp_room), true);
    gr_out_room = graph_get_payload(gr, temp_room);
    ck_assert_ptr_eq(gr_out_room, r_first);
    room_destroy(temp_room);
    temp_room = NULL;

    // Verify the copy of the floor_grid, portals, and treasures in a single step by just comparing the room_render against a hardcoded known good string using a seed=15 in starter.ini
    int num = (gr_out_room->width) * (gr_out_room->height); 
    char *buff0 = calloc(num + 1, sizeof(char));
    ck_assert_int_eq(room_render(gr_out_room, &set, buff0, gr_out_room->width, gr_out_room->height), OK);
    ck_assert_str_eq(ROOM_0, buff0);
    free(buff0);
    buff0 = NULL;

    temp_room = room_create(1, NULL, 1, 1);
    ck_assert_int_eq(graph_contains(gr, temp_room), true);
    gr_out_room = graph_get_payload(gr, temp_room);
    ck_assert_int_eq(gr_out_room->id, 1);
    ck_assert_int_eq(gr_out_room->width, 19);
    ck_assert_int_eq(gr_out_room->height, 13);
    ck_assert_int_eq(gr_out_room->portal_count, 3);
    ck_assert_int_eq(gr_out_room->treasure_count, 4);
    room_destroy(temp_room);
    temp_room = NULL;

    num = (gr_out_room->width) * (gr_out_room->height); 
    char *buff1 = calloc(num + 1, sizeof(char));
    ck_assert_int_eq(room_render(gr_out_room, &set, buff1, gr_out_room->width, gr_out_room->height), OK);
    ck_assert_str_eq(ROOM_1, buff1);
    free(buff1);
    buff1 = NULL;

    temp_room = room_create(2, NULL, 1, 1);
    ck_assert_int_eq(graph_contains(gr, temp_room), true);
    gr_out_room = graph_get_payload(gr, temp_room);
    ck_assert_int_eq(gr_out_room->id, 2);
    ck_assert_int_eq(gr_out_room->width, 22);
    ck_assert_int_eq(gr_out_room->height, 17);
    ck_assert_int_eq(gr_out_room->portal_count, 2);
    ck_assert_int_eq(gr_out_room->treasure_count, 4);
    room_destroy(temp_room);
    temp_room = NULL;

    num = (gr_out_room->width) * (gr_out_room->height); 
    char *buff2 = calloc(num + 1, sizeof(char));
    ck_assert_int_eq(room_render(gr_out_room, &set, buff2, gr_out_room->width, gr_out_room->height), OK);
    ck_assert_str_eq(ROOM_2, buff2);
    free(buff2);
    buff2 = NULL;

    ck_assert_int_eq(set.wall, '#');
    ck_assert_int_eq(set.floor, '.');
    ck_assert_int_eq(set.player, '@');
    ck_assert_int_eq(set.portal, 'X');
    ck_assert_int_eq(set.treasure, '$');

    // Free all graph and room memory
    graph_destroy(gr);
}
END_TEST

/* ============================================================
 * Test 4: Call convert_dg_err_to_wl on all inputs
 * Tests that convert_dg_err_to_wl successfully converts
 * ============================================================ */
START_TEST(test_convert_dg_err_to_wl)
{
    ck_assert_int_eq(convert_dg_err_to_wl(DG_ERR_CONFIG), WL_ERR_CONFIG);
    ck_assert_int_eq(convert_dg_err_to_wl(DG_ERR_OOM), NO_MEMORY);
    ck_assert_int_eq(convert_dg_err_to_wl(DG_ERR_INTERNAL), WL_ERR_DATAGEN);
}
END_TEST

/* ============================================================
 * Test 5: Call loader_load_world on config with pushables
 * Tests that loader_load_world successfully deals with pushables
 * ============================================================ */
START_TEST(test_world_loader_pushables)
{
    Graph *gr = NULL;
    Room *r_first = NULL;
    int room_num = 0;
    Charset set = {0};

    ck_assert_int_eq(loader_load_world("../assets/pushables.ini", &gr, &r_first, &room_num, &set), OK);

    // Free all graph and room memory
    graph_destroy(gr);
}
END_TEST

/* ============================================================
 * Test 6: Call loader_load_world on config with switches
 * Tests that loader_load_world successfully deals with switches
 * ============================================================ */
START_TEST(test_world_loader_switches)
{
    Graph *gr = NULL;
    Room *r_first = NULL;
    int room_num = 0;
    Charset set = {0};

    ck_assert_int_eq(loader_load_world("../assets/switches.ini", &gr, &r_first, &room_num, &set), OK);

    // Free all graph and room memory
    graph_destroy(gr);
}
END_TEST

/* ============================================================
 * Suite Creation Function
 * 
 * This function builds and returns a test suite for the Check framework.
 * A Suite is a collection of test cases that are run together.
 * ============================================================ */

Suite *world_loader_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("World Loader");
    
    // Create a test case to group related tests
    TCase *tc_load = tcase_create("Load");

    // Attach setup and teardown functions to run before/after each test
    tcase_add_checked_fixture(tc_load, NULL, NULL);

    // Add individual test functions to the test case
    tcase_add_test(tc_load, test_world_loader_bad_path);
    tcase_add_test(tc_load, test_world_loader_null_path);
    tcase_add_test(tc_load, test_world_loader_full);

    tcase_add_test(tc_load, test_convert_dg_err_to_wl);
    tcase_add_test(tc_load, test_world_loader_pushables);
    tcase_add_test(tc_load, test_world_loader_switches);

    // Add the test case to the suite
    suite_add_tcase(s, tc_load);
    
    // Return the complete suite so main() can run it
    return s;
}
