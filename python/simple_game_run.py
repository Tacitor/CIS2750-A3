from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction

eng = GameEngine("../assets/single_room.ini")

le_input = ""
while le_input != "q":
    str = eng.render_current_room()
    out = str.split('@')
    out[0] = out[0] + "\033[35m@\033[0m"    
    print(out[0] + out[1], end='')
    
    le_input = input()
    
    if le_input == 'w':
        eng.move_player(Direction.NORTH)
    elif le_input == 'a':
        eng.move_player(Direction.WEST)
    elif le_input == 's':
        eng.move_player(Direction.SOUTH)
    elif le_input == 'd':
        eng.move_player(Direction.EAST)

eng.destroy()
