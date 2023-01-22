
#include "extdll.h"
#include "util.h"
#include "cbase.h"

// ==============================================
// particle_object
// ==============================================
class CParticleObject : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void Think() override;
};

LINK_ENTITY_TO_CLASS(particle_object, CParticleObject);

void CParticleObject::Spawn()
{
	Precache();
	SET_MODEL(edict(), "sprites/cexplo.spr");
	// Set render mode to Additive, else there will be a visible black outline
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 128.0f; // DO NOT forget to set render amount
	// Let it bounce around
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->friction = 0.5f;
	// Let the particle have a 0.5^3 unit bounding box for collision
	UTIL_SetSize(pev, Vector(-0.25f, -0.25f, -0.25f), Vector(0.25f, 0.25f, 0.25f));
	pev->scale = 0.33f;				 // Start at 33% scale
	pev->frame = RANDOM_LONG(0, 24); // Randomise between frame 0 and 24 (25 total for this sprite)

	// Think right now
	pev->nextthink = gpGlobals->time + 0.016f;
}

void CParticleObject::Precache()
{
	PRECACHE_MODEL("sprites/cexplo.spr");
}

void CParticleObject::Think()
{
	// Decrease opacity until it's zero, and then
	// delete the entity to not waste edicts
	pev->renderamt -= 2.5f;
	if (pev->renderamt <= 0.0f)
	{
		UTIL_Remove(this);
	}

	// Also increase the scale for a cool visual effect
	pev->scale += 0.05f;

	pev->nextthink = gpGlobals->time + 0.016f;
}

// ==============================================
// example_attachment
// ==============================================
class CAttachmentExample : public CBaseAnimating
{
public:
	void Spawn() override;
	void Precache() override;

	void Think() override;
};

LINK_ENTITY_TO_CLASS(example_attachment, CAttachmentExample);

void CAttachmentExample::Spawn()
{
	Precache();

	SET_MODEL(edict(), "models/barney.mdl");
	UTIL_SetOrigin(pev, pev->origin);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	// Play 'reload'
	pev->sequence = LookupSequence("reload");
	// Initialise animation stuff
	InitBoneControllers();
	ResetSequenceInfo();
	m_fSequenceLoops = true; // force looping
	// Turn on the gun bodygroup
	SetBodygroup(1, 1);
	// Kick off and start Think'ing
	pev->nextthink = gpGlobals->time + 2.5f;
}

void CAttachmentExample::Precache()
{
	PRECACHE_MODEL("models/barney.mdl");
	// This precaches CParticleObject
	// Note that you put the map classname of the entity,
	// not the C++ class name
	UTIL_PrecacheOther("particle_object");
}

void CAttachmentExample::Think()
{
	StudioFrameAdvance();

	// Slowly rotate over time
	pev->angles.y += 45.0f * gpGlobals->frametime;

	// Spew out particles from the gun attachment
	Vector gunOrigin, gunAngles, gunForwardDirection;
	GetAttachment(0, gunOrigin, gunAngles);
	// Attachments don't seem to actually store angles, Valve jank
	gunAngles = pev->angles;
	UTIL_MakeVectorsPrivate(gunAngles, gunForwardDirection, nullptr, nullptr);

	// Randomise the direction a bit
	gunForwardDirection.x += RANDOM_FLOAT(-0.1f, 0.1f);
	gunForwardDirection.y += RANDOM_FLOAT(-0.1f, 0.1f);
	gunForwardDirection.z += RANDOM_FLOAT(-0.1f, 0.1f);

	// Yes, this is how you spawn an entity... unfortunately
	CParticleObject* particle = GetClassPtr<CParticleObject>(nullptr);
	// Before we call particle->Spawn(), set up the position and velocity
	particle->pev->origin = gunOrigin;
	particle->pev->velocity = gunForwardDirection.Normalize() * 800.0f;
	// Solidity, movetype etc. are handled in Spawn :)
	particle->Spawn();

	pev->nextthink = gpGlobals->time + 0.016f;
}

// ==============================================
// example_controller
// ==============================================
class CControllerExample : public CBaseAnimating
{
public:
	void Spawn() override;
	void Precache() override;

	void Think() override;

private:
	float sign = 1.0f;
	float torso = 0.0f;
};

LINK_ENTITY_TO_CLASS(example_controller, CControllerExample);

void CControllerExample::Spawn()
{
	Precache();

	SET_MODEL(edict(), "models/player.mdl");

	// Set up solidity, movetype etc.
	UTIL_SetOrigin(pev, pev->origin);
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	// Initialise animation stuff
	pev->sequence = LookupSequence("idle");
	InitBoneControllers();
	ResetSequenceInfo();

	pev->nextthink = gpGlobals->time + 1.0f;
}

void CControllerExample::Precache()
{
	PRECACHE_MODEL("models/player.mdl");
}

