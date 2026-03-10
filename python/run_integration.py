#!/usr/bin/env python3

from treasure_runner.models.game_engine import GameEngine
from treasure_runner.models.exceptions import GameError, ImpassableError
from treasure_runner.bindings import Direction
import argparse
import os
import time

def render_and_print_room(eng):
    str = eng.render_current_room()
    out = str.split('@')
    out[0] = out[0] + "\033[35m@\033[0m"    
    print(out[0] + out[1], end='')

def player_state(room, x, y, collected):
    return "room=" + str(room) + "|x=" + str(x) + "|y=" + str(y) + "|collected=" + str(collected)

def run_integration(config_path: str, log_path: str, debug: bool) -> None:
    step_count = 0
    
    with open(log_path, "w", encoding="utf-8") as log_file:
        # 1. Set up the engine and the first two lines of the log
        eng = GameEngine(config_path)
        width, height = eng.get_room_dimensions()
        log_file.write("RUN_START|config=" + str(config_path) + "|rooms=" + str(eng.get_room_count()) + "|room_width=" + str(width) + "|room_height=" + str(height) + "\n")
        player_old_x, player_old_y = eng.player.get_position()
        player_old_room = eng.player.get_room()
        log_file.write("STATE|step=" + str(step_count) + "|phase=SPAWN|state=" + player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count()) + "\n")
        
        # 2. Now check the four move directions. Stop if a good one is found
        entry_dirs_tried = 0
        dir_order = (Direction.SOUTH, Direction.WEST, Direction.NORTH, Direction.EAST)
        test_entry = True

        while test_entry:
            try:
                eng.move_player(dir_order[entry_dirs_tried])

                player_new_x, player_new_y = eng.player.get_position()
                player_new_room = eng.player.get_room()

                if ((player_new_x != player_old_x or player_new_y != player_old_y) and player_new_room == player_old_room):
                    test_entry = False
                else:
                    entry_dirs_tried+=1

                    if (entry_dirs_tried == len(dir_order)):
                        test_entry = False
                eng.reset()
            except ImpassableError:
                entry_dirs_tried+=1

                if (entry_dirs_tried == len(dir_order)):
                    test_entry = False

        #Evalute what just happned and what entry direction to take
        if (entry_dirs_tried >= len(dir_order)):
            raise RuntimeError("ERROR: failed to find any valid direction upon entering the room!")

        log_file.write("ENTRY|direction=" + str(dir_order[entry_dirs_tried].name) + "\n")

        # 3. Do the entry move
        eng.reset()
        before = player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count())
        collected_before = eng.player.get_collected_count()
        move_result = "OK"
        
        try:
            step_count+=1
            if debug:
                render_and_print_room(eng)
                time.sleep(1)
            eng.move_player(dir_order[entry_dirs_tried])
            if debug:
                render_and_print_room(eng)
                time.sleep(1)
        except GameError:
            move_result = "ERROR"

        player_old_x, player_old_y = eng.player.get_position()
        player_old_room = eng.player.get_room()
        after = player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count())
        log_file.write("MOVE|step=" + str(step_count) + "|phase=ENTRY|dir=" + str(dir_order[entry_dirs_tried].name) + "|result=" + move_result + "|before=" + before + "|after=" + after + "|delta_collected=" + str(eng.player.get_collected_count() - collected_before) + "\n")

        if move_result == "ERROR":
            log_file.write("TERMINATED: Initial Move Error\n")
            log_file.write("RUN_END|steps=" + str(step_count) + "|collected_total=" + str(eng.player.get_collected_count()) + "\n")
            return

        # 4. Time for directional sweeps
        for i in range(len(dir_order)):
            log_file.write("SWEEP_START|phase=SWEEP_" + dir_order[i].name + "|dir=" + dir_order[i].name + "\n")
            sweep_step_count = 0
            move_result = "OK"
            prev_player_states = []
            
            while move_result == "OK":
                before = player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count())
                prev_player_states.append(before)
                collected_before = eng.player.get_collected_count()
                move_result = "OK"
                
                try:
                    step_count+=1
                    eng.move_player(dir_order[i])
                    if debug:
                        render_and_print_room(eng)
                        time.sleep(1)
                    sweep_step_count+=1
                except ImpassableError:
                    move_result = "BLOCKED"
                except:
                    move_result = "ERROR"

                player_old_x, player_old_y = eng.player.get_position()
                player_old_room = eng.player.get_room()
                after = player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count())

                # check for "Move causes no state change"
                if (before == after and move_result == "OK"):
                    move_result = "BLOCKED"
                else:
                    # check for a cycle
                    for state in prev_player_states:
                        if state == after and move_result == "OK":
                            move_result = "CYCLE_DETECTED"

                log_file.write("MOVE|step=" + str(step_count) + "|phase=SWEEP_" + dir_order[i].name + "|dir=" + dir_order[i].name + "|result=" + move_result + "|before="
                               + before + "|after=" + after + "|delta_collected=" + str(eng.player.get_collected_count() - collected_before) + "\n")

            log_file.write("SWEEP_END|phase=SWEEP_" + dir_order[i].name + "|reason=" + move_result + "|moves=" + str(sweep_step_count) + "\n")

        # Final Log Lines
        log_file.write("STATE|step=" + str(step_count) + "|phase=FINAL|state=" + player_state(player_old_room, player_old_x, player_old_y, eng.player.get_collected_count()) + "\n")
        log_file.write("RUN_END|steps=" + str(step_count) + "|collected_total=" + str(eng.player.get_collected_count()) + "\n")
    eng.destroy()
    #===== END OF run_integration() =====#

def main() -> None:
    parser = argparse.ArgumentParser(description="Treasure Runner integration test logger")
    parser.add_argument("--config", type=str, required=True, metavar="<path>",
                        help="path to the INI configuration file used to create the game world")

    parser.add_argument("--log", type=str, required=True, metavar="<path>",
                        help="path to the output log file")
    args = parser.parse_args()
    run_integration(os.path.abspath(args.config), os.path.abspath(args.log), False)
    
if __name__ == "__main__":
    raise SystemExit(main())
