#include "source/GcLib/pch.h"

#include "StgControlScript.hpp"
#include "StgSystem.hpp"

#include "../DnhExecutor/GcLibImpl.hpp"
#include "./RoEW/DxObjectRoEW.hpp"

//*******************************************************************
//StgControlScriptManager
//*******************************************************************
StgControlScriptManager::StgControlScriptManager() {

}
StgControlScriptManager::~StgControlScriptManager() {

}

//*******************************************************************
//StgControlScriptInformation
//*******************************************************************
StgControlScriptInformation::StgControlScriptInformation() {
	replayManager_ = new ReplayInformationManager();
}
StgControlScriptInformation::~StgControlScriptInformation() {}
void StgControlScriptInformation::LoadFreePlayerList() {
	const std::wstring& dir = EPathProperty::GetPlayerScriptRootDirectory();
	listFreePlayer_ = ScriptInformation::FindPlayerScriptInformationList(dir);

	std::sort(listFreePlayer_.begin(), listFreePlayer_.end(), ScriptInformation::PlayerListSort());
}
void StgControlScriptInformation::LoadReplayInformation(std::wstring pathMainScript) {
	replayManager_->UpdateInformationList(pathMainScript);
}

//*******************************************************************
//StgControlScript
//*******************************************************************
static const std::vector<function> stgControlFunction = {
	//関数：
	//STG制御共通関数：共通データ
	{ "SaveCommonDataAreaA1", StgControlScript::Func_SaveCommonDataAreaA1, 1 },
	{ "LoadCommonDataAreaA1", StgControlScript::Func_LoadCommonDataAreaA1, 1 },
	{ "SaveCommonDataAreaA2", StgControlScript::Func_SaveCommonDataAreaA2, 2 },
	{ "LoadCommonDataAreaA2", StgControlScript::Func_LoadCommonDataAreaA2, 2 },

	//STG制御共通関数：キー系
	{ "AddVirtualKey", StgControlScript::Func_AddVirtualKey, 3 },
	{ "AddReplayTargetVirtualKey", StgControlScript::Func_AddReplayTargetVirtualKey, 1 },
	{ "SetSkipModeKey", StgControlScript::Func_SetSkipModeKey, 1 },

	//STG制御共通関数：システム関連
	{ "GetScore", StgControlScript::Func_StgStageInformation_int64_void<&StgStageInformation::GetScore>, 0 },
	{ "SetScore", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::SetScore>, 1 },
	{ "AddScore", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::AddScore>, 1 },
	{ "GetGraze", StgControlScript::Func_StgStageInformation_int64_void<&StgStageInformation::GetGraze>, 0 },
	{ "SetGraze", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::SetGraze>, 1 },
	{ "AddGraze", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::AddGraze>, 1 },
	{ "GetPoint", StgControlScript::Func_StgStageInformation_int64_void<&StgStageInformation::GetPoint>, 0 },
	{ "SetPoint", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::SetPoint>, 1 },
	{ "AddPoint", StgControlScript::Func_StgStageInformation_void_int64<&StgStageInformation::AddPoint>, 1 },

	{ "IsReplay", StgControlScript::Func_IsReplay, 0 },

	{ "AddArchiveFile", StgControlScript::Func_AddArchiveFile, 1 },
	{ "AddArchiveFile", StgControlScript::Func_AddArchiveFile, 2 },		//Overloaded
	{ "GetArchiveFilePathList", StgControlScript::Func_GetArchiveFilePathList, 2 },

	{ "GetCurrentFps", StgControlScript::Func_GetCurrentRenderFps, 0 },
	{ "GetCurrentUpdateFps", StgControlScript::Func_GetCurrentUpdateFps, 0 },
	{ "GetCurrentRenderFps", StgControlScript::Func_GetCurrentRenderFps, 0 },
	{ "GetLastFrameUpdateSpeed", StgControlScript::Func_GetLastFrameUpdateSpeed, 0 },
	{ "GetLastFrameRenderSpeed", StgControlScript::Func_GetLastFrameRenderSpeed, 0 },
	{ "GetStageTime", StgControlScript::Func_GetStageTime, 0 },
	{ "GetStageTimeF", StgControlScript::Func_GetStageTimeF, 0 },
	{ "GetPackageTime", StgControlScript::Func_GetPackageTime, 0 },

	{ "GetStgFrameLeft", StgControlScript::Func_GetStgFrameLeft, 0 },
	{ "GetStgFrameTop", StgControlScript::Func_GetStgFrameTop, 0 },
	{ "GetStgFrameWidth", StgControlScript::Func_GetStgFrameWidth, 0 },
	{ "GetStgFrameHeight", StgControlScript::Func_GetStgFrameHeight, 0 },

	{ "GetMainPackageScriptPath", StgControlScript::Func_GetMainPackageScriptPath, 0 },
	{ "GetScriptPathList", StgControlScript::Func_GetScriptPathList, 2 },
	{ "GetScriptInfoA1", StgControlScript::Func_GetScriptInfoA1, 2 },

	//Engine utility
	{ "IsEngineFastMode", StgControlScript::Func_IsEngineFastMode, 0 },
	{ "GetConfigWindowSizeIndex", StgControlScript::Func_GetConfigWindowSizeIndex, 0 },
	{ "GetConfigWindowSizeList", StgControlScript::Func_GetConfigWindowSizeList, 0 },
	{ "GetConfigVirtualKeyMapping", StgControlScript::Func_GetConfigVirtualKeyMapping, 1 },
	{ "GetConfigWindowTitle", StgControlScript::Func_GetConfigWindowTitle, 0 },
	{ "SetWindowTitle", StgControlScript::Func_SetWindowTitle, 1 },
	{ "SetEnableUnfocusedProcessing", StgControlScript::Func_SetEnableUnfocusedProcessing, 1 },
	{ "IsWindowFocused", StgControlScript::Func_IsWindowFocused, 0 },

	//STG共通関数：描画関連
	{ "ClearInvalidRenderPriority", StgControlScript::Func_ClearInvalidRenderPriority, 0 },
	{ "SetInvalidRenderPriorityA1", StgControlScript::Func_SetInvalidRenderPriorityA1, 2 },
	{ "GetReservedRenderTargetName", StgControlScript::Func_GetReservedRenderTargetName, 1 },
	{ "RenderToTextureA1", StgControlScript::Func_RenderToTextureA1, 4 },
	{ "RenderToTextureB1", StgControlScript::Func_RenderToTextureB1, 3 },
	{ "SaveSnapShotA1", StgControlScript::Func_SaveSnapShotA1, 1 },
	{ "SaveSnapShotA2", StgControlScript::Func_SaveSnapShotA2, 5 },
	{ "SaveSnapShotA3", StgControlScript::Func_SaveSnapShotA3, 6 },

	//STG制御共通関数：自機関連
	{ "GetPlayerID", StgControlScript::Func_GetPlayerID, 0 },
	{ "GetPlayerReplayName", StgControlScript::Func_GetPlayerReplayName, 0 },

	//STG制御共通関数：ユーザスクリプト
	{ "SetPauseScriptPath", StgControlScript::Func_SetPauseScriptPath, 1 },
	{ "SetEndSceneScriptPath", StgControlScript::Func_SetEndSceneScriptPath, 1 },
	{ "SetReplaySaveSceneScriptPath", StgControlScript::Func_SetReplaySaveSceneScriptPath, 1 },

	//STG制御共通関数：自機スクリプト
	{ "GetLoadFreePlayerScriptList", StgControlScript::Func_GetLoadFreePlayerScriptList, 0 },
	{ "GetFreePlayerScriptCount", StgControlScript::Func_GetFreePlayerScriptCount, 0 },
	{ "GetFreePlayerScriptInfo", StgControlScript::Func_GetFreePlayerScriptInfo, 2 },

	//STG制御共通関数：リプレイ関連
	{ "LoadReplayList", StgControlScript::Func_LoadReplayList, 0 },
	{ "GetValidReplayIndices", StgControlScript::Func_GetValidReplayIndices, 0 },
	{ "IsValidReplayIndex", StgControlScript::Func_IsValidReplayIndex, 1 },
	{ "GetReplayInfo", StgControlScript::Func_GetReplayInfo, 2 },
	{ "SetReplayInfo", StgControlScript::Func_SetReplayInfo, 2 },
	{ "GetReplayUserData", StgControlScript::Func_GetReplayUserData, 2 },
	{ "SetReplayUserData", StgControlScript::Func_SetReplayUserData, 2 },
	{ "IsReplayUserDataExists", StgControlScript::Func_IsReplayUserDataExists, 2 },
	{ "SaveReplay", StgControlScript::Func_SaveReplay, 2 },

	// RoEW specific stuff!!
	{ "SetSkipRate", StgControlScript::Func_SetSkipRate, 1 },
	{ "SetStdFrameRate", StgControlScript::Func_SetStdFrameRate, 1 },
	{ "ToggleSkipMode", StgControlScript::Func_ToggleSkipMode, 1 },
	
	{ "Config_SetVkeyMap", StgControlScript::Func_Config_SetVKeyMap, 4 },
	{ "Config_SetResIndex", StgControlScript::Func_Config_SetResIndex, 1 },
	{ "Config_SetFullscreen", StgControlScript::Func_Config_SetFullscreen, 1 },
	{ "Config_SetVsync", StgControlScript::Func_Config_SetVsync, 1 },
	{ "Config_SetPseudoFs", StgControlScript::Func_Config_SetPseudoFs, 1 },

	{ "Config_GetVkeyMap", StgControlScript::Func_Config_GetVKeyMap, 1 },
	{ "Config_GetResIndex", StgControlScript::Func_Config_GetResIndex, 0 },
	{ "Config_GetFullscreen", StgControlScript::Func_Config_GetFullscreen, 0 },
	{ "Config_GetVsync", StgControlScript::Func_Config_GetVsync, 0 },
	{ "Config_GetPseudoFs", StgControlScript::Func_Config_GetPseudoFs, 0 },

	{ "SaveConfigFile", StgControlScript::Func_SaveConfigFile, 0 },

	{ "ObjMenu_Create", StgControlScript::Func_ObjMenu_Create, 0 },
	{ "ObjMenu_Regist", StgControlScript::Func_ObjMenu_Regist, 1 },

	{ "ObjMenu_GetParent", StgControlScript::Func_ObjMenu_GetParent, 1 },
	{ "ObjMenu_GetDisabled", StgControlScript::Func_ObjMenu_GetDisabled, 1 },
	{ "ObjMenu_GetRelatedObject", StgControlScript::Func_ObjMenu_GetRelatedObject, 2 },
	{ "ObjMenu_GetOptionIndex", StgControlScript::Func_ObjMenu_GetOptionIndex, 1 },
	{ "ObjMenu_GetOptionIndexX", StgControlScript::Func_ObjMenu_GetOptionIndexX, 2 },
	{ "ObjMenu_GetMaxIndex", StgControlScript::Func_ObjMenu_GetMaxIndex, 1 },
	{ "ObjMenu_GetMaxIndexX", StgControlScript::Func_ObjMenu_GetMaxIndexX, 2 },
	{ "ObjMenu_GetSliderValue", StgControlScript::Func_ObjMenu_GetSliderValue, 2 },
	{ "ObjMenu_GetSliderMax", StgControlScript::Func_ObjMenu_GetSliderMax, 2 },
	{ "ObjMenu_GetSliderMin", StgControlScript::Func_ObjMenu_GetSliderMin, 2 },
	{ "ObjMenu_GetSliderIncr", StgControlScript::Func_ObjMenu_GetSliderIncr, 2 },
	{ "ObjMenu_GetOptionType", StgControlScript::Func_ObjMenu_GetOptionType, 2 },
	{ "ObjMenu_GetActionFlag", StgControlScript::Func_ObjMenu_GetActionFlag, 1 },
	{ "ObjMenu_GetReturnValue", StgControlScript::Func_ObjMenu_GetReturnValue, 1 },
	{ "ObjMenu_GetKeyboardInput", StgControlScript::Func_ObjMenu_GetKeyboardInput, 1 },

	{ "ForceCloseAllMenus", StgControlScript::Func_ForceCloseAllMenus, 0 },

	{ "ObjMenu_SetParent", StgControlScript::Func_ObjMenu_SetParent, 2 },
	{ "ObjMenu_AddRelatedObject", StgControlScript::Func_ObjMenu_AddRelatedObject, 2 },
	{ "ObjMenu_SetMaxIndex", StgControlScript::Func_ObjMenu_SetMaxIndex, 2 },
	{ "ObjMenu_SetMaxIndexX", StgControlScript::Func_ObjMenu_SetMaxIndexX, 3 },
	{ "ObjMenu_SetSliderMax", StgControlScript::Func_ObjMenu_SetSliderMax, 3 },
	{ "ObjMenu_SetSliderMin", StgControlScript::Func_ObjMenu_SetSliderMin, 3 },
	{ "ObjMenu_SetSliderIncr", StgControlScript::Func_ObjMenu_SetSliderIncr, 3 },
	{ "ObjMenu_SetOptionType", StgControlScript::Func_ObjMenu_SetOptionType, 3 },
	{ "ObjMenu_SetReturnValue", StgControlScript::Func_ObjMenu_SetReturnValue, 2 },
};
static const std::vector<constant> stgControlConstant = {
	//Events
	constant("EV_USER_COUNT", StgControlScript::EV_USER_COUNT),
	constant("EV_USER", StgControlScript::EV_USER),
	constant("EV_USER_SYSTEM", StgControlScript::EV_USER_SYSTEM),
	constant("EV_USER_STAGE", StgControlScript::EV_USER_STAGE),
	constant("EV_USER_PLAYER", StgControlScript::EV_USER_PLAYER),
	constant("EV_USER_PACKAGE", StgControlScript::EV_USER_PACKAGE),

	constant("EV_APP_LOSE_FOCUS", StgControlScript::EV_APP_LOSE_FOCUS),
	constant("EV_APP_RESTORE_FOCUS", StgControlScript::EV_APP_RESTORE_FOCUS),

	//GetScriptInfoA1 script types
	constant("TYPE_SCRIPT_ALL", StgControlScript::TYPE_SCRIPT_ALL),
	constant("TYPE_SCRIPT_PLAYER", StgControlScript::TYPE_SCRIPT_PLAYER),
	constant("TYPE_SCRIPT_SINGLE", StgControlScript::TYPE_SCRIPT_SINGLE),
	constant("TYPE_SCRIPT_PLURAL", StgControlScript::TYPE_SCRIPT_PLURAL),
	constant("TYPE_SCRIPT_STAGE", StgControlScript::TYPE_SCRIPT_STAGE),
	constant("TYPE_SCRIPT_PACKAGE", StgControlScript::TYPE_SCRIPT_PACKAGE),

	//Script infos
	constant("INFO_SCRIPT_TYPE", StgControlScript::INFO_SCRIPT_TYPE),
	constant("INFO_SCRIPT_PATH", StgControlScript::INFO_SCRIPT_PATH),
	constant("INFO_SCRIPT_ID", StgControlScript::INFO_SCRIPT_ID),
	constant("INFO_SCRIPT_TITLE", StgControlScript::INFO_SCRIPT_TITLE),
	constant("INFO_SCRIPT_TEXT", StgControlScript::INFO_SCRIPT_TEXT),
	constant("INFO_SCRIPT_IMAGE", StgControlScript::INFO_SCRIPT_IMAGE),
	constant("INFO_SCRIPT_REPLAY_NAME", StgControlScript::INFO_SCRIPT_REPLAY_NAME),

	//Replay data infos
	constant("REPLAY_FILE_PATH", StgControlScript::REPLAY_FILE_PATH),
	constant("REPLAY_DATE_TIME", StgControlScript::REPLAY_DATE_TIME),
	constant("REPLAY_USER_NAME", StgControlScript::REPLAY_USER_NAME),
	constant("REPLAY_TOTAL_SCORE", StgControlScript::REPLAY_TOTAL_SCORE),
	constant("REPLAY_FPS_AVERAGE", StgControlScript::REPLAY_FPS_AVERAGE),
	constant("REPLAY_PLAYER_NAME", StgControlScript::REPLAY_PLAYER_NAME),
	constant("REPLAY_STAGE_INDEX_LIST", StgControlScript::REPLAY_STAGE_INDEX_LIST),
	constant("REPLAY_STAGE_START_SCORE_LIST", StgControlScript::REPLAY_STAGE_START_SCORE_LIST),
	constant("REPLAY_STAGE_LAST_SCORE_LIST", StgControlScript::REPLAY_STAGE_LAST_SCORE_LIST),
	constant("REPLAY_COMMENT", StgControlScript::REPLAY_COMMENT),

	//Replay index
	constant("REPLAY_INDEX_ACTIVE", ReplayInformation::INDEX_ACTIVE),
	constant("REPLAY_INDEX_DIGIT_MIN", ReplayInformation::INDEX_DIGIT_MIN),
	constant("REPLAY_INDEX_DIGIT_MAX", ReplayInformation::INDEX_DIGIT_MAX),
	constant("REPLAY_INDEX_USER", ReplayInformation::INDEX_USER),

	//Common script results
	constant("RESULT_CANCEL", StgControlScript::RESULT_CANCEL),
	constant("RESULT_END", StgControlScript::RESULT_END),
	constant("RESULT_RETRY", StgControlScript::RESULT_RETRY),
	constant("RESULT_SAVE_REPLAY", StgControlScript::RESULT_SAVE_REPLAY),

	// RoEW!!
	constant("MENU_TYPE_INVALID", 0),
	constant("MENU_TYPE_KEYBOARD", 1),
	constant("MENU_TYPE_XAXIS", 2),
	constant("MENU_TYPE_SLIDER", 3),
	constant("MENU_TYPE_MAIN", 4),
	constant("MENU_TYPE_NORMAL", 5),
};

