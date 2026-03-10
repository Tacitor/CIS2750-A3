#include <check.h>
#include <stdlib.h>
#include "my_room.h"

#define ID 33
#define WIDTH 6
#define HEIGHT 5
#define NAME "TheSuperAwesomeAndGreatTestRoom"

/* ============================================================
 * Setup and Teardown fixtures
 * ============================================================ */

static Room *r = NULL;

static void setup_room(void)
{
    r = room_create(ID, NAME, WIDTH, HEIGHT);
    ck_assert_ptr_nonnull(r);
}

static void teardown_room(void)
{
    /* room_destroy must free all memory related to the room */
    room_destroy(r);
    r = NULL;
}

// Introduce two defects into expected grid to test tollerance to boolean correction
// width = 6, height = 5
bool expected_grid_small[] = {false, false, false, false, false, false, 
                        false, true, true, true, true, false,
                        false, true, true, true, true, false,
                        false, 2, true, true, true, false,
                        false, 0, false, false, false, false};

/* ============================================================
 * Test 1: Create a single room
 * Tests that room_create successfully returns a non null Room pointer
 * ============================================================ */

START_TEST(test_room_create)
{
    char *n = {"Test Room"};

    Room *room = room_create(1, n, 15, 21);
    ck_assert_ptr_nonnull(room);
    room_destroy(room);
    
}
END_TEST

/* ============================================================
 * Test 2: Create a room verify setup
 * Tests that room_create successfully returns a room with the right values
 * ============================================================ */

START_TEST(test_room_create_value)
{
    char *n = {"Test Room"};

    ck_assert_ptr_nonnull(r = room_create(1, n, 15, 21));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_int_eq(r->id, 1);
    ck_assert_str_eq(n, r->name);
    ck_assert_int_eq(r->width, 15);
    ck_assert_int_eq(r->height, 21);
}
END_TEST

/* ============================================================
 * Test 3: Create a room verify null values
 * Tests that room_create successfully returns a room with the right values
 * but this also means that the values not passes as params in 
 * room_create() are NULL or 0;
 * ============================================================ */

START_TEST(test_room_create_null)
{
    char *n = {"NULL Test Room"};

    ck_assert_ptr_nonnull(r = room_create(-999, n, 987321, 444444444));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_int_eq(r->id, -999);
    ck_assert_str_eq(n, r->name);
    ck_assert_int_eq(r->width, 987321);
    ck_assert_int_eq(r->height, 444444444);

    // Verify the others are null
    ck_assert_ptr_null(r->floor_grid);
    ck_assert_ptr_null(r->treasures);
    ck_assert_ptr_null(r->portals);

    // Verify the counts are 0
    ck_assert_int_eq(r->portal_count, 0);
    ck_assert_int_eq(r->treasure_count, 0);
}
END_TEST

/* ============================================================
 * Test 4: Create a noname room
 * Tests that room_create successfully deals with an empty string
 * ============================================================ */

START_TEST(test_room_create_noname)
{
    ck_assert_ptr_nonnull(r = room_create(1, "", 15, 21));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_str_eq("", r->name);
}
END_TEST

/* ============================================================
 * Test 5: Create a null name room
 * Tests that room_create successfully deals with an NULL string
 * ============================================================ */

START_TEST(test_room_create_null_name)
{
    ck_assert_ptr_nonnull(r = room_create(1, NULL, 15, 21));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_ptr_null(r->name);
}
END_TEST

/* ============================================================
 * Test 6: Min & max val getting
 * ============================================================ */

START_TEST(test_room_get_max)
{
    //Calculate the min and max value of the int type
    int height, width;
    height = width = (pow(2,8*sizeof(int))/2)-1;

    ck_assert_ptr_nonnull(r = room_create(1, NULL, width, height));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_int_eq(r->width, width);
    ck_assert_int_eq(r->height, height);

    ck_assert_int_eq(room_get_width(r), width);
    ck_assert_int_eq(room_get_height(r), height);
}
END_TEST

/* ============================================================
 * Test 7: Test getting width and height with null room
 * Tests that room_get_width and room_get_height successfully returns 0 when given NULL room
 * ============================================================ */

START_TEST(test_room_get_null)
{
    ck_assert_int_eq(room_get_width(NULL), 0);
    ck_assert_int_eq(room_get_height(NULL), 0);
}
END_TEST

/* ============================================================
 * Test 8: Test passing a NULL room to the set floor grid function
 * Tests that room_set_floor_grid returns INVALID_ARGUMENT
 * ============================================================ */
START_TEST(test_room_grid_null)
{
    ck_assert_int_eq(room_set_floor_grid(NULL, NULL), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 9: Test passing a NULL room to the set floor grid function
 * Tests that room_set_floor_grid assigns the implicit perimeter walls with open interior on NULL floor_grid
 * ============================================================ */
START_TEST(test_room_grid_implicit)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    // Itterate over returned r->room_grid and make sure AT LEAST width * height bools
    for (int y_pos = 0; y_pos < r->height; y_pos++) {
        for (int x_pos = 0; x_pos < r->width; x_pos++) {
            int tile = r->floor_grid[y_pos * r->width + x_pos];
            ck_assert(tile == 0 || tile == 1);
        }
    }

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(r->floor_grid[i], expected_grid_small[i]);
    }
}
END_TEST

/* ============================================================
 * Test 10: Test overwritting the floor_grid
 * Tests that room_set_floor_grid assigns the new grid
 * ============================================================ */
START_TEST(test_room_grid_new)
{
    //New grid with all walls
    int num = r->width * r->height;
    bool *new_grid = calloc(num, sizeof(bool));

    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(r->floor_grid[i], expected_grid_small[i]);
    }

    bool *old_grid = r->floor_grid;

    ck_assert_int_eq(room_set_floor_grid(r, new_grid), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(r->floor_grid[i], 0);
    }

    ck_assert_ptr_eq(r->floor_grid, new_grid);
    ck_assert_ptr_ne(r->floor_grid, old_grid);
}
END_TEST

