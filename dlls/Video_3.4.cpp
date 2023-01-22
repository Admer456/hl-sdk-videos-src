
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"

// g_sModelIndexFireball is located here now, no longer in explode.h
#include "weapons.h"

class CEffectTest : public CBaseEntity
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(effect_test, CEffectTest);

void CEffectTest::Spawn()
{
	// Nothing really
}

void CEffectTest::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(g_sModelIndexFireball);
	WRITE_BYTE(20); // scale * 10
	WRITE_BYTE(15); // framerate
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();
}

class CEffectTest2 : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	void TE_BeamPoints();
	void TE_Gunshot();
	void TE_Smoke();
	void TE_Tracer();
	void TE_Lightning();
	void TE_Sparks();
	void TE_BspDecal();
	void TE_Implosion();
	void TE_SpriteTrail();
	void TE_Sprite();
	void TE_BeamTorus();
	void TE_DLight();
	void TE_ELight();
	void TE_Line();
	void TE_Box();
	void TE_LargeFunnel();
	void TE_BloodStream();
	void TE_Fizz();
	void TE_Model();
	void TE_ExplodeModel();
	void TE_BreakModel();
	void TE_SpriteSpray();
	void TE_ArmorRicochet();
	void TE_Bubbles();
	void TE_BubbleTrail();
	void TE_ParticleBurst();
	void TE_FireField();

private:
	short m_BeamSprite{};
	short m_FireSprite{};
	short m_SmokeSprite{};
	short m_ShockwaveSprite{};
	short m_LightningSprite{};
	short m_GlowSprite{};
	short m_RockModel{};
	short m_SkullModel{};
};

LINK_ENTITY_TO_CLASS(effect_test2, CEffectTest2);

void CEffectTest2::Spawn()
{
	m_BeamSprite = PRECACHE_MODEL("sprites/lgtning.spr");
	m_FireSprite = PRECACHE_MODEL("sprites/fire.spr");
	m_SmokeSprite = PRECACHE_MODEL("sprites/steam1.spr");
	m_ShockwaveSprite = PRECACHE_MODEL("sprites/shockwave.spr");
	m_LightningSprite = PRECACHE_MODEL("sprites/xbeam5.spr");
	m_GlowSprite = PRECACHE_MODEL("sprites/xspark4.spr");
	m_RockModel = PRECACHE_MODEL("models/rockgibs.mdl");
	m_SkullModel = PRECACHE_MODEL("models/gib_skull.mdl");
}

void CEffectTest2::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	switch (pev->button)
	{
	case 0: TE_BeamPoints(); break;
	case 1: TE_Gunshot(); break;
	case 2: TE_Smoke(); break;
	case 3: TE_Tracer(); break;
	case 4: TE_Lightning(); break;
	case 5: TE_Sparks(); break;
	case 6: TE_BspDecal(); break;
	case 7: TE_Implosion(); break;
	case 8: TE_SpriteTrail(); break;
	case 9: TE_Sprite(); break;
	case 10: TE_BeamTorus(); break;
	case 11: TE_DLight(); break;
	case 12: TE_ELight(); break;
	case 13: TE_Line(); break;
	case 14: TE_LargeFunnel(); break;
	case 15: TE_BloodStream(); break;
	case 16: TE_Fizz(); break;
	case 17: TE_Model(); break;
	case 18: TE_ExplodeModel(); break;
	case 19: TE_BreakModel(); break;
	case 20: TE_SpriteSpray(); break;
	case 21: TE_ArmorRicochet(); break;
	case 22: TE_Bubbles(); break;
	case 23: TE_BubbleTrail(); break;
	case 24: TE_ParticleBurst(); break;
	case 25: TE_FireField(); break;
	}
}