StgControlScript::StgControlScript(StgSystemController* systemController) {
	systemController_ = systemController;

	_AddFunction(&stgControlFunction);
	_AddConstant(&stgControlConstant);

	menuManager_ = std::shared_ptr<DxMenuObjectManager>(new DxMenuObjectManager());

	SetScriptEngineCache(systemController->GetScriptEngineCache());
}

//STG制御共通関数：共通データ
gstd::value StgControlScript::Func_SaveCommonDataAreaA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring area = argv[0].as_string();
	std::string sArea = StringUtility::ConvertWideToMulti(area);
	ScriptCommonDataManager* commonDataManager = ScriptCommonDataManager::GetInstance();

	bool res = false;

	shared_ptr<ScriptCommonData> commonData = commonDataManager->GetData(sArea);
	if (commonData) {
		const std::wstring& pathMain = infoSystem->GetMainScriptInformation()->pathScript_;
		std::wstring pathSave = EPathProperty::GetCommonDataPath(pathMain, area);
		std::wstring dirSave = PathProperty::GetFileDirectory(pathSave);

		File::CreateFileDirectory(dirSave);

		RecordBuffer record;
		commonData->WriteRecord(record);
		res = record.WriteToFile(pathSave, GAME_VERSION_NUM, 
			ScriptCommonData::HEADER_SAVED_DATA, ScriptCommonData::HEADER_SAVED_DATA_SIZE);
	}

	return script->CreateBooleanValue(res);
}
gstd::value StgControlScript::Func_LoadCommonDataAreaA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring area = argv[0].as_string();
	std::string sArea = StringUtility::ConvertWideToMulti(area);
	ScriptCommonDataManager* commonDataManager = ScriptCommonDataManager::GetInstance();

	bool res = false;

	const std::wstring& pathMain = infoSystem->GetMainScriptInformation()->pathScript_;
	std::wstring pathSave = EPathProperty::GetCommonDataPath(pathMain, area);

	RecordBuffer record;
	res = record.ReadFromFile(pathSave, GAME_VERSION_NUM, 
		ScriptCommonData::HEADER_SAVED_DATA, ScriptCommonData::HEADER_SAVED_DATA_SIZE);
	if (res) {
		shared_ptr<ScriptCommonData> commonData(new ScriptCommonData());
		commonData->ReadRecord(record);
		commonDataManager->SetData(sArea, commonData);
	}

	return script->CreateBooleanValue(res);
}

