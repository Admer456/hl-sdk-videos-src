
#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CTriggerPrint : public CBaseEntity
{
public:
	// Is called when the entity spawns
	void Spawn() override;
	// Is called when the entity is triggered by some other entity
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iPrintMode = at_console;
};

LINK_ENTITY_TO_CLASS(trigger_print, CTriggerPrint);

TYPEDESCRIPTION CTriggerPrint::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerPrint, m_iPrintMode, FIELD_INTEGER)};

IMPLEMENT_SAVERESTORE(CTriggerPrint, CBaseEntity);

void CTriggerPrint::Spawn()
{
	// do nothing here
}

void CTriggerPrint::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "%s\n", STRING(pev->message));
}

bool CTriggerPrint::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "printMode"))
	{
		m_iPrintMode = atoi(pkvd->szValue);
		
		// A small change:
		// In halflife-updated, BOOL is now bool, so you use true and false
		// instead of TRUE and FALSE

		// Likewise, in KeyValue we return true instead of setting pkvd->fHandled
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}
