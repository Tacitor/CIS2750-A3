#include <check.h>
#include <stdlib.h>
#include "player.h"

#define ROOM 11
#define X_POS 22
#define Y_POS 33

/* ============================================================
 * Setup and Teardown fixtures
 * ============================================================ */

static Player *p = NULL;

static void setup_player(void)
{
    ck_assert_int_eq(player_create(ROOM, X_POS, Y_POS, &p), OK);
    ck_assert_ptr_nonnull(p);
}


static void teardown_player(void)
{
    /* player_destroy must free all memory related to the player */
    player_destroy(p);
    p = NULL;
}

/* ============================================================
 * Test 1: Create a NULL player
 * Tests that player_create successfully returns INVALID_ARGUMENT
 * ============================================================ */

START_TEST(test_player_create_null)
{
    int r = 0, x = 0, y = 0;

    // Create a new player with all 0 initial conditions.
    // Pass a null pointer for the player.
    ck_assert_int_eq(player_create(r, x, y, NULL), INVALID_ARGUMENT);

    // Verify p must still be null
    ck_assert_ptr_null(p);
}
END_TEST

/* ============================================================
 * Test 2: Create a player
 * Tests that player_create successfully returns OK on regular conditions
 * ============================================================ */

START_TEST(test_player_create_reg)
{
    // Create a regular player and verify sucess
    ck_assert_int_eq(player_create(ROOM, X_POS, Y_POS, &p), OK);

    // Verify that p now points to valid memory
    ck_assert_ptr_nonnull(p);
}
END_TEST

/* ============================================================
 * Test 3: Create a player with negative room, x, and y
 * Tests that player_create successfully returns OK on negative values
 * ============================================================ */

START_TEST(test_player_create_neg)
{
    int r = -100, x = -1, y = -56789;

    // Create a player and verify sucess
    ck_assert_int_eq(player_create(r, x, y, &p), OK);

    // Verify that p now points to valid memory
    ck_assert_ptr_nonnull(p);
}
END_TEST

/* ============================================================
 * Test 4: Get a player's location regular
 * Tests that player_get_position successfully returns the
 * correct room ID, x position, and y position.
 * ============================================================ */

START_TEST(test_player_get_reg)
{
    int xOut = 0, yOut = 0;

    // Verify that all 3 intial conds are correct and unique
    ck_assert_int_eq(player_get_room(p), ROOM);
    ck_assert_int_eq(player_get_position(p, &xOut, &yOut), OK);
    ck_assert_int_eq(xOut, X_POS);
    ck_assert_int_eq(yOut, Y_POS);
}
END_TEST

/* ============================================================
 * Test 5: Set a player's position negative
 * Tests that player_set_position successfully returns the
 * correct x position, and y position even if the values are less than 0.
 * ============================================================ */

START_TEST(test_player_set_pos_neg)
{
    int x = -1, y = -56789;
    int xOut = 0, yOut = 0;

    // Set the new position and verify success
    ck_assert_int_eq(player_set_position(p, x, y), OK);

    // Verify that the negative values are still processed
    ck_assert_int_eq(player_get_position(p, &xOut, &yOut), OK);
    ck_assert_int_eq(xOut, x);
    ck_assert_int_eq(yOut, y);
}
END_TEST

/* ============================================================
 * Test 6: Set a player's room negative
 * Tests that player_set_position successfully returns the
 * correct x position, and y position even if the values are less than 0.
 * ============================================================ */

START_TEST(test_player_set_room_neg)
{
    int r = -100;

    // Set the new room and verify success
    ck_assert_int_eq(player_move_to_room(p, r), OK);

    // Verify that the negative value is still processed
    ck_assert_int_eq(player_get_room(p), r);
}
END_TEST

/* ============================================================
 * Test 7: Get a player's location from NULL PTR
 * Tests that player_get_position & player_get_room successfully return the
 * INVALID_ARGUMENT Status when passed a NULL instead of player.
 * ============================================================ */

START_TEST(test_player_get_null)
{
    int xOut = 1, yOut = 2;

    // Verify the room is the error room (-1)
    ck_assert_int_eq(player_get_room(NULL), -1);

    // Verify the INVALID_ARGUMENT return and that the xOut and yOut are still the same. Should not have been touched since init.
    ck_assert_int_eq(player_get_position(p, NULL, &yOut), INVALID_ARGUMENT);
    ck_assert_int_eq(xOut, 1);
    ck_assert_int_eq(yOut, 2);
}
END_TEST

/* ============================================================
 * Test 8: Set a player's position to int MAX
 * Tests that player_set_position successfully returns the
 * correct x position, and y position even if the values are less than 0.
 * ============================================================ */