gstd::value StgControlScript::Func_SaveCommonDataAreaA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::string area = StringUtility::ConvertWideToMulti(argv[0].as_string());
	ScriptCommonDataManager* commonDataManager = ScriptCommonDataManager::GetInstance();

	bool res = false;

	shared_ptr<ScriptCommonData> commonData = commonDataManager->GetData(area);
	if (commonData) {
		std::wstring pathSave = argv[1].as_string();
		std::wstring dirSave = PathProperty::GetFileDirectory(pathSave);

		File::CreateFileDirectory(dirSave);

		RecordBuffer record;
		commonData->WriteRecord(record);
		res = record.WriteToFile(pathSave, GAME_VERSION_NUM, 
			ScriptCommonData::HEADER_SAVED_DATA, ScriptCommonData::HEADER_SAVED_DATA_SIZE);
	}

	return script->CreateBooleanValue(res);
}
gstd::value StgControlScript::Func_LoadCommonDataAreaA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::string area = StringUtility::ConvertWideToMulti(argv[0].as_string());
	ScriptCommonDataManager* commonDataManager = ScriptCommonDataManager::GetInstance();

	bool res = false;

	std::wstring pathSave = argv[1].as_string();
	RecordBuffer record;
	res = record.ReadFromFile(pathSave, GAME_VERSION_NUM, 
		ScriptCommonData::HEADER_SAVED_DATA, ScriptCommonData::HEADER_SAVED_DATA_SIZE);
	if (res) {
		shared_ptr<ScriptCommonData> commonData(new ScriptCommonData());
		commonData->ReadRecord(record);
		commonDataManager->SetData(area, commonData);
	}

	return script->CreateBooleanValue(res);
}

//STG制御共通関数：キー系
gstd::value StgControlScript::Func_AddVirtualKey(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	EDirectInput* input = EDirectInput::GetInstance();
	int padIndex = input->GetPadIndex();

	int16_t id = argv[0].as_int();
	int16_t key = argv[1].as_int();
	int16_t padButton = argv[2].as_int();

	ref_count_ptr<VirtualKey> vkey = new VirtualKey(key, padIndex, padButton);
	input->AddKeyMap(id, vkey);

	return value();
}
gstd::value StgControlScript::Func_AddReplayTargetVirtualKey(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();

	int16_t id = argv[0].as_int();
	infoSystem->AddReplayTargetKey(id);
	if (stageController) {
		ref_count_ptr<KeyReplayManager> keyReplayManager = stageController->GetKeyReplayManager();
		keyReplayManager->AddTarget(id);
	}

	return value();
}
gstd::value StgControlScript::Func_SetSkipModeKey(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	EFpsController* fpsController = EFpsController::GetInstance();
	fpsController->SetFastModeKey((int16_t)argv[0].as_float());
	return value();
}

//STG制御共通関数：システム関連
template<int64_t(StgStageInformation::* Func)()>
gstd::value StgControlScript::Func_StgStageInformation_int64_void(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	int64_t res = 0;

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController) {
		auto stgInfo = stageController->GetStageInformation().get();
		res = (stgInfo->*Func)();
	}

	return script->CreateIntValue(res);
}
template<void(StgStageInformation::* Func)(int64_t)>
gstd::value StgControlScript::Func_StgStageInformation_void_int64(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController) {
		int64_t inc = argv[0].as_int();
		auto stgInfo = stageController->GetStageInformation().get();
		(stgInfo->*Func)(inc);
	}

	return value();
}

gstd::value StgControlScript::Func_IsReplay(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	bool res = false;

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController)
		res = stageController->GetStageInformation()->IsReplay();

	return script->CreateBooleanValue(res);
}
gstd::value StgControlScript::Func_AddArchiveFile(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	FileManager* fileManager = FileManager::GetBase();
	std::wstring path = argv[0].as_string();
	size_t readOff = argc > 1 ? argv[1].as_int() : 0;
	bool res = fileManager->AddArchiveFile(path, readOff);
	return StgControlScript::CreateBooleanValue(res);
}
gstd::value StgControlScript::Func_GetArchiveFilePathList(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	FileManager* fileManager = FileManager::GetBase();

	std::vector<std::wstring> pathList;
	std::wstring name = argv[0].as_string();
	bool bExtendPath = argv[1].as_boolean();

	shared_ptr<ArchiveFile> archive = fileManager->GetArchiveFile(name);
	if (archive) {
		std::wstring archiveBaseDir = PathProperty::GetFileDirectory(archive->GetPath());

		auto mapFileArchive = archive->GetEntryMap();
		for (auto itr = mapFileArchive.begin(); itr != mapFileArchive.end(); ++itr) {
			shared_ptr<ArchiveFileEntry> entry = itr->second;
			std::wstring path = entry->directory + entry->name;
			if (bExtendPath) path = archiveBaseDir + path;
			pathList.push_back(PathProperty::ReplaceYenToSlash(path));
		}
	}

	return StgControlScript::CreateStringArrayValue(pathList);
}
gstd::value StgControlScript::Func_GetCurrentUpdateFps(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	EFpsController* fpsController = EFpsController::GetInstance();
	return StgControlScript::CreateFloatValue(fpsController->GetCurrentWorkFps());
}
gstd::value StgControlScript::Func_GetCurrentRenderFps(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	EFpsController* fpsController = EFpsController::GetInstance();
	return StgControlScript::CreateFloatValue(fpsController->GetCurrentRenderFps());
}
gstd::value StgControlScript::Func_GetLastFrameUpdateSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	ETaskManager* taskManager = ETaskManager::GetInstance();
	return StgControlScript::CreateIntValue(taskManager->SetWorkTime());
}
gstd::value StgControlScript::Func_GetLastFrameRenderSpeed(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	ETaskManager* taskManager = ETaskManager::GetInstance();
	return StgControlScript::CreateIntValue(taskManager->GetRenderTime());
}
gstd::value StgControlScript::Func_GetStageTime(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	uint64_t res = 0;

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController) {
		ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

		uint64_t time = SystemUtility::GetCpuTime2();
		uint64_t timeStart = infoStage->GetStageStartTime();
		res = (timeStart > 0) ? time - timeStart : 0;
	}

	return script->CreateIntValue(res);
}
gstd::value StgControlScript::Func_GetStageTimeF(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	DWORD res = 0;

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController) {
		ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
		res = infoStage->GetCurrentFrame();
	}

	return script->CreateIntValue(res);
}
gstd::value StgControlScript::Func_GetPackageTime(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	DWORD res = 0;

	StgPackageController* packageController = script->systemController_->GetPackageController();
	if (packageController) {
		ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();

		uint64_t time = SystemUtility::GetCpuTime2();
		uint64_t timeStart = infoPackage->GetPackageStartTime();
		res = (timeStart > 0) ? time - timeStart : 0;
	}

	return script->CreateIntValue(res);
}

