# Julia's Jubilant Journey - A3 CIS\*2750

## Download and Run Instructions

### Download via Git (preferred)
1. Ensure that you have git installed. On debian based Linux run the command `sudo apt install git`
2. Navigate to the folder where you would like to have a new folder called `CIS2750-A3` and run `git clone --branch v1.0 --depth 1 https://github.com/Tacitor/CIS2750-A3.git` for the 1.0 initial release of the game.

### Download via GitHut Release (NOT preferred)
1. Go to [https://github.com/Tacitor/CIS2750-A3/releases](https://github.com/Tacitor/CIS2750-A3/releases) and select the release of your choosing. The initial 1.0 is recommended.
2. Download the source code (either `zip` or `tar.gz`)
3. Upzip the source code and now you should have a `CIS2750-A3` folder

### Prerequisites
1. Need to run on Linux preferable Debian based
2. `make`
3. `gcc`
4. `python3`

On debian run `sudo apt install make gcc python3` to get all three of them

### Compile the binaries
1. Navigate to `CIS2750-A3/python/`
2. Run `make dist`
Then if there have been no errors it should be ready to go.

### Run the game
For quick testing you can run the command `make play` which will use the `../assets/test.json` player profile.

Once you are ready to play on your own profile run `python3 run_game.py --config ../assets/<CONFIG_FILE>.ini --profile ../assets/<PLAYER_PROFILE_FILE>.json`. The `<PLAYER_PROFILE_FILE>` may be an existing file, or one that doesn't exist yet. The game will handel it. There are plenty of existing configs in `../assets/`, you are also
welcome to make a copy of an existing one and customize to your heart's content.

## Additional documentation from A1
1. `loader_load_world()` will return `NULL_POINTER` if any of the output pointer are `NULL`
2. `room_is_walkable()` will return `false` if `floor_grid` is `NULL`
3. `room_render()` will return `INVALID_ARGUMENT` if `floor_grid` is `NULL`


## Additional documentation from A3
1. Made use of the curses demo provided in class.
2. Made use of the persistence demo provided in class.