START_TEST(test_player_set_pos_min_max)
{
    //Calculate the min and max value of the int type
    int x = (pow(2,8*sizeof(int))/2)-1, y = 0-x-1;
    int xOut = 0, yOut = 0;

    // Set the new position and verify success
    ck_assert_int_eq(player_set_position(p, x, y), OK);

    // Verify that the negative values are still processed
    ck_assert_int_eq(player_get_position(p, &xOut, &yOut), OK);
    ck_assert_int_eq(xOut, x);
    ck_assert_int_eq(yOut, y);
}
END_TEST

/* ============================================================
 * Test 9: Set a player's through reset
 * Tests that player_reset_to_start successfully returns the
 * correct room, x position, and y position at full reset.
 * ============================================================ */

START_TEST(test_player_reset)
{
    int x = 5, y = -14, r = 2;
    int xOut = 0, yOut = 0;

    // Verify that all 3 intial conds are correct and unique
    ck_assert_int_eq(player_get_room(p), ROOM);
    ck_assert_int_eq(player_get_position(p, &xOut, &yOut), OK);
    ck_assert_int_eq(xOut, X_POS);
    ck_assert_int_eq(yOut, Y_POS);

    // Verify sucess on reset of player
    ck_assert_int_eq(player_reset_to_start(p, r, x, y), OK);

    // Verify that all 3 conds are now reset
    ck_assert_int_eq(player_get_room(p), r);
    ck_assert_int_eq(player_get_position(p, &xOut, &yOut), OK);
    ck_assert_int_eq(xOut, x);
    ck_assert_int_eq(yOut, y);
}
END_TEST

/* ============================================================
 * Test 10: Null Testing
 * Tests that the remaining player related functions can handle null inputs.
 * ============================================================ */

