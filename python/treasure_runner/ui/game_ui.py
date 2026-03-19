#GameUI should query the GameEngine for state and call rendering functions only.
import curses
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction
from treasure_runner.models.exceptions import GameEngineError, ImpassableError

class GameUI:
    def __init__(self, config_path: str, profile_path: str):
        self._config = config_path
        self._profile = profile_path
        self._eng = GameEngine(self._config)

        self.game_name = "Julia's Jubilant Journey"
        self.email = "lkrampit@uoguelph.ca"
        self.game_controls = "Game Controls: Arrow-keys/WASD to move, > to use a portal, q to quit, and r to reset whole game."

        self._message_bar = "Start by moving around"
        self._message_bar_colour = 4

    def launch(self):
        print("Launching the TUI...")
        try:
            curses.wrapper(self._run_tui)
        except BufferError as exc:
            # TODOFIXME: Is this what they mean by raise an error? Should I be catching it again?
            print("[ERROR] Sorry it looks like your terminal is too small to display the game.")
            print(str(exc))
        print("Done game and done with the GameEngine")
        self._eng.destroy()
        del self._eng

    def _run_tui(self, stdscr):
        curses.curs_set(0)
        stdscr.clear() #'stdscr' is the main window
        curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_MAGENTA)
        curses.init_pair(2, curses.COLOR_MAGENTA, curses.COLOR_BLACK)
        curses.init_pair(3, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(4, curses.COLOR_YELLOW, curses.COLOR_BLACK)

        self._splash_startup(stdscr)
        self._splash_player_info(stdscr)

        self._ui_main_game(stdscr)

        self._splash_end(stdscr)
        self._splash_player_info(stdscr)

    def _ui_main_game(self, stdscr):
        user_input = ord('z')

        while user_input != ord('q') and user_input != ord('Q'):
            room_row = 3
            room_width, room_height = self._eng.get_room_dimensions()

            stdscr.clear()

            safe_addstr_colour(stdscr, 0, 0, self._message_bar, curses.color_pair(self._message_bar_colour))
            safe_addstr(stdscr, 1, 0, "Room " + str(self._eng.player.get_room()) + " \"" + self._eng.get_room_name() + "\"")

            self._ui_render_room(stdscr, room_row)
            self._ui_render_side_bar_and_status(stdscr, room_row, room_width, room_height)

            stdscr.refresh()

            user_input = self._get_user_input(stdscr)

    def _ui_render_room(self, stdscr, room_row):
        room_str = self._eng.render_current_room()
        player_x, player_y = self._eng.player.get_position()
        charset = self._eng.get_charset()
        safe_addstr(stdscr, room_row, 0, room_str)
        # TODOFIXME: Make safe?
        stdscr.addch(room_row + player_y, 0 + player_x, charset.contents.player.decode("utf-8"), curses.color_pair(2))

    def _ui_render_side_bar_and_status(self, stdscr, room_row, room_width, room_height):
        game_element_height_offset = room_height + room_row + 1

        side_bar_lowest = 10

        term_y, term_x = stdscr.getmaxyx()
        term_y += 1 # Useless operation on this var to let the linter be happy

        self._ui_render_side_bar(stdscr, room_width, side_bar_lowest)

        if game_element_height_offset < side_bar_lowest + 2:
            game_element_height_offset = side_bar_lowest + 2
        # TODOFIXME: Add > portal traversal support.
        # TODOFIXME: Add r to reset support.
        safe_addstr(stdscr, game_element_height_offset, 0, self.game_controls)
        safe_addstr_colour(stdscr, game_element_height_offset, 0, "Game Controls:", curses.color_pair(4))
        # TODOFIXME: Implement this
        safe_addstr(stdscr, game_element_height_offset + 2, 0, "<NAME> Status: " + str(self._eng.player.get_collected_count()) + " gold collected, 1 room(s) played, 4 room(s) left")
        # TODOFIXME: Put name from the .json here. Considered a room played once all the treasures in this room have been collected.
        safe_addstr_colour(stdscr, game_element_height_offset + 2, 0, "<NAME> Status:", curses.color_pair(4))
        safe_addstr_colour(stdscr, game_element_height_offset + 3, 0, self.game_name, curses.color_pair(2))

        self._ui_render_email(stdscr, term_x, game_element_height_offset)

    def _ui_render_side_bar(self, stdscr, room_width, side_bar_lowest):
        game_element_width_offset = room_width + 5

        safe_addstr(stdscr, 3, game_element_width_offset, "Game Elements:")
        charset = self._eng.get_charset()
        safe_addstr(stdscr, 5, game_element_width_offset, charset.contents.player.decode("utf-8") + " - Julia")
        safe_addstr_colour(stdscr, 5, game_element_width_offset, charset.contents.player.decode("utf-8"), curses.color_pair(2))
        safe_addstr(stdscr, 6, game_element_width_offset, charset.contents.wall.decode("utf-8") + " - wall")
        safe_addstr(stdscr, 7, game_element_width_offset, charset.contents.treasure.decode("utf-8") + " - gold")
        safe_addstr(stdscr, 8, game_element_width_offset, charset.contents.portal.decode("utf-8") + " - portal")
        safe_addstr(stdscr, 9, game_element_width_offset, charset.contents.floor.decode("utf-8") + " - floor")
        safe_addstr(stdscr, side_bar_lowest, game_element_width_offset, charset.contents.pushable.decode("utf-8") + " - pushable obstacle")
        # safe_addstr(stdscr, 11, game_element_width_offset, charset.contents.switch_off.decode("utf-8") + " - switch off")
        # safe_addstr(stdscr, side_bar_lowest, game_element_width_offset, charset.contents.switch_on.decode("utf-8") + " - switch on")

    def _ui_render_email(self, stdscr, term_x, game_element_height_offset):
        email_pos = term_x - len(self.email)
        if len(self.game_controls) < term_x:
            email_pos = len(self.game_controls) - len(self.email)
        safe_addstr(stdscr, game_element_height_offset + 3, email_pos, self.email)

    def _get_user_input(self, stdscr) -> int:
        user_input = stdscr.getch()
        self._message_bar = ""
        self._message_bar_colour = 0

        try:
            if user_input == ord('w') or user_input == 259 or user_input == ord('W'):
                self._eng.move_player(Direction.NORTH)
            elif user_input == ord('a') or user_input == 260 or user_input == ord('A'):
                self._eng.move_player(Direction.WEST)
            elif user_input == ord('s') or user_input == 258 or user_input == ord('S'):
                self._eng.move_player(Direction.SOUTH)
            elif user_input == ord('d') or user_input == 261 or user_input == ord('D'):
                self._eng.move_player(Direction.EAST)
            elif user_input == ord('r') or user_input == ord('R'):
                self._message_bar = "The game as been reset"
                self._message_bar_colour = 4
                self._eng.reset()
            elif user_input == ord('>'):
                # TODOFIXME: Add this. There should be a message for successful teleportation and another for if the player is not on/near a portal
                self._message_bar = "This form of portal travel has not been added yet"

            # TODOFIXME: Add message bar text for treasure collection
        except ImpassableError:
            self._message_bar = "You can't go that way!"
            self._message_bar_colour = 2
        except GameEngineError:
            self._message_bar = "The GameEngine had a GameEngineError"
            self._message_bar_colour = 1

        return user_input

    def _splash_startup(self, stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, self.game_name, curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "is a Sokoban-style puzzle game.", curses.color_pair(2))
        safe_addstr_colour(stdscr, 2, 0, "Take control of Julia as she explores a world of mazes on her search for treasures.", curses.color_pair(2))

    def _splash_end(self , stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, "Game over", curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "Thanks for playing", curses.color_pair(2))

    def _splash_player_info(self, stdscr):
        safe_addstr(stdscr, 4, 0, "Player name:")
        safe_addstr(stdscr, 5, 0, "Games played:")
        safe_addstr(stdscr, 6, 0, "Treasure high score:")
        safe_addstr(stdscr, 7, 0, "Biggest world completed:")
        safe_addstr(stdscr, 8, 0, "Last played:")
        safe_addstr_colour(stdscr, 10, 5, "Press any key to continue...", curses.color_pair(3))
        stdscr.refresh() # Screen is redrawn — not printed

        char = stdscr.getch() # Waits for a single key press (no Enter required)
        # To improve UX don't count window resizing as a keypress, all other are fine
        while char == 410:
            char = stdscr.getch()

def safe_addstr_colour(stdscr, row, col, string, colour_pair):
    try:
        stdscr.addstr(row, col, string, colour_pair)
    except curses.error as exc:
        raise BufferError(str(exc)) from exc

def safe_addstr(stdscr, row, col, string):
    try:
        stdscr.addstr(row, col, string)
    except curses.error as exc:
        raise BufferError(str(exc)) from exc
