import unittest
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.bindings import Direction

class TestTreasureRunnerPlayer(unittest.TestCase):

    def setUp(self):
        self.eng = GameEngine("../assets/starter.ini")
        self.player = self.eng.player

    def test_player_get_reg(self):
        room = self.player.get_room()
        self.assertEqual(room, 0)
        x, y = self.player.get_position()
        self.assertEqual(x, 0)
        self.assertEqual(y, 5)

    def test_player_get_mov(self):
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)

        room = self.player.get_room()
        self.assertEqual(room, 0)
        x, y = self.player.get_position()
        self.assertEqual(x, 2)
        self.assertEqual(y, 7)

    def test_player_get_portal(self):
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.WEST)

        room = self.player.get_room()
        self.assertEqual(room, 2)
        x, y = self.player.get_position()
        self.assertEqual(x, 0)
        self.assertEqual(y, 8)

    def test_player_get_collected_count(self):
        col = self.player.get_collected_count()
        self.assertEqual(col, 0)

        #Grab 2 treasures
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        
        col = self.player.get_collected_count()
        self.assertEqual(col, 2)

    def test_player_has_collected(self):
        has = self.player.has_collected_treasure(0)
        self.assertEqual(has, False)
        has = self.player.has_collected_treasure(1)
        self.assertEqual(has, False)
        has = self.player.has_collected_treasure(2)
        self.assertEqual(has, False)

        #Grab 2 treasures
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)

        has = self.player.has_collected_treasure(0)
        self.assertEqual(has, False)
        has = self.player.has_collected_treasure(1)
        self.assertEqual(has, True)
        has = self.player.has_collected_treasure(2)
        self.assertEqual(has, True)

    def test_player_get_collected_list(self):
        #Grab 2 treasures
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.EAST)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        self.eng.move_player(Direction.SOUTH)
        
        list = self.player.get_collected_treasures()
        self.assertEqual(len(list), 2)
        self.assertEqual(list[0]["id"], 2)
        self.assertEqual(list[1]["id"], 1)
        self.assertEqual(list[0]["name"], None)
        self.assertEqual(list[1]["name"], None)
        self.assertEqual(list[0]["starting_room_id"], 0)
        self.assertEqual(list[1]["starting_room_id"], 0)
        self.assertEqual(list[0]["collected"], True)
        self.assertEqual(list[1]["collected"], True)

if __name__ == "__main__":
    unittest.main()
