#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_strap.h"
#include <json/cJSON.h>
#include "../cgame/animtable.h"

static int fmLoadCounter;

static ID_INLINE void BG_ParseWeaponStatsFlags ( weaponData_t *weaponData, const char *flagStr )
{
    if ( Q_stricmp (flagStr, "cookable") == 0 )
    {
        weaponData->hasCookAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "knockback") == 0 )
    {
        weaponData->hasKnockBack = qtrue;
    }
    else if ( Q_stricmp (flagStr, "roll") == 0 )
    {
        weaponData->hasRollAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "zoom") == 0 )
    {
        weaponData->zoomType = ZOOM_CONTINUOUS;
    }
    else if ( Q_stricmp (flagStr, "togglezoom") == 0 )
    {
        weaponData->zoomType = ZOOM_TOGGLE;
    }
}

static ID_INLINE void BG_ParseFireModeFiringType ( weaponFireModeStats_t *fireMode, const char *firingTypeStr )
{
    if ( Q_stricmp (firingTypeStr, "auto") == 0 )
    {
        fireMode->firingType = FT_AUTOMATIC;
    }
    else if ( Q_stricmp (firingTypeStr, "semi") == 0 )
    {
        fireMode->firingType = FT_SEMI;
    }
    else if ( Q_stricmp (firingTypeStr, "burst") == 0 )
    {
        fireMode->firingType = FT_BURST;
    }
}

static const stringID_table_t MODTable[] =
{
	ENUM2STRING (MOD_STUN_BATON),
	ENUM2STRING (MOD_MELEE),
	ENUM2STRING (MOD_SABER),
	ENUM2STRING (MOD_BRYAR_PISTOL),
	ENUM2STRING (MOD_BRYAR_PISTOL_ALT),
	ENUM2STRING (MOD_BLASTER),
	ENUM2STRING (MOD_TURBLAST),
	ENUM2STRING (MOD_DISRUPTOR),
	ENUM2STRING (MOD_DISRUPTOR_SPLASH),
	ENUM2STRING (MOD_DISRUPTOR_SNIPER),
	ENUM2STRING (MOD_BOWCASTER),
	ENUM2STRING (MOD_REPEATER),
	ENUM2STRING (MOD_REPEATER_ALT),
	ENUM2STRING (MOD_REPEATER_ALT_SPLASH),
	ENUM2STRING (MOD_DEMP2),
	ENUM2STRING (MOD_DEMP2_ALT),
	ENUM2STRING (MOD_FLECHETTE),
	ENUM2STRING (MOD_FLECHETTE_ALT_SPLASH),
	ENUM2STRING (MOD_ROCKET),
	ENUM2STRING (MOD_ROCKET_SPLASH),
	ENUM2STRING (MOD_ROCKET_HOMING),
	ENUM2STRING (MOD_ROCKET_HOMING_SPLASH),
	ENUM2STRING (MOD_THERMAL),
	ENUM2STRING (MOD_THERMAL_SPLASH),
	ENUM2STRING (MOD_TRIP_MINE_SPLASH),
	ENUM2STRING (MOD_TIMED_MINE_SPLASH),
	ENUM2STRING (MOD_DET_PACK_SPLASH),
	ENUM2STRING (MOD_VEHICLE),
	ENUM2STRING (MOD_CONC),
	ENUM2STRING (MOD_CONC_ALT),
	ENUM2STRING (MOD_FORCE_DARK),
	ENUM2STRING (MOD_SENTRY),
	ENUM2STRING (MOD_WATER),
	ENUM2STRING (MOD_SLIME),
	ENUM2STRING (MOD_LAVA),
	ENUM2STRING (MOD_CRUSH),
	ENUM2STRING (MOD_TELEFRAG),
	ENUM2STRING (MOD_FALLING),
	ENUM2STRING (MOD_SUICIDE),
	ENUM2STRING (MOD_TARGET_LASER),
	ENUM2STRING (MOD_TRIGGER_HURT),
	ENUM2STRING (MOD_TEAM_CHANGE),
	
	{ NULL, -1 }
};

static const stringID_table_t AmmoTable[] =
{
    ENUM2STRING (AMMO_NONE),
	ENUM2STRING (AMMO_FORCE),
	ENUM2STRING (AMMO_BLASTER),
	ENUM2STRING (AMMO_POWERCELL),
	ENUM2STRING (AMMO_METAL_BOLTS),
	ENUM2STRING (AMMO_CONCUSSION),
	ENUM2STRING (AMMO_ROCKETS),
	ENUM2STRING (AMMO_EMPLACED),
	ENUM2STRING (AMMO_THERMAL),
	ENUM2STRING (AMMO_TRIPMINE),
	ENUM2STRING (AMMO_DETPACK),
	ENUM2STRING (AMMO_MAX),
	
	{ NULL, -1 }
};