START_TEST(test_player_null_combo)
{
    int xOut = 1, yOut = 2;

    // Verify the room is the error room (-1)
    ck_assert_int_eq(player_get_room(NULL), -1);

    // Verify the INVALID_ARGUMENT return and that the xOut and yOut are still the same. Should not have been touched since init.
    ck_assert_int_eq(player_get_position(p, &xOut, NULL), INVALID_ARGUMENT);
    //The xOut is NULL case is covered above in test_player_get_null and is omitted here
    ck_assert_int_eq(player_get_position(NULL, &xOut, &yOut), INVALID_ARGUMENT);
    ck_assert_int_eq(xOut, 1);
    ck_assert_int_eq(yOut, 2);

    // Verify there is no statefullness or memory of the last valid Player pointer
    player_destroy(NULL);
    ck_assert_ptr_nonnull(p);

    ck_assert_int_eq(player_set_position(NULL, 3, 4), INVALID_ARGUMENT);

    ck_assert_int_eq(player_move_to_room(NULL, 5), INVALID_ARGUMENT);

    ck_assert_int_eq(player_reset_to_start(NULL, 6, 7, 8), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 11: Get a player to collect NULL
 * Tests that player_try_collect successfully returns NULL_POINTER
 * ============================================================ */

START_TEST(test_player_collect_null)
{
    Treasure t;
    t.collected = false;
    t.id = 5;
    t.initial_x = t.x = 6;
    t.initial_y = t.y = 7;
    t.name = "TreasureTest";
    t.starting_room_id = ROOM;

    ck_assert_int_eq(player_try_collect(p, NULL), NULL_POINTER);
    ck_assert_int_eq(player_try_collect(NULL, &t), NULL_POINTER);

    p->collected_count = 1;
    ck_assert_int_eq(player_try_collect(p, &t), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 12: Get a player to collect a treasure
 * Tests that player_try_collect successfully returns OK and takes the treasure
 * ============================================================ */

START_TEST(test_player_collect_single)
{
    Treasure t;
    t.collected = false;
    t.id = 5;
    t.initial_x = t.x = 6;
    t.initial_y = t.y = 7;
    t.name = "TreasureTest";
    t.starting_room_id = ROOM;

    ck_assert(t.collected == false);
    ck_assert_int_eq(p->collected_count, 0);
    ck_assert_ptr_null(p->collected_treasures);
    ck_assert(player_has_collected_treasure(p, t.id) == false);

    ck_assert_int_eq(player_try_collect(p, &t), OK);

    ck_assert_int_eq(p->collected_count, 1);
    ck_assert_ptr_nonnull(p->collected_treasures);
    ck_assert_int_eq(p->collected_treasures[0]->id, 5);
    ck_assert(t.collected == true);
    ck_assert(player_has_collected_treasure(p, t.id) == true);
    ck_assert_int_eq(player_get_collected_count(p), 1);

    ck_assert_int_eq(player_try_collect(p, &t), INVALID_ARGUMENT);
    ck_assert(player_has_collected_treasure(p, t.id) == true);
}
END_TEST

/* ============================================================
 * Test 13: Get a player to collect two treasures
 * Tests that player_try_collect successfully returns OK and takes the treasure
 * ============================================================ */

START_TEST(test_player_collect_two)
{
    Treasure t1;
    t1.collected = false;
    t1.id = 5;
    t1.initial_x = t1.x = 6;
    t1.initial_y = t1.y = 7;
    t1.name = "TreasureTest1";
    t1.starting_room_id = ROOM;

    Treasure t2;
    t2.collected = false;
    t2.id = 15;
    t2.initial_x = t2.x = 16;
    t2.initial_y = t2.y = 17;
    t2.name = "TreasureTest2";
    t2.starting_room_id = ROOM;

    ck_assert(player_has_collected_treasure(p, t1.id) == false);
    ck_assert(player_has_collected_treasure(p, t2.id) == false);

    ck_assert_int_eq(player_try_collect(p, &t1), OK);
    ck_assert_int_eq(player_try_collect(p, &t2), OK);

    ck_assert_int_eq(p->collected_count, 2);
    ck_assert_int_eq(p->collected_treasures[0]->id, 5);
    ck_assert_int_eq(p->collected_treasures[1]->id, 15);

    ck_assert_int_eq(player_get_collected_count(p), 2);
    ck_assert(t1.collected == true);
    ck_assert(player_has_collected_treasure(p, t1.id) == true);
    ck_assert(t2.collected == true);
    ck_assert(player_has_collected_treasure(p, t2.id) == true);

    int count_out = -1;
    const Treasure * const * t = NULL;
    t = player_get_collected_treasures(p, &count_out);
    ck_assert_ptr_nonnull(t);
    ck_assert_int_eq(t[0]->id, 5);
    ck_assert_int_eq(t[1]->id, 15);
}
END_TEST

/* ============================================================
 * Test 14: Get a bool from a player if they have collected a treasure around edge cases
 * Tests that player_has_collected_treasure successfully returns false
 * if the player is NULL or if the treasure is not collected
 * ============================================================ */

START_TEST(test_player_has_collected_edge)
{
    ck_assert_int_eq(player_has_collected_treasure(NULL, 3), false);
    ck_assert_int_eq(player_has_collected_treasure(p, 3), false);
}
END_TEST

/* ============================================================
 * Test 15: Getting the number of collected treasures from a player
 * Tests that player_get_collected_count is successfull in returning 0 on NULL player
 * ============================================================ */

START_TEST(test_player_get_collected_count_null)
{
    ck_assert_int_eq(player_get_collected_count(NULL), 0);
}
END_TEST

/* ============================================================
 * Test 16: Getting the collected treasures from a NULL player
 * Tests that player_get_collected_treasures successfuly returns NULL
 * ============================================================ */

START_TEST(test_player_get_collected_treasures_null)
{
    int count_out = -1;
    ck_assert_ptr_null(player_get_collected_treasures(NULL, &count_out));
    ck_assert_ptr_null(player_get_collected_treasures(p, NULL));
}
END_TEST

/* ============================================================
 * Suite Creation Function
 * 
 * This function builds and returns a test suite for the Check framework.
 * A Suite is a collection of test cases that are run together.
 * ============================================================ */

Suite *player_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("Player");
    
    // Create a test case to group related tests
    TCase *tc_create = tcase_create("Create");
    TCase *tc_get_Set = tcase_create("GetSet");

    // Attach setup and teardown functions to run before/after each test
    // setup_player() runs before each test to create a fresh graph
    // teardown_player() runs after each test to clean up the graph
    tcase_add_checked_fixture(tc_create, NULL, teardown_player);
    tcase_add_checked_fixture(tc_get_Set, setup_player, teardown_player);

    // Add individual test functions to the test case
    tcase_add_test(tc_create, test_player_create_null);
    tcase_add_test(tc_create, test_player_create_reg);
    tcase_add_test(tc_create, test_player_create_neg);

    tcase_add_test(tc_get_Set, test_player_get_reg);
    tcase_add_test(tc_get_Set, test_player_get_null);
    tcase_add_test(tc_get_Set, test_player_set_pos_neg);
    tcase_add_test(tc_get_Set, test_player_set_pos_min_max);
    tcase_add_test(tc_get_Set, test_player_set_room_neg);
    tcase_add_test(tc_get_Set, test_player_reset);
    tcase_add_test(tc_get_Set, test_player_null_combo);

    tcase_add_test(tc_get_Set, test_player_collect_null);
    tcase_add_test(tc_get_Set, test_player_collect_single);
    tcase_add_test(tc_get_Set, test_player_collect_two);

    tcase_add_test(tc_get_Set, test_player_has_collected_edge);
    tcase_add_test(tc_get_Set, test_player_get_collected_count_null);
    tcase_add_test(tc_get_Set, test_player_get_collected_treasures_null);

    // Add the test case to the suite
    suite_add_tcase(s, tc_create);
    suite_add_tcase(s, tc_get_Set);
    
    // Return the complete suite so main() can run it
    return s;
}
