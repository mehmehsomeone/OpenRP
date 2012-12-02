#include "g_local.h"

//[Reload]
int ClipSize(int ammo,gentity_t *ent)
{
	switch(ammo)
	{
	case AMMO_THERMAL:
	case AMMO_DETPACK:
		case AMMO_TRIPMINE:
		return 3;
	case AMMO_TUSKEN_RIFLE:
		{/*
			int tuskenLevel = ent->client->skillLevel[SK_TUSKEN_RIFLE];
			if(tuskenLevel == FORCE_LEVEL_1)
				return 50;
			if(tuskenLevel == FORCE_LEVEL_2)
				return 60;
			if(tuskenLevel == FORCE_LEVEL_3)
				return 70;*/
			return 10; // Original: 10
		}
	case AMMO_BLASTER:
		return 51; //Original: 21
	case AMMO_ROCKETS:
		return 1;
	case AMMO_POWERCELL:
		if(ent->client->skillLevel[SK_BOWCASTER] >= ent->client->skillLevel[SK_DISRUPTOR])
		{
			switch(ent->client->skillLevel[SK_BOWCASTER])
			{
			case FORCE_LEVEL_3:
				return 30;
				break;
			case FORCE_LEVEL_2:
				return 20;
				break;
			case FORCE_LEVEL_1:
				return 10;
				break;
			}
		}
		else
		{
			switch(ent->client->skillLevel[SK_DISRUPTOR])
			{
			case FORCE_LEVEL_3:
				return 30;
				break;
			case FORCE_LEVEL_2:
				return 20;
				break;
			case FORCE_LEVEL_1:
				return 10;
				break;
			}
		}
		switch(ent->client->skillLevel[SK_DEMP])
			{
			case FORCE_LEVEL_3:
				return 30;
				break;
			case FORCE_LEVEL_2:
				return 20;
				break;
			case FORCE_LEVEL_1:
				return 10;
				break;
			}
		break;
	case AMMO_METAL_BOLTS:
		if(ent->client->skillLevel[SK_REPEATER] >= ent->client->skillLevel[SK_FLECHETTE])
		{
			switch(ent->client->skillLevel[SK_REPEATER])
			{
			case FORCE_LEVEL_3:
				return 200; //Orig: 100
				break;
			case FORCE_LEVEL_2:
				return 100; //Orig: 50
				break;
			case FORCE_LEVEL_1:
				return 50; //Orig: 20
				break;
			}
		}
		else
		{
			switch(ent->client->skillLevel[SK_FLECHETTE])
			{
			case FORCE_LEVEL_3:
				return 200; //Same as above
				break;
			case FORCE_LEVEL_2:
				return 100;
				break;
			case FORCE_LEVEL_1:
				return 50;
				break;
			}
		}
		switch(ent->client->skillLevel[SK_CONC])
			{
			case FORCE_LEVEL_3:
				return 10;
				break;
			case FORCE_LEVEL_2:
				return 5;
				break;
			case FORCE_LEVEL_1:
				return 3;
				break;
			}
		break;

	//case WP_BRYAR_PISTOL:
	//	return 12;
	}
	return -1;
}

int SkillLevelForWeap(gentity_t *ent,int weap)
{
	switch(weap)
	{
	case WP_TUSKEN_RIFLE:
		return ent->client->skillLevel[SK_TUSKEN_RIFLE];
	case WP_BRYAR_PISTOL:
		return ent->client->skillLevel[SK_PISTOL];
	case WP_BLASTER:
		return ent->client->skillLevel[SK_BLASTER];
	case WP_THERMAL:
		return ent->client->skillLevel[SK_THERMAL];
	case WP_ROCKET_LAUNCHER:
		return ent->client->skillLevel[SK_ROCKET];
	case WP_BOWCASTER:
		return ent->client->skillLevel[SK_BOWCASTER];
	case WP_DET_PACK:
		return ent->client->skillLevel[SK_DETPACK];
	case WP_REPEATER:
		return ent->client->skillLevel[SK_REPEATER];
	case WP_DISRUPTOR:
		return ent->client->skillLevel[SK_DISRUPTOR];
	case WP_FLECHETTE:
		return ent->client->skillLevel[SK_FLECHETTE];
	case WP_CONCUSSION:
		return ent->client->skillLevel[SK_CONC];
	case WP_DEMP2:
		return ent->client->skillLevel[SK_DEMP];
	default:
		return -1;
	}
}

int ReloadTime(gentity_t *ent)
{
	switch(ent->client->ps.weapon)
	{
	case WP_CONCUSSION:
		return 5500;
	case WP_DEMP2:
		return 500;
	case WP_DISRUPTOR:
	case WP_REPEATER:
	case WP_FLECHETTE:
		return 5000; //Orig: 6000
	case WP_BOWCASTER:
		return 5000; //Orig: 5000
	case WP_BLASTER:
		return 2000; //Orig: 3000
	case WP_BRYAR_PISTOL:
		return 500; //Orig: 3000
	case WP_TUSKEN_RIFLE:
		return 3000; //Orig: 10000
	case WP_ROCKET_LAUNCHER:
		return 6000;
	}
	return -1;
}