#ifdef QAGAME
#include "jkg_damagetypes.h"
static void BG_ParseDamage ( weaponFireModeStats_t *fireModeStats, cJSON *damageNode, qboolean secondary )
{
    if ( !damageNode )
    {
        return;
    }
    
    if ( !secondary && cJSON_IsNumber (damageNode) )
    {
        fireModeStats->baseDamage = (short)cJSON_ToIntegerOpt (damageNode, 0);
    }
    else if ( cJSON_IsObject (damageNode) )
    {
        damageSettings_t darea;
        cJSON *node = NULL;
        qhandle_t areaHandle = 0;
        
        memset (&darea, 0, sizeof (darea));
        
        node = cJSON_GetObjectItem (damageNode, "damageradius");
        if ( node )
        {
            cJSON *child = NULL;
            const char *s = NULL;
            
            darea.radial = qtrue;
            
            child = cJSON_GetObjectItem (node, "start");
            darea.radiusParams.startRadius = (float)cJSON_ToNumber (child);
            
            child = cJSON_GetObjectItem (node, "end");
            darea.radiusParams.endRadius = (float)cJSON_ToNumber (child);
            
            child = cJSON_GetObjectItem (node, "parm");
            darea.radiusParams.generic1 = cJSON_ToInteger (child);
            
            child = cJSON_GetObjectItem (node, "falloff");
            s = cJSON_ToStringOpt (child, "constant");
            if ( Q_stricmp (s, "constant") == 0 )
                darea.radiusParams.damageFunc = DF_CONSTANT;
            else if ( Q_stricmp (s, "linear") == 0 )
                darea.radiusParams.damageFunc = DF_LINEAR;
            else if ( Q_stricmp (s, "gaussian") == 0 )
                darea.radiusParams.damageFunc = DF_GAUSSIAN;
            else
            {
                darea.radiusParams.damageFunc = DF_CONSTANT;
                Com_Printf ("Unknown damage falloff type used: %s. Defaulting to linear falloff.\n", s);
            }
                
            child = cJSON_GetObjectItem (node, "function");
            s = cJSON_ToStringOpt (child, "constant");
            if ( Q_stricmp (s, "linear") == 0 )
                darea.radiusParams.radiusFunc = RF_LINEAR;
            else if ( Q_stricmp (s, "nonlinear") == 0 )
                darea.radiusParams.radiusFunc = RF_NONLINEAR;
            else if ( Q_stricmp (s, "clamp") == 0 )
                darea.radiusParams.radiusFunc = RF_CLAMP;
            else if ( Q_stricmp (s, "wave") == 0 )
                darea.radiusParams.radiusFunc = RF_WAVE;
            else if ( Q_stricmp (s, "constant") == 0 )
                darea.radiusParams.radiusFunc = RF_CONSTANT;
            else
            {
                darea.radiusParams.radiusFunc = RF_CONSTANT;
                Com_Printf ("Unknown radius function used: %s; Defaulting to constant radius.\n", s);
            }
        }
        
        node = cJSON_GetObjectItem (damageNode, "duration");
        darea.lifetime = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "delay");
        darea.delay = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "damage");
        darea.damage = cJSON_ToIntegerOpt (node, 0);
        fireModeStats->baseDamage = darea.damage;
        
        node = cJSON_GetObjectItem (damageNode, "damagedelay");
        darea.damageDelay = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "penetration");
        switch ( cJSON_ToIntegerOpt (node, 0) )
        {
            default:
            case 0: darea.penetrationType = PT_NONE; break;
            case 1: darea.penetrationType = PT_SHIELD; break;
            case 2: darea.penetrationType = PT_SHIELD_ARMOR; break;
            case 3: darea.penetrationType = PT_SHIELD_ARMOR_BUILDING; break;
        }
        
        node = cJSON_GetObjectItem (damageNode, "damagetype");
        if ( node )
        {
            int i = 0;
            const char *types[NUM_DAMAGE_TYPES];
            int numTypes = cJSON_ReadStringArray (node, NUM_DAMAGE_TYPES, types);
            
            for ( i = 0; i < numTypes; i++ )
            {
                if ( Q_stricmp (types[i], "annihilate") == 0 )
                    darea.damageType |= (1 << DT_ANNIHILATION);
                else if ( Q_stricmp (types[i], "concussion") == 0 )
                    darea.damageType |= (1 << DT_CONCUSSION);
                else if ( Q_stricmp (types[i], "cut") == 0 )
                    darea.damageType |= (1 << DT_CUT);
                else if ( Q_stricmp (types[i], "disintegrate") == 0 )
                    darea.damageType |= (1 << DT_DISINTEGRATE);
                else if ( Q_stricmp (types[i], "electric") == 0 )
                    darea.damageType |= (1 << DT_ELECTRIC);
                else if ( Q_stricmp (types[i], "explosion") == 0 )
                    darea.damageType |= (1 << DT_EXPLOSION);
                else if ( Q_stricmp (types[i], "fire") == 0 )
                    darea.damageType |= (1 << DT_FIRE);
                else if ( Q_stricmp (types[i], "freeze") == 0 )
                    darea.damageType |= (1 << DT_FREEZE);
                else if ( Q_stricmp (types[i], "implosion") == 0 )
                    darea.damageType |= (1 << DT_IMPLOSION);
                else if ( Q_stricmp (types[i], "stun") == 0 )
                    darea.damageType |= (1 << DT_STUN);
                else if ( Q_stricmp (types[i], "carbonite") == 0 )
                    darea.damageType |= (1 << DT_CARBONITE);
                else
                    Com_Printf ("Unknown damage type used: %s.\n", types[i]);
            }
        }
        
        areaHandle = JKG_RegisterDamageSettings (&darea);
        if ( !secondary )
        {
            fireModeStats->damageTypeHandle = areaHandle;
        }
        else
        {
            fireModeStats->secondaryDmgHandle = areaHandle;
        }
    }
}
#endif

