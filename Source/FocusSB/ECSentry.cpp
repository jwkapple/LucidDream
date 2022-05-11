// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSentry.h"
#include "FocusSBCharacter.h"
#include "Kismet/GameplayStatics.h"


AECSentry::AECSentry()
{
	UStaticMeshComponent* StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SMC"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM(TEXT("/Game/Geometry/Meshes/BluePunishment.BluePunishment"));
	if(SM.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(SM.Object);
	}

	
	MaterialInstance = CreateDefaultSubobject<UMaterialInstance>(TEXT("MaterialInstance"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MI(TEXT("/Game/Enemy/Sentry/BluePunishment_MI.BluePunishment_MI"));
	if(MI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Found Material Instance"));
		MaterialInstance = MI.Object;
		StaticMeshComponent->SetMaterial(0, MaterialInstance);
	}
	
	constexpr float RSSkillDmg = 5;
	constexpr float RSTimeBef = 0.5f;
	constexpr float RSTimeRn = 0.2f;
	constexpr float RSTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(RSSkillDmg, RSTimeBef, RSTimeRn, RSTimeAft));
	SkillRangeList.push_back(StaticMeshComponent);

	constexpr float BPSkillDmg = 15;
	constexpr float BPTimeBef = 3.0f;
	constexpr float BPTimeRn = 0.2f;
	constexpr float BPTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(BPSkillDmg, BPTimeBef, BPTimeRn, BPTimeAft));
	StaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AECSentry::OnBeginOverlap);
	StaticMeshComponent->OnComponentEndOverlap.AddDynamic(this, &AECSentry::OnOverlapEnd);
	SkillRangeList.push_back(StaticMeshComponent);
	
	constexpr float GSSkillDmg = 10;
	constexpr float GSTimeBef = 0.5f;
	constexpr float GSTimeRn = 0.2f;
	constexpr float GSTimeAft = 0.3f;
	
	SkillList.push_back(FEnemySkill(GSSkillDmg, GSTimeBef, GSTimeRn, GSTimeAft));
	SkillRangeList.push_back(StaticMeshComponent);
	
	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetCollisionProfileName(FName("NoCollision"));
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
	SkillRangeList[BLUEPUNISHMENT]->SetMaterial(0, MaterialInstance);
	SkillRangeList[BLUEPUNISHMENT]->SetWorldLocation(FVector(0.0f, 0.0f, 30.0f));
	SkillRangeList[BLUEPUNISHMENT]->SetWorldRotation(FRotator(30.0f, 00.0f, 0.0f));
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

	SkillRangeList[CurSkill]->SetVisibility(false);
	SkillRangeList[CurSkill]->SetCollisionProfileName(FName("NoCollision"));
	
	isPlayerOn = false;
	
	GetWorldTimerManager().ClearTimer(Timer);
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

	GetWorldTimerManager().SetTimer(Timer, this, &AECSentry::DamagePlayer, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Player ATTACKED"));
}

void AECSentry::GodsShout()
{
	
}
