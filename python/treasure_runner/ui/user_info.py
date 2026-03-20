from datetime import datetime, timezone

class UserInfo:
    def __init__(self):
        self.name = ""
        self.games_played = 0
        self.max_treasure_collected = 0
        self.most_rooms_world_completed = 0
        self.timestamp_last_played = ""

    def to_dict(self):
        """Return a JSON-safe snapshot. The timestamp will auto update base on when this object is serialized"""
        return {
            "player_name":                  self.name,
            "games_played":                 self.games_played,
            "max_treasure_collected":       self.max_treasure_collected,
            "most_rooms_world_completed":   self.most_rooms_world_completed,
            "timestamp_last_played":        datetime.now(timezone.utc).isoformat(),
        }

    @classmethod
    def from_dict(cls, json_data):
        """Reconstruct a UserInfo object from a saved dict."""
        usr = cls()
        usr.name                        = json_data["player_name"]
        usr.games_played                = json_data["games_played"]
        usr.max_treasure_collected      = json_data["max_treasure_collected"]
        usr.most_rooms_world_completed  = json_data["most_rooms_world_completed"]
        usr.timestamp_last_played       = json_data["timestamp_last_played"]
        return usr