void CControllerExample::Think()
{
	// Torso goes from +30 to -30 according to the model viewer
	torso += gpGlobals->frametime * 15.0f * sign;
	if (torso <= -30.0f)
	{
		sign = 1.0f;
	}
	else if (torso >= 30.0f)
	{
		sign = -1.0f;
	}
	// Set it for all controllers
	for (int i = 0; i < 5; i++)
	{
		SetBoneController(i, torso);
	}

	pev->nextthink = gpGlobals->time + 0.001f;
}

// ==============================================
// example_hitbox
// ==============================================
class CHitboxExample : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void TraceAttack(entvars_t* pevAttacker, float flDamage,
		Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
};

LINK_ENTITY_TO_CLASS(example_hitbox, CHitboxExample);

void CHitboxExample::Spawn()
{
	Precache();
	SET_MODEL(edict(), "models/hgrunt.mdl");

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_BBOX;
	// VERY IMPORTANT
	pev->takedamage = DAMAGE_YES;
	// ALSO IMPORTANT
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
}

void CHitboxExample::Precache()
{
	PRECACHE_MODEL("models/hgrunt.mdl");
}

void CHitboxExample::TraceAttack(entvars_t* pevAttacker, float flDamage,
	Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	switch (ptr->iHitgroup)
	{
	case 0: ALERT(at_console, "Hitgroup: generic\n"); break;
	case 1: ALERT(at_console, "Hitgroup: head\n"); break;
	case 2: ALERT(at_console, "Hitgroup: chest\n"); break;
	case 3: ALERT(at_console, "Hitgroup: stomach\n"); break;
	case 4: ALERT(at_console, "Hitgroup: left arm\n"); break;
	case 5: ALERT(at_console, "Hitgroup: right arm\n"); break;
	case 6: ALERT(at_console, "Hitgroup: left leg\n"); break;
	case 7: ALERT(at_console, "Hitgroup: right leg\n"); break;
	default: ALERT(at_console, "Hitgroup: unknown\n"); break;
	}
}

// ==============================================
// example_animevent
// ==============================================
class CAnimEventExample : public CBaseAnimating
{
public:
	void Spawn() override;
	void Precache() override;

	void Shoot();

	void Think() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
};

LINK_ENTITY_TO_CLASS(example_animevent, CAnimEventExample);

void CAnimEventExample::Spawn()
{
	Precache();
	SET_MODEL(edict(), "models/barney.mdl");

	UTIL_SetOrigin(pev, pev->origin);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	// Play 'shootgun'
	pev->sequence = LookupSequence("shootgun");
	// Initialise animation stuff
	InitBoneControllers();
	ResetSequenceInfo();
	// Turn on the gun bodygroup
	SetBodygroup(1, 1);
	// Kick off and start Think'ing
	pev->nextthink = gpGlobals->time + 1.5f;
}

void CAnimEventExample::Precache()
{
	PRECACHE_MODEL("models/barney.mdl");
}

void CAnimEventExample::Shoot()
{
	// Spew out particles from the gun attachment
	Vector gunOrigin, gunAngles, gunForwardDirection;
	GetAttachment(0, gunOrigin, gunAngles);
	UTIL_MakeVectorsPrivate(pev->angles, gunForwardDirection, nullptr, nullptr);

	// Randomise the direction a bit
	gunForwardDirection.x += RANDOM_FLOAT(-0.1f, 0.1f);
	gunForwardDirection.y += RANDOM_FLOAT(-0.1f, 0.1f);
	gunForwardDirection.z += RANDOM_FLOAT(-0.1f, 0.1f);

	// Yes, this is how you spawn an entity... unfortunately
	CParticleObject* particle = GetClassPtr<CParticleObject>(nullptr);
	// Before we call particle->Spawn(), set up the position and velocity
	particle->pev->origin = gunOrigin;
	particle->pev->velocity = gunForwardDirection.Normalize() * 800.0f;
	// Solidity, movetype etc. are handled in Spawn :)
	particle->Spawn();
}

void CAnimEventExample::Think()
{
	pev->angles.y += 15.0f * gpGlobals->frametime;

	// This will automatically figure out WHEN to call HandleAnimEvent,
	// by looking through all the anim events of the current sequence,
	// and then it compares the current frame to the event's frame
	// If the frame is right, then it handles that event
	DispatchAnimEvents();
	// Play animation as normal
	StudioFrameAdvance();
	// Manually loop the animation because 'shootgun' does not have
	// the STUDIO_LOOPING flag
	if (m_fSequenceFinished)
	{
		pev->frame = 0.0f;
	}

	pev->nextthink = gpGlobals->time + 0.1f;
}

constexpr int ANIMEVENT_SHOOT = 3;

void CAnimEventExample::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	// You can insert more stuff here as you wish.
	// In case you're implementing a custom anim event, the ID is generally entirely
	// up to you, just make sure it's unique. There are already a lot of existing
	// reserved animation IDs, which you can see on the303.org:
	//
	// http://the303.org/tutorials/gold_qc.htm#A1

	switch (pEvent->event)
	{
	case ANIMEVENT_SHOOT:
		return Shoot();

	default:
		return CBaseAnimating::HandleAnimEvent(pEvent);
	}
}