/* ============================================================
 * Test 11: Test room rendering on NULL
 * Tests that room_render returns INVALID_ARGUMENT on NULL input
 * ============================================================ */
START_TEST(test_room_render_null)
{
    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';

    char buff[r->width * r->height];

    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), INVALID_ARGUMENT);

    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    ck_assert_int_eq(room_render(r, &set, NULL, r->width, r->height), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, NULL, buff, r->width, r->height), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(NULL, &set, buff, r->width, r->height), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 12: Test room rendering on bad dimensions
 * Tests that room_render returns INVALID_ARGUMENT if buffer dimensions don't match room dimensions
 * ============================================================ */
START_TEST(test_room_render_bad_dim)
{
    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';

    char buff[(r->width - 1) * (r->height + 1)];
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    ck_assert_int_eq(room_render(r, &set, buff, r->width - 1, r->height), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height + 1), INVALID_ARGUMENT);
    ck_assert_int_eq(room_render(r, &set, buff, r->width - 1, r->height + 1), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 13: Test room rendering on null floor grid
 * Tests that room_render returns INVALID_ARGUMENT if floor_grid is NULL
 * ============================================================ */
START_TEST(test_room_render_null_floor)
{
    // Ensure the floor_grid is NULL
    Room *temp = room_create(1, NULL, 15, 11);
    ck_assert_ptr_nonnull(temp);
    ck_assert_ptr_null(temp->floor_grid);

    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';

    char buff[(temp->width) * (temp->height)];

    ck_assert_int_eq(room_render(temp, &set, buff, temp->width, temp->height), INVALID_ARGUMENT);

    // render must not assign any value to the floor_grid
    ck_assert_ptr_null(temp->floor_grid);

    room_destroy(temp);
}
END_TEST

/* ============================================================
 * Test 14: Test room rendering on custom floor grid
 * Tests that room_render returns the right buffer for a given room
 * ============================================================ */
START_TEST(test_room_render_cust_floor)
{
    int num = (r->width) * (r->height);
    bool cust_grid_small[] = {false, false, false, false, false, false, 
                              false, true, false, true, true, false,
                              false, true, false, true, false, false,
                              false, true, true, true, false, false,
                              false, false, false, false, false, false};
    bool *cust_grid_small_ptr = malloc(num);

    for (int i = 0; i < num; i++) {
        cust_grid_small_ptr[i] = cust_grid_small[i];
    }

    ck_assert_int_eq(room_set_floor_grid(r, cust_grid_small_ptr), OK);

    char cust_buff_small[] = {'#', '#', '#', '#', '#', '#', 
                              '#', '.', '#', '.', '.', '#',
                              '#', '.', '#', '.', '#', '#',
                              '#', '.', '.', '.', '#', '#',
                              '#', '#', '#', '#', '#', '#'};

    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';

    char buff[num];
    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(buff[i], cust_buff_small[i]);
    }
}
END_TEST

/* ============================================================
 * Test 15: Test setting portals on NULL
 * Tests that room_set_portals returns INVALID_ARGUMENT on NULL input
 * ============================================================ */
START_TEST(test_set_portals_null)
{
    int port_num = 3;
    Portal p[port_num];
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = "TEST_PORTAL";
        p[i].target_room_id = ID;
        p[i].x = 0;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, NULL, port_num), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_portals(NULL, p, port_num), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 16: Test setting portals on short array
 * Tests that room_set_portals side effects are correct in setting the portals
 * ============================================================ */
START_TEST(test_set_portals_reg)
{
    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = ID;
        p[i].x = 0;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);

    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);
}
END_TEST

/* ============================================================
 * Test 17: Test overwitting portals
 * Tests that room_set_portals side effects are correct in overwitting the portals
 * ============================================================ */
START_TEST(test_set_portals_overwrite)
{
    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = ID;
        p[i].x = 0;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);

    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    port_num = 4;
    Portal *q = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        q[i].id = 5 + i;
        q[i].name = calloc(8, sizeof(char));
        strcpy(q[i].name, "portal#");
        q[i].name[6] = (char)(48 + q[i].id); //Convert from value of int to char of said digit
        q[i].target_room_id = ID;
        q[i].x = 0;
        q[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, q, port_num), OK);

    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(q, r->portals);

    ck_assert_int_eq(r->portals[0].id, 5);
    ck_assert_int_eq(r->portals[1].id, 6);
    ck_assert_int_eq(r->portals[2].id, 7);
}
END_TEST

/* ============================================================
 * Test 18: Test setting treasures on NULL
 * Tests that room_set_treasures returns INVALID_ARGUMENT on NULL input
 * ============================================================ */
START_TEST(test_set_treasures_null)
{
    int treasure_num = 3;
    Treasure t[treasure_num];
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = "TEST_TREASURE";
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, NULL, treasure_num), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_treasures(NULL, t, treasure_num), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 19: Test setting treasures on short array
 * Tests that room_set_treasures side effects are correct in setting the treasures
 * ============================================================ */
START_TEST(test_set_treasures_reg)
{
    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);

    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);
}
END_TEST

/* ============================================================
 * Test 20: Test overwitting treasures
 * Tests that room_set_treasures side effects are correct in overwitting the treasures
 * ============================================================ */
START_TEST(test_set_treasures_overwrite)
{
    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);

    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);

    treasure_num = 4;
    Treasure *u = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        u[i].id = 5 + i;
        u[i].name = calloc(8, sizeof(char));
        strcpy(u[i].name, "treasu#");
        u[i].name[6] = (char)(48 + u[i].id); //Convert from value of int to char of said digit
        u[i].starting_room_id = ID;
        u[i].x = 1;
        u[i].y = i + 1;
        u[i].initial_x = 1;
        u[i].initial_y = i + 1;
        u[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, u, treasure_num), OK);

    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(u, r->treasures);

    ck_assert_int_eq(r->treasures[0].id, 5);
    ck_assert_int_eq(r->treasures[1].id, 6);
    ck_assert_int_eq(r->treasures[2].id, 7);
}
END_TEST

