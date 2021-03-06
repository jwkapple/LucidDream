// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "EnemyCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHPEnemyHPChangeDelegate);

/*
USTRUCT(Atomic, BlueprintType)
struct FSkillTime
{
	GENERATED_BODY();
public:
	FSkillTime() : SkillDmg(0), TimeBef(0), TimeRn(0), TimeAft(0) {};
	FSkillTime(float SkillDamage, float TBef, float TRange, float TAft)
		: SkillDmg(SkillDamage), TimeBef(TBef), TimeRn(TRange), TimeAft(TAft)
	{
		TimeTot = TimeBef + TimeRn + TimeAft;
	};
	const float SkillDmg;
	const float TimeBef;
	const float TimeRn;
	const float TimeAft;
	float TimeTot;
};
*/

USTRUCT(Atomic, BlueprintType)
struct FEnemySkill
{
	GENERATED_BODY()
public:
	FEnemySkill() : SkillDmg(0), TimeBef(0), TimeRn(0), TimeAft(0){ TimeTot = 0; };
	
	FEnemySkill(float SkillDamage, float TBef, float TRange, float TAft)
		: SkillDmg(SkillDamage), TimeBef(TBef), TimeRn(TRange), TimeAft(TAft)
	{
		TimeTot = TimeBef + TimeRn + TimeAft;
	}

	float SkillDmg;
	float TimeBef;
	float TimeRn;
	float TimeAft;
	float TimeTot;
};


UCLASS()
class FOCUSSB_API AEnemyCharacter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Activate(){};

	virtual void DeActivate(){};

	virtual void DamagePlayer(){};
	
	UFUNCTION(BlueprintCallable)
	FEnemySkill GetSkill(const int& value) { return SkillList[value]; };
	
	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetSkillRange(const int& value) { return SkillRangeList[value];};

	UFUNCTION(BlueprintCallable)
	void SetHP(const float& value);

	UFUNCTION(BlueprintCallable)
	const float& GetHP() const { return HP; }

public:

	UPROPERTY(BlueprintAssignable)
	FHPEnemyHPChangeDelegate OnEnemyHPChange;

	UPROPERTY(VisibleAnywhere)
	int CurSkill = -1;
protected:
	void InitSkill(UStaticMeshComponent* StaticMeshComponent);
	
	std::vector<FEnemySkill> SkillList;

	TArray<class UNiagaraSystem*> SkillEffects;
	
	TArray<UStaticMeshComponent*> SkillRangeList;

	UPROPERTY(VisibleAnywhere, Category = Player)
	class AFocusSBCharacter* PlayerCharacter;

	UPROPERTY(VisibleAnywhere, Category = Player)
	class AUnveilPlatform* UnveilPlatform;
	
	UPROPERTY(VisibleAnywhere, Category = Stat)
	float HP = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = Stat)
	bool PatternVisibility = false;
};


