// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OWSettings.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Gravity Settings"))
class UNREALWILDS_API UOWSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// 在 Project Settings UI 里可编辑的引力常量，自动保存到 DefaultGame.ini
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Physics", meta=(DisplayName="Gravity Constant", ToolTip="全局引力常量，单位 m/s^2"))
	float GravityConstant = 0.001f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Physics", meta=(DisplayName="SunMass", ToolTip="太阳质量，单位 kg"))
	float SunMass = 399999991808.0f;
};