static void BG_ParseWeaponFireMode ( weaponFireModeStats_t *fireModeStats, cJSON *fireModeNode )
{
    cJSON *node;
    const char *str = NULL;
    
    if ( fireModeNode == NULL )
    {
        return;
    }
    
    node = cJSON_GetObjectItem (fireModeNode, "ammo");
    str = cJSON_ToString (node);
    if ( str && str[0] )
    {
        fireModeStats->ammo = BG_GetAmmo (str);
    }

    node = cJSON_GetObjectItem (fireModeNode, "damage");
#ifdef QAGAME
    BG_ParseDamage (fireModeStats, node, qfalse);
    
    node = cJSON_GetObjectItem (fireModeNode, "secondarydamage");
    BG_ParseDamage (fireModeStats, node, qtrue);
#else
	fireModeStats->baseDamage = cJSON_ToInteger (node);
#endif
    
    node = cJSON_GetObjectItem (fireModeNode, "grenade");
    fireModeStats->isGrenade = (qboolean)cJSON_ToBooleanOpt (node, 0);

	node = cJSON_GetObjectItem (fireModeNode, "grenadeBounces");
    fireModeStats->grenadeBounces = (qboolean)cJSON_ToBooleanOpt (node, 1);

	node = cJSON_GetObjectItem (fireModeNode, "grenadeBounceDMG");
    fireModeStats->grenadeBounceDMG = (char)cJSON_ToIntegerOpt (node, 10);
    
    node = cJSON_GetObjectItem (fireModeNode, "ballistic");
    fireModeStats->applyGravity = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "bounces");
    fireModeStats->bounceCount = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "hitscan");
    fireModeStats->hitscan = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectiles");
    fireModeStats->shotCount = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "collisionsize");
    fireModeStats->boxSize = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxchargetime");
    fireModeStats->chargeMaximum = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedamage");
    fireModeStats->chargeMultiplier = (float)cJSON_ToNumberOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedelay");
    fireModeStats->chargeTime = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "ammocost");
    fireModeStats->cost = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "firingtype");
    BG_ParseFireModeFiringType (fireModeStats, cJSON_ToStringOpt (node, "auto"));
    
    // 0 means fully automatic, otherwise one burst will fire weapon n times.
    node = cJSON_GetObjectItem (fireModeNode, "shotsperburst");
    fireModeStats->shotsPerBurst = (char)cJSON_ToIntegerOpt (node, 0);
    
    // 0 means infinite delay (semi-automatic), otherwise n milliseconds between
    // rounds in a burst.
    node = cJSON_GetObjectItem (fireModeNode, "burstshotdelay");
    fireModeStats->burstFireDelay = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "firedelay");
    fireModeStats->delay = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "range");
    fireModeStats->range = (float)cJSON_ToIntegerOpt (node, WPR_M);
    
    node = cJSON_GetObjectItem (fireModeNode, "splashrange");
    fireModeStats->rangeSplash = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "recoil");
    fireModeStats->recoil = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "spread");
    fireModeStats->spread = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectilespeed");
    fireModeStats->speed = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectileclass");
    str = cJSON_ToStringOpt (node, "unknown_proj");
    Q_strncpyz (fireModeStats->weaponClass, str, sizeof (fireModeStats->weaponClass));
    
    node = cJSON_GetObjectItem (fireModeNode, "meansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
    fireModeStats->weaponMOD = GetIDForString (MODTable, str);
    
    node = cJSON_GetObjectItem (fireModeNode, "splashmeansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
    fireModeStats->weaponSplashMOD = GetIDForString (MODTable, str);
}

//=========================================================
// BG_ParseWeaponStats
//---------------------------------------------------------
// Description:
// Parses the stats block of a weapons file.
//=========================================================
static void BG_ParseWeaponStats ( weaponData_t *weaponData, cJSON *statsNode )
{
    cJSON *node;
    const char *flags[4];
    const char *ammo;
    
    if ( statsNode == NULL )
    {
        return;
    }

    node = cJSON_GetObjectItem (statsNode, "slot");
    weaponData->weaponSlot = (unsigned char)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "reloadtime");
    weaponData->weaponReloadTime = (unsigned short)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "ammoIndex");
    ammo = cJSON_ToStringOpt (node, "AMMO_NONE");
	weaponData->ammoIndex = BG_GetAmmo(ammo)->ammoIndex;

	node = cJSON_GetObjectItem (statsNode, "ammoOnSpawn");
	weaponData->ammoOnSpawn = (unsigned int)cJSON_ToIntegerOpt (node, ((weaponData->ammoIndex < AMMO_ROCKETS) ? 400 : ((weaponData->ammoIndex != AMMO_ROCKETS) ? 444 : 10)));	// Gives 300 ammo for non-explosives

	node = cJSON_GetObjectItem (statsNode, "ammoOnPickup");
	weaponData->ammoOnPickup = (unsigned int)cJSON_ToIntegerOpt (node, ((weaponData->ammoIndex < AMMO_ROCKETS) ? 20 : 1));	// Gives 20 ammo for non-explosives

	node = cJSON_GetObjectItem (statsNode, "clipSize");
	weaponData->clipSize = (unsigned int)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "flags");
    if ( node != NULL )
    {
        int numFlags = cJSON_ReadStringArray (node, 4, flags);
        int i;

        for ( i = 0; i < numFlags; i++ )
        {
            BG_ParseWeaponStatsFlags (weaponData, flags[i]);
        }
    }

    node = cJSON_GetObjectItem (statsNode, "speed");
    weaponData->speedModifier = (float)cJSON_ToNumberOpt (node, 1.0);
        
    node = cJSON_GetObjectItem (statsNode, "reloadmodifier");
    weaponData->reloadModifier = (float)cJSON_ToNumberOpt (node, 1.0);
    
    node = cJSON_GetObjectItem (statsNode, "startzoomfov");
    weaponData->startZoomFov = (float)cJSON_ToNumberOpt (node, 50.0);
    
    node = cJSON_GetObjectItem (statsNode, "endzoomfov");
    weaponData->endZoomFov = (float)cJSON_ToNumberOpt (node, weaponData->zoomType == ZOOM_TOGGLE ? weaponData->startZoomFov : 5.0);
    
    node = cJSON_GetObjectItem (statsNode, "zoomtime");
    weaponData->zoomTime = cJSON_ToIntegerOpt (node, 2000);
}