void CEffectTest2::TE_BeamPoints()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(pev->origin.x); // start position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z - 64.0f);
	WRITE_COORD(pev->origin.x); // end position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 64.0f);
	WRITE_SHORT(m_BeamSprite); // sprite index
	WRITE_BYTE(0);			   // starting frame
	WRITE_BYTE(100);		   // frame rate in 0.1's
	WRITE_BYTE(60);			   // life in 0.1's
	WRITE_BYTE(50);			   // line width in 0.1's
	WRITE_BYTE(20);			   // noise amplitude in 0.01's
	WRITE_BYTE(255);		   // colour R
	WRITE_BYTE(160);		   // colour G
	WRITE_BYTE(0);			   // colour B
	WRITE_BYTE(255);		   // brightness
	WRITE_BYTE(50);			   // scroll speed in 0.1's

	MESSAGE_END();
}

void CEffectTest2::TE_Gunshot()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_GUNSHOT);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);

	MESSAGE_END();
}

void CEffectTest2::TE_Smoke()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_SmokeSprite); // smoke sprite index
	WRITE_BYTE(50);				// 5x scale
	WRITE_BYTE(10);				// 10fps

	MESSAGE_END();
}

void CEffectTest2::TE_Tracer()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_TRACER);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);

	MESSAGE_END();
}

void CEffectTest2::TE_Lightning()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_LIGHTNING);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 64.0f);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z - 64.0f);

	WRITE_BYTE(50);	 // 5 seconds of beam life
	WRITE_BYTE(200); // 20 units wide
	WRITE_BYTE(40);	 // 40% noise amplitude
	WRITE_SHORT(m_LightningSprite);

	MESSAGE_END();
}

void CEffectTest2::TE_Sparks()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_SPARKS);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);

	MESSAGE_END();
}

#include "decals.h"

void CEffectTest2::TE_BspDecal()
{
	// First we actually gotta trace down to da ground
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin - Vector(0.0f, 0.0f, 100.0f), ignore_monsters, edict(), &tr);

	// We use MSG_BROADCAST here because we wanna make sure
	// the decal spawns in each client's world
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(tr.vecEndPos.x);
	WRITE_COORD(tr.vecEndPos.y);
	WRITE_COORD(tr.vecEndPos.z);
	WRITE_SHORT(gDecals[DECAL_SCORCH1].index); // Decal ID
	WRITE_SHORT(0);							   // Worldspawn

	MESSAGE_END();
}

void CEffectTest2::TE_Implosion()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_IMPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(200); // radius
	WRITE_BYTE(150); // count
	WRITE_BYTE(2);	 // 0.2 seconds of life

	MESSAGE_END();
}

void CEffectTest2::TE_SpriteTrail()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_SPRITETRAIL);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_GlowSprite);
	WRITE_BYTE(150); // count
	WRITE_BYTE(2);	 // 0.2 seconds of life
	WRITE_BYTE(15);	 // 1.5x scale
	WRITE_BYTE(30);	 // 300 units/sec along vector
	WRITE_BYTE(2);	 // 20 units of randomness

	MESSAGE_END();
}

void CEffectTest2::TE_Sprite()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_GlowSprite);
	WRITE_BYTE(10);	 // 1x scale
	WRITE_BYTE(100); // about 40% brightness

	MESSAGE_END();
}

void CEffectTest2::TE_BeamTorus()
{
	for (int i = 1; i <= 4; i++)
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);

		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z - 32.0f + (i * 16.0f));
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + (1200.0f / i)); // point up, radius 1000 units
		WRITE_SHORT(m_BeamSprite);
		WRITE_BYTE(i);				// starting frame
		WRITE_BYTE(250);			// 25 fps
		WRITE_BYTE(20 + (i * 3));	// 2.3 then 2.6 seconds of life
		WRITE_BYTE(50);				// 5 units wide
		WRITE_BYTE(0);				// no noise (doesn't work)
		WRITE_BYTE(230 + (i * 4));	// colour R
		WRITE_BYTE(220 - (i * 10)); // colour G
		WRITE_BYTE(210 - (i * 30)); // colour B
		WRITE_BYTE(255);			// brightness
		WRITE_BYTE(0);				// don't scroll (doesn't work)

		MESSAGE_END();
	}
}