gstd::value StgControlScript::Func_GetStgFrameLeft(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();

	LONG res = 0;
	if (stageController)
		res = stageController->GetStageInformation()->GetStgFrameRect()->left;

	return script->CreateFloatValue(res);
}
gstd::value StgControlScript::Func_GetStgFrameTop(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();

	LONG res = 0;
	if (stageController)
		res = stageController->GetStageInformation()->GetStgFrameRect()->top;

	return script->CreateFloatValue(res);
}
gstd::value StgControlScript::Func_GetStgFrameWidth(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();

	LONG res = 0;
	if (stageController) {
		DxRect<LONG>* rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = rect->right - rect->left;
	}

	return script->CreateFloatValue(res);
}
gstd::value StgControlScript::Func_GetStgFrameHeight(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();

	LONG res = 0;
	if (stageController) {
		DxRect<LONG>* rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = rect->bottom - rect->top;
	}

	return script->CreateFloatValue(res);
}
gstd::value StgControlScript::Func_GetMainPackageScriptPath(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	StgPackageController* packageController = script->systemController_->GetPackageController();

	std::wstring path = L"";
	if (packageController) {
		ref_count_ptr<ScriptInformation> infoScript =
			packageController->GetPackageInformation()->GetMainScriptInformation();
		path = infoScript->pathScript_;
	}

	return script->CreateStringValue(path);
}
gstd::value StgControlScript::Func_GetScriptPathList(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	std::vector<std::wstring> listRes;
	std::wstring dir = argv[0].as_string();
	dir = PathProperty::GetFileDirectory(dir);

	int typeScript = argv[1].as_int();
	std::vector<std::wstring> listFile = File::GetFilePathList(dir);
	for (auto itr = listFile.begin(); itr != listFile.end(); ++itr) {
		std::wstring path = *itr;

		//明らかに関係なさそうな拡張子は除外
		std::wstring ext = PathProperty::GetFileExtension(path);
		if (ScriptInformation::IsExcludeExtention(ext)) continue;

		path = PathProperty::GetUnique(path);
		ref_count_ptr<ScriptInformation> infoScript = ScriptInformation::CreateScriptInformation(path, true);
		if (infoScript == nullptr) continue;
		if (typeScript != TYPE_SCRIPT_ALL && typeScript != infoScript->type_) continue;

		script->mapScriptInfo_[path] = infoScript;
		listRes.push_back(path);
	}

	return script->CreateStringArrayValue(listRes);
}
gstd::value StgControlScript::Func_GetScriptInfoA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	std::wstring path = argv[0].as_string();
	int type = argv[1].as_int();

	ref_count_ptr<ScriptInformation> infoScript = nullptr;
	auto itr = script->mapScriptInfo_.find(path);
	if (itr != script->mapScriptInfo_.end())
		infoScript = itr->second;
	else {
		infoScript = ScriptInformation::CreateScriptInformation(path, true);
		script->mapScriptInfo_[path] = infoScript;
	}
	if (infoScript == nullptr)
		throw gstd::wexception(L"GetScriptInfoA1: " + ErrorUtility::GetFileNotFoundErrorMessage(path, true));

	value res;
	switch (type) {
	case INFO_SCRIPT_TYPE:
		res = script->CreateIntValue(infoScript->type_);
		break;
	case INFO_SCRIPT_PATH:
		res = script->CreateStringValue(infoScript->pathScript_);
		break;
	case INFO_SCRIPT_ID:
		res = script->CreateStringValue(infoScript->id_);
		break;
	case INFO_SCRIPT_TITLE:
		res = script->CreateStringValue(infoScript->title_);
		break;
	case INFO_SCRIPT_TEXT:
		res = script->CreateStringValue(infoScript->text_);
		break;
	case INFO_SCRIPT_IMAGE:
		res = script->CreateStringValue(infoScript->pathImage_);
		break;
	case INFO_SCRIPT_REPLAY_NAME:
		res = script->CreateStringValue(infoScript->replayName_);
		break;
	}

	return res;
}

gstd::value StgControlScript::Func_IsEngineFastMode(script_machine* machine, int argc, const value* argv) {
	EFpsController* controller = EFpsController::GetInstance();
	return StgControlScript::CreateBooleanValue(controller->IsFastMode());
}
gstd::value StgControlScript::Func_GetConfigWindowSizeIndex(script_machine* machine, int argc, const value* argv) {
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	size_t index = config->windowSizeIndex_;
	return StgControlScript::CreateIntValue(index);
}
gstd::value StgControlScript::Func_GetConfigWindowSizeList(script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	const auto& sizeList = config->windowSizeList_;
	std::vector<gstd::value> resListSizes;
	if (!sizeList.empty()) {
		for (const POINT& iPoint : sizeList)
			resListSizes.push_back(StgControlScript::CreateIntArrayValue((const LONG*)&iPoint, 2U));
	}
	return script->CreateValueArrayValue(resListSizes);
}
gstd::value StgControlScript::Func_GetConfigVirtualKeyMapping(script_machine* machine, int argc, const value* argv) {
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	int16_t vk = (int16_t)argv->as_int();

	int16_t key_pad[2] = { EDirectInput::KEY_INVALID, EDirectInput::KEY_INVALID };

	auto itr = config->mapKey_.find(vk);
	if (itr != config->mapKey_.end()) {
		key_pad[0] = itr->second->GetKeyCode();
		key_pad[1] = itr->second->GetPadButton();
	}

	return StgControlScript::CreateIntArrayValue(key_pad, 2U);
}
gstd::value StgControlScript::Func_GetConfigWindowTitle(script_machine* machine, int argc, const value* argv) {
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();
	return StgControlScript::CreateStringValue(graphics->GetDefaultWindowTitle());
}
gstd::value StgControlScript::Func_SetWindowTitle(script_machine* machine, int argc, const value* argv) {
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();

	std::wstring title = argv[0].as_string();
	graphics->SetWindowTitle(title);

	return value();
}
gstd::value StgControlScript::Func_SetEnableUnfocusedProcessing(script_machine* machine, int argc, const value* argv) {
	DnhConfiguration* config = DnhConfiguration::GetInstance();

	bool enable = argv[0].as_boolean();
	config->bEnableUnfocusedProcessing_ = enable;

	return value();
}
value StgControlScript::Func_IsWindowFocused(gstd::script_machine* machine, int argc, const value* argv) {
	return StgControlScript::CreateBooleanValue(EApplication::GetInstance()->IsWindowFocused());
}

//STG共通関数：描画関連
gstd::value StgControlScript::Func_ClearInvalidRenderPriority(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	StgSystemController* systemController = script->systemController_;
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();

	infoSystem->SetInvalidRenderPriority(-1, -1);

	return value();
}
gstd::value StgControlScript::Func_SetInvalidRenderPriorityA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	StgSystemController* systemController = script->systemController_;
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();

	int priMin = argv[0].as_int();
	int priMax = argv[1].as_int();
	infoSystem->SetInvalidRenderPriority(priMin, priMax);

	return value();
}