static void BG_ParseAnimationObject ( cJSON *animationNode, int *torsoAnimation, int *legsAnimation )
{
    cJSON *node = cJSON_GetObjectItem (animationNode, "torso");
    const char *anim = cJSON_ToStringOpt (node, "BOTH_STAND1");
    
    *torsoAnimation = GetIDForString (animTable, anim);
    
    node = cJSON_GetObjectItem (animationNode, "legs");
    if ( node == NULL )
    {
        *legsAnimation = *torsoAnimation;
        return;
    }
    
    anim = cJSON_ToString (node);
    *legsAnimation = GetIDForString (animTable, anim);
}

static void BG_ParseWeaponPlayerAnimations ( weaponData_t *weaponData, cJSON *playerAnimNode )
{
    cJSON *node;
    
    node = cJSON_GetObjectItem (playerAnimNode, "ready");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.ready.torsoAnim, &weaponData->anims.ready.legsAnim);
	}
    
    node = cJSON_GetObjectItem (playerAnimNode, "firing");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.firing.torsoAnim, &weaponData->anims.firing.legsAnim);
	}
    
    /*node = cJSON_GetObjectItem (playerAnimNode, "idle");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);
    
    node = cJSON_GetObjectItem (playerAnimNode, "drop");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);
    
    node = cJSON_GetObjectItem (playerAnimNode, "raise");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);*/

	
    
    node = cJSON_GetObjectItem (playerAnimNode, "reload");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.reload.torsoAnim, &weaponData->anims.reload.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "backwardWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.backwardWalk.torsoAnim, &weaponData->anims.backwardWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "forwardWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.forwardWalk.torsoAnim, &weaponData->anims.forwardWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "crouchWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.crouchWalk.torsoAnim, &weaponData->anims.crouchWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "crouchWalkBack");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.crouchWalkBack.torsoAnim, &weaponData->anims.crouchWalkBack.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "jump");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.jump.torsoAnim, &weaponData->anims.jump.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "land");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.land.torsoAnim, &weaponData->anims.land.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "run");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.run.torsoAnim, &weaponData->anims.run.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sprint");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.sprint.torsoAnim, &weaponData->anims.sprint.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sprintStyle");
	if(node)
	{
		weaponData->firstPersonSprintStyle = cJSON_ToNumber(node);
	}
}

static void ReadString ( cJSON *parent, const char *field, char *dest, size_t destSize )
{
    cJSON *node = NULL;
    const char *str = NULL;
    
    node = cJSON_GetObjectItem (parent, field);
    str = cJSON_ToString (node);
    if ( str && str[0] )
    {
        int len = strlen (str);
        if ( (len + 1) > destSize )
        {
            Com_Printf (S_COLOR_YELLOW "WARNING: %s is too long (%d chars). Max length for this string is %d.\n", str, len, destSize - 1);
        }
        Q_strncpyz (dest, str, destSize);
    }
}

#ifdef CGAME

