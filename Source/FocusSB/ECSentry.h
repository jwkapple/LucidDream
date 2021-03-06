// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "ECSentry.generated.h"

class USoundCue;
/**
 * 
 */
UCLASS()
class FOCUSSB_API AECSentry : public AEnemyCharacter
{
	GENERATED_BODY()

	enum ESkill{ ROONSTONE = 0, BLUEPUNISHMENT, GODSSHOUT };
	
public:
	AECSentry();
	
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	virtual void Activate() override;

	virtual void DeActivate() override;

	virtual void DamagePlayer() override;

	UFUNCTION(BlueprintCallable)
	void SetPatternVisibility()
	{
		if(CurSkill == -1) return;
		PatternVisibility = true;
		SkillRangeList[CurSkill]->SetVisibility(PatternVisibility);
	}
	
	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	
	void RoonStone();

	void BluePunishment();

	void GodsShout();

	void PlayEffect()
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SkillEffects[CurSkill],
			FVector::ZeroVector, FRotator::ZeroRotator);
		
		GetWorldTimerManager().ClearTimer(EffectTimer);
	}
private:
	UMaterialInstance* MaterialInstance;
	bool isPlayerOn;

	FTimerHandle SkillTimer;
	FTimerHandle EffectTimer;
	FTimerHandle DamageTimer;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* GodsShoutAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* GodsShoutCue;
};
