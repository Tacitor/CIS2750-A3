#GameUI should query the GameEngine for state and call rendering functions only.
import curses

class GameUI:
    def __init__(self, config_path: str, profile_path: str):
        self._config = config_path
        self._profile = profile_path
        
        print("UI Time: " + self._profile + ", " + self._config)

    def launch(self):
        print("Launching the TUI...")
        curses.wrapper(self._run_tui)

    def _run_tui(self, stdscr):
        curses.curs_set(0)
        stdscr.clear() #'stdscr' is the main window
        stdscr.addstr(0, 0, "Hello, World!")
        stdscr.addstr(1, 1, "We are running this TUI so good")
        stdscr.addstr(10, 0, "Press any key to exit...")
        stdscr.refresh() # Screen is redrawn — not printed
        stdscr.getch() # Waits for a single key press (no Enter required)
