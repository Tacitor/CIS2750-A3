import unittest
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.models.exceptions import GameEngineError
from treasure_runner.bindings import Direction

class TestTreasureRunnerGameEngine(unittest.TestCase):

    def setUp(self):
        self.eng = GameEngine("../assets/starter.ini")

    def test_default_game_engine(self):
        result = self.eng.get_room_count()
        self.assertEqual(result, 3)

    def test_game_engine_setup_invalid(self):
        with self.assertRaises(GameEngineError):
            self.eng = GameEngine("../bad/file/path.ini")

    def test_game_engine_destory(self):
        self.eng.destroy()

    def test_game_engine_render(self):
        room_expected = "###################\n#..........#......#\n#$..#......#......#\n#...#......#......X\n#.#........#...##.#\n@...##....##..#####\n#...#......#...#..#\n#...#......#...$..#\n#...#......#......#\nX..$.......#......#\n#..$#......#......#\n#.................X\n###################\n"
        str = self.eng.render_current_room()
        self.assertEqual(room_expected, str)

    def test_game_engine_get_room_dim(self):
        width, height = self.eng.get_room_dimensions()
        self.assertEqual(width, 19)
        self.assertEqual(height, 13)

    def test_game_engine_get_room_ids(self):
        list = self.eng.get_room_ids()
        self.assertEqual(list[0], 0)
        self.assertEqual(list[1], 1)
        self.assertEqual(list[2], 2)

    def test_game_engine_reset(self):
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.WEST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.WEST)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST) #pick up first treasure on this spot
        self.eng.move_player(Direction.EAST) #pick up the second treasure on this same spot
        ##############XX###
        #....#............#
        #....#............#
        ##.#####.#..##.####
        #....#.........@..#
        #....#...#........#
        #....$...#........#
        #..#.#...###......#
        #....#...###......#
        #....#...#........#
        #..$.#...#........#
        #....#............#
        ##############X####
        count = self.eng.player.get_collected_count()
        self.assertEqual(count, 2)
        room = self.eng.player.get_room()
        self.assertEqual(room, 1)
        x, y = self.eng.player.get_position()
        self.assertEqual(x, 15)
        self.assertEqual(y, 4)
        
        self.eng.reset()
        
        count = self.eng.player.get_collected_count()
        self.assertEqual(count, 0)
        room = self.eng.player.get_room()
        self.assertEqual(room, 0)
        x, y = self.eng.player.get_position()
        self.assertEqual(x, 0)
        self.assertEqual(y, 5)

if __name__ == "__main__":
    unittest.main()
