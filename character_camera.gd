extends Camera

export var _target_path := NodePath()
var _target: Spatial = null

export var move_speed := 5.0

var _offset := Vector3.ZERO

func _ready() -> void:
	_target = get_node(_target_path)
	_offset = translation

func _process(delta: float) -> void:
	if not _target:
		return
	
	translation = translation.linear_interpolate(_target.translation + _offset, move_speed * delta)
