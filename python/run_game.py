"""
You will create a run_game.py file at the root of the python folder. It is responsible for getting the location
of the desired .ini file and the profile file from the user via a --config parameter and a --profile
parameter. Use the same mechanism for getting command line arguments as you did for A2.

The run_game program consists of a main that adjusts the system path if necessary, collects the command
line arguments from the user and then launches the game_ui. More information on system paths will be
provided in the UI lectures.
"""
import argparse
import os
from treasure_runner.ui.game_ui import GameUI

def main() -> None:
    parser = argparse.ArgumentParser(description="Treasure Runner game")
    parser.add_argument("--config", type=str, required=True, metavar="<path>",
                        help="Path to the INI configuration file used to create the game world")

    parser.add_argument("--profile", type=str, required=True, metavar="<path>",
                        help="A JSON profile file that tracks player statistics")
    args = parser.parse_args()

    game_ui = GameUI(os.path.abspath(args.config), os.path.abspath(args.profile))
    game_ui.launch()
    
if __name__ == "__main__":
    raise SystemExit(main())
