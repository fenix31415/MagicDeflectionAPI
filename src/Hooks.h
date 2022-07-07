#pragma once
#include "Subscribers.h"

class MagicDeflectionHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_sub_1407528E0 = trmpl.write_call<5>(REL::ID(43014).address() + 0x1a3, sub_1407528E0);  // SkyrimSE.exe+7527C3

		{
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func_addr)
				{
					// last arg = point
					mov(rbp, ptr[rsp + (0x8 + 0xA8 + 0x18)]);
					mov(ptr[rsp + (0x8 + 0x0A8 - 0x88)], rbp);
					xor_(rbp, rbp);

					// 3rd arg = proj
					mov(r8, rdi);

					mov(rax, func_addr);
					jmp(rax);
				}
			} xbyakCode{ uintptr_t(on_ward_hit) };

			_on_ward_hit = FenixUtils::add_trampoline<5, 43014, 0x112, true>(&xbyakCode);  // SkyrimSE.exe+752732
		}
	}

private:
	static void sub_1407528E0(RE::Projectile* proj, RE::TESObjectREFR* _victim, RE::NiPoint3* P, float a4, float a5)
	{
		if (_victim) {
			if (auto victim = _victim->As<RE::Actor>()) {
				if (int ind = Subscribers::get_deflect(victim, proj); ind >= 0) {
					Subscribers::on_deflect(ind, proj, victim, P);
					proj->explosion = nullptr;
					return;
				}
			}
		}

		_sub_1407528E0(proj, _victim, P, a4, a5);
	}

	static uint32_t on_ward_hit(RE::Character* target, RE::Character* shooter, RE::Projectile* proj, float* damaged, RE::NiPoint3* P)
	{
		int ind;
		if (ind = Subscribers::get_deflect_ward(target, proj), ind >= 0) {
			Subscribers::on_deflect_ward(ind, proj, target, P);
			return 1;
		} else {
			return _on_ward_hit(target, shooter, proj->spell, damaged, nullptr);
		}
	}

	static inline REL::Relocation<decltype(sub_1407528E0)> _sub_1407528E0;
	static inline REL::Relocation<uint32_t(RE::Character* target, RE::Character* shooter, RE::MagicItem* spel, float* damaged, RE::Effect* eff)> _on_ward_hit;
};

class PlayerCharacterHook
{
public:
	static void Hook()
	{
#ifdef DEBUG
		_Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0])).write_vfunc(0xad, Update);
#endif
	}

private:
	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);

#ifdef DEBUG
		DebugAPI_IMPL::DebugAPI::Update();
#endif
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};
