extends Label

onready var _projectile_manager := get_parent().get_node("ProjectileManager")

func _process(delta: float) -> void:
	text = "FPS:%s\nProjectiles:%s\nEnemies:%s" % [
		Engine.get_frames_per_second(), 
		_projectile_manager.get_projectile_count(),
		_projectile_manager.get_enemy_count()
		]
