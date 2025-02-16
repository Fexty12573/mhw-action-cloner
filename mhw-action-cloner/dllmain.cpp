#include "pch.h"
#include <Windows.h>
#include <util.h>
#include <loader.h>
#include "Monster.h"
#include <game_functions.h>

#include <dti/cActionController.h>
#include <dti/MtAction.h>
#include <dti/MtObject.h>
#include <Memory.hpp>

#include "ActionCloner.h"
#include "FunctionHook.h"


CreateHook(MH::ActionController::SetActionSet, SetActionSetHook, void, Monster* thisptr, u32 set, ActionTable* table, u32 count, int ac_idx) {
	using namespace loader;
    if (thisptr->id() != Monster::Banbaro) {
        original(thisptr, set, table, count, ac_idx);
        return;
    }

	if (table != nActEnemy::GetActionTable(Monster::Banbaro)) {
		original(thisptr, set, table, count, ac_idx);
		return ((void(*)(void*))0x141c18300)(thisptr);
	}

	const auto new_table = (ActionTable*)malloc(sizeof(ActionTableEntry) * (count + 1));
	if (!new_table) {
	    LOG(ERR) << "Failed to allocate memory for new action table";
        return ((void(*)(void*))0x141c18300)(thisptr);
	}

	std::memcpy(new_table, table, sizeof(ActionTableEntry) * count);
	new_table->mActions[count] = table->mActions[19]; // Threat
	new_table->mActions[count].mID = count;
	original(thisptr, set, new_table, count + 1, ac_idx);

	LOG(INFO) << "Banbaro Action Table Resized: " << count;

	const ActionTableEntry* entry = &new_table->mActions[count]; // Threat
	//ActionInfo info = {
	//    .mActionSet = set,
	//    .mActionId = count
	//};

	//const auto action = (cActionBase*)MH::ActionController::InitializeAction(
	//	&thisptr->at<cActionController>(0x61C8 + ac_idx * cActionController::Size), 
	//	&info, 
	//	entry->mDTI
	//);

	const auto action = thisptr->get<cActionController>(0x61C8 + ac_idx * cActionController::Size).GetActionList(set).mList[count];

	//action->SetParent(thisptr);
	//action->mFlags = entry->mFlags;
	action->mActionName = "ACTION::SEREGIOS_ROAR";
	//action->Initialize(thisptr);
	action->at<u32>(0x1B0) = 0x407B; // LMT ID

	free(new_table);
	LOG(INFO) << "Banbaro New Action ID: " << count;

	return ((void(*)(void*))0x141c18300)(thisptr);
}


static void initialize() {
    // Patch out a `jmp` instruction that would otherwise crash the game
    // We will execute this code manually in our hook
    memory::Patch((void*)0x141bfd056, { 0xC3, 0xCC, 0xCC, 0xCC, 0xCC }); // Old: 0x141bfc286

	try {
		ActionCloner::get()->initialize();
	}
	catch (const std::exception& e) {
		loader::LOG(loader::ERR) << "Failed to initialize ActionCloner: " << e.what();
		return;
	}

	loader::LOG(loader::INFO) << "MHWActionCloner Ready";
}

BOOL APIENTRY DllMain(HMODULE hDll, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
        initialize();
	}

	return TRUE;
}
