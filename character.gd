extends Spatial

onready var _projectile_manager := get_parent().get_node("ProjectileManager")

onready var _barrel := $Barrel

export var move_speed := 5.0

export var _spread_shot := true
export var _spread_amount := 10 #Total amount will be n^2
export var _spread_angle := 90.0 #In euler degrees

export var _firing_time := 0.1
var _elapsed_firing_time := 0.0

var _look_plane := Plane()

func _init() -> void:
	_look_plane.normal = Vector3.UP

func _ready() -> void:
	_projectile_manager.projectile_speed = 25.0
	_projectile_manager.projectile_lifetime = 3.0

func _process(delta: float) -> void:
	var mouse = get_viewport().get_mouse_position()
	var camera = get_viewport().get_camera()
	var origin = camera.project_ray_origin(mouse)
	var direction = camera.project_ray_normal(mouse)
	
	var result = _look_plane.intersects_ray(origin, direction)
	if result:
		var look_transform = transform.looking_at(result, Vector3.UP)
		transform = look_transform
	
	
	var horizontal = Input.get_action_strength("move_right") - Input.get_action_strength("move_left")
	var vertical = Input.get_action_strength("move_back") - Input.get_action_strength("move_forward")
	var move_direction = Vector3(horizontal, 0, vertical).normalized()
	translation += move_direction * move_speed * delta
	
	
	if Input.is_mouse_button_pressed(BUTTON_LEFT):
		if _elapsed_firing_time >= _firing_time:
			if _spread_shot:
				_projectile_manager.create_projectiles_spreadshot(_spread_amount, _barrel.global_transform, _spread_angle)
			else:
				_projectile_manager.create_projectiles(1000, _barrel.global_transform)
			
			_elapsed_firing_time = 0.0
	
	if Input.is_mouse_button_pressed(BUTTON_RIGHT):
		var begin = 0
		var end = min(100, _projectile_manager.get_projectile_count())
		if end <= _projectile_manager.get_projectile_count():
			_projectile_manager.destroy_projectiles(begin, end)
	
	_elapsed_firing_time += delta
	
	
	if Input.is_key_pressed(KEY_SPACE):
		var radius = 20
		var position = Vector3(rand_range(-radius, radius), 0, rand_range(-radius, radius))
		_projectile_manager.create_enemies(1, Transform(Basis(), position))