static void BG_ParseVisualsFireMode ( weaponVisualFireMode_t *fireMode, cJSON *fireModeNode, int numFireModes )
{
    cJSON *node = NULL;
    cJSON *child = NULL;
    qboolean isGrenade = qfalse;
    qboolean isBlaster = qfalse;
    qboolean isTripmine = qfalse;
    qboolean isDetpack = qfalse;

    ReadString (fireModeNode, "type", fireMode->type, sizeof (fireMode->type));
    isGrenade = (qboolean)(Q_stricmp (fireMode->type, "grenade") == 0);
    isBlaster = (qboolean)(Q_stricmp (fireMode->type, "blaster") == 0);
    isTripmine = (qboolean)(Q_stricmp (fireMode->type, "tripmine") == 0);
    isDetpack = (qboolean)(Q_stricmp (fireMode->type, "detpack") == 0);

	ReadString (fireModeNode, "displayName", fireMode->displayName, 128);

	ReadString (fireModeNode, "crosshairShader", fireMode->crosshairShader, MAX_QPATH);
	ReadString (fireModeNode, "switchToSound", fireMode->switchToSound, MAX_QPATH);
	if(!fireMode->switchToSound || !fireMode->switchToSound[0])
	{
		if(fmLoadCounter == 0)
		{
			Com_sprintf(fireMode->switchToSound, MAX_QPATH, "sound/weapons/common/click%i.wav", numFireModes);
		}
		else
		{
			Com_sprintf(fireMode->switchToSound, MAX_QPATH, "sound/weapons/common/click1.wav");
		}
	}

	node = cJSON_GetObjectItem (fireModeNode, "animType");
    fireMode->animType = cJSON_ToInteger (node);

	fireMode->overrideIndicatorFrame = -1;											// set this as default since 0 is still a valid frame
	node = cJSON_GetObjectItem (fireModeNode, "overrideIndicatorFrame");
	if(node)
	{
		fireMode->overrideIndicatorFrame = cJSON_ToInteger (node);
	}
    
    // TODO: Need to tie this to the table in cg_weapons.c somehow...
    // Weapon Render
    node = cJSON_GetObjectItem (fireModeNode, "muzzlelightintensity");
    fireMode->weaponRender.generic.muzzleLightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "muzzlelightcolor", fireMode->weaponRender.generic.muzzleLightColor, sizeof (fireMode->weaponRender.generic.muzzleLightColor));
    ReadString (fireModeNode, "chargingfx", fireMode->weaponRender.generic.chargingEffect, sizeof (fireMode->weaponRender.generic.chargingEffect));    
    ReadString (fireModeNode, "muzzlefx", fireMode->weaponRender.generic.muzzleEffect, sizeof (fireMode->weaponRender.generic.muzzleEffect));
    
    // Weapon Fire
    child = cJSON_GetObjectItem (fireModeNode, "firesound");
    if ( child )
    {
        if ( cJSON_IsArray (child) )
        {
            const char *fireSounds[8];
            int numFireSounds = cJSON_ReadStringArray (child, 8, fireSounds);
            int i;
            
            for ( i = 0; i < numFireSounds; i++ )
            {
                Q_strncpyz (fireMode->weaponFire.generic.fireSound[i], fireSounds[i], sizeof (fireMode->weaponFire.generic.fireSound[i]));
            }
        }
        else
        {
            const char *s = cJSON_ToString (child);
            if ( s ) Q_strncpyz (fireMode->weaponFire.generic.fireSound[0], s, sizeof (fireMode->weaponFire.generic.fireSound[0]));
        }
    }
    
    // Traceline Render
    ReadString (fireModeNode, "tracelineshader", fireMode->tracelineRender.generic.tracelineShader, sizeof (fireMode->tracelineRender.generic.tracelineShader));
    
    node = cJSON_GetObjectItem (fireModeNode, "minsize");
    fireMode->tracelineRender.generic.minSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxsize");
    fireMode->tracelineRender.generic.maxSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "lifetime");
    fireMode->tracelineRender.generic.lifeTime = cJSON_ToInteger (node);
    
    // Weapon Charge
    ReadString (fireModeNode, "chargingsound", fireMode->weaponCharge.chargingSound, sizeof (fireMode->weaponCharge.chargingSound));
    
    // Projectile render
    ReadString (fireModeNode, "projectilemodel", fireMode->projectileRender.generic.projectileModel, sizeof (fireMode->projectileRender.generic.projectileModel));
    ReadString (fireModeNode, "projectilefx", fireMode->projectileRender.generic.projectileEffect, sizeof (fireMode->projectileRender.generic.projectileEffect));
    ReadString (fireModeNode, "runsound", fireMode->projectileRender.generic.runSound, sizeof (fireMode->projectileRender.generic.runSound));
    
    node = cJSON_GetObjectItem (fireModeNode, "lightintensity");
    fireMode->projectileRender.generic.lightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "lightcolor", fireMode->projectileRender.generic.lightColor, sizeof (fireMode->projectileRender.generic.lightColor));
  
    ReadString (fireModeNode, "deathfx", fireMode->projectileRender.generic.deathEffect, sizeof (fireMode->projectileRender.generic.deathEffect));
    
    // Projectile miss event
    child = cJSON_GetObjectItem (fireModeNode, "miss");
    if ( isTripmine || isDetpack )
    {
        ReadString (child, "sticksound", fireMode->projectileMiss.explosive.stickSound, sizeof (fireMode->projectileMiss.explosive.stickSound));
    }
    else
    {
        ReadString (child, "impactfx", fireMode->projectileMiss.generic.impactEffect, sizeof (fireMode->projectileMiss.generic.impactEffect));
        ReadString (child, "shockwavefx", fireMode->projectileMiss.grenade.shockwaveEffect, sizeof (fireMode->projectileMiss.grenade.shockwaveEffect));
    }
    
    // Projectile hit event
    child = cJSON_GetObjectItem (fireModeNode, "hit");
    ReadString (child, "impactfx", fireMode->projectileHitPlayer.generic.impactEffect, sizeof (fireMode->projectileHitPlayer.generic.impactEffect));
    ReadString (child, "shockwavefx", fireMode->projectileHitPlayer.grenade.shockwaveEffect, sizeof (fireMode->projectileHitPlayer.grenade.shockwaveEffect));
    
    // Projectile deflected event
    ReadString (fireModeNode, "deflectedfx", fireMode->projectileDeflected.generic.deflectEffect, sizeof (fireMode->projectileDeflected.generic.deflectEffect));
    
    // Grenade bounce event
    child = cJSON_GetObjectItem (fireModeNode, "bouncesound");
    if ( child )
    {
        const char *bounceSounds[2];
        int numBounceSounds = cJSON_ReadStringArray (child, 2, bounceSounds);
        int i;
        
        for ( i = 0; i < numBounceSounds; i++ )
        {
            Q_strncpyz (fireMode->grenadeBounce.grenade.bounceSound[i], bounceSounds[i], sizeof (fireMode->grenadeBounce.grenade.bounceSound[i]));
        }
    }
    
    // Explosive render
    ReadString (fireModeNode, "g2model", fireMode->explosiveRender.detpack.g2Model, sizeof (fireMode->explosiveRender.detpack.g2Model));
    
    node = cJSON_GetObjectItem (fireModeNode, "g2radius");
    fireMode->explosiveRender.detpack.g2Radius = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "linefx", fireMode->explosiveRender.tripmine.lineEffect, sizeof (fireMode->explosiveRender.tripmine.lineEffect));
    
    // Explosive blow event
    ReadString (fireModeNode, "explodefx", fireMode->explosiveBlow.generic.explodeEffect, sizeof (fireMode->explosiveBlow.generic.explodeEffect));
    
    // Explosive armed event
    ReadString (fireModeNode, "armsound", fireMode->explosiveArm.armSound, sizeof (fireMode->explosiveArm.armSound));

	fmLoadCounter++;
}

