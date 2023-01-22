#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"

class FuncExploder : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	string_t explodeTarget;
};

LINK_ENTITY_TO_CLASS(func_exploder, FuncExploder);


TYPEDESCRIPTION FuncExploder::m_SaveData[] =
{
	DEFINE_FIELD(FuncExploder, explodeTarget, FIELD_STRING)
};

IMPLEMENT_SAVERESTORE(FuncExploder, CBaseEntity);

void FuncExploder::Spawn()
{
}

bool FuncExploder::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "explode_target"))
	{
		explodeTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void FuncExploder::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* ent = nullptr;
	while (ent = UTIL_FindEntityByTargetname(ent, STRING(explodeTarget)))
	{
		ExplosionCreate(ent->Center(), ent->pev->angles, edict(), 200, true);
		ent->pev->effects |= EF_NODRAW;
		ent->pev->solid = SOLID_NOT;
	}
}