gstd::value StgControlScript::Func_GetReservedRenderTargetName(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();

	int index = argv[0].as_int();
	std::wstring name = textureManager->GetReservedRenderTargetName(index);

	return script->CreateStringValue(name);
}
gstd::value StgControlScript::Func_RenderToTextureA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();

	std::wstring name = argv[0].as_string();
	int priMin = argv[1].as_int();
	int priMax = argv[2].as_int();
	bool bClear = argv[3].as_boolean();

	DxScriptResourceCache* rsrcCache = script->pResouceCache_;

	shared_ptr<Texture> texture = rsrcCache->GetTexture(name);
	if (texture == nullptr) {
		texture = textureManager->GetTexture(name);
		if (texture == nullptr) {
			bool bExist = false;
			auto itrData = textureManager->IsDataExistsItr(name, &bExist);
			if (bExist) {
				texture = std::make_shared<Texture>();
				texture->CreateFromData(itrData->second);
				textureManager->Add(name, texture);
			}
		}
	}

	if (texture && texture->GetType() == TextureData::Type::TYPE_RENDER_TARGET) {
		DirectGraphics* graphics = DirectGraphics::GetBase();
		graphics->SetAllowRenderTargetChange(false);

		graphics->SetRenderTarget(texture);
		graphics->ResetDeviceState();

		graphics->BeginScene(false, bClear);
		script->systemController_->RenderScriptObject(priMin, priMax);
		graphics->EndScene(false);

		graphics->SetRenderTarget(nullptr);

		graphics->SetAllowRenderTargetChange(true);
	}

	return value();
}
gstd::value StgControlScript::Func_RenderToTextureB1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();

	std::wstring name = argv[0].as_string();
	int id = argv[1].as_int();
	bool bClear = argv[2].as_boolean();

	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if (obj) {
		DxScriptResourceCache* rsrcCache = script->pResouceCache_;

		shared_ptr<Texture> texture = rsrcCache->GetTexture(name);
		if (texture == nullptr) {
			texture = textureManager->GetTexture(name);
			if (texture == nullptr) {
				bool bExist = false;
				auto itrData = textureManager->IsDataExistsItr(name, &bExist);
				if (bExist) {
					texture = std::make_shared<Texture>();
					texture->CreateFromData(itrData->second);
					textureManager->Add(name, texture);
				}
			}
		}

		if (texture && texture->GetType() == TextureData::Type::TYPE_RENDER_TARGET) {
			DirectGraphics* graphics = DirectGraphics::GetBase();
			graphics->SetAllowRenderTargetChange(false);

			graphics->SetRenderTarget(texture);
			graphics->ResetDeviceState();

			graphics->BeginScene(false, bClear);
			obj->Render();
			graphics->EndScene(false);

			graphics->SetRenderTarget(nullptr);

			graphics->SetAllowRenderTargetChange(true);
		}
	}

	return value();
}

gstd::value StgControlScript::Func_SaveSnapShotA1(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring path = argv[0].as_string();

	DirectGraphics* graphics = DirectGraphics::GetBase();

	//Create the directory (if it doesn't exist)
	std::wstring dir = PathProperty::GetFileDirectory(path);
	File::CreateFileDirectory(dir);

	IDirect3DSurface9* pSurface = graphics->GetBaseSurface();
	DxRect<LONG> rect(0, 0, graphics->GetScreenWidth(), graphics->GetScreenHeight());
	HRESULT hr = D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP,
		pSurface, nullptr, (RECT*)&rect);
	return script->CreateBooleanValue(SUCCEEDED(hr));
}
gstd::value StgControlScript::Func_SaveSnapShotA2(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring path = argv[0].as_string();
	DxRect<LONG> rect(argv[1].as_int(), argv[2].as_int(),
		argv[3].as_int(), argv[4].as_int());

	DirectGraphics* graphics = DirectGraphics::GetBase();

	//Create the directory (if it doesn't exist)
	std::wstring dir = PathProperty::GetFileDirectory(path);
	File::CreateFileDirectory(dir);

	IDirect3DSurface9* pSurface = graphics->GetBaseSurface();
	HRESULT hr = D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP,
		pSurface, nullptr, (RECT*)&rect);
	return script->CreateBooleanValue(SUCCEEDED(hr));
}
gstd::value StgControlScript::Func_SaveSnapShotA3(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring path = argv[0].as_string();
	DxRect<LONG> rect(argv[1].as_int(), argv[2].as_int(),
		argv[3].as_int(), argv[4].as_int());
	BYTE imgFormat = argv[5].as_int();

	if (imgFormat < 0)
		imgFormat = 0;
	if (imgFormat > D3DXIFF_PPM)
		imgFormat = D3DXIFF_PPM;

	DirectGraphics* graphics = DirectGraphics::GetBase();

	//Create the directory (if it doesn't exist)
	std::wstring dir = PathProperty::GetFileDirectory(path);
	File::CreateFileDirectory(dir);

	IDirect3DSurface9* pSurface = graphics->GetBaseSurface();
	HRESULT hr = D3DXSaveSurfaceToFile(path.c_str(), (D3DXIMAGE_FILEFORMAT)imgFormat,
		pSurface, nullptr, (RECT*)&rect);
	return script->CreateBooleanValue(SUCCEEDED(hr));
}

//STG制御共通関数：自機関連
gstd::value StgControlScript::Func_GetPlayerID(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	std::wstring id = L"";

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController)
		id = stageController->GetStageInformation()->GetPlayerScriptInformation()->id_;

	return script->CreateStringValue(id);
}
gstd::value StgControlScript::Func_GetPlayerReplayName(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;

	std::wstring replayName = L"";

	shared_ptr<StgStageController> stageController = script->systemController_->GetStageController();
	if (stageController)
		replayName = stageController->GetStageInformation()->GetPlayerScriptInformation()->replayName_;

	return script->CreateStringValue(replayName);
}

//STG制御共通関数：ユーザスクリプト
gstd::value StgControlScript::Func_SetPauseScriptPath(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetPauseScriptPath(path);

	return value();
}
gstd::value StgControlScript::Func_SetEndSceneScriptPath(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetEndSceneScriptPath(path);

	return value();
}
gstd::value StgControlScript::Func_SetReplaySaveSceneScriptPath(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetReplaySaveSceneScriptPath(path);

	return value();
}

//STG制御共通関数：自機スクリプト
gstd::value StgControlScript::Func_GetLoadFreePlayerScriptList(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();

	infoControlScript->LoadFreePlayerList();
	std::vector<ref_count_ptr<ScriptInformation>>& listFreePlayer = infoControlScript->GetFreePlayerList();

	return script->CreateFloatValue(listFreePlayer.size());
}
gstd::value StgControlScript::Func_GetFreePlayerScriptCount(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();

	std::vector<ref_count_ptr<ScriptInformation>>& listFreePlayer = infoControlScript->GetFreePlayerList();

	return script->CreateFloatValue(listFreePlayer.size());
}
gstd::value StgControlScript::Func_GetFreePlayerScriptInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();

	std::vector<ref_count_ptr<ScriptInformation>>& listFreePlayer = infoControlScript->GetFreePlayerList();

	int index = argv[0].as_int();
	int type = argv[1].as_int();
	if (index < 0 || index >= listFreePlayer.size())
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	ref_count_ptr<ScriptInformation> infoPlayer = listFreePlayer[index];
	value res;
	switch (type) {
	case INFO_SCRIPT_PATH:
		res = script->CreateStringValue(infoPlayer->pathScript_);
		break;
	case INFO_SCRIPT_ID:
		res = script->CreateStringValue(infoPlayer->id_);
		break;
	case INFO_SCRIPT_TITLE:
		res = script->CreateStringValue(infoPlayer->title_);
		break;
	case INFO_SCRIPT_TEXT:
		res = script->CreateStringValue(infoPlayer->text_);
		break;
	case INFO_SCRIPT_IMAGE:
		res = script->CreateStringValue(infoPlayer->pathImage_);
		break;
	case INFO_SCRIPT_REPLAY_NAME:
		res = script->CreateStringValue(infoPlayer->replayName_);
		break;
	}

	return res;
}

