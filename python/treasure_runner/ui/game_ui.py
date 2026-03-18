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

        self._message_bar = "Start by moving around"
        self._lowest_str = 0
        self._rightest_str = 0

    def launch(self):
        print("Launching the TUI...")
        try:
            curses.wrapper(self._run_tui)
        except BufferError as exc:
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

        self._splash_startup(stdscr)
        self._splash_player_info(stdscr)

        self._ui_main_game(stdscr)

        self._splash_end(stdscr)
        self._splash_player_info(stdscr)

    def _ui_main_game(self, stdscr):
        game_element_offset = 30

        user_input = ord('z')
        while user_input != ord('q') and user_input != ord('Q'):
            # TODOFIXME: Update this
            self._lowest_str = 23
            # TODOFIXME: Update this
            self._rightest_str = game_element_offset

            stdscr.clear()
            safe_addstr(stdscr, 0, 0, self._message_bar)
            safe_addstr(stdscr, 1, 0, "<<room number and name here>>")

            room_str = self._eng.render_current_room()
            room_row = 3
            room_col = 0
            player_x, player_y = self._eng.player.get_position()
            safe_addstr(stdscr, room_row, room_col, room_str)
            # TODOFIXME: Make safe?
            stdscr.addch(room_row + player_y, room_col + player_x, "@", curses.color_pair(2))

            safe_addstr(stdscr, 3, game_element_offset, "Game Elements:")
            safe_addstr(stdscr, 5, game_element_offset, "@ - Julia")
            safe_addstr(stdscr, 6, game_element_offset, "# - wall")
            safe_addstr(stdscr, 7, game_element_offset, "$ - gold")
            safe_addstr(stdscr, 8, game_element_offset, "X - portal")
            safe_addstr(stdscr, 9, game_element_offset, "<<other elements/names here>>")

            safe_addstr(stdscr, 20, 0, "Game Controls: <<list the keys used to control the game>>")
            safe_addstr(stdscr, 22, 0, "<< Player Status bar here (e.g. gold collected, rooms played, rooms left>>")
            safe_addstr(stdscr, 23, 0, "<<A title for your game here>>\t\t\t<<your email address here>>")
            stdscr.refresh()

            user_input = self._get_user_input(stdscr)

    def _get_user_input(self, stdscr) -> int:
        user_input = stdscr.getch()

        try:
            if user_input == ord('w') or user_input == 259 or user_input == ord('W'):
                self._eng.move_player(Direction.NORTH)
            elif user_input == ord('a') or user_input == 260 or user_input == ord('A'):
                self._eng.move_player(Direction.WEST)
            elif user_input == ord('s') or user_input == 258 or user_input == ord('S'):
                self._eng.move_player(Direction.SOUTH)
            elif user_input == ord('d') or user_input == 261 or user_input == ord('D'):
                self._eng.move_player(Direction.EAST)

            self._message_bar = ""
            # TODOFIXME: Remove this
            self._message_bar = str(user_input)
        except ImpassableError:
            self._message_bar = "You can't go that way!"
        except GameEngineError:
            self._message_bar = "The GameEngine had a GameEngineError"

        return user_input

    def _splash_startup(self, stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, "Julia's Jubilant Journey", curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "is a Sokoban-style puzzle game.", curses.color_pair(2))
        safe_addstr_colour(stdscr, 2, 0, "Take control of Julia as she explores a world of mazes on her search for treasures.", curses.color_pair(2))

    def _splash_end(self , stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, "Game over", curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "Thanks for playing", curses.color_pair(2))

    def _splash_player_info(self, stdscr):
        # TODOFIXME: Update this
        self._lowest_str = 10
        # TODOFIXME: Update this
        self._rightest_str = 5
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
