extends Node

## Smoke test for the binding: reports whether the native runtime was found and,
## if a compiled byProd project is present, loads it and plays one event.
##
## Runs headless too (`godot --headless --quit-after 2`), which is what CI uses to
## prove the extension at least registers its classes without the SDK installed.

## A project compiled by the byProd editor. Left empty in the repository — the
## binary is yours, not something this repo can ship.
@export_file("*.bpb") var project_path: String = ""

## Event to play once at startup, if the project above is set.
@export var event_path: String = "events/test"

var _manager: ByProdSoundManager
var _instance: ByProdEventInstance


func _ready() -> void:
	print("byProd runtime available: %s" % ByProdSoundManager.is_runtime_available())

	if not ByProdSoundManager.is_runtime_available():
		print("byProd runtime not loaded: %s" % ByProdSoundManager.get_last_error())
		print("Drop byProd.dll / libbyProd.so into addons/byprod/bin/ to go further.")
		return

	print("byProd runtime version: %d" % ByProdSoundManager.get_runtime_version())

	_manager = ByProdSoundManager.create()
	if _manager == null:
		push_error(ByProdSoundManager.get_last_error())
		return

	if project_path.is_empty():
		print("No compiled project assigned — nothing to play.")
		return

	if not _manager.load_project(FileAccess.get_file_as_bytes(project_path)):
		push_error("byProd refused the project at %s" % project_path)
		return

	var description := _manager.get_event_description(event_path)
	if description == null:
		push_error("No such event: %s" % event_path)
		return

	_instance = description.create_instance()
	_instance.start()


func _process(_delta: float) -> void:
	if _manager != null:
		_manager.update()
