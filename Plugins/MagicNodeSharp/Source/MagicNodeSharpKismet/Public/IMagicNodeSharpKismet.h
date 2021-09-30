//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharpKismet.h"
#include "KCS_MonoCore.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include "ContentBrowserModule.h"

#include "LevelEditor.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PropertyEditing.h"

#include "Modules/ModuleManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IMagicNodeSharpKismet : public IModuleInterface, public IMonoKismet {
public:
	static inline IMagicNodeSharpKismet &Get() {return FModuleManager::LoadModuleChecked<IMagicNodeSharpKismet>("MagicNodeSharpKismet");}
	static inline bool IsAvailable() {return FModuleManager::Get().IsModuleLoaded("MagicNodeSharpKismet");}
};

class FMagicNodeSharpKismet : public IMagicNodeSharpKismet {
private:
	bool HandleSettingsSaved() {
	  #if WITH_EDITOR
		const auto &Settings = GetMutableDefault<UCS_Settings>();
		Settings->SaveConfig(); return true;
	  #endif
	return false;}
	//
	void RegisterSettings() {
	  #if WITH_EDITOR
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
			ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");
			SettingsContainer->DescribeCategory("Synaptech",LOCTEXT("SynaptechCategoryName","Synaptech"),
			LOCTEXT("SynaptechCategoryDescription","Configuration of Synaptech Systems."));
			//
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project","Synaptech","KCS_Settings",
				LOCTEXT("KCS_SettingsName","(C#) Magic Node Settings"),
				LOCTEXT("KCS_SettingsDescription","Kismet Settings for the Magic Node Plugin"),
			GetMutableDefault<UCS_Settings>());
			//
			if (SettingsSection.IsValid()) {SettingsSection->OnModified().BindRaw(this,&FMagicNodeSharpKismet::HandleSettingsSaved);}
		}///
	  #endif
	}///
	//
	void UnregisterSettings() {
	  #if WITH_EDITOR
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
			SettingsModule->UnregisterSettings("Project","Synaptech","KCS_Settings");
		}///
	  #endif
	}///
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
public:
	virtual bool SupportsDynamicReloading() override {return false;}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////