/* ============================================================
 * Test 21: Test placing treasures on NULL
 * Tests that room_place_treasure returns INVALID_ARGUMENT on NULL input
 * ============================================================ */
START_TEST(test_place_treasures_null)
{
    Treasure *t = calloc(1, sizeof(Treasure));
    t->id = 1;
    t->name = calloc(8, sizeof(char));
    strcpy(t->name, "treasu#");
    t->name[6] = (char)(48 + t->id); //Convert from value of int to char of said digit
    t->starting_room_id = ID;
    t->x = 1;
    t->y = 1;
    t->initial_x = 1;
    t->initial_y = 1;
    t->collected = false;

    ck_assert_int_eq(room_place_treasure(r, NULL), INVALID_ARGUMENT);
    ck_assert_int_eq(room_place_treasure(NULL, t), INVALID_ARGUMENT);

    free(t->name);
    free(t);
}
END_TEST

/* ============================================================
 * Test 22: Test placing a treasure into a NULL treasures array
 * Tests that room_place_treasure returns OK even if room_set_treasures has never been called
 * ============================================================ */
START_TEST(test_place_treasures_no_init)
{
    Treasure *t = calloc(1, sizeof(Treasure));
    t->id = 1;
    t->name = calloc(8, sizeof(char));
    strcpy(t->name, "treasu#");
    t->name[6] = (char)(48 + t->id); //Convert from value of int to char of said digit
    t->starting_room_id = ID;
    t->x = 1;
    t->y = 1;
    t->initial_x = 1;
    t->initial_y = 1;
    t->collected = false;

    ck_assert_int_eq(room_place_treasure(r, t), OK);

    ck_assert_int_eq(r->treasure_count, 1);
    ck_assert_ptr_ne(t, r->treasures);
    ck_assert_int_eq(r->treasures[0].id, 1);
    ck_assert_str_eq("treasu1", r->treasures[0].name);

    strcpy(t->name, "NewName");
    ck_assert_str_eq("treasu1", r->treasures[0].name);

    free(t->name);
    free(t);
}
END_TEST

/* ============================================================
 * Test 23: Test placing a treasure into a an existing treasures array
 * Tests that room_place_treasure returns OK even if room_set_treasures has been called
 * ============================================================ */
START_TEST(test_place_treasures_init)
{
    Treasure *t = calloc(1, sizeof(Treasure));
    t->id = 1;
    t->name = NULL;
    t->starting_room_id = ID;
    t->x = 1;
    t->y = 1;
    t->initial_x = 1;
    t->initial_y = 1;
    t->collected = false;

    int treasure_num = 4;
    Treasure *u = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        u[i].id = 5 + i;
        u[i].name = calloc(8, sizeof(char));
        strcpy(u[i].name, "treasu#");
        u[i].name[6] = (char)(48 + u[i].id); //Convert from value of int to char of said digit
        u[i].starting_room_id = ID;
        u[i].x = 1;
        u[i].y = i + 1;
        u[i].initial_x = 1;
        u[i].initial_y = i + 1;
        u[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, u, treasure_num), OK);

    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(u, r->treasures);

    ck_assert_int_eq(r->treasures[0].id, 5);
    ck_assert_int_eq(r->treasures[1].id, 6);
    ck_assert_int_eq(r->treasures[2].id, 7);

    ck_assert_int_eq(room_place_treasure(r, t), OK);

    ck_assert_int_eq(r->treasure_count, 5);
    ck_assert_ptr_ne(t, r->treasures);

    ck_assert_int_eq(r->treasures[4].id, 1);
    ck_assert_int_eq(r->treasures[4].collected, 0);
    ck_assert_int_eq(r->treasures[4].initial_x, t->initial_x);
    ck_assert_int_eq(r->treasures[4].initial_y, t->initial_y);
    ck_assert_int_eq(r->treasures[4].x, t->x);
    ck_assert_int_eq(r->treasures[4].y, t->y);
    ck_assert_int_eq(r->treasures[4].starting_room_id, t->starting_room_id);
    ck_assert_ptr_null(r->treasures[4].name);

    free(t->name);
    free(t);
}
END_TEST

/* ============================================================
 * Test 24: Test room rendering on custom floor grid with portals and treasures added
 * Tests that room_render returns the right buffer for a given room
 * ============================================================ */
START_TEST(test_room_render_port_tres)
{
    int num = (r->width) * (r->height);
    bool cust_grid_small[] = {false, false, false, false, false, false, 
                              true, true, false, true, true, false,
                              true, true, false, true, false, false,
                              true, true, true, true, true, false,
                              false, false, false, false, false, false};
    bool *cust_grid_small_ptr = malloc(num);

    for (int i = 0; i < num; i++) {
        cust_grid_small_ptr[i] = cust_grid_small[i];
    }

    ck_assert_int_eq(room_set_floor_grid(r, cust_grid_small_ptr), OK);

    char cust_buff_small[] = {'#', '#', '#', '#', '#', '#', 
                              'X', '$', '#', '.', '.', '#',
                              'X', '.', '#', '.', '#', '#',
                              'X', '.', '.', '.', '$', '#',
                              '#', '#', '#', '#', '#', '#'};

    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';

    Treasure *t = calloc(3, sizeof(Treasure));
    t[0].id = 1;
    t[0].name = calloc(8, sizeof(char));
    strcpy(t[0].name, "treasu#");
    t[0].name[6] = (char)(48 + t[0].id); //Convert from value of int to char of said digit
    t[0].starting_room_id = ID;
    t[0].x = 1;
    t[0].y = 1;
    t[0].initial_x = 1;
    t[0].initial_y = 1;
    t[0].collected = false;

    t[1].id = 2;
    t[1].name = calloc(8, sizeof(char));
    strcpy(t[1].name, "treasu#");
    t[1].name[6] = (char)(48 + t[1].id); //Convert from value of int to char of said digit
    t[1].starting_room_id = ID;
    t[1].x = 4;
    t[1].y = 3;
    t[1].initial_x = 4;
    t[1].initial_y = 3;
    t[1].collected = false;

    t[2].id = 3;
    t[2].name = calloc(8, sizeof(char));
    strcpy(t[2].name, "treasu#");
    t[2].name[6] = (char)(48 + t[2].id); //Convert from value of int to char of said digit
    t[2].starting_room_id = ID;
    t[2].x = 1;
    t[2].y = 2;
    t[2].initial_x = 1;
    t[2].initial_y = 3;
    t[2].collected = true;

    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = ID;
        p[i].x = 0;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_treasures(r, t, 3), OK);
    ck_assert_int_eq(r->treasure_count, 3);
    ck_assert_ptr_eq(t, r->treasures);

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);
    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    char buff[num];
    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(buff[i], cust_buff_small[i]);
    }
}
END_TEST