//STG制御共通関数：リプレイ関連
gstd::value StgControlScript::Func_LoadReplayList(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring& pathMainScript = infoSystem->GetMainScriptInformation()->pathScript_;
	infoControlScript->LoadReplayInformation(pathMainScript);

	return value();
}
/*
gstd::value StgControlScript::Func_GetValidReplayIndices(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	std::vector<int> listValidIndices = replayInfoManager->GetIndexList();
	return script->CreateIntArrayValue(listValidIndices);
}
gstd::value StgControlScript::Func_IsValidReplayIndex(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	int index = argv[0].as_int();
	return script->CreateBooleanValue(replayInfoManager->GetInformation(index) != nullptr);
}*/
gstd::value StgControlScript::Func_GetValidReplayIndices(gstd::script_machine* machine, int argc, const gstd::value* argv) { //now returns an array of strings
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	std::vector<std::wstring> listValidIndices = replayInfoManager->GetIndexList();
	return script->CreateStringArrayValue(listValidIndices); //CreateIntArrayValue
}
gstd::value StgControlScript::Func_IsValidReplayIndex(gstd::script_machine* machine, int argc, const gstd::value* argv) { //now checks for a string, not an int
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	std::wstring index = argv[0].as_string();
	return script->CreateBooleanValue(replayInfoManager->GetInformation(index) != nullptr);
}
//as_string();
gstd::value StgControlScript::Func_GetReplayInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) { //first argument now a string
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring index = argv[0].as_string();
	int type = argv[1].as_int();

	ref_count_ptr<ReplayInformation> replayInfo = replayInfoManager->GetInformation(index);

	if (replayInfo == nullptr)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	value res;
	switch (type) {
	case REPLAY_FILE_PATH:
		res = script->CreateStringValue(replayInfo->GetPath());
		break;
	case REPLAY_DATE_TIME:
		res = script->CreateStringValue(replayInfo->GetDateAsString());
		break;
	case REPLAY_USER_NAME:
		res = script->CreateStringValue(replayInfo->GetUserName());
		break;
	case REPLAY_TOTAL_SCORE:
		res = script->CreateIntValue(replayInfo->GetTotalScore());
		break;
	case REPLAY_FPS_AVERAGE:
		res = script->CreateFloatValue(replayInfo->GetAverageFps());
		break;
	case REPLAY_PLAYER_NAME:
		res = script->CreateStringValue(replayInfo->GetPlayerScriptReplayName());
		break;
	case REPLAY_STAGE_INDEX_LIST:
	{
		std::vector<int> listStageI = replayInfo->GetStageIndexList();
		res = script->CreateIntArrayValue(listStageI);
		break;
	}
	case REPLAY_STAGE_START_SCORE_LIST:
	{
		std::vector<int64_t> listScoreD;
		for (int iStage : replayInfo->GetStageIndexList()) {
			ref_count_ptr<ReplayInformation::StageData> data = replayInfo->GetStageData(iStage);
			listScoreD.push_back(data->GetStartScore());
		}
		res = script->CreateIntArrayValue(listScoreD);
		break;
	}
	case REPLAY_STAGE_LAST_SCORE_LIST:
	{
		std::vector<int64_t> listScoreD;
		for (int iStage : replayInfo->GetStageIndexList()) {
			ref_count_ptr<ReplayInformation::StageData> data = replayInfo->GetStageData(iStage);
			listScoreD.push_back(data->GetLastScore());
		}
		res = script->CreateIntArrayValue(listScoreD);
		break;
	}
	case REPLAY_COMMENT:
		res = script->CreateStringValue(replayInfo->GetComment());
		break;
	}

	return res;
}
gstd::value StgControlScript::Func_SetReplayInfo(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ReplayInformation> replayInfo = infoSystem->GetActiveReplayInformation();
	if (replayInfo == nullptr)
		script->RaiseError("Cannot find a target replay data.");

	int type = argv[0].as_int();

	switch (type) {
	case REPLAY_COMMENT:
		replayInfo->SetComment(argv[1].as_string());
		break;
	}

	return value();
}
gstd::value StgControlScript::Func_GetReplayUserData(gstd::script_machine* machine, int argc, const gstd::value* argv) { //argument now a string
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring index = argv[0].as_string();
	std::string key = StringUtility::ConvertWideToMulti(argv[1].as_string());

	ref_count_ptr<ReplayInformation> replayInfo;
	if (StringUtility::ToInteger(index) == ReplayInformation::INDEX_ACTIVE) replayInfo = infoSystem->GetActiveReplayInformation();
	else replayInfo = replayInfoManager->GetInformation(index);

	if (replayInfo == nullptr)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	return replayInfo->GetUserData(key);
}
gstd::value StgControlScript::Func_SetReplayUserData(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ReplayInformation> replayInfo = infoSystem->GetActiveReplayInformation();
	if (replayInfo == nullptr)
		script->RaiseError("The replay data is not found.");

	std::string key = StringUtility::ConvertWideToMulti(argv[0].as_string());
	gstd::value val = argv[1];
	replayInfo->SetUserData(key, val);

	return value();
}
gstd::value StgControlScript::Func_IsReplayUserDataExists(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring index = argv[0].as_string();
	std::string key = StringUtility::ConvertWideToMulti(argv[1].as_string());

	ref_count_ptr<ReplayInformation> replayInfo;
	if (StringUtility::ToInteger(index) == ReplayInformation::INDEX_ACTIVE) replayInfo = infoSystem->GetActiveReplayInformation();
	else replayInfo = replayInfoManager->GetInformation(index);

	if (replayInfo == nullptr)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	return script->CreateBooleanValue(replayInfo->IsUserDataExists(key));
}

gstd::value StgControlScript::Func_SaveReplay(gstd::script_machine* machine, int argc, const gstd::value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ScriptInformation> infoMain = script->systemController_->GetSystemInformation()->GetMainScriptInformation();

	ref_count_ptr<ReplayInformation> replayInfoActive = infoSystem->GetActiveReplayInformation();
	if (replayInfoActive == nullptr)
		script->RaiseError("The replay data is not found.");

	std::wstring fname = argv[0].as_string();
	replayInfoActive->SetFileName(fname);
	if (fname.length() <= 0)
		script->RaiseError("Invalid replay index.");

	std::wstring userName = argv[1].as_string();
	replayInfoActive->SetUserName(userName);

	bool res = replayInfoActive->SaveToFile(infoMain->pathScript_, fname);
	return script->CreateBooleanValue(res);
}

