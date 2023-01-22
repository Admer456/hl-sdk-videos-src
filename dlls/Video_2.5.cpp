
#include "extdll.h"
#include "util.h"
#include "cbase.h"

// ==============================================
// ambient_simple
// ==============================================
class CAmbientSimple : public CBaseEntity
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(ambient_simple, CAmbientSimple);

void CAmbientSimple::Spawn()
{
	if (!pev->targetname)
	{
		ALERT(at_warning, "Unnamed ambient_simple, deleting\n");
		UTIL_Remove(this);
		return;
	}

	if (!pev->noise)
	{
		ALERT(at_warning, "ambient_simple '%s' has no sound, deleting\n",
			STRING(pev->targetname));

		UTIL_Remove(this);
		return;
	}

	// Le sound must be precached before use
	PRECACHE_SOUND(STRING(pev->noise));
}

void CAmbientSimple::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EMIT_SOUND(edict(), CHAN_STATIC, STRING(pev->noise), 1.0f, ATTN_NORM);
}

// ==============================================
// trigger_timer
// ==============================================
class CTriggerTimer : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

protected:
	float delayTime = 0.0f;
	bool enabled = false;
};

LINK_ENTITY_TO_CLASS(trigger_timer, CTriggerTimer);

TYPEDESCRIPTION CTriggerTimer::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerTimer, delayTime, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerTimer, enabled, FIELD_BOOLEAN)
};

IMPLEMENT_SAVERESTORE(CTriggerTimer, CBaseEntity);

void CTriggerTimer::Spawn()
{
	// Do nuffin
	// Maybe validation or something, like, delayTime can't be negative
}

bool CTriggerTimer::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "delayTime"))
	{
		delayTime = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CTriggerTimer::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Previously, we had a BOOL here, so all we could do is
	// enabled = enabled == TRUE ? FALSE : TRUE;
	// But now that bools can be save-restored, it's no longer necessary
	enabled = !enabled;
	// Start thinking
	pev->nextthink = gpGlobals->time + 0.01f;
}

void CTriggerTimer::Think()
{
	if (enabled)
	{
		SUB_UseTargets(this, USE_ON, 0.0f);
	}

	// Homework: add the option of random timing
	pev->nextthink = gpGlobals->time + delayTime;
}