void CEffectTest2::TE_DLight()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(20);	 // 200 units of radius
	WRITE_BYTE(255); // colour R
	WRITE_BYTE(100); // colour G
	WRITE_BYTE(100); // colour B
	WRITE_BYTE(160); // brightness
	WRITE_BYTE(10);	 // 100 units of "life"
	WRITE_BYTE(1);	 // decay 10 units per second, affects radius

	MESSAGE_END();
}

void CEffectTest2::TE_ELight()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(0); // Do not follow any entity
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(200); // 200 units of radius
	WRITE_BYTE(255);  // colour R
	WRITE_BYTE(100);  // colour G
	WRITE_BYTE(100);  // colour B
	WRITE_BYTE(10);	  // 100 units of "life"
	WRITE_COORD(10);  // decay 10 units per second, affects radius

	MESSAGE_END();
}

void CEffectTest2::TE_Line()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_LINE);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(10); // 1 second of life
	WRITE_BYTE(255); // colour R
	WRITE_BYTE(255); // colour G
	WRITE_BYTE(255); // colour B

	MESSAGE_END();
}

void CEffectTest2::TE_Box()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BOX);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y - 32.0f);
	WRITE_COORD(pev->origin.z - 16.0f);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y + 32.0f);
	WRITE_COORD(pev->origin.z + 64.0f);
	WRITE_SHORT(10); // 1 second of life
	WRITE_BYTE(255); // colour R
	WRITE_BYTE(255); // colour G
	WRITE_BYTE(255); // colour B

	MESSAGE_END();
}

void CEffectTest2::TE_LargeFunnel()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_LARGEFUNNEL);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_GlowSprite);
	WRITE_SHORT(0); // flag 1 = go in reverse

	MESSAGE_END();
}

void CEffectTest2::TE_BloodStream()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(pev->origin.x); // start position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(0.5f); // spray vector
	WRITE_COORD(0.0f);
	WRITE_COORD(0.5f);
	WRITE_BYTE(71);	 // colour from the engine's colour palette (71 is really red)
	WRITE_BYTE(150); // speed

	MESSAGE_END();
}

void CEffectTest2::TE_Fizz()
{
	// Load any random model & set self to invisible
	SET_MODEL(edict(), "sprites/steam1.spr");
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 1;

	// Must also set bbox for the fizz to appear in
	pev->size = {64.0f, 64.0f, 64.0f};
	pev->maxs = pev->size / 2.0f;
	pev->mins = pev->mins * -1.0f;
	pev->absmax = pev->origin + pev->size / 2.0f;
	pev->absmax = pev->origin - pev->size / 2.0f;
	// Link into the BSP so the entity can be transmitted to clients
	UTIL_SetOrigin(pev, pev->origin);

	// rendercolor encodes the velocity vector for the bubbles
	pev->rendercolor.x = 4.0f;
	pev->rendercolor.y = 0.0f;
	pev->rendercolor.z = 2.0f;

	for (int i = 0; i < 4; i++)
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);

		WRITE_BYTE(TE_FIZZ);
		WRITE_SHORT(ENTINDEX(edict())); // emit from this entity
										// this entity must be transmitted to the client!!!
		WRITE_SHORT(m_GlowSprite);
		WRITE_BYTE(20); // density

		MESSAGE_END();
	}
}

void CEffectTest2::TE_Model()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(pev->origin.x); // starting position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(20.0f); // velocity
	WRITE_COORD(0.0f);
	WRITE_COORD(100.0f);
	WRITE_ANGLE(0.0f);			 // initial yaw
	WRITE_SHORT(m_RockModel);	 // model index
	WRITE_BYTE(BOUNCE_CONCRETE); // bounce sound type
	WRITE_BYTE(100);			 // 10 seconds of life

	MESSAGE_END();
}

