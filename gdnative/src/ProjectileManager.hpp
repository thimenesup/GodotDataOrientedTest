#pragma once

#include <Godot.hpp>
#include <Node.hpp>
#include <MultiMesh.hpp>

#include "EntityArchetype.hpp"

namespace godot {

struct LifeTime {
	float value;
};

struct Health {
	int32_t value;
};

class ProjectileManager : public Node {

	GODOT_CLASS(ProjectileManager, Node)

public:

	static const size_t FloatsInTransform = sizeof(Transform) / sizeof(float);

	Ref<MultiMesh> projectileMultiMesh;
	Ref<MultiMesh> enemyMultiMesh;

	float projectileSpeed;
	float projectileLifeTime;

	static void _register_methods();

	void _init();

	void _process(float delta);

	void create_projectiles_spreadshot(uint32_t projectileSpread, const Transform& transform, float angle);
	void create_projectiles(uint32_t count, const Transform& transform);
	void destroy_projectile(uint32_t id); //WARNING: Input must be valid
	void destroy_projectiles(uint32_t begin, uint32_t end); //WARNING: Input must be valid
	inline uint32_t get_projectile_count() const;

	void create_enemies(uint32_t count, const Transform& transform);
	void destroy_enemy(uint32_t id); //WARNING: Input must be valid
	void destroy_enemies(uint32_t begin, uint32_t end); //WARNING: Input must be valid
	inline uint32_t get_enemy_count() const;

	inline static void write_transform(float* transformWrite, const Transform& transform, int32_t o);
	inline static bool intersects_circle(const Vector3& a, const Vector3& b, float squaredDistance);

private:

	EntityArchetype<Transform, LifeTime> projectiles;

	PoolRealArray projectileTransformData;

	EntityArchetype<Transform, Health> enemies;

	PoolRealArray enemyTransformData;

};

}