/*
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/
// RoEW specific functions!!

value StgControlScript::Func_SetSkipRate(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	int newRate = argv[0].as_int();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->fastModeSpeed_ = newRate;

	EFpsController* fpsControl = EFpsController::GetInstance();
	fpsControl->SetFastModeRate(newRate);

	return value();
}

value StgControlScript::Func_SetStdFrameRate(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	int newFps = argv[0].as_int();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->fpsStandard_ = newFps;

	EFpsController* fpsControl = EFpsController::GetInstance();
	fpsControl->SetFps(newFps);

	return value();
}

value StgControlScript::Func_ToggleSkipMode(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	bool fastMode = argv[0].as_boolean();

	EFpsController* fpsControl = EFpsController::GetInstance();
	fpsControl->SetFastMode(fastMode);

	return value();
}

value StgControlScript::Func_Config_SetVKeyMap(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	int16_t keycode = argv[0].as_int();
	int16_t newKey = argv[1].as_int();
	int16_t newPadButton = argv[2].as_int();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	ref_count_ptr<VirtualKey> vkey = config->mapKey_[keycode];
	vkey->SetKeyCode(newKey);
	vkey->SetPadButton(newPadButton);

	return value();
}

value StgControlScript::Func_Config_SetResIndex(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	uint32_t resindex = argv[0].as_int();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->windowSizeIndex_ = resindex;

	return value();
}

value StgControlScript::Func_Config_SetFullscreen(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	bool fs = argv[0].as_boolean();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	ScreenMode sm = SCREENMODE_WINDOW;
	if (fs) {
		sm = SCREENMODE_FULLSCREEN;
	}
	config->modeScreen_ = sm;

	return value();
}

value StgControlScript::Func_Config_SetVsync(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	bool vs = argv[0].as_boolean();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->bVSync_ = vs;

	return value();
}

value StgControlScript::Func_Config_SetPseudoFs(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	bool pfs = argv[0].as_boolean();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->bPseudoFullscreen_ = pfs;

	return value();
}

value StgControlScript::Func_Config_GetVKeyMap(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	int16_t keycode = argv[0].as_int();
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	ref_count_ptr<VirtualKey> vkey = config->mapKey_[keycode];
	int16_t key = vkey->GetKeyCode();
	int16_t padId = vkey->GetPadIndex();
	int16_t padButton = vkey->GetPadButton();
	std::vector<int16_t> keyMap;
	keyMap.push_back(key);
	keyMap.push_back(padId);
	keyMap.push_back(padButton);

	return script->CreateIntArrayValue(keyMap);
}

value StgControlScript::Func_Config_GetResIndex(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	int64_t resindex = config->windowSizeIndex_;

	return script->CreateIntValue(resindex);
}

value StgControlScript::Func_Config_GetFullscreen(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	bool fs = false;
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	ScreenMode sm = config->modeScreen_;
	if (sm == SCREENMODE_FULLSCREEN) {
		fs = true;
	}

	return script->CreateBooleanValue(fs);
}

value StgControlScript::Func_Config_GetVsync(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	DnhConfiguration* config = DnhConfiguration::GetInstance();
	bool vs = config->bVSync_;

	return script->CreateBooleanValue(vs);
}

value StgControlScript::Func_Config_GetPseudoFs(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	DnhConfiguration* config = DnhConfiguration::GetInstance();
	bool pfs = config->bPseudoFullscreen_;

	return script->CreateBooleanValue(pfs);
}

value StgControlScript::Func_SaveConfigFile(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	DnhConfiguration* config = DnhConfiguration::GetInstance();
	config->SaveConfigFile();

	return value();
}

value StgControlScript::Func_ObjMenu_Create(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();

	ref_unsync_ptr<DxMenuObject> obj = new DxMenuObject();
	int id = script->AddObject(obj);
	return script->CreateIntValue(id);
}

value StgControlScript::Func_ObjMenu_Regist(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	obj->Activate();
	std::shared_ptr<DxMenuObjectManager> manager = script->GetMenuObjectManager();
	manager->AddMenuID(id);

	return value();
}

value StgControlScript::Func_ObjMenu_GetParent(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int pid = ID_INVALID;
	if (obj) {
		pid = obj->GetParent();
	}
	return script->CreateIntValue(pid);
}

value StgControlScript::Func_ObjMenu_GetDisabled(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	bool ret = false;
	if (obj) {
		ret = obj->GetDisabled();
	}
	return script->CreateBooleanValue(ret);
}

value StgControlScript::Func_ObjMenu_GetRelatedObject(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret_id = ID_INVALID;
	int index = argv[1].as_int();
	if (obj) {
		ret_id = obj->GetRelatedObject(index);
	}
	return script->CreateIntValue(ret_id);
}

value StgControlScript::Func_ObjMenu_GetOptionIndex(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	if (obj) {
		ret = obj->GetOptionIndex();
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetOptionIndexX(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetOptionIndexX(index);
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetMaxIndex(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	if (obj) {
		ret = obj->GetMaxIndex();
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetMaxIndexX(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetMaxIndexX(index);
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetSliderValue(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	float ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetSliderValue(index);
	}
	return script->CreateFloatValue(ret);
}

value StgControlScript::Func_ObjMenu_GetSliderMin(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetSliderMin(index);
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetSliderMax(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetSliderMax(index);
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetSliderIncr(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	float ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetSliderIncr(index);
	}
	return script->CreateFloatValue(ret);
}

value StgControlScript::Func_ObjMenu_GetOptionType(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	int ret = DEFAULT_INT;
	int index = argv[1].as_int();
	if (obj) {
		ret = obj->GetSliderIncr(index);
	}
	return script->CreateIntValue(ret);
}

value StgControlScript::Func_ObjMenu_GetActionFlag(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	bool ret = false;
	if (obj) {
		ret = obj->GetActionFlag();
	}
	return script->CreateBooleanValue(ret);
}

value StgControlScript::Func_ObjMenu_GetKeyboardInput(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	std::wstring ret;
	if (obj) {
		ret = obj->GetKeyboardInput();
	}
	return script->CreateStringValue(ret);
}

value StgControlScript::Func_ObjMenu_GetReturnValue(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	gstd::value ret = value();
	std::shared_ptr<DxMenuObjectManager> manager = script->GetMenuObjectManager();
	ret = manager->GetReturnValue(id);

	return ret;
}

value StgControlScript::Func_ForceCloseAllMenus(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	std::shared_ptr<DxMenuObjectManager> manager = script->GetMenuObjectManager();
	//Moved this logic cuz friend functionality wasn't playing nice...  now moved it back......
	//manager->ForceCloseMenus(script);

	for (const int& i : manager->menuIDs) { //woah, TIL C++ has range-based for loops, awesome
		int id = i;
		script->DeleteObject(id);
	}
	// I see little reason why this should fail, but I suppose I could eat those words later...
	manager->menuIDs.clear();

	return value();
}

value StgControlScript::Func_ObjMenu_SetParent(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int pid = argv[1].as_int();
		/*
		DxMenuObject* parent = nullptr;
		if (pid != ID_INVALID) {
			parent = script->GetObjectPointerAs<DxMenuObject>(pid);
		}
		*/
		obj->SetParent(pid);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_AddRelatedObject(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int obj_id = argv[1].as_int();
		DxScriptObjectBase* obj_a = script->GetObjectPointer(obj_id);
		if (obj_a) {
			obj->AddRelatedObject(obj_id); //used to be obj_a
		}
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetMaxIndex(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int arg = argv[1].as_int();
		obj->SetMaxIndex(arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetMaxIndexX(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int index = argv[1].as_int();
		int arg = argv[2].as_int();
		obj->SetMaxIndexX(index, arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetSliderMax(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int index = argv[1].as_int();
		int arg = argv[2].as_int();
		obj->SetSliderMax(index, arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetSliderMin(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int index = argv[1].as_int();
		int arg = argv[2].as_int();
		obj->SetSliderMin(index, arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetSliderIncr(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int index = argv[1].as_int();
		float arg = argv[2].as_float();
		obj->SetSliderIncr(index, arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetOptionType(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	DxMenuObject* obj = script->GetObjectPointerAs<DxMenuObject>(id);
	if (obj) {
		int index = argv[1].as_int();
		int arg = argv[2].as_int();
		obj->SetOptionType(index, arg);
	}
	return value();
}

value StgControlScript::Func_ObjMenu_SetReturnValue(gstd::script_machine* machine, int argc, const value* argv) {
	StgControlScript* script = (StgControlScript*)machine->data;
	script->CheckRunInMainThread();
	int id = argv[0].as_int();
	gstd::value arg = argv[1];
	std::shared_ptr<DxMenuObjectManager> manager = script->GetMenuObjectManager();
	manager->SetReturnValue(id, arg);

	return value();
}

// END: RoEW specific functions!!


//*******************************************************************
//ScriptInfoPanel
//*******************************************************************
ScriptInfoPanel::ScriptInfoPanel() {
}
ScriptInfoPanel::~ScriptInfoPanel() {
}
bool ScriptInfoPanel::_AddedLogger(HWND hTab) {
	Create(hTab);

	gstd::WButton::Style buttonStyle;
	buttonStyle.SetStyle(WS_CHILD | WS_VISIBLE | BS_FLAT | 
		BS_PUSHBUTTON | BS_TEXT);
	buttonTerminateAllScript_.Create(hWnd_, buttonStyle);
	buttonTerminateAllScript_.SetText(L"Terminate All Scripts");
	buttonTerminateSingleScript_.Create(hWnd_, buttonStyle);
	buttonTerminateSingleScript_.SetText(L"Terminate Selected Script");

	gstd::WListView::Style styleListView;
	styleListView.SetStyle(WS_CHILD | WS_VISIBLE |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOSORTHEADER);
	styleListView.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListView.SetListViewStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	wndManager_.Create(hWnd_, styleListView);
	wndManager_.AddColumn(64, 0, L"Address");
	wndManager_.AddColumn(64, 1, L"Thread ID");
	wndManager_.AddColumn(96, 2, L"Scripts Running");
	wndManager_.AddColumn(96, 3, L"Scripts Loaded");

	wndCache_.Create(hWnd_, styleListView);
	wndCache_.AddColumn(40, 0, L"Uses");
	wndCache_.AddColumn(128, 1, L"Cached Script");
	wndCache_.AddColumn(448, 2, L"Full Path");

	wndScript_.Create(hWnd_, styleListView);
	wndScript_.AddColumn(64, 0, L"Address");
	wndScript_.AddColumn(32, 1, L"ID");
	wndScript_.AddColumn(60, 2, L"Type");
	wndScript_.AddColumn(224, 3, L"Name");
	wndScript_.AddColumn(64, 4, L"Status");
	wndScript_.AddColumn(80, 5, L"Task Count");
	wndScript_.AddColumn(80, 6, L"CPU Time (μs)");

	wndSplitter_.Create(hWnd_, WSplitter::TYPE_HORIZONTAL);
	wndSplitter_.SetRatioY(0.5f);

	wndSplitter2_.Create(hWnd_, WSplitter::TYPE_VERTICAL);
	wndSplitter2_.SetRatioX(0.45f);

	SetWindowVisible(false);
	PanelInitialize();

	return true;
}
void ScriptInfoPanel::LocateParts() {
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	int xButton1 = wx + 16;
	int yButton1 = wy + 8;
	int wButton1 = 144;
	int hButton1 = 32;

	int xButton2 = xButton1 + wButton1 + 16;

	buttonTerminateAllScript_.SetBounds(xButton1, yButton1, wButton1, hButton1);
	buttonTerminateSingleScript_.SetBounds(xButton2, yButton1, wButton1, hButton1);

	int yManager = xButton1 + hButton1 + 8;

	int yLowerSec = (int)(wHeight * wndSplitter_.GetRatioY());
	int wSplitter = 4;
	int hSplitter = 6;

	int wManager = wWidth * wndSplitter2_.GetRatioX();
	int hManager = yLowerSec - yManager;

	int xRightSec = wx + wManager;
	int xCache = xRightSec + wSplitter;

	wndManager_.SetBounds(wx, yManager, wManager, hManager);
	wndSplitter2_.SetBounds(xRightSec, yManager, wSplitter, hManager);
	wndCache_.SetBounds(xCache, yManager, wWidth - xCache, hManager);

	wndSplitter_.SetBounds(wx, yLowerSec, wWidth, hSplitter);

	int yScriptList = yLowerSec + hSplitter;
	int wScriptList = 64 + 32 + 192 + 64 + 82 + 16;
	int hScriptList = wHeight - yScriptList;

	wndScript_.SetBounds(wx, yScriptList, wWidth, hScriptList);
}

void ScriptInfoPanel::_TerminateScriptAll() {
	ETaskManager* taskManager = ETaskManager::GetInstance();
	std::list<shared_ptr<TaskBase>>& listTask = taskManager->GetTaskList();
	for (auto itr = listTask.begin(); itr != listTask.end(); ++itr) {
		StgSystemController* systemController = dynamic_cast<StgSystemController*>(itr->get());
		if (systemController)
			systemController->TerminateScriptAll();
	}
}

const wchar_t* ScriptInfoPanel::GetScriptTypeName(ManagedScript* script) {
	if (script == nullptr) return L"Null";
	if (dynamic_cast<StgStageScript*>(script)) {
		switch (script->GetScriptType()) {
		case StgStageScript::TYPE_SYSTEM:
			return L"System";
		case StgStageScript::TYPE_STAGE:
			return L"Stage";
		case StgStageScript::TYPE_PLAYER:
			return L"Player";
		case StgStageScript::TYPE_ITEM:
			return L"Item";
		case StgStageScript::TYPE_SHOT:
			return L"Shot";
		}
	}
	else if (dynamic_cast<StgPackageScript*>(script)) {
		switch (script->GetScriptType()) {
		case StgPackageScript::TYPE_PACKAGE_MAIN:
			return L"Package";
		}
	}
	else if (dynamic_cast<StgUserExtendSceneScript*>(script)) {
		switch (script->GetScriptType()) {
		case StgUserExtendSceneScript::TYPE_PAUSE_SCENE:
			return L"PauseScene";
		case StgUserExtendSceneScript::TYPE_END_SCENE:
			return L"EndScene";
		case StgUserExtendSceneScript::TYPE_REPLAY_SCENE:
			return L"ReplaySaveScene";
		}
	}
	return L"Unknown";
}

void ScriptInfoPanel::PanelUpdate() {
	if (!IsWindowVisible()) return;

	ETaskManager* taskManager = ETaskManager::GetInstance();
	if (taskManager) {
		bool bSystemAvailable = false;
		std::list<shared_ptr<TaskBase>>& listTask = taskManager->GetTaskList();
		for (auto itr = listTask.begin(); itr != listTask.end(); ++itr) {
			StgSystemController* systemController = dynamic_cast<StgSystemController*>(itr->get());
			if (systemController) {
				Update(systemController);
				bSystemAvailable = true;
			}
		}
		if (!bSystemAvailable)
			Update(nullptr);
	}
}
void ScriptInfoPanel::Update(StgSystemController* systemController) {
	std::vector<ScriptManager*> vecScriptManager;
	std::list<weak_ptr<ScriptManager>> listScriptManager;
	if (systemController)
		systemController->GetAllScriptList(listScriptManager);

	std::set<shared_ptr<ScriptManager>> setScriptManager;
	for (auto itr = listScriptManager.begin(); itr != listScriptManager.end(); ++itr) {
		if (auto manager = itr->lock()) {
			setScriptManager.insert(manager);

			std::list<weak_ptr<ScriptManager>>& listRelative = manager->GetRelativeManagerList();
			for (auto itrRelative = listRelative.begin(); itrRelative != listRelative.end(); ++itrRelative) {
				if (auto managerRelative = itrRelative->lock())
					setScriptManager.insert(managerRelative);
			}
		}
	}

	{
		int iCache = 0;
		int orgRowCount = wndCache_.GetRowCount();

		if (systemController) {
			auto& pCacheMap = systemController->GetScriptEngineCache()->GetMap();
			for (auto itr = pCacheMap.cbegin(); itr != pCacheMap.cend(); ++itr, ++iCache) {
				const ref_count_ptr<ScriptEngineData>& pData = itr->second;

				size_t uses = pData.use_count();
				std::wstring path = PathProperty::GetPathWithoutModuleDirectory(pData->GetPath());

				wndCache_.SetText(iCache, 0, StringUtility::Format(L"%u", uses));
				wndCache_.SetText(iCache, 1, StringUtility::Format(L"%s", PathProperty::GetFileName(path).c_str()));
				wndCache_.SetText(iCache, 2, StringUtility::Format(L"%s", path.c_str()));
			}
		}

		for (int i = iCache; i < orgRowCount; ++i)
			wndCache_.DeleteRow(i);
	}

	{
		{
			Lock lock(lock_);

			size_t i = 0;
			for (auto itr = setScriptManager.begin(); itr != setScriptManager.end(); ++itr, ++i) {
				const shared_ptr<ScriptManager>& manager = *itr;
				wndManager_.SetText(i, 0, StringUtility::Format(L"%08x", (int)manager.get()));
				wndManager_.SetText(i, 1, StringUtility::Format(L"%d", manager->GetMainThreadID()));
				wndManager_.SetText(i, 2, StringUtility::Format(L"%u", manager->GetRunningScriptList().size()));
				wndManager_.SetText(i, 3, StringUtility::Format(L"%u", manager->GetMapScriptLoad().size()));
				vecScriptManager.push_back(manager.get());
			}
		}

		{
			listScript_.clear();

			int iScript = 0;
			int orgRowCount = wndScript_.GetRowCount();
			int selectedIndex = wndManager_.GetSelectedRow();
			if (selectedIndex >= 0 && selectedIndex < vecScriptManager.size()) {
				auto AddScript = [&](shared_ptr<ManagedScript>& script, const std::wstring& status) {
					listScript_.push_back(script);
					wndScript_.SetText(iScript, 0, StringUtility::Format(L"%08x", (int)script.get()));
					wndScript_.SetText(iScript, 1, StringUtility::Format(L"%d", script->GetScriptID()));
					wndScript_.SetText(iScript, 2, GetScriptTypeName(script.get()));
					wndScript_.SetText(iScript, 3,
						StringUtility::Format(L"%s", PathProperty::GetFileName(script->GetPath()).c_str()));
					wndScript_.SetText(iScript, 4, status);
					wndScript_.SetText(iScript, 5, StringUtility::Format(L"%u", script->GetThreadCount()));
					wndScript_.SetText(iScript, 6, StringUtility::Format(L"%u", script->GetScriptRunTime()));
				};

				ScriptManager* manager = vecScriptManager[selectedIndex];
				{
					std::map<int64_t, shared_ptr<ManagedScript>>& mapLoad = manager->GetMapScriptLoad();
					for (auto itr = mapLoad.begin(); itr != mapLoad.end(); ++itr, ++iScript)
						AddScript(itr->second, L"Loaded");
				}
				{
					std::list<shared_ptr<ManagedScript>>& listRun = manager->GetRunningScriptList();
					for (auto itr = listRun.begin(); itr != listRun.end(); ++itr, ++iScript) {
						shared_ptr<ManagedScript>& script = *itr;
						AddScript(script, script->IsPaused() ? L"Paused" : L"Running");
					}
				}
			}

			for (int i = iScript; i < orgRowCount; ++i)
				wndScript_.DeleteRow(i);
		}
	}
}

LRESULT ScriptInfoPanel::_WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_SIZE:
	{
		LocateParts();
		break;
	}
	case WM_COMMAND:
	{
		int id = wParam & 0xffff;
		if (id == buttonTerminateAllScript_.GetWindowId()) {
			_TerminateScriptAll();
			return FALSE;
		}
		else if (id == buttonTerminateSingleScript_.GetWindowId()) {
			int selectedIndex = wndScript_.GetSelectedRow();
			if (selectedIndex >= 0 && selectedIndex < listScript_.size()) {
				auto itr = std::next(listScript_.begin(), selectedIndex);
				if (auto script = itr->lock()) {
					ScriptManager* manager = script->GetScriptManager();
					if (manager) manager->CloseScript(script);
				}
			}
			return TRUE;
		}
	}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}