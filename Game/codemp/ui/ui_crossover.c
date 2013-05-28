/*================================================================
//
//  UI <--> CGame crossover module
//  Allows UI and CGame to directly interact with eachother
//
//  Used in Jedi Knight Galaxies to enable the UI to directly communicate with the server
//
//  IMPORTANT: If you plan to use syscalls, use the CO_SysCall_** functions before you do so!
//
//  By BobaFett
//
//================================================================*/

// System concept: (updated)
// At startup, both UI and Cgame as linked up the auxiliary library
// To make the crossover system work, the crossover structures are
// sent to the auxiliary library, where the other module can obtain them
//
// Once a module is unloaded, it simply breaks the link with the aux-lib,
// causing the crossover interface to be discarded

// UI Part

static int coTrapEscape = 0;

#include "ui_local.h"
#include "ui_shared.h"
#include "ui_public.h"
#include "ui_jkgscript.h"

#include "aux_ui_exports.h"
//extern gl_ui_imports_t *gl_ui_imports;

struct {	
	// Filled by UI
	void (* DoAntiCheat)();
	qboolean (* HandleServerCommand)(const char *command);
	void (* SetEscapeTrap)(int activate);
	void (* PartyMngtNotify)(int msg);
	
	void (* InventoryNotify)(int msg);
	void (* ShopNotify)(int msg);
	weaponData_t *(* GetWeaponDatas)(unsigned char weapon, unsigned char variation);
} ui_crossover;

struct {
	// Filled by Cgame
	void (* SendClientCommand)(const char *command);
	qboolean (* EscapeTrapped)();
	void *(* PartyMngtDataRequest)(int data);
	void *(* InventoryDataRequest)(int data);
	void *(* InventoryAttachToACI)(int itemNum, int slot, int attach);
	weaponData_t *(* GetWeaponDatas)(unsigned char weapon, unsigned char variation);
	int (* GetRedTeam)(void);
	int (* GetBlueTeam)(void);
} * cg_crossover;

void CO_SysCall_UI() {
	// Enable UI syscalls
#ifdef WIN32
	// Engine hack
	*(int *)0x12A4F18 = *(int *)0xB28224; // CurrentVm = uivm;
#else
	#error "CO_SysCall_UI: LINUX/MAC"
#endif
}
void CO_SysCall_CG() {
	// Enable CGame syscalls
#ifdef WIN32
	// Engine hack
	*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
#else
	#error "CO_SysCall_CG: LINUX/MAC"
#endif
}

void CO_BreakLinkup() {
	/*if (gl_ui_imports) {
		gl_ui_imports->GL_RegisterCrossover(NULL);
	}*/
}

qboolean CO_HandleServerCommand(const char *command) {

	return UI_RunSvCommand(command);
}

int CO_GetRedTeam(void)
{
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if(cg_crossover)
		{
			return cg_crossover->GetRedTeam();
		}
	}
	return -1;*/
}

int CO_GetBlueTeam(void)
{
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if(cg_crossover)
		{
			return cg_crossover->GetBlueTeam();
		}
	}
	return -1;*/
}

void CO_SendClientCommand(const char *cmd) {
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			cg_crossover->SendClientCommand(cmd);
		}
	}*/
}

void *CO_PartyMngtDataRequest(int data) {
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			return cg_crossover->PartyMngtDataRequest(data);
		}
	}
	return 0;*/
}

void *CO_InventoryDataRequest(int data) {
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			return cg_crossover->InventoryDataRequest(data);
		}
	}
	return 0;*/
}

weaponData_t *CO_GetWeaponDatas ( unsigned char weapon, unsigned char variation ) {
	/*if(gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			return cg_crossover->GetWeaponDatas ( weapon, variation );
		}
	}
	return NULL;*/
}

void CO_InventoryAttachToACI(int itemNum, int slot, int attach) {
	/*if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			cg_crossover->InventoryAttachToACI(itemNum, slot, attach);
		}
	}*/
}

qboolean CO_EscapeTrapped() {
	// Return 1 to block, 0 to pass through
	/*if (!coTrapEscape) {
		return 0;
	}
	if (gl_ui_imports) {
		cg_crossover = gl_ui_imports->GL_GetCrossover();
		if (cg_crossover) {
			return cg_crossover->EscapeTrapped();
		}
	}
	return 0;*/
}

void CO_EscapeTrap(int activate) {
	coTrapEscape = activate;
}

void JKG_PartyMngt_UpdateNotify(int msg);
void JKG_Inventory_UpdateNotify(int msg);
void JKG_Shop_UpdateNotify(int msg);

void CO_InitCrossover() {
	/*ui_crossover.HandleServerCommand = CO_HandleServerCommand;
	ui_crossover.DoAntiCheat = CO_RunAntiCheat;
	ui_crossover.SetEscapeTrap = CO_EscapeTrap;
	ui_crossover.PartyMngtNotify = JKG_PartyMngt_UpdateNotify;
	ui_crossover.InventoryNotify = JKG_Inventory_UpdateNotify;
	ui_crossover.ShopNotify = JKG_Shop_UpdateNotify;
	ui_crossover.GetWeaponDatas = CO_GetWeaponDatas;

	// Transmit this structure to the auxlib so UI can use it
	if (gl_ui_imports) {	// This should never be NULL, but just in case
		gl_ui_imports->GL_RegisterCrossover(&ui_crossover);
	}*/
}