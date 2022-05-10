// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FocusSBGameInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ETurn : uint8
{
	Player UMETA(DisplayName = "Player Turn"),
	Enemy  UMETA(DisplayName = "Enemy Turn")
};
UCLASS()


class FOCUSSB_API UFocusSBGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UFocusSBGameInstance();

	virtual void Init() override;
};
