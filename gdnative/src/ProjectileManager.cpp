#include "ProjectileManager.hpp"

#include <atomic>

using namespace godot;

void ProjectileManager::_register_methods() {

	register_property<ProjectileManager, Ref<MultiMesh>>("projectile_multimesh", &ProjectileManager::projectileMultiMesh, nullptr);
	register_property<ProjectileManager, float>("projectile_speed", &ProjectileManager::projectileSpeed, 10.0);
	register_property<ProjectileManager, float>("projectile_lifetime", &ProjectileManager::projectileLifeTime, 1.0);
	register_property<ProjectileManager, Ref<MultiMesh>>("enemy_multimesh", &ProjectileManager::enemyMultiMesh, nullptr);

	register_method("_init", &ProjectileManager::_init);
	
	register_method("_process", &ProjectileManager::_process);

	register_method("create_projectiles_spreadshot", &ProjectileManager::create_projectiles_spreadshot);
	register_method("create_projectiles", &ProjectileManager::create_projectiles);
	register_method("destroy_projectile", &ProjectileManager::destroy_projectile);
	register_method("destroy_projectiles", &ProjectileManager::destroy_projectiles);
	register_method("get_projectile_count", &ProjectileManager::get_projectile_count);

	register_method("create_enemies", &ProjectileManager::create_enemies);
	register_method("destroy_enemy", &ProjectileManager::destroy_enemy);
	register_method("destroy_enemies", &ProjectileManager::destroy_enemies);
	register_method("get_enemy_count", &ProjectileManager::get_enemy_count);
}

void ProjectileManager::_init() {

}


void ProjectileManager::_process(float delta) { //Make sure it executes after everything else! Or there will be some displayed mesh instances with uninitialized ("glitched") mesh transforms for a single frame

	projectiles.remove_batch_if([this](int32_t i) {
		const LifeTime& lifeTime = projectiles.get_component<LifeTime>(i);
		return lifeTime.value <= 0.0;
	});
	projectileMultiMesh->set_visible_instance_count(projectiles.size());

	enemies.remove_batch_if([this](int32_t i) {
		const Health& health = enemies.get_component<Health>(i);
		return health.value <= 0;
	});
	enemyMultiMesh->set_visible_instance_count(enemies.size());

	const Vector3 motion = Vector3(0.0, 0.0, -1.0) * projectileSpeed * delta;

	float* projectileTransformDataPtr = projectileTransformData.write().ptr();

	#pragma omp parallel for 
	for (int32_t i = 0; i < projectiles.size(); ++i) {
		Transform& projectileTransform = projectiles.get_component<Transform>(i);
		projectileTransform = projectileTransform.translated(motion);
		write_transform(projectileTransformDataPtr, projectileTransform, i); //GODOT BOTTLENECK: Paying an innecessary copy because we must use a GodotArray to be able to interface with the engine

		LifeTime& lifeTime = projectiles.get_component<LifeTime>(i);
		lifeTime.value -= delta;

		for (int32_t e = 0; e < enemies.size(); ++e) {
			const Transform& enemyTransform = enemies.get_component<Transform>(e);
			if (intersects_circle(projectileTransform.origin, enemyTransform.origin, 1.0)) {
				Health& health = enemies.get_component<Health>(e);
				auto& enemyHealth = reinterpret_cast<std::atomic<decltype(health.value)>&>(health.value); //Should work properly, if it doesnt, replace it by compiler-specific intrisics
				enemyHealth.fetch_sub(1, std::memory_order_relaxed);
			}
		}
	}

	float* enemyTransformDataPtr = enemyTransformData.write().ptr();

	#pragma omp parallel for
	for (int32_t i = 0; i < enemies.size(); ++i) {
		const Transform& transform = enemies.get_component<Transform>(i);
		write_transform(enemyTransformDataPtr, transform, i);
	}

	if (projectiles.size() > 0)
		projectileMultiMesh->set_as_bulk_array(projectileTransformData); //GODOT BOTTLENECK: This will do a copy of the data, and also recalculate the AABB, we dont need that https://github.com/godotengine/godot/blob/3.2/drivers/gles3/rasterizer_storage_gles3.cpp#L5035

	if (enemies.size() > 0)
		enemyMultiMesh->set_as_bulk_array(enemyTransformData);
}


