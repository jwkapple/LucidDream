// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyCharacter.generated.h"

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
protected:
	std::vector<FEnemySkill> SkillList;
	std::vector<UStaticMeshComponent*> SkillRangeList;

	UPROPERTY(VisibleAnywhere, Category = Player)
	class AFocusSBCharacter* PlayerCharacter;
};