static void BG_ParseVisuals ( weaponData_t *weaponData, cJSON *visualsNode )
{
    cJSON *node = NULL;
    cJSON *child = NULL;
    weaponVisual_t *weaponVisuals = &weaponData->visuals;
	int i;

    ReadString (visualsNode, "worldmodel", weaponVisuals->world_model, sizeof (weaponVisuals->world_model));
    ReadString (visualsNode, "viewmodel", weaponVisuals->view_model, sizeof (weaponVisuals->view_model));
    ReadString (visualsNode, "hudicon", weaponVisuals->icon, sizeof (weaponVisuals->icon));
    ReadString (visualsNode, "hudnaicon", weaponVisuals->icon_na, sizeof (weaponVisuals->icon_na));
    ReadString (visualsNode, "selectsound", weaponVisuals->selectSound, sizeof (weaponVisuals->selectSound));
    
    child = cJSON_GetObjectItem (visualsNode, "indicators");
    if ( child )
    {
        int numGroupIndicators = 0;
        int i;
        char *shaders[3];
        cJSON *subchild = cJSON_GetObjectItem (child, "ammo");
        if ( subchild )
        {
            weaponVisuals->indicatorType = IND_NORMAL;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, shaders);
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        else if ( (subchild = cJSON_GetObjectItem (child, "leds")) != NULL )
        {
            weaponVisuals->indicatorType = IND_GRENADE;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, shaders);
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        
        ReadString (child, "firemode", weaponVisuals->firemodeIndicatorShader, sizeof (weaponVisuals->firemodeIndicatorShader));
    }
    
    ReadString (visualsNode, "gunposition", weaponVisuals->gunPosition, sizeof (weaponVisuals->gunPosition));
    ReadString (visualsNode, "ironsightsPosition", weaponVisuals->ironsightsPosition, sizeof (weaponVisuals->ironsightsPosition));
    
    node = cJSON_GetObjectItem (visualsNode, "ironsightsFov");
    weaponVisuals->ironsightsFov = (float)cJSON_ToNumberOpt (node, 0.0);
    
    // Scope toggle
    child = cJSON_GetObjectItem (visualsNode, "scope");
    ReadString (child, "startsound", weaponVisuals->scopeStartSound, sizeof (weaponVisuals->scopeStartSound));
    ReadString (child, "stopsound", weaponVisuals->scopeStopSound, sizeof (weaponVisuals->scopeStopSound));
    
    // Scope zoom
    node = cJSON_GetObjectItem (child, "looptime");
    weaponVisuals->scopeSoundLoopTime = cJSON_ToInteger (node);
    
    ReadString (child, "loopsound", weaponVisuals->scopeLoopSound, sizeof (weaponVisuals->scopeLoopSound));
    
    // Scope render
    ReadString (child, "mask", weaponVisuals->scopeShader, sizeof (weaponVisuals->scopeShader));

	//Crosshair -- eezstreet add
	node = cJSON_GetObjectItem (visualsNode, "crosshairValue");
	weaponVisuals->crosshairValue = (int)cJSON_ToNumberOpt(node, (double)1);
    
    /*node = cJSON_GetObjectItem (visualsNode, "primary");
    BG_ParseVisualsFireMode (&weaponVisuals->primary, node);
    
    node = cJSON_GetObjectItem (visualsNode, "secondary");
    BG_ParseVisualsFireMode (&weaponVisuals->secondary, node);*/

	for ( i = 0; i < weaponData->numFiringModes; i++ )
	{
		node = cJSON_GetObjectItem (visualsNode, va("firemode%i", i) );
		if(node)
		{
			BG_ParseVisualsFireMode(&weaponVisuals->visualFireModes[i], node, weaponData->numFiringModes);
		}
	}
}
#endif