/* ============================================================
 * Test 25: Test getting treasures
 * Tests that room_get_treasure_at handels a NULL room
 * ============================================================ */
START_TEST(test_get_treasure_at_null)
{
    ck_assert_int_eq(room_get_treasure_at(NULL, 1, 2), -1);
}
END_TEST

/* ============================================================
 * Test 26: Test getting treasures
 * Tests that room_get_treasure_at reports correctly when there is no treasure at a place
 * ============================================================ */
START_TEST(test_get_treasure_at_none)
{
    ck_assert_int_eq(room_get_treasure_at(r, 1, 2), -1);
}
END_TEST

/* ============================================================
 * Test 27: Test getting treasures
 * Tests that room_get_treasure_at handels a regular case
 * ============================================================ */
START_TEST(test_get_treasure_at_reg)
{
    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);
    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);

    ck_assert_int_eq(room_get_treasure_at(r, 1, 1), 0);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 2), 1);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 3), 2);
}
END_TEST

/* ============================================================
 * Test 28: Test getting treasures
 * Tests that room_get_treasure_at reports correctly a regular case after the treasure moved around
 * ============================================================ */
START_TEST(test_get_treasure_at_moved)
{
    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);
    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);

    ck_assert_int_eq(room_get_treasure_at(r, 1, 1), 0);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 2), 1);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 3), 2);

    //Move them around
    r->treasures[0].x = 2;

    r->treasures[1].x = 4;
    r->treasures[1].y = 1;

    r->treasures[2].x = 3;

    ck_assert_int_eq(room_get_treasure_at(r, 1, 1), -1);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 2), -1);
    ck_assert_int_eq(room_get_treasure_at(r, 1, 3), -1);

    ck_assert_int_eq(room_get_treasure_at(r, 2, 1), 0);
    ck_assert_int_eq(room_get_treasure_at(r, 4, 1), 1);
    ck_assert_int_eq(room_get_treasure_at(r, 3, 3), 2);
}
END_TEST

/* ============================================================
 * Test 29: Create a room verify setup
 * Tests that room_create successfully clamps the dimensions such that a room is never smaller than a 1*1
 * ============================================================ */

START_TEST(test_room_create_clamped_dim)
{
    char *n = {"Clamped Room"};

    ck_assert_ptr_nonnull(r = room_create(8, n, 0, -5));
    ck_assert_ptr_nonnull(r);

    // Verify the values are as expected
    ck_assert_int_eq(r->id, 8);
    ck_assert_str_eq(n, r->name);
    ck_assert_int_eq(r->width, 1);
    ck_assert_int_eq(r->height, 1);
}
END_TEST

/* ============================================================
 * Test 30: Test getting portals
 * Tests that room_get_portal_destination handels a NULL room
 * ============================================================ */
START_TEST(test_get_portal_dest_null)
{
    ck_assert_int_eq(room_get_portal_destination(NULL, 1, 2), -1);
}
END_TEST

/* ============================================================
 * Test 31: Test getting portals
 * Tests that room_get_portal_destination reports correctly when there is no portal at a place
 * ============================================================ */
START_TEST(test_get_portal_dest_none)
{
    ck_assert_int_eq(room_get_portal_destination(r, 1, 2), -1);
}
END_TEST

/* ============================================================
 * Test 32: Test getting portals
 * Tests that room_get_portal_destination handels a regular case
 * ============================================================ */
START_TEST(test_get_portal_dest_reg)
{
    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = 5 + i;
        p[i].x = i % 2 == 1? 0 : 5;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);
    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    ck_assert_int_eq(room_get_portal_destination(r, 5, 1), 5);
    ck_assert_int_eq(room_get_portal_destination(r, 0, 2), 6);
    ck_assert_int_eq(room_get_portal_destination(r, 5, 3), 7);
}
END_TEST

/* ============================================================
 * Test 33: Test if a NULL room is walkable
 * Tests if room_is_walkable returns false if the room is NULL
 * ============================================================ */
START_TEST(test_room_is_walkable_null)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_int_eq(room_is_walkable(NULL, 1, 2), false);
}
END_TEST

/* ============================================================
 * Test 34: Test if a out of bounds room is walkable
 * Tests if room_is_walkable returns false if the position given is bad for the room
 * ============================================================ */
START_TEST(test_room_is_walkable_bad_bound)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_int_eq(room_is_walkable(r, -1, 2), false);
    ck_assert_int_eq(room_is_walkable(r, 2, -1), false);
    ck_assert_int_eq(room_is_walkable(r, WIDTH, 2), false);
    ck_assert_int_eq(room_is_walkable(r, 2, HEIGHT), false);
}
END_TEST