void ProjectileManager::create_projectiles_spreadshot(uint32_t projectileSpread, const Transform& transform, float angle) {

	const float deg2rad = 3.14159265358979323846 / 180;
	const float centerAlignment = (angle / 2) * deg2rad;

	const uint32_t totalAmount = projectileSpread * projectileSpread;
	size_t index = projectiles.size();

	projectiles.resize(projectiles.size() + totalAmount);
	if (projectiles.capacity() > projectileMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		projectileMultiMesh->set_instance_count(projectiles.capacity());
		projectileTransformData.resize(projectiles.capacity() * FloatsInTransform);
	}

	for (size_t x = 0; x < projectileSpread; ++x) {
		for (size_t y = 0; y < projectileSpread; ++y) {
			const float xAngle = (x * (angle / projectileSpread)) * deg2rad;
			const float yAngle = (y * (angle / projectileSpread)) * deg2rad;

			Transform spreadTransform = transform;
			spreadTransform.basis = spreadTransform.basis.rotated(Vector3(0, 1, 0), xAngle - centerAlignment);
			spreadTransform.basis = spreadTransform.basis.rotated(Vector3(1, 0, 0), yAngle - centerAlignment);

			Transform& projectileTransform = projectiles.get_component<Transform>(index);
			projectileTransform = spreadTransform;

			LifeTime& lifeTime = projectiles.get_component<LifeTime>(index);
			lifeTime.value = projectileLifeTime;

			index++;
		}
	}

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}


void ProjectileManager::create_projectiles(uint32_t count, const Transform& transform) {

	const size_t begin = projectiles.size();
	
	projectiles.resize(projectiles.size() + count);
	if (projectiles.capacity() > projectileMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		projectileMultiMesh->set_instance_count(projectiles.capacity());
		projectileTransformData.resize(projectiles.capacity() * FloatsInTransform);
	}

	for (size_t i = begin; i < projectiles.size(); ++i) {
		Transform& projectileTransform = projectiles.get_component<Transform>(i);
		projectileTransform = transform;

		LifeTime& lifeTime = projectiles.get_component<LifeTime>(i);
		lifeTime.value = projectileLifeTime;
	}

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}

void ProjectileManager::destroy_projectile(uint32_t id) {

	projectiles.erase(id);

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}

void ProjectileManager::destroy_projectiles(uint32_t begin, uint32_t end) {

	projectiles.erase(begin, end);

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}

uint32_t ProjectileManager::get_projectile_count() const {

	return projectiles.size();
}


void ProjectileManager::create_enemies(uint32_t count, const Transform& transform) {

	const size_t begin = enemies.size();

	enemies.resize(enemies.size() + count);
	if (enemies.capacity() > enemyMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		enemyMultiMesh->set_instance_count(enemies.capacity());
		enemyTransformData.resize(enemies.capacity() * FloatsInTransform);
	}

	for (size_t i = begin; i < enemies.size(); ++i) {
		Transform& enemyTransform = enemies.get_component<Transform>(i);
		enemyTransform = transform;

		Health& health = enemies.get_component<Health>(i);
		health.value = 1;
	}

	enemyMultiMesh->set_visible_instance_count(enemies.size());
}

void ProjectileManager::destroy_enemy(uint32_t id) {

	enemies.erase(id);

	enemyMultiMesh->set_visible_instance_count(enemies.size());
}

void ProjectileManager::destroy_enemies(uint32_t begin, uint32_t end) {

	enemies.erase(begin, end);

	enemyMultiMesh->set_visible_instance_count(enemies.size());
}

uint32_t ProjectileManager::get_enemy_count() const {

	return enemies.size();
}


void ProjectileManager::write_transform(float* transformWrite, const Transform& transform, int32_t i) {

	const int32_t o = i * FloatsInTransform;
	transformWrite[o    ] = transform.basis.elements[0][0];
	transformWrite[o + 1] = transform.basis.elements[0][1];
	transformWrite[o + 2] = transform.basis.elements[0][2];
	transformWrite[o + 3] = transform.origin.x;
	transformWrite[o + 4] = transform.basis.elements[1][0];
	transformWrite[o + 5] = transform.basis.elements[1][1];
	transformWrite[o + 6] = transform.basis.elements[1][2];
	transformWrite[o + 7] = transform.origin.y;
	transformWrite[o + 8] = transform.basis.elements[2][0];
	transformWrite[o + 9] = transform.basis.elements[2][1];
	transformWrite[o + 10] = transform.basis.elements[2][2];
	transformWrite[o + 11] = transform.origin.z;
}

bool ProjectileManager::intersects_circle(const Vector3& a, const Vector3& b, float squaredRadius) {

	Vector3 d = a - b;
	float squaredDistance = d.x * d.x + d.z * d.z;
	return squaredDistance <= squaredRadius;
}