stringID_table_t WPTable[]; // From bg_saga.c

#define MAX_WEAPON_FILE_LENGTH (16384) // 16kb should be enough, 4kb apparently wasn't!
static qboolean BG_ParseWeaponFile ( const char *weaponFilePath )
{
    cJSON *json = NULL;
    cJSON *jsonNode = NULL;
    char error[MAX_STRING_CHARS];
    const char *str = NULL;
    int weapon;
	int i;
    
    char weaponFileData[MAX_WEAPON_FILE_LENGTH];
    fileHandle_t f;
    int fileLen = strap_FS_FOpenFile (weaponFilePath, &f, FS_READ);
    
    weaponData_t weaponData;

	fmLoadCounter = 0;
    
    if ( !f || fileLen == -1 )
    {
        Com_Printf (S_COLOR_RED "%s: failed to read the weapon file. File is unreadable or is empty.\n", weaponFilePath);
        return qfalse;
    }
    
    if ( (fileLen + 1) >= MAX_WEAPON_FILE_LENGTH )
    {
        trap_FS_FCloseFile (f);
        Com_Printf (S_COLOR_RED "%s: file too big (%d bytes, maximum is %d).\n", weaponFilePath, fileLen, MAX_WEAPON_FILE_LENGTH - 1);
        
        return qfalse;
    }
    
    strap_FS_Read (&weaponFileData, fileLen, f);
    weaponFileData[fileLen] = '\0';
    
    strap_FS_FCloseFile (f);
    
    json = cJSON_ParsePooled (weaponFileData, error, sizeof (error));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "%s: %s\n", weaponFilePath, error);
        
        return qfalse;
    }
    
    BG_InitializeWeaponData (&weaponData);
    
    ReadString (json, "classname", weaponData.classname, sizeof (weaponData.classname));
    
    jsonNode = cJSON_GetObjectItem (json, "type");
    str = cJSON_ToString (jsonNode);
    weapon = GetIDForString (WPTable, str);
    weaponData.weaponBaseIndex = weapon;
    
    jsonNode = cJSON_GetObjectItem (json, "variation");
    weapon = cJSON_ToNumber (jsonNode);
    weaponData.weaponModIndex = weapon;

    jsonNode = cJSON_GetObjectItem (json, "stats");
    BG_ParseWeaponStats (&weaponData, jsonNode);
    
	weaponData.numFiringModes = 0;
	for(i = 0; i < MAX_FIREMODES; i++)
	{
		jsonNode = cJSON_GetObjectItem (json, va("firemode%i", i));
		if(jsonNode != NULL)
		{
			BG_ParseWeaponFireMode (&weaponData.firemodes[i], jsonNode);
			weaponData.numFiringModes++;
		}
	}
	// Old stuff for when we had primary/alt attacks --eez
    /*jsonNode = cJSON_GetObjectItem (json, "primaryattack");
    BG_ParseWeaponFireMode (&weaponData.firemodes[0], jsonNode);
    
    jsonNode = cJSON_GetObjectItem (json, "secondaryattack");
    if ( jsonNode != NULL )
    {
        weaponData.hasSecondary = 1;
        BG_ParseWeaponFireMode (&weaponData.firemodes[1], jsonNode);
    }*/
    
    jsonNode = cJSON_GetObjectItem (json, "playeranims");
    BG_ParseWeaponPlayerAnimations (&weaponData, jsonNode);
    
    jsonNode = cJSON_GetObjectItem (json, "name");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.displayName, str, sizeof (weaponData.displayName));
    
