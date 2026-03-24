#GameUI should query the GameEngine for state and call rendering functions only.
import curses
import json
from pathlib import Path
from datetime import datetime, timezone
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction
from treasure_runner.models.exceptions import GameEngineError, ImpassableError, NoPortalError
from .user_info import UserInfo

class GameUI:
    def __init__(self, config_path: str, profile_path: str):
        self._config = config_path
        self._profile = profile_path
        self._eng = GameEngine(self._config)
        self._user_info_valid, self._user_info = load_player_info(self._profile)

        self._cache_player_collected_gold = self._eng.player.get_collected_count() # Local cache of player gold for UI display purpose. Used to update the message bar if a player collects a treasure
        self._cache_player_room_id = self._eng.player.get_room() # Another local cache just for UI detection and status message prompting
        self._cache_world_gold_total_count = self._eng.get_world_treasure_count()
        self._cache_charset = self._eng.get_charset()

        self.rooms_played = 0
        self.rooms_left = 0
        self._update_treasure_stats()

        self.game_name = "Julia's Jubilant Journey"
        self.email = "lkrampit@uoguelph.ca"
        self.game_controls = "Game Controls: Arrow-keys/WASD to move, > to use an underfoot portal, q to quit, and r to reset whole game."

        gold_char = self._cache_charset["treasure"]
        self._message_bar = f"Collect all the gold ({gold_char}). Start by moving around."
        self._message_bar_colour = 4

        self.colour_magenta = 2
        self.colour_green = 3
        self.colour_yellow = 4
        self.colour_cyan = 5
        self.colour_black = 6

    def launch(self):
        self._validate_user_info()

        print(f"Launching {self.game_name}...")
        try:
            curses.wrapper(self._run_tui)
        except (BufferError, curses.error):
            # Give a user prompt on how to fix issue
            print("[ERROR] Sorry it looks like your terminal is too small to display the game.")
            print("Please resize the terminal to be bigger, and try again.")
        self._eng.destroy()
        del self._eng

    def _run_tui(self, stdscr):
        curses.curs_set(0)

        # Lab 5 boiler plate
        curses.cbreak() # Don't wait for Enter
        curses.noecho() # Don't show typed characters
        stdscr.keypad(True) # Enable arrow keys
        curses.start_color() # Enable color support
        curses.use_default_colors() # Use terminal defaults

        stdscr.clear()
        curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_MAGENTA)
        curses.init_pair(self.colour_magenta, curses.COLOR_MAGENTA, -1) # User default background colour
        curses.init_pair(self.colour_green, curses.COLOR_GREEN, -1)
        curses.init_pair(self.colour_yellow, curses.COLOR_YELLOW, -1)
        curses.init_pair(self.colour_cyan, curses.COLOR_CYAN, -1)

        if curses.can_change_color():
            curses.init_color(curses.COLOR_BLACK, 400, 400, 400)

        curses.init_pair(self.colour_black, curses.COLOR_BLACK, -1)

        self._splash_startup(stdscr)
        self._splash_player_info(stdscr)

        self._ui_main_game(stdscr)

        if self._eng.player.get_collected_count() < self._cache_world_gold_total_count:
            self._splash_quit(stdscr)
        else:
            self._splash_win(stdscr)
        self._splash_player_info(stdscr)

    def _ui_main_game(self, stdscr):
        user_input = ord('z')
        room_row = 3

        while user_input != ord('q') and user_input != ord('Q') and self._eng.player.get_collected_count() < self._cache_world_gold_total_count:
            room_width, room_height = self._eng.get_room_dimensions()

            stdscr.clear()

            safe_addstr_colour(stdscr, 0, 0, self._message_bar, curses.color_pair(self._message_bar_colour))
            safe_addstr(stdscr, 1, 0, "Room " + str(self._eng.player.get_room()) + " \"" + self._eng.get_room_name() + "\"")

            self._ui_render_room(stdscr, room_row)
            self._ui_render_side_bar_and_status(stdscr, room_row, room_width, room_height)

            stdscr.refresh()

            user_input = self._get_user_input(stdscr)
            self._update_treasure_stats()
            self._check_player_room()
            self._check_player_collected_gold()
            self._check_player_gold_progress()

        # Once the user has quit (or win condition) update the save profile
        self._update_player_info()
        save_player_info(self._profile, self._user_info)

    def _ui_render_room(self, stdscr, room_row):
        room_str = self._eng.render_current_room()
        room_width, room_height = self._eng.get_room_dimensions()

        for y in range(room_height):
            for x in range(room_width):
                char = room_str[(y * room_width) + x + y]
                colour = self._get_char_colour_render(char)

                if char == self._cache_charset["portal"]:
                    colour = self._get_portal_colour_render(x, y)

                stdscr.addch(room_row + y, x, char, curses.color_pair(colour))

    def _get_char_colour_render(self, char):
        charset = self._cache_charset
        if char == charset["player"]:
            colour = self.colour_magenta
        elif char == charset["treasure"]:
            colour = self.colour_yellow
        elif char == charset["pushable"]:
            colour = self.colour_green
        elif char == charset["portal"]:
            colour = 1 # This colour NEEDS to be updated in _get_portal_colour_render(). Get it alarm colour to see if required call is skipped.
        elif char == charset["switch_off"]:
            colour = self.colour_magenta
        elif char == charset["switch_on"]:
            colour = self.colour_cyan
        elif char == charset["floor"]:
            colour = self.colour_black
        else:
            colour = 0

        return colour

    def _get_portal_colour_render(self, x, y):
        has_locked, x_gated, y_gated = self._eng.query_gated_portal()

        if has_locked and x == x_gated and y == y_gated:
            return self.colour_magenta

        return self.colour_cyan

    def _ui_render_side_bar_and_status(self, stdscr, room_row, room_width, room_height):
        game_element_height_offset = room_height + room_row + 1

        side_bar_lowest = 13

        term_y, term_x = stdscr.getmaxyx()
        term_y += 1 # Useless operation on this var to let the linter be happy

        self._ui_render_side_bar(stdscr, room_width, side_bar_lowest)

        if game_element_height_offset < side_bar_lowest + 2:
            game_element_height_offset = side_bar_lowest + 2

        safe_addstr(stdscr, game_element_height_offset, 0, self.game_controls)
        safe_addstr_colour(stdscr, game_element_height_offset, 0, "Game Controls:", curses.color_pair(self.colour_yellow))
        safe_addstr(stdscr, game_element_height_offset + 2, 0, self._user_info.name + " Status: " + str(self._eng.player.get_collected_count()) + f" gold collected, {self.rooms_played} room(s) played, {self.rooms_left} room(s) left")
        safe_addstr_colour(stdscr, game_element_height_offset + 2, 0, self._user_info.name + " Status:", curses.color_pair(self.colour_yellow))
        safe_addstr_colour(stdscr, game_element_height_offset + 3, 0, self.game_name, curses.color_pair(self.colour_magenta))

        self._ui_render_email(stdscr, term_x, game_element_height_offset)

    def _ui_render_side_bar(self, stdscr, room_width, side_bar_lowest):
        game_element_width_offset = room_width + 5

        safe_addstr(stdscr, 3, game_element_width_offset, "Game Elements:")
        charset = self._cache_charset
        safe_addstr(stdscr, 5, game_element_width_offset, charset["player"] + " - Julia")
        safe_addstr_colour(stdscr, 5, game_element_width_offset, charset["player"], curses.color_pair(self.colour_magenta))

        safe_addstr(stdscr, 6, game_element_width_offset, charset["portal"] + " - portal off")
        safe_addstr_colour(stdscr, 6, game_element_width_offset, charset["portal"], curses.color_pair(self.colour_magenta))
        safe_addstr(stdscr, 7, game_element_width_offset, charset["portal"] + " - portal on")
        safe_addstr_colour(stdscr, 7, game_element_width_offset, charset["portal"], curses.color_pair(self.colour_cyan))

        safe_addstr(stdscr, 8, game_element_width_offset, charset["switch_off"] + " - switch off")
        safe_addstr_colour(stdscr, 8, game_element_width_offset, charset["switch_off"], curses.color_pair(self.colour_magenta))
        safe_addstr(stdscr, 9, game_element_width_offset, charset["switch_on"] + " - switch on")
        safe_addstr_colour(stdscr, 9, game_element_width_offset, charset["switch_on"], curses.color_pair(self.colour_cyan))

        safe_addstr(stdscr, 10, game_element_width_offset, charset["wall"] + " - wall")

        safe_addstr(stdscr, 11, game_element_width_offset, charset["floor"] + " - floor")
        safe_addstr_colour(stdscr, 11, game_element_width_offset, charset["floor"], curses.color_pair(self.colour_black))

        safe_addstr(stdscr, 12, game_element_width_offset, charset["treasure"] + " - gold")
        safe_addstr_colour(stdscr, 12, game_element_width_offset, charset["treasure"], curses.color_pair(self.colour_yellow))

        safe_addstr(stdscr, side_bar_lowest, game_element_width_offset, charset["pushable"] + " - pushable obstacle")
        safe_addstr_colour(stdscr, side_bar_lowest, game_element_width_offset, charset["pushable"], curses.color_pair(self.colour_green))

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
            self._parse_input(user_input)

        except ImpassableError:
            self._message_bar = "You can't go that way!"
            self._message_bar_colour = 2
        except NoPortalError:
            portal_char = self._cache_charset["portal"]
            self._message_bar = f"You need to be standing on a portal ({portal_char}) to teleport."
            self._message_bar_colour = 4
        except GameEngineError:
            self._message_bar = "The GameEngine had a GameEngineError!"
            self._message_bar_colour = 1

        return user_input

    def _parse_input(self, user_input):
        if user_input == ord('w') or user_input == 259 or user_input == ord('W'):
            self._eng.move_player(Direction.NORTH)
        elif user_input == ord('a') or user_input == 260 or user_input == ord('A'):
            self._eng.move_player(Direction.WEST)
        elif user_input == ord('s') or user_input == 258 or user_input == ord('S'):
            self._eng.move_player(Direction.SOUTH)
        elif user_input == ord('d') or user_input == 261 or user_input == ord('D'):
            self._eng.move_player(Direction.EAST)
        elif user_input == ord('r') or user_input == ord('R'):
            self._message_bar = "The game as been reset."
            self._message_bar_colour = 4
            self._eng.reset()
        elif user_input == ord('>'):
            self._eng.underfoot_portal()

    def _check_player_room(self):
        before = self._cache_player_room_id
        self._cache_player_room_id = self._eng.player.get_room()

        if self._message_bar == "" and before != self._cache_player_room_id:
            self._message_bar = "You teleported to another room."
            self._message_bar_colour = 0

    def _check_player_collected_gold(self):
        before = self._cache_player_collected_gold
        self._cache_player_collected_gold = self._eng.player.get_collected_count()

        if self._message_bar == "" and before != self._cache_player_collected_gold:
            self._message_bar = "You picked up gold."
            self._message_bar_colour = 0

    def _check_player_gold_progress(self):
        if self._message_bar == "":
            self._message_bar = f"{self._eng.player.get_collected_count()}/{self._cache_world_gold_total_count} gold collected."

    def _update_treasure_stats(self):
        self.rooms_played = self._eng.get_complete_room_count()
        self.rooms_left = self._eng.get_room_count() - self.rooms_played

    def _splash_startup(self, stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, self.game_name, curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "is a Sokoban-style puzzle game.", curses.color_pair(self.colour_magenta))
        safe_addstr_colour(stdscr, 2, 0, "Take control of Julia as she explores a world of mazes on her search for treasures.", curses.color_pair(self.colour_magenta))

    def _splash_quit(self , stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, "Game over", curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "Thanks for playing", curses.color_pair(self.colour_magenta))
        time1 = datetime.fromisoformat(self._user_info.timestamp_last_played)
        time2 = datetime.fromisoformat(self._user_info.timestamp_started_game)
        time_diff = time1 - time2
        safe_addstr(stdscr, 2, 0, "Time elapsed: " + str(time_diff))

    def _splash_win(self , stdscr):
        stdscr.clear()
        safe_addstr_colour(stdscr, 0, 0, "You Win!", curses.color_pair(1))
        safe_addstr_colour(stdscr, 1, 0, "Thanks for playing. Congratulations on your victory.", curses.color_pair(self.colour_magenta))
        time1 = datetime.fromisoformat(self._user_info.timestamp_last_played)
        time2 = datetime.fromisoformat(self._user_info.timestamp_started_game)
        time_diff = time1 - time2
        safe_addstr(stdscr, 2, 0, "Time elapsed: " + str(time_diff))
        safe_addstr(stdscr, 3, 0, "Rooms visited: " + str(self.rooms_played))

    def _splash_player_info(self, stdscr):
        safe_addstr(stdscr, 4, 0, "Player name: " + self._user_info.name)
        safe_addstr(stdscr, 5, 0, "Games played: " + str(self._user_info.games_played))
        safe_addstr(stdscr, 6, 0, "Treasure high score: " + str(self._user_info.max_treasure_collected))
        safe_addstr(stdscr, 7, 0, "Biggest world completed: " + str(self._user_info.most_rooms_world_completed))
        safe_addstr(stdscr, 8, 0, "Last played: " + self._user_info.timestamp_last_played)
        safe_addstr_colour(stdscr, 10, 5, "Press any key to continue...", curses.color_pair(self.colour_green))
        stdscr.refresh() # Screen is redrawn — not printed

        char = stdscr.getch() # Waits for a single key press (no Enter required)
        # To improve UX don't count window resizing as a keypress, all other are fine
        while char == 410:
            char = stdscr.getch()

    def _validate_user_info(self):
        if not self._user_info_valid:
            print("[ERROR] The provided profile file-path either does not exist or was not vaid JSON format. It will be overwritten.")
            print("Please enter a new name:")
            new_name = input()
            self._user_info.name = new_name
            self._user_info_valid = True

        self._user_info.timestamp_started_game = datetime.now(timezone.utc).isoformat()

    def _update_player_info(self):
        if self._user_info.max_treasure_collected < self._eng.player.get_collected_count():
            self._user_info.max_treasure_collected = self._eng.player.get_collected_count()

        if self.rooms_left == 0 and self._user_info.most_rooms_world_completed < self.rooms_played:
            self._user_info.most_rooms_world_completed = self.rooms_played

        self._user_info.games_played += 1
        self._user_info.timestamp_last_played = datetime.now(timezone.utc).isoformat()

def save_player_info(path: str, user_info: UserInfo):
    with open(path, "w", encoding="utf-8") as file:
        json.dump(user_info.to_dict(), file, indent=2)

def load_player_info(path) -> tuple[bool, UserInfo]:
    if not Path(path).exists():
        return False, UserInfo()

    try:
        with open(path, "r", encoding="utf-8") as file:
            data = json.load(file)
        return True, UserInfo.from_dict(data)

    except json.JSONDecodeError:
        return False, UserInfo()

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