/* ============================================================
 * Test 35: Test if a room with a NULL grid is walkable
 * Tests if room_is_walkable returns false values a NULL floor grid
 * ============================================================ */
START_TEST(test_room_is_walkable_null_grid)
{
    ck_assert_ptr_null(r->floor_grid);
    ck_assert_int_eq(room_is_walkable(r, 2, 2), false);
    ck_assert_int_eq(room_is_walkable(r, 0, 2), false);
}
END_TEST

/* ============================================================
 * Test 36: Test if a room with a NULL grid is walkable
 * Tests if room_is_walkable returns true values for the implicit walls
 * ============================================================ */
START_TEST(test_room_is_walkable_grid_init)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_ptr_nonnull(r->floor_grid);
    ck_assert_int_eq(room_is_walkable(r, 2, 2), true);
    ck_assert_int_eq(room_is_walkable(r, 0, 2), false);
}
END_TEST

/* ============================================================
 * Test 37: Test if a NULL room can be classified
 * Tests if room_classify_tile returns ROOM_TILE_INVALID if the room is NULL
 * ============================================================ */
START_TEST(test_room_classify_tile_null)
{
    int id_out = -1;
    ck_assert_int_eq(room_classify_tile(NULL, 1, 2, &id_out), ROOM_TILE_INVALID);
}
END_TEST

/* ============================================================
 * Test 38: Test if a out of bounds room can be classified
 * Tests if room_classify_tile returns ROOM_TILE_INVALID if the position given is bad for the room
 * ============================================================ */
START_TEST(test_room_classify_tile_bad_bound)
{
    int id_out = -1;
    ck_assert_int_eq(room_classify_tile(r, -1, 2, &id_out), ROOM_TILE_INVALID);
    ck_assert_int_eq(room_classify_tile(r, 2, -1, &id_out), ROOM_TILE_INVALID);
    ck_assert_int_eq(room_classify_tile(r, WIDTH, 2, &id_out), ROOM_TILE_INVALID);
    ck_assert_int_eq(room_classify_tile(r, 2, HEIGHT, &id_out), ROOM_TILE_INVALID);
}
END_TEST

/* ============================================================
 * Test 39: Test if a room with a NULL grid can be classified
 * Tests if room_classify_tile returns ROOM_TILE_FLOOR and ROOM_TILE_WALL values for the implicit walls
 * ============================================================ */
START_TEST(test_room_classify_tile_grid_init)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_ptr_nonnull(r->floor_grid);

    int id_out = -1;
    ck_assert_int_eq(room_classify_tile(r, 2, 2, &id_out), ROOM_TILE_FLOOR);
    ck_assert_int_eq(room_classify_tile(r, 3, 2, &id_out), ROOM_TILE_FLOOR);
    ck_assert_int_eq(room_classify_tile(r, 2, 3, &id_out), ROOM_TILE_FLOOR);
    ck_assert_int_eq(room_classify_tile(r, 3, 3, &id_out), ROOM_TILE_FLOOR);

    ck_assert_int_eq(room_classify_tile(r, 0, 1, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 0, 2, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 0, 3, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 0, 4, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 1, 0, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 3, 0, &id_out), ROOM_TILE_WALL);
    ck_assert_int_eq(room_classify_tile(r, 4, 0, &id_out), ROOM_TILE_WALL);
}
END_TEST

/* ============================================================
 * Test 40: Test if a room with a portals and treasures can be classified
 * Tests that room_classify_tile handels a full case
 * ============================================================ */
START_TEST(test_room_classify_tile_port_treas)
{
    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = 5 + i;
        p[i].x = i % 2 == 1? 0 : 5;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);
    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);
    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);

    int id_out = -1;
    ck_assert_int_eq(room_classify_tile(r, 5, 1, &id_out), ROOM_TILE_PORTAL);
    ck_assert_int_eq(id_out, 5);
    ck_assert_int_eq(room_classify_tile(r, 0, 2, NULL), ROOM_TILE_PORTAL);
    ck_assert_int_eq(room_classify_tile(r, 0, 2, &id_out), ROOM_TILE_PORTAL);
    ck_assert_int_eq(id_out, 6);
    ck_assert_int_eq(room_classify_tile(r, 5, 3, &id_out), ROOM_TILE_PORTAL);
    ck_assert_int_eq(id_out, 7);

    ck_assert_int_eq(room_classify_tile(r, 1, 1, &id_out), ROOM_TILE_TREASURE);
    ck_assert_int_eq(id_out, 0);
    ck_assert_int_eq(room_classify_tile(r, 1, 2, NULL), ROOM_TILE_TREASURE);
    ck_assert_int_eq(room_classify_tile(r, 1, 2, &id_out), ROOM_TILE_TREASURE);
    ck_assert_int_eq(id_out, 1);
    ck_assert_int_eq(room_classify_tile(r, 1, 3, &id_out), ROOM_TILE_TREASURE);
    ck_assert_int_eq(id_out, 2);
}
END_TEST

/* ============================================================
 * Test 41: Test if a NULL room has a start position
 * Tests if room_get_start_position returns INVALID_ARGUMENT if the room is NULL
 * ============================================================ */