#ifdef CGAME
    // TODO: Maybe we can turn this into a loop somehow? It's just turning into a stupidly long list.

    jsonNode = cJSON_GetObjectItem (json, "weaponanims");
    // TODO
    
    jsonNode = cJSON_GetObjectItem (json, "description");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.visuals.description, str, sizeof (weaponData.visuals.description));
    
    jsonNode = cJSON_GetObjectItem (json, "visual");
    BG_ParseVisuals (&weaponData, jsonNode);
#endif
    

    /*if ( weaponData.zoomType != ZOOM_NONE )
    {
        // If we have zoom mode, then copy over the data from the primary to the secondary
        // so it's as if we haven't changed fire modes at all! Ingenious! (And also temporary)
        weaponData.firemodes[1] = weaponData.firemodes[0];
    }*/
    
    BG_AddWeaponData (&weaponData);
    
    cJSON_Delete (json);
    
    return qtrue;
}

#define WEAPON_DATA_CACHE_VERSION (1)
typedef struct weaponDataCacheHeader_s
{
    int version;
    int numWeapons;
} weaponDataCacheHeader_t;

static qboolean LoadWeaponCacheFile ( weaponData_t *weaponDataTable, int *numLoadedWeapons, int *numWeapons )
{
    fileHandle_t f;
    int fileLen;
    
    char *buffer = NULL;
    char *p = NULL;
    weaponDataCacheHeader_t *header = NULL;
    int i;
    
    fileLen = trap_FS_FOpenFile ("ext_data/weapons/weapcache.bin", &f, FS_READ);
    if ( fileLen == -1 || !f )
    {
        return qfalse;
    }
    
    buffer = (char *)malloc (fileLen + 1);
    trap_FS_Read ((void *)buffer, fileLen, f);
    trap_FS_FCloseFile (f);
    buffer[fileLen] = '\0';
    
    header = (weaponDataCacheHeader_t *)&buffer[0];
    if ( header->version != WEAPON_DATA_CACHE_VERSION )
    {
        free (buffer);
        return qfalse;
    }
    
    p = buffer + sizeof (weaponDataCacheHeader_t);
    memcpy (weaponDataTable, p, header->numWeapons * sizeof (weaponData_t));
    *numLoadedWeapons = header->numWeapons;
    
    memset (numWeapons, 0, sizeof (int) * MAX_WEAPONS);
    for ( i = 0; i < *numLoadedWeapons; i++ )
    {
        numWeapons[weaponDataTable[i].weaponBaseIndex]++;
    }
    
    free (buffer);
    
    return qtrue;
}

static void WriteWeaponCacheFile ( weaponData_t *weaponDataTable, int *numLoadedWeapons )
{
    fileHandle_t f;
    char *buffer;
    char *p;
    weaponDataCacheHeader_t *header = NULL;
    int bufferSize;
    
    trap_FS_FOpenFile ("ext_data/weapons/weapcache.bin", &f, FS_WRITE);
    if ( !f )
    {
        return;
    }
    
    bufferSize = sizeof (weaponDataCacheHeader_t) + sizeof (weaponData_t) * *numLoadedWeapons;
    buffer = malloc (bufferSize);
    p = buffer;
    header = (weaponDataCacheHeader_t *)p;
    header->version = WEAPON_DATA_CACHE_VERSION;
    header->numWeapons = *numLoadedWeapons;
    
    p += sizeof (weaponDataCacheHeader_t);
    memcpy (p, weaponDataTable, sizeof (weaponData_t) * *numLoadedWeapons);
    
    trap_FS_Write ((void *)buffer, bufferSize, f);
    trap_FS_FCloseFile (f);
    
    free (buffer);
}

int trap_Milliseconds ( void );
qboolean BG_LoadWeapons ( weaponData_t *weaponDataTable, int *numLoadedWeapons, int *numWeapons )
{
    int i;
    char weaponFiles[8192];
    int numFiles = strap_FS_GetFileList ("ext_data/weapons", ".wpn", weaponFiles, sizeof (weaponFiles));
    const char *weaponFile = weaponFiles;
    int successful = 0;
    int failed = 0;
    
    int t = trap_Milliseconds();
    
    Com_Printf ("------- Weapon Initialization -------\n");
    
    //if ( !LoadWeaponCacheFile (weaponDataTable, numLoadedWeapons, numWeapons) )
    {
        for ( i = 0; i < numFiles; i++ )
        {
            if ( BG_ParseWeaponFile (va ("ext_data/weapons/%s", weaponFile)) )
            {
                successful++;
            }
            else
            {
                failed++;
            }
            
            weaponFile += strlen (weaponFile) + 1;
        }
        
        //WriteWeaponCacheFile (weaponDataTable, numLoadedWeapons);
    }
    /*else
    {
        successful = *numLoadedWeapons;
    }*/
    
    Com_Printf ("Successfully loaded %d weapons, failed to load %d weapons.\n", successful, failed);
    Com_Printf ("Took %d milliseconds.\n", trap_Milliseconds() - t);
    Com_Printf ("-------------------------------------\n");
    
    return (qboolean)(successful > 0);
}
