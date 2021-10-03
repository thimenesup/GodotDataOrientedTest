#include "ProjectileManager.hpp"

using namespace godot;

void ProjectileManager::_register_methods() {

	register_property<ProjectileManager, Ref<MultiMesh>>("projectile_multimesh", &ProjectileManager::projectileMultiMesh, nullptr);
	register_property<ProjectileManager, float>("projectile_speed", &ProjectileManager::projectileSpeed, 10.0);
	register_property<ProjectileManager, Ref<MultiMesh>>("enemy_multimesh", &ProjectileManager::enemyMultiMesh, nullptr);

	register_method("_init", &ProjectileManager::_init);
	
	register_method("_process", &ProjectileManager::_process);

	register_method("create_projectiles_shot", &ProjectileManager::create_projectiles_shot);
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


void ProjectileManager::_process(float delta) {

	float* projectileTransformWrite = projectileTransformData.write().ptr();

	const Vector3 motion = Vector3(0.0, 0.0, -1.0) * projectileSpeed * delta;

	//#pragma omp parallel for 
	for (int32_t i = 0; i < projectiles.size(); ++i) {
		Projectile& projectile = projectiles[i];
		projectile.transform = projectile.transform.translated(motion);
		write_transform(projectileTransformWrite, projectile.transform, i);

		for (int32_t e = 0; e < enemies.size(); ++e) {
			Enemy& enemy = enemies[e];
			if (intersects_circle(projectile.transform.origin, enemy.transform.origin, 1.0)) { //TODO: Destroy enemy in a thread and iteratable safe way
				destroy_enemy(e);
			}
		}
	}

	for (int32_t i = 0; i < enemies.size(); ++i) {
		Enemy& enemy = enemies[i];
		write_transform(enemyTransformData.write().ptr(), enemy.transform, i);
	}

	if (projectiles.size() > 0)
		projectileMultiMesh->set_as_bulk_array(projectileTransformData);

	if (enemies.size() > 0)
		enemyMultiMesh->set_as_bulk_array(enemyTransformData);
}


void ProjectileManager::create_projectiles_shot(uint32_t projectileSpread, const Transform& transform) {

	const int32_t max = projectileSpread / 2;
	const int32_t min = -max;
	const int32_t totalAmount = projectileSpread * projectileSpread;

	Vector3 rotation = transform.basis.get_euler();

	size_t index = projectiles.size();
	
	projectiles.resize(projectiles.size() + totalAmount);
	if (projectiles.capacity() > projectileMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		projectileMultiMesh->set_instance_count(projectiles.capacity());
		projectileTransformData.resize(projectiles.capacity() * FloatsInTransform);
	}

	for (int32_t x = min; x < max; ++x) {

		rotation.z = fmod((rotation.z + 3 * x), 360); //TODO FIX: Rotation axis are wrong

		for (int32_t y = min; y < max; ++y) {
			rotation.y = fmod((rotation.y + 3 * y), 360);

			Transform spreadTransform = transform;
			spreadTransform.basis = Basis(rotation);

			projectiles[index].transform = spreadTransform;

			//write_transform(projectileTransformData.write().ptr(), transform, index); //NOTE: &1 For some reason, if we don't immediately write the transform data and wait for _process(), there will be a frame where the new entities have an unitialized transform! 

			index++;
		}
	}

	//projectileMultiMesh->set_as_bulk_array(projectileTransformData); //&1

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}

void ProjectileManager::create_projectiles(uint32_t count, const Transform& transform) {

	const size_t begin = projectiles.size() - 1;
	
	projectiles.resize(projectiles.size() + count);
	if (projectiles.capacity() > projectileMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		projectileMultiMesh->set_instance_count(projectiles.capacity());
		projectileTransformData.resize(projectiles.capacity() * FloatsInTransform);
	}

	for (size_t i = begin; i < projectiles.size(); ++i) {
		Projectile& projectile = projectiles[i];
		projectile.transform = transform;

		//write_transform(projectileTransformData.write().ptr(), transform, i); //&1
	}

	//projectileMultiMesh->set_as_bulk_array(projectileTransformData); //&1

	projectileMultiMesh->set_visible_instance_count(projectiles.size());
}

void ProjectileManager::destroy_projectile(uint32_t id) {

	projectileMultiMesh->set_visible_instance_count(projectiles.size() - 1);

	projectiles.erase(projectiles.begin() + id);
}

void ProjectileManager::destroy_projectiles(uint32_t begin, uint32_t end) {

	projectileMultiMesh->set_visible_instance_count(projectiles.size() - (end - begin));

	projectiles.erase(projectiles.begin() + begin, projectiles.begin() + end);
}

uint32_t ProjectileManager::get_projectile_count() const {

	return projectiles.size();
}


void ProjectileManager::create_enemies(uint32_t count, const Transform& transform) {

	const size_t begin = enemies.size() - 1;

	enemies.resize(enemies.size() + count);
	if (enemies.capacity() > enemyMultiMesh->get_instance_count()) { //Avoid re/allocations as much as possible by reserving capacity
		enemyMultiMesh->set_instance_count(enemies.capacity());
		enemyTransformData.resize(enemies.capacity() * FloatsInTransform);
	}

	for (size_t i = begin; i < enemies.size(); ++i) {
		Enemy& enemy = enemies[i];
		enemy.transform = transform;

		//write_transform(enemyTransformData.write().ptr(), transform, enemies.size() - 1); //&1
	}

	//enemyMultiMesh->set_as_bulk_array(enemyTransformData); //&1

	enemyMultiMesh->set_visible_instance_count(enemies.size());
}

void ProjectileManager::destroy_enemy(uint32_t id) {

	enemyMultiMesh->set_visible_instance_count(enemies.size() - 1);

	enemies.erase(enemies.begin() + id);
}

void ProjectileManager::destroy_enemies(uint32_t begin, uint32_t end) {

	enemyMultiMesh->set_visible_instance_count(enemies.size() - (end - begin));

	enemies.erase(enemies.begin() + begin, enemies.begin() + end);
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
