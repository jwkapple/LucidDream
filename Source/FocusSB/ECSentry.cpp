// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSentry.h"
#include "FocusSBCharacter.h"
#include "Kismet/GameplayStatics.h"


AECSentry::AECSentry()
{
	InitSkill("ROONSTONE", "/Game/Geometry/Meshes/BluePunishment.BluePunishment", "ROONSTONEMI", "/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI");
	InitSkill("BLUEPUNISHMENT", "/Game/Geometry/Meshes/BluePunishment.BluePunishment", "BLUEPUNISHMENTMI", "/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI");
	InitSkill("GODSSHOUT", "/Game/Geometry/Meshes/GodsShout.GodsShout", "GODSSHOUTMI", "/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI");

	for(auto p : SkillRangeList)
	{
		p->OnComponentBeginOverlap.AddDynamic(this, &AECSentry::OnBeginOverlap);
		p->OnComponentEndOverlap.AddDynamic(this, &AECSentry::OnOverlapEnd);
	}
	
	constexpr float RSSkillDmg = 5;
	constexpr float RSTimeBef = 0.5f;
	constexpr float RSTimeRn = 0.2f;
	constexpr float RSTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(RSSkillDmg, RSTimeBef, RSTimeRn, RSTimeAft));


	constexpr float BPSkillDmg = 15;
	constexpr float BPTimeBef = 3.0f;
	constexpr float BPTimeRn = 0.2f;
	constexpr float BPTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(BPSkillDmg, BPTimeBef, BPTimeRn, BPTimeAft));

	constexpr float GSSkillDmg = 10;
	constexpr float GSTimeBef = 0.5f;
	constexpr float GSTimeRn = 0.2f;
	constexpr float GSTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(GSSkillDmg, GSTimeBef, GSTimeRn, GSTimeAft));
}

void AECSentry::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFocusSBCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(PlayerCharacter != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Found PlayerCharacter"));
	}

	isPlayerOn = false;
	
	PlayerCharacter->OnPlayerTurnEnd.AddDynamic(this,&AECSentry::Activate);

}

void AECSentry::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AECSentry::Activate()
{
	Super::Activate();

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Activate"));
	ESkill useSkill = BLUEPUNISHMENT;

	switch(useSkill)
	{
		case ROONSTONE: RoonStone(); break;

		case BLUEPUNISHMENT: BluePunishment(); break;

		case GODSSHOUT: GodsShout(); break;

		default: break;
	}
}

void AECSentry::DeActivate()
{
	Super::DeActivate();
}

void AECSentry::DamagePlayer()
{
	Super::DamagePlayer();

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: DamagePlayer"));
	
	if(isPlayerOn) PlayerCharacter->UseHP(SkillList[CurSkill].SkillDmg);

	auto pCurRange = SkillRangeList[CurSkill];
	
	pCurRange->SetCollisionProfileName(FName("NoCollision"));
	pCurRange->SetVisibility(false);
	
	isPlayerOn = false;
	
	GetWorldTimerManager().ClearTimer(ECBPTimer);
}

void AECSentry::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: OnBeginOverlap"));
	isPlayerOn = true;
}

void AECSentry::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: OnOverlapEnd"));
	isPlayerOn = false;
}

void AECSentry::RoonStone()
{
	
}

void AECSentry::BluePunishment()
{
	CurSkill = BLUEPUNISHMENT;
	
	auto pCurRange = SkillRangeList[CurSkill];

	pCurRange->SetCollisionProfileName(FName("DamageZone"));
	pCurRange->SetVisibility(true);

	GetWorldTimerManager().SetTimer(ECBPTimer, this, &AECSentry::DamagePlayer, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Player ATTACKED"));
}

void AECSentry::GodsShout()
{
	CurSkill = GODSSHOUT;

	auto pCurRange = SkillRangeList[CurSkill];

	pCurRange->SetCollisionProfileName(FName("DamageZone"));
	pCurRange->SetVisibility(true);

	GetWorldTimerManager().SetTimer(ECBPTimer, this, &AECSentry::DamagePlayer, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Player ATTACKED"));
}