void SetupReload(gentity_t *ent)
{
	int reloadTime = ReloadTime(ent);
	if(ent->reloadCooldown > level.time)
		return;

	if(ent->client->ps.weapon == WP_MELEE || ent->client->ps.weapon == WP_SABER || ent->client->ps.weapon == WP_THERMAL ||
		ent->client->ps.weapon == WP_DET_PACK)
	{
		ent->bulletsToReload = 0;
		ent->reloadTime = -1;
		return;
	}

	if(ent->bullets[ent->client->ps.weapon] <= 0)
		return;

	ent->bulletsToReload = ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent) - ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex];

	if(ent->bulletsToReload >= ent->bullets[ent->client->ps.weapon])
		ent->bulletsToReload = ent->bullets[ent->client->ps.weapon];

	ent->reloadTime = level.time + reloadTime;
	ent->client->ps.zoomMode = 0;

	ent->client->isHacking = ent->client->ps.clientNum;
	//ent->client->isHacking = -100;
	ent->client->ps.hackingTime = level.time + reloadTime;
	ent->client->ps.hackingBaseTime = reloadTime;
}

void Reload(gentity_t *ent)
{
	//int clipsize = ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent) - ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex];
	if(ent->bullets[ent->client->ps.weapon] < 1 || ent->bulletsToReload < 1
		|| ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
	{
		ent->bulletsToReload = 0;
		ent->reloadTime = -1;
		return;
	}

/*
	if(ent->client->ps.weapon == WP_REPEATER && ent->client->skillLevel[SK_REPEATERUPGRADE] > FORCE_LEVEL_0)
	{
		int i;
		for(i=0;i<4;i++)
		{
		ent->bullets[ent->client->ps.weapon]--;
		ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
		ent->bulletsToReload--;
		ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
		if(ent->bullets[ent->client->ps.weapon] < 1)
		return;
		if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
			return;
		}
		ent->reloadTime = level.time + ReloadTime(ent);
	}
	else if(ent->client->ps.weapon == WP_REPEATER)
	{
		int i;
		for(i=0; i<3; i++)
		{
		ent->bullets[ent->client->ps.weapon]--;
		ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
		ent->bulletsToReload--;
		ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
		if(ent->bullets[ent->client->ps.weapon] < 1)
		return;
		if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
			return;
		}
		ent->reloadTime = level.time + ReloadTime(ent);
	}
	else if(ent->client->ps.weapon == WP_FLECHETTE)
	{
		if(SkillLevelForWeap(ent,WP_FLECHETTE) == FORCE_LEVEL_1)
		{
			int i;
			for(i=0; i<5; i++)
			{
				ent->bullets[ent->client->ps.weapon]--;
				ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
				ent->bulletsToReload--;
				ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
				if(ent->bullets[ent->client->ps.weapon] < 1)
					return;
				if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
					return;
			}		
		}
		else if(SkillLevelForWeap(ent,WP_FLECHETTE) == FORCE_LEVEL_2)
		{
			int i;
			for(i=0; i<10; i++)
			{
				ent->bullets[ent->client->ps.weapon]--;
				ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
				ent->bulletsToReload--;
				ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
				if(ent->bullets[ent->client->ps.weapon] < 1)
					return;
				if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
					return;
			}		
		}
		else if(SkillLevelForWeap(ent,WP_FLECHETTE) == FORCE_LEVEL_3)
		{
			int i;
			for(i=0; i<15; i++)
			{
				ent->bullets[ent->client->ps.weapon]--;
				ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
				ent->bulletsToReload--;
				ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
				if(ent->bullets[ent->client->ps.weapon] < 1)
					return;
				if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] >= ClipSize(weaponData[ent->client->ps.weapon].ammoIndex,ent))
					return;
			}		
		}
		ent->reloadTime = level.time + ReloadTime(ent);
	}
	else
	{
		ent->bullets[ent->client->ps.weapon]--;
		ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]++;
		ent->reloadTime = level.time + ReloadTime(ent);
		ent->bulletsToReload--;
	}*/

	ent->bullets[ent->client->ps.weapon] -= ent->bulletsToReload;
	ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex] += ent->bulletsToReload;
	ent->bulletsToReload = 0;
	ent->reloadTime = -1;
	ent->client->ps.stats[STAT_AMMOPOOL] = ent->bullets[ent->client->ps.weapon];
	ent->client->ps.weaponTime = 0;
	ent->client->ps.torsoTimer = 0;
}

void CancelReload(gentity_t *ent)
{
	ent->reloadTime = -1;
	ent->reloadCooldown = level.time + 3000;
	ent->client->isHacking = 0;
	ent->client->ps.weaponTime = 0;
	ent->client->ps.torsoTimer = 0;
}
//[/Reload]