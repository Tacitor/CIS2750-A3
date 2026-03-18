#GameUI should query the GameEngine for state and call rendering functions only.
import curses
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction

class GameUI:
    def __init__(self, config_path: str, profile_path: str):
        self._config = config_path
        self._profile = profile_path
        self._eng = GameEngine(self._config)

    def launch(self):
        print("Launching the TUI...")
        curses.wrapper(self._run_tui)
        print("Done game and done with the GameEngine")
        self._eng.destroy()
        del self._eng

    def _run_tui(self, stdscr):
        curses.curs_set(0)
        stdscr.clear() #'stdscr' is the main window
        stdscr.refresh()
        curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_MAGENTA)
        curses.init_pair(2, curses.COLOR_MAGENTA, curses.COLOR_BLACK)

        self._splash_startup(stdscr)
        self._splash_player_info(stdscr)

        self._ui_main_game(stdscr)

        self._splash_end(stdscr)
        self._splash_player_info(stdscr)

    def _ui_main_game(self, stdscr):
        game_element_offset = 30

        user_input = ord('z')
        while user_input != ord('q'):
            stdscr.clear()
            stdscr.addstr(0, 0, "<<message bar here>>")
            stdscr.addstr(1, 0, "<<room number and name here>>")

            room_str = self._eng.render_current_room()
            room_row = 3
            room_col = 0
            player_x, player_y = self._eng.player.get_position()
            stdscr.addstr(room_row, room_col, room_str)
            stdscr.addch(room_row + player_y, room_col + player_x, "@", curses.color_pair(2))

            stdscr.addstr(3, game_element_offset, "Game Elements:")
            stdscr.addstr(5, game_element_offset, "@ - Julia")
            stdscr.addstr(6, game_element_offset, "# - wall")
            stdscr.addstr(7, game_element_offset, "$ - gold")
            stdscr.addstr(8, game_element_offset, "X - portal")
            stdscr.addstr(9, game_element_offset, "<<other elements/names here>>")

            stdscr.addstr(20, 0, "Game Controls: <<list the keys used to control the game>>")
            stdscr.addstr(22, 0, "<< Player Status bar here (e.g. gold collected, rooms played, rooms left>>")
            stdscr.addstr(23, 0, "<<A title for your game here>>\t\t\t<<your email address here>>")
            stdscr.refresh()

            user_input = stdscr.getch()

            if user_input == ord('w'):
                self._eng.move_player(Direction.NORTH)
            elif user_input == ord('a'):
                self._eng.move_player(Direction.WEST)
            elif user_input == ord('s'):
                self._eng.move_player(Direction.SOUTH)
            elif user_input == ord('d'):
                self._eng.move_player(Direction.EAST)

    def _splash_startup(self, stdscr):
        stdscr.clear()
        stdscr.addstr(0, 0, "Julia's Jubilant Journey", curses.color_pair(1))
        stdscr.addstr(1, 0, "is a Sokoban-style puzzle game.", curses.color_pair(2))
        stdscr.addstr(2, 0, "Take control of Julia as she explores a world of mazes on her search for treasures.", curses.color_pair(2))

    def _splash_end(self , stdscr):
        stdscr.clear()
        stdscr.addstr(0, 0, "Game over", curses.color_pair(1))
        stdscr.addstr(1, 0, "Thanks for playing", curses.color_pair(2))

    def _splash_player_info(self, stdscr):
        stdscr.addstr(4, 0, "Player name:")
        stdscr.addstr(5, 0, "Games played:")
        stdscr.addstr(6, 0, "Treasure high score:")
        stdscr.addstr(7, 0, "Biggest world completed:")
        stdscr.addstr(8, 0, "Last played:")
        stdscr.addstr(10, 0, "Press any key to continue...")
        stdscr.refresh() # Screen is redrawn — not printed
        stdscr.getch() # Waits for a single key press (no Enter required)
