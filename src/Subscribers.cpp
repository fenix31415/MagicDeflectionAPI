#include "Subscribers.h"
#include "FenixProjectilesAPI.h"

static cast_t _API_cast;
static cast_CustomPos_t _API_cast_CustomPos_t;

uint32_t API_cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	return (*_API_cast)(caster, spel, start_pos, rot);
}

uint32_t API_cast_CustomPos_t(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	return (*_API_cast_CustomPos_t)(caster, spel, start_pos, rot);
}

namespace Subscribers
{
	void init()
	{
		if (auto pluginHandle = GetModuleHandleA("FenixProjectilesAPI.dll")) {
			_API_cast = (cast_t)GetProcAddress(pluginHandle, "FenixProjectilesAPI_cast");
			_API_cast_CustomPos_t = (cast_CustomPos_t)GetProcAddress(pluginHandle, "FenixProjectilesAPI_cast_CustomPos");
		}
	}

	class Impl
	{
		std::vector<std::unique_ptr<API>> data;
		mutable std::mutex subscribers_mutex;

	public:
		void add(std::unique_ptr<API> api)
		{
			const std::lock_guard<std::mutex> lock(subscribers_mutex);

			data.push_back(std::move(api));
		}

		int get_deflect(RE::Actor* target, RE::Projectile* proj) const
		{
			const std::lock_guard<std::mutex> lock(subscribers_mutex);

			if (proj->spell && proj->spell->As<RE::SpellItem>()) {
				for (int i = 0; i < data.size(); i++) {
					if (data[i]->can_deflect(target, proj))
						return i;
				}
			}
			return -1;
		}

		int get_deflect_ward(RE::Actor* target, RE::Projectile* proj) const
		{
			const std::lock_guard<std::mutex> lock(subscribers_mutex);

			if (proj->spell && proj->spell->As<RE::SpellItem>()) {
				for (int i = 0; i < data.size(); i++) {
					if (data[i]->can_deflect_ward(target, proj))
						return i;
				}
			}
			return -1;
		}

		void on_deflect(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P) const
		{
			DeflectionData ddata{ *P, {0} };
			data[ind]->on_deflect(target, proj, ddata);
			API_cast_CustomPos_t(target, proj->spell->As<RE::SpellItem>(), ddata.P, ddata.rot);
		}

		void on_deflect_ward(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P) const
		{
			DeflectionData ddata{ *P, { 0 } };
			data[ind]->on_deflect_ward(target, proj, ddata);
			API_cast_CustomPos_t(target, proj->spell->As<RE::SpellItem>(), ddata.P, ddata.rot);
		}
	} subs;

	int get_deflect(RE::Actor* target, RE::Projectile* proj)
	{
		return subs.get_deflect(target, proj);
	}

	int get_deflect_ward(RE::Actor* target, RE::Projectile* proj)
	{
		return subs.get_deflect_ward(target, proj);
	}

	void on_deflect(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P)
	{
		return subs.on_deflect(ind, proj, target, P);
	}

	void on_deflect_ward(int ind, RE::Projectile* proj, RE::Actor* target, RE::NiPoint3* P)
	{
		return subs.on_deflect_ward(ind, proj, target, P);
	}

}

extern "C" DLLEXPORT void MagicDeflectionAPI_AddSubscriber(std::unique_ptr<API> api)
{
	return Subscribers::subs.add(std::move(api));
}
