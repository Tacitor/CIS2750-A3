.PHONY: rig rib ric pl pycov pyhtml play

rig:
	python3 run_integration.py --config "/workspace/assets/integration.ini" --log "/workspace/assets/integration_run.log"

rib:
	python3 run_integration.py --config "/workspace/assets/blocked_start.ini" --log "/workspace/assets/integration_run.log"

ric:
	python3 run_integration.py --config "/workspace/assets/sweep_cycle.ini" --log "/workspace/assets/sweep_cycle.log"

# Run Pylint local
pl: 
	pylint --rcfile .pylintrc treasure_runner

# Run the unit tests local
pycov:
	python3 -m coverage run -m unittest discover

# Check to see how much code the unit tests cover
pyhtml:
	python3 -m coverage html -d coverage_html

play:
	python3 run_game.py --config /workspace/assets/starter.ini --profile /workspace/assets/test.json