void CEffectTest2::TE_ExplodeModel()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_EXPLODEMODEL);
	WRITE_COORD(pev->origin.x); // starting position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(500.0f);	   // velocity
	WRITE_SHORT(m_SkullModel); // model index
	WRITE_SHORT(50);		   // count
	WRITE_BYTE(40);			   // 4 seconds of life

	MESSAGE_END();
}

void CEffectTest2::TE_BreakModel()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(pev->origin.x); // starting position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(100.0f); // bbox size
	WRITE_COORD(100.0f);
	WRITE_COORD(100.0f);
	WRITE_COORD(0.0f); // velocity
	WRITE_COORD(0.0f);
	WRITE_COORD(200.0f);
	WRITE_BYTE(4);				// 40 units of randomness
	WRITE_SHORT(m_RockModel);	// model index
	WRITE_BYTE(0);				// count - let the client decide
	WRITE_BYTE(40);				// 4 seconds of life
	WRITE_BYTE(BREAK_CONCRETE); // flags

	MESSAGE_END();
}

void CEffectTest2::TE_SpriteSpray()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_SPRITE_SPRAY);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(0.0f);
	WRITE_COORD(0.0f);
	WRITE_COORD(100.0f);
	WRITE_SHORT(m_GlowSprite);
	WRITE_BYTE(50); // count
	WRITE_BYTE(10); // speed??
	WRITE_BYTE(20); // noise

	MESSAGE_END();
}

void CEffectTest2::TE_ArmorRicochet()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_ARMOR_RICOCHET);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(10); // 1x scale

	MESSAGE_END();
}

void CEffectTest2::TE_Bubbles()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y - 64.0f);
	WRITE_COORD(pev->origin.z - 64.0f);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y + 64.0f);
	WRITE_COORD(pev->origin.z + 64.0f);
	WRITE_COORD(100.0f); // float up to 50 units
	WRITE_SHORT(m_SmokeSprite);
	WRITE_BYTE(50);		// 50 of em
	WRITE_COORD(50.0f); // speed

	MESSAGE_END();
}

void CEffectTest2::TE_BubbleTrail()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(pev->origin.x - 64.0f);
	WRITE_COORD(pev->origin.y - 64.0f);
	WRITE_COORD(pev->origin.z - 8.0f);
	WRITE_COORD(pev->origin.x + 64.0f);
	WRITE_COORD(pev->origin.y + 64.0f);
	WRITE_COORD(pev->origin.z + 8.0f);
	WRITE_COORD(100.0f); // float up to 50 units
	WRITE_SHORT(m_SmokeSprite);
	WRITE_BYTE(50);		// 50 of em
	WRITE_COORD(50.0f); // speed

	MESSAGE_END();
}

void CEffectTest2::TE_ParticleBurst()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

	WRITE_BYTE(TE_PARTICLEBURST);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(50.0f);			 // radius
	WRITE_BYTE(BLOOD_COLOR_RED); // colour
	WRITE_BYTE(10);				 // duration

	MESSAGE_END();
}

void CEffectTest2::TE_FireField()
{
	// Also trace to the ground
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin - Vector(0.0f, 0.0f, 100.0f), ignore_monsters, edict(), &tr);

	// This loop is a chunky one, it sends 110 bytes total
	// It would be better to spawn this dynamically, in phases, in some Think method
	for (int i = 0; i < 10; i++)
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);

		WRITE_BYTE(TE_FIREFIELD);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		// offset the Z a little bit so the fire mostly isn't inside the ground
		WRITE_COORD(tr.vecEndPos.z + 30.0f - float(i * 2));
		WRITE_SHORT(64 + (i * 24)); // radius
		WRITE_SHORT(m_FireSprite);
		WRITE_BYTE(20);						// count; will be 200 total
		WRITE_BYTE(TEFIRE_FLAG_LOOP			// loop at 15fps
				   | TEFIRE_FLAG_PLANAR		// spawn on a flat plane
				   | TEFIRE_FLAG_ADDITIVE); // additive render mode
		WRITE_BYTE(150 + (i * 10));			// inner ring -> 15 secs; outer ring -> 24 secs

		MESSAGE_END();
	}
}
