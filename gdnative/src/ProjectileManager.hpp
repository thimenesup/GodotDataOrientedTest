#pragma once

#include <Godot.hpp>
#include <Node.hpp>
#include <MultiMesh.hpp>

#include <vector>

namespace godot {

struct Projectile {
	Transform transform;
};

struct Enemy {
	Transform transform;
};

class ProjectileManager : public Node {

	GODOT_CLASS(ProjectileManager, Node)

public:

	static const size_t FloatsInTransform = sizeof(Transform) / sizeof(float);

	Ref<MultiMesh> projectileMultiMesh;
	Ref<MultiMesh> enemyMultiMesh;

	float projectileSpeed;

	static void _register_methods();

	void _init();

	void _process(float delta);

	void create_projectiles_shot(uint32_t projectileSpread, const Transform& transform);
	void create_projectiles(uint32_t count, const Transform& transform);
	void destroy_projectile(uint32_t id); //WARNING: Input must be valid
	void destroy_projectiles(uint32_t begin, uint32_t end); //WARNING: Input must be valid
	inline uint32_t get_projectile_count() const;

	void create_enemies(uint32_t count, const Transform& transform);
	void destroy_enemy(uint32_t id); //WARNING: Input must be valid
	void destroy_enemies(uint32_t begin, uint32_t end);
	inline uint32_t get_enemy_count() const;

	inline static void write_transform(float* transformWrite, const Transform& transform, int32_t o);
	inline static bool intersects_circle(const Vector3& a, const Vector3& b, float squaredDistance);

private:

	std::vector<Projectile> projectiles;

	PoolRealArray projectileTransformData;

	std::vector<Enemy> enemies;

	PoolRealArray enemyTransformData;

};

}