START_TEST(test_room_start_pos_null)
{
    int x_out = -1;
    int y_out = -1;
    ck_assert_int_eq(room_get_start_position(NULL, &x_out, &y_out), INVALID_ARGUMENT);
    ck_assert_int_eq(room_get_start_position(r, NULL, &y_out), INVALID_ARGUMENT);
    ck_assert_int_eq(room_get_start_position(r, &x_out, NULL), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 42: Test if a room that is ALL walls has a start position
 * Tests if room_get_start_position returns ROOM_NOT_FOUND if the room is just wall-to-wall walls
 * ============================================================ */
START_TEST(test_room_start_pos_wall_to_wall_walls)
{
    bool *cust_grid_small = calloc(WIDTH * HEIGHT, sizeof(bool));
    ck_assert_int_eq(room_set_floor_grid(r, cust_grid_small), OK);

    int x_out = -1;
    int y_out = -1;
    ck_assert_int_eq(room_get_start_position(r, &x_out, &y_out), ROOM_NOT_FOUND);
}
END_TEST

/* ============================================================
 * Test 43: Test if a room with no portals has a start position
 * Tests if room_get_start_position returns OK and the right position
 * ============================================================ */
START_TEST(test_room_start_pos_no_portals)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    int x_out = -1;
    int y_out = -1;
    ck_assert_int_eq(room_get_start_position(r, &x_out, &y_out), OK);
    ck_assert_int_eq(x_out, 1);
    ck_assert_int_eq(y_out, 1);
}
END_TEST

/* ============================================================
 * Test 44: Test if a room with a portal has a start position
 * Tests if room_get_start_position returns OK and the right position NOT on the portal since the portal is not walkable
 * ============================================================ */
START_TEST(test_room_start_pos_with_portal_on_wall)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = 5 + i;
        p[i].x = i % 2 == 1? 0 : 5;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);
    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    int x_out = -1;
    int y_out = -1;
    ck_assert_int_eq(room_get_start_position(r, &x_out, &y_out), OK);
    ck_assert_int_eq(x_out, 1);
    ck_assert_int_eq(y_out, 1);
}
END_TEST

/* ============================================================
 * Test 45: Test if a room with a portal has a start position
 * Tests if room_get_start_position returns OK and the right position on the portal since the portal is walkable
 * ============================================================ */
START_TEST(test_room_start_pos_with_portal_on_floor)
{
    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);

    int port_num = 3;
    Portal *p = calloc(port_num, sizeof(Portal));
    for (int i = 0; i < port_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "portal#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].target_room_id = 5 + i;
        p[i].x = i % 2 == 1? 0 : 4;
        p[i].y = i + 1;
    }

    ck_assert_int_eq(room_set_portals(r, p, port_num), OK);
    ck_assert_int_eq(r->portal_count, port_num);
    ck_assert_ptr_eq(p, r->portals);

    int x_out = -1;
    int y_out = -1;
    ck_assert_int_eq(room_get_start_position(r, &x_out, &y_out), OK);
    ck_assert_int_eq(x_out, 4);
    ck_assert_int_eq(y_out, 1);
}
END_TEST

/* ============================================================
 * Test 46: Test if a NULL room have a pushable
 * Tests if room_has_pushable_at returns false if the room is NULL
 * ============================================================ */
START_TEST(test_room_pushable_at_null)
{
    ck_assert_int_eq(room_has_pushable_at(NULL, 1, 2, NULL), false);
    ck_assert_int_eq(room_has_pushable_at(r, 1, 2, NULL), false);
    int index = -1;
    ck_assert_int_eq(room_has_pushable_at(NULL, 1, 2, &index), false);

    ck_assert_int_eq(room_has_pushable_at(r, 1, 2, &index), false);
}
END_TEST

/* ============================================================
 * Test 47: Test if a room has a pushable
 * Tests if room_has_pushable_at returns true where the pushable is
 * ============================================================ */
START_TEST(test_room_pushable_at_single)
{
    ck_assert_int_eq(room_has_pushable_at(r, 1, 2, NULL), false);

    
    Pushable *push = calloc(1, sizeof(Pushable));
    push->id = 5;
    push->x = 1;
    push->y = 2;
    r->pushables = push;
    r->pushable_count = 1;

    int index = -1;
    ck_assert_int_eq(room_has_pushable_at(r, 1, 2, NULL), true);
    ck_assert_int_eq(room_has_pushable_at(r, 1, 2, &index), true);
    ck_assert_int_eq(index, 0);

    index = -1;
    ck_assert_int_eq(room_has_pushable_at(r, 2, 2, &index), false);
    ck_assert_int_eq(room_has_pushable_at(r, 1, 1, &index), false);
    ck_assert_int_eq(index, -1);
}
END_TEST

/* ============================================================
 * Test 48: Test setting pushables on NULL
 * Tests that room_set_pushables returns INVALID_ARGUMENT on NULL input
 * ============================================================ */
START_TEST(test_set_pushables_null)
{
    int push_num = 3;
    Pushable t[push_num];
    for (int i = 0; i < push_num; i++) {
        t[i].id = i;
        t[i].name = "TEST_PUSHABLE";
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
    }

    ck_assert_int_eq(room_set_pushables(r, NULL, push_num), INVALID_ARGUMENT);
    ck_assert_int_eq(room_set_pushables(NULL, t, push_num), INVALID_ARGUMENT);
}
END_TEST

/* ============================================================
 * Test 49: Test setting pushables
 * Tests that room_set_pushables returns OK on good input
 * ============================================================ */
