#GameUI should query the GameEngine for state and call rendering functions only.

class GameUI:
    def __init__(self, config_path: str, profile_path: str):
        self._config = config_path
        self._profile = profile_path
        
        print("UI Time: " + self._profile + ", " + self._config)
