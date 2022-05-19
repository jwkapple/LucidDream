// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSentry.h"
#include "FocusSBCharacter.h"
#include "Kismet/GameplayStatics.h"


AECSentry::AECSentry()
{

	// ====================== ROONSTONE ======================
	auto RoonstoneSMC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ROONSTONE"));
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RoonstoneSM(TEXT("/Game/Geometry/Meshes/1M_Cube_Chamfer.1M_Cube_Chamfer"));
	if( RoonstoneSM.Succeeded())
	{
		RoonstoneSMC->SetStaticMesh(RoonstoneSM.Object);
	}
	InitSkill(RoonstoneSMC);
	
	MaterialInstance = CreateDefaultSubobject<UMaterialInstance>(TEXT("ROONSTONEMI"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> RoonstoneMI(TEXT("/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI"));
	if(RoonstoneMI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter:: Found Material Instance"));
		MaterialInstance = RoonstoneMI.Object;
		RoonstoneSMC->SetMaterial(0, MaterialInstance);
	}

	SkillRangeList.Emplace(RoonstoneSMC);
	
	// ====================== BLUEPUNISHMENT ======================
	auto BluePunishmentSMC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BLUEPUNISHMENT"));
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BluePunishmentSM(TEXT("/Game/Geometry/Meshes/BluePunishment.BluePunishment"));
	if(BluePunishmentSM.Succeeded())
	{
		BluePunishmentSMC->SetStaticMesh(BluePunishmentSM.Object);
	}
	InitSkill(BluePunishmentSMC);
	
	MaterialInstance = CreateDefaultSubobject<UMaterialInstance>(TEXT("BLUEPUNISHMENTMI"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> BluePunishmentMI(TEXT("/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI"));
	if(BluePunishmentMI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter:: Found Material Instance"));
		MaterialInstance = BluePunishmentMI.Object;
		BluePunishmentSMC->SetMaterial(0, MaterialInstance);
	}

	SkillRangeList.Emplace(BluePunishmentSMC);
	
	// ====================== GODSSHOUT ======================
	auto GodsShoutSMC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GODSSHOUT"));
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GodsShoutSM(TEXT("/Game/Geometry/Meshes/GodsShout.GodsShout"));
	if(GodsShoutSM.Succeeded())
	{
		GodsShoutSMC->SetStaticMesh(GodsShoutSM.Object);
	}
	InitSkill(GodsShoutSMC);
	
	MaterialInstance = CreateDefaultSubobject<UMaterialInstance>(TEXT("GODSSHOUTMI"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> GodsShoutMI(TEXT("/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI"));
	if(GodsShoutMI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter:: Found Material Instance"));
		MaterialInstance =  GodsShoutMI.Object;
		GodsShoutSMC->SetMaterial(0, MaterialInstance);
	}

	SkillRangeList.Emplace(GodsShoutSMC);


	
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
	PlayerCharacter->OnPatternVisible.AddDynamic(this, &AECSentry::SetPatternVisibility);
}

void AECSentry::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AECSentry::Activate()
{
	Super::Activate();

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Activate"));

	const int skillCase = FMath::RandRange(1, 2);
	switch (skillCase)
	{
	case ROONSTONE:
		{
			RoonStone();
			break;
		}
	case BLUEPUNISHMENT:
		{
			BluePunishment();
			break;
		}
	case GODSSHOUT:
		{
			GodsShout(); 
			break;
		}
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

	auto pCurRange = GetSkillRange(CurSkill);
	
	pCurRange->SetCollisionProfileName(FName("NoCollision"));
	pCurRange->SetVisibility(false);
	
	isPlayerOn = false;
	CurSkill = -1;
	
	GetWorldTimerManager().ClearTimer(ECBPTimer);
}


void AECSentry::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isPlayerOn = true;
}

void AECSentry::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	isPlayerOn = false;
}

void AECSentry::RoonStone()
{
	
}

void AECSentry::BluePunishment()
{
	CurSkill = BLUEPUNISHMENT;
	
	auto pCurRange = GetSkillRange(CurSkill);

	pCurRange->SetCollisionProfileName(FName("DamageZone"));

	GetWorldTimerManager().SetTimer(ECBPTimer, this, &AECSentry::DamagePlayer, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Player ATTACKED"));
}

void AECSentry::GodsShout()
{
	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: GODSSHOUT ACTIVATE"));
	CurSkill = GODSSHOUT;

	UStaticMeshComponent* pCurRange = GetSkillRange(CurSkill);

	pCurRange->SetCollisionProfileName(FName("DamageZone"));

	GetWorldTimerManager().SetTimer(ECBPTimer, this, &AECSentry::DamagePlayer, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Player ATTACKED"));
}