START_TEST(test_set_pushables_reg)
{
    int push_num = 3;
    Pushable *p = calloc(push_num, sizeof(Pushable));
    for (int i = 0; i < push_num; i++) {
        p[i].id = i;
        p[i].name = calloc(8, sizeof(char));
        strcpy(p[i].name, "pushab#");
        p[i].name[6] = (char)(48 + p[i].id); //Convert from value of int to char of said digit
        p[i].x = i % 2 == 1? 0 : 5;
        p[i].y = i + 1;
        p[i].initial_x = p[i].x;
        p[i].initial_y = p[i].y;
    }

    ck_assert_int_eq(room_set_pushables(r, p, push_num), OK);

    ck_assert_int_eq(r->pushables[0].id, 0);
    ck_assert_int_eq(r->pushables[1].id, 1);
    ck_assert_int_eq(r->pushables[2].id, 2);

    Pushable *q = calloc(push_num, sizeof(Pushable));
    for (int i = 0; i < push_num; i++) {
        q[i].id = i + 3;
        q[i].name = calloc(8, sizeof(char));
        strcpy(q[i].name, "pushab#");
        q[i].name[6] = (char)(48 + q[i].id); //Convert from value of int to char of said digit
        q[i].x = i + 2;
        q[i].y = i + 1;
        q[i].initial_x = q[i].x;
        q[i].initial_y = q[i].y;
    }

    ck_assert_int_eq(room_set_pushables(r, q, push_num), OK);

    // Veryify pushables in room
    ck_assert_int_eq(r->pushables[0].id, 3);
    ck_assert_int_eq(r->pushables[1].id, 4);
    ck_assert_int_eq(r->pushables[2].id, 5);

    ck_assert_int_eq(room_has_pushable_at(r, 2, 1, NULL), true);
    ck_assert_int_eq(room_is_walkable(r, 2, 1), false);

    ck_assert_int_eq(room_classify_tile(r, 2, 1, NULL), ROOM_TILE_PUSHABLE);
    int id_out = -1;
    ck_assert_int_eq(room_classify_tile(r, 2, 1, &id_out), ROOM_TILE_PUSHABLE);
    ck_assert_int_eq(id_out, 0);
    
    // Verify pushables render
    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';
    set.pushable='O';
    int num = (r->width) * (r->height);
    char buff[num];

    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), OK);

    char cust_buff_small[] = {"#######.O..##..O.##...O#######"};

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(buff[i], cust_buff_small[i]);
    }
    /*
    ######
    #.O..#
    #..O.#
    #...O#
    ######*/
}
END_TEST

/* ============================================================
 * Test 50: Test getting the ID of the room
 * Tests that room_get_id returns the right value
 * ============================================================ */
START_TEST(test_room_get_id)
{
    ck_assert_int_eq(room_get_id(NULL), -1);
    ck_assert_int_eq(room_get_id(r), ID);
}
END_TEST

/* ============================================================
 * Test 51: Test destorying treasure
 * Tests that destroy_treasure is correct
 * ============================================================ */
START_TEST(test_room_destory_treasure)
{
    Treasure *t = NULL;
    destroy_treasure(t);
    ck_assert_ptr_null(t);

    int treasure_num = 1;
    t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    destroy_treasure(t);
    ck_assert_ptr_nonnull(t);
    t = NULL;
}
END_TEST

/* ============================================================
 * Test 52: Test picking up treasure
 * Tests that room_pick_up_treasure returns the right value and that destroy_treasure is correct
 * ============================================================ */
START_TEST(test_room_pickup_treasure)
{
    int treasure_num = 3;
    Treasure *t = calloc(treasure_num, sizeof(Treasure));
    for (int i = 0; i < treasure_num; i++) {
        t[i].id = i;
        t[i].name = calloc(8, sizeof(char));
        strcpy(t[i].name, "treasu#");
        t[i].name[6] = (char)(48 + t[i].id); //Convert from value of int to char of said digit
        t[i].starting_room_id = ID;
        t[i].x = 1;
        t[i].y = i + 1;
        t[i].initial_x = 1;
        t[i].initial_y = i + 1;
        t[i].collected = false;
    }

    ck_assert_int_eq(room_set_treasures(r, t, treasure_num), OK);
    ck_assert_int_eq(r->treasure_count, treasure_num);
    ck_assert_ptr_eq(t, r->treasures);

    Treasure *t_out;
    ck_assert_int_eq(room_pick_up_treasure(NULL, 1, &t_out), INVALID_ARGUMENT);
    ck_assert_int_eq(room_pick_up_treasure(r, 1, NULL), INVALID_ARGUMENT);

    ck_assert_int_eq(room_pick_up_treasure(r, 1, &t_out), OK);
    ck_assert_int_eq(t_out->id, 1);

    ck_assert_int_eq(room_pick_up_treasure(r, 1, &t_out), INVALID_ARGUMENT);
    ck_assert_int_eq(room_pick_up_treasure(r, 4, &t_out), ROOM_NOT_FOUND);
}
END_TEST

/* ============================================================
 * Test 53: Test pushing pushables
 * Tests that room_try_push is correct
 * ============================================================ */
START_TEST(test_psuh_pushables)
{
    ck_assert_int_eq(room_try_push(r, 0, DIR_WEST), INVALID_ARGUMENT);
    r->pushable_count = 1;
    ck_assert_int_eq(room_try_push(r, 0, DIR_WEST), INVALID_ARGUMENT);

    Charset set;
    set.wall='#';
    set.floor='.';
    set.player='@';
    set.treasure='$';
    set.portal='X';
    set.pushable='O';
    int num = (r->width) * (r->height);
    char buff[num];
    char cust_buff_2[] = {"#######....##O.O.##...O#######"};
    char cust_buff_3[] = {"#######...O##O...##...O#######"};

    int push_num = 3;
    Pushable *q = calloc(push_num, sizeof(Pushable));
    for (int i = 0; i < push_num; i++) {
        q[i].id = i + 3;
        q[i].name = calloc(8, sizeof(char));
        strcpy(q[i].name, "pushab#");
        q[i].name[6] = (char)(48 + q[i].id); //Convert from value of int to char of said digit
        q[i].x = i + 2;
        q[i].y = i + 1;
        q[i].initial_x = q[i].x;
        q[i].initial_y = q[i].y;
    }

    ck_assert_int_eq(room_set_floor_grid(r, NULL), OK);
    ck_assert_int_eq(room_set_pushables(r, q, push_num), OK);

    // Edge and negative tests
    ck_assert_int_eq(room_try_push(NULL, 0, DIR_WEST), INVALID_ARGUMENT);
    ck_assert_int_eq(room_try_push(r, -1, DIR_WEST), INVALID_ARGUMENT);
    ck_assert_int_eq(room_try_push(r, push_num, DIR_WEST), INVALID_ARGUMENT);
    ck_assert_int_eq(room_try_push(r, 0, -1), INVALID_ARGUMENT);
    ck_assert_int_eq(room_try_push(r, 0, 4), INVALID_ARGUMENT);
    
    // Get all cardinals atleast once
    // West and south
    ck_assert_int_eq(room_try_push(r, 0, DIR_WEST), OK);
    ck_assert_int_eq(room_try_push(r, 0, DIR_SOUTH), OK);

    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(buff[i], cust_buff_2[i]);
    }
    /*
    ######
    #....#
    #O.O.#
    #...O#
    ######
    */

    // North and east
    ck_assert_int_eq(room_try_push(r, 1, DIR_NORTH), OK);
    ck_assert_int_eq(room_try_push(r, 1, DIR_EAST), OK);

    ck_assert_int_eq(room_render(r, &set, buff, r->width, r->height), OK);

    for (int i = 0; i < (r->width * r->height); i++) {
        ck_assert_int_eq(buff[i], cust_buff_3[i]);
    }
    /*
    ######
    #...O#
    #O...#
    #...O#
    ######
    */

    //Go out of bounds
    r->pushables[1].y = 0;
    /*
    ####O#
    #....#
    #O...#
    #...O#
    ######*/
    ck_assert_int_eq(room_try_push(r, 1, DIR_NORTH), INVALID_ARGUMENT);

    //Go into a wall
    ck_assert_int_eq(room_try_push(r, 0, DIR_WEST), ROOM_IMPASSABLE);
}
END_TEST

