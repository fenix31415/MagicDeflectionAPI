#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include "MagicDeflectionAPI.h"

using namespace MagicDeflectionAPI;

namespace Subscribers
{
	void init();

	int get_deflect(RE::Actor* target, RE::Projectile* proj);
	int get_deflect_ward(RE::Actor* target, RE::Projectile* proj);

	void on_deflect(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P);
	void on_deflect_ward(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P);
}
