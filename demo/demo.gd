extends Node

## Smoke test for the binding: reports whether the native runtime was found and,
## if a compiled byProd project is present, loads it and plays one event.
##
## Runs headless too (`godot --headless --quit-after 2`), which is what CI uses to
## prove the extension at least registers its classes without the SDK installed.

## A project compiled by the byProd editor, alongside its .bybank files. Not in
## the repository — copy the SDK's samples/sample_project/build here to try it.
@export_file("*.byprod") var project_path: String = "res://demo/audio/sample_project.byprod"

## Event to play once at startup, if the project above is present.
@export var event_path: String = "event:/ui/skill_unlock"

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
		# byProd opens its own device, and there is none in a headless session.
		# Host-mixed needs no device, which is what makes this demo verifiable
		# from the command line; a real game would surface the failure instead.
		print("Device mode unavailable (%s), retrying host-mixed." % ByProdSoundManager.get_last_error())
		_manager = ByProdSoundManager.create_host_mixed(44100)

	if _manager == null:
		push_error(ByProdSoundManager.get_last_error())
		return

	if project_path.is_empty() or not FileAccess.file_exists(project_path):
		print("No compiled project at %s — nothing to play." % project_path)
		return

	# The .bybank files sit next to the project; without this the project loads
	# but carries no wave data, and byProd reports every bank as unavailable.
	_manager.set_bank_directory(project_path.get_base_dir())

	if not _manager.load_project(FileAccess.get_file_as_bytes(project_path)):
		push_error("byProd refused the project at %s" % project_path)
		return

	# Proves the bank callbacks reach the host: the project itself carries no wave
	# data, so a failure here means nothing would ever be audible.
	print("Preloaded bank \"main\": %s" % _manager.preload_bank("main"))

	var description := _manager.get_event_description(event_path)
	if description == null:
		push_error("No such event: %s" % event_path)
		return

	_instance = description.create_instance()
	_instance.start()
	print("Started %s, state now %d" % [event_path, _instance.get_state()])


func _process(_delta: float) -> void:
	if _manager != null:
		_manager.update()