/* ============================================================
 * Suite Creation Function
 * 
 * This function builds and returns a test suite for the Check framework.
 * A Suite is a collection of test cases that are run together.
 * ============================================================ */

Suite *room_suite(void)
{
    // Create a new test suite with a descriptive name
    Suite *s = suite_create("Room");
    
    // Create a test case to group related tests
    TCase *tc_create = tcase_create("Create");
    TCase *tc_other = tcase_create("Other");

    // Attach setup and teardown functions to run before/after each test
    tcase_add_checked_fixture(tc_create, NULL, teardown_room);
    tcase_add_checked_fixture(tc_other, setup_room, teardown_room);

    // Add individual test functions to the test case
    tcase_add_test(tc_create, test_room_create);
    tcase_add_test(tc_create, test_room_create_value);
    tcase_add_test(tc_create, test_room_create_null);
    tcase_add_test(tc_create, test_room_create_noname);
    tcase_add_test(tc_create, test_room_create_null_name);
    tcase_add_test(tc_create, test_room_get_max);
    tcase_add_test(tc_create, test_room_get_null);
    tcase_add_test(tc_create, test_room_render_null_floor);
    tcase_add_test(tc_create, test_room_create_clamped_dim);

    tcase_add_test(tc_other, test_room_grid_null);
    tcase_add_test(tc_other, test_room_grid_implicit);
    tcase_add_test(tc_other, test_room_grid_new);
    tcase_add_test(tc_other, test_room_render_null);
    tcase_add_test(tc_other, test_room_render_bad_dim);
    tcase_add_test(tc_other, test_room_render_cust_floor);

    tcase_add_test(tc_other, test_set_portals_null);
    tcase_add_test(tc_other, test_set_portals_reg);
    tcase_add_test(tc_other, test_set_portals_overwrite);
    tcase_add_test(tc_other, test_set_treasures_null);
    tcase_add_test(tc_other, test_set_treasures_reg);
    tcase_add_test(tc_other, test_set_treasures_overwrite);

    tcase_add_test(tc_other, test_place_treasures_null);
    tcase_add_test(tc_other, test_place_treasures_no_init);
    tcase_add_test(tc_other, test_place_treasures_init);

    tcase_add_test(tc_other, test_room_render_port_tres);
    tcase_add_test(tc_other, test_get_treasure_at_null);
    tcase_add_test(tc_other, test_get_treasure_at_none);
    tcase_add_test(tc_other, test_get_treasure_at_reg);
    tcase_add_test(tc_other, test_get_treasure_at_moved);

    
    tcase_add_test(tc_other, test_get_portal_dest_null);
    tcase_add_test(tc_other, test_get_portal_dest_none);
    tcase_add_test(tc_other, test_get_portal_dest_reg);

    tcase_add_test(tc_other, test_room_is_walkable_null);
    tcase_add_test(tc_other, test_room_is_walkable_bad_bound);
    tcase_add_test(tc_other, test_room_is_walkable_null_grid);
    tcase_add_test(tc_other, test_room_is_walkable_grid_init);

    tcase_add_test(tc_other, test_room_classify_tile_null);
    tcase_add_test(tc_other, test_room_classify_tile_bad_bound);
    tcase_add_test(tc_other, test_room_classify_tile_grid_init);
    tcase_add_test(tc_other, test_room_classify_tile_port_treas);

    tcase_add_test(tc_other, test_room_start_pos_null);
    tcase_add_test(tc_other, test_room_start_pos_wall_to_wall_walls);
    tcase_add_test(tc_other, test_room_start_pos_no_portals);
    tcase_add_test(tc_other, test_room_start_pos_with_portal_on_wall);
    tcase_add_test(tc_other, test_room_start_pos_with_portal_on_floor);
    
    tcase_add_test(tc_other, test_room_pushable_at_null);
    tcase_add_test(tc_other, test_room_pushable_at_single);

    tcase_add_test(tc_other, test_set_pushables_null);
    tcase_add_test(tc_other, test_set_pushables_reg);

    tcase_add_test(tc_other, test_room_get_id);
    tcase_add_test(tc_other, test_room_destory_treasure);
    tcase_add_test(tc_other, test_room_pickup_treasure);
    tcase_add_test(tc_other, test_psuh_pushables);

    // Add the test case to the suite
    suite_add_tcase(s, tc_create);
    suite_add_tcase(s, tc_other);
    
    // Return the complete suite so main() can run it
    return s;
}
