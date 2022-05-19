// Fill out your copyright notice in the Description page of Project Settings.


#include "UnveilPlatform.h"

#include "FocusSBCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnveilPlatform::AUnveilPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("STATICMESH"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Platform_SM(TEXT("/Game/Geometry/Meshes/LightPlatform.LightPlatform"));
	if(Platform_SM.Succeeded())
	{
		mStaticMeshComponent->SetStaticMesh(Platform_SM.Object);
		mStaticMeshComponent->SetVisibility(false);
	}

	mStaticMeshComponent->SetCollisionProfileName(FName("NoCollision"));
	SetRootComponent(mStaticMeshComponent);
	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Platform_MI(TEXT("/Game/UnveilPlatform_MI.UnveilPlatform_MI"));
	if(Platform_MI.Succeeded())
	{
		mMaterialInstance = Platform_MI.Object;	
	}

	mHitBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HITBOX"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HitBox_SM(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder.Shape_Cylinder"));
	if(HitBox_SM.Succeeded())
	{
		mHitBox->SetStaticMesh(HitBox_SM.Object);
		mHitBox->SetWorldScale3D(FVector(3.0f, 3.0f, 1.0f));
		mHitBox->SetVisibility(false);
		mHitBox->SetCollisionProfileName(FName("DamageZone"));
		mHitBox->SetupAttachment(GetRootComponent());
		mHitBox->OnComponentBeginOverlap.AddDynamic(this, &AUnveilPlatform::OnBeginOverlap);
	}
}

void AUnveilPlatform::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == PlayerCharacter || mStaticMeshComponent->IsVisible() == true)
	{
		PlayerCharacter->SetPatternVisible(true);
		mStaticMeshComponent->SetVisibility(false);
	}
}

// Called when the game starts or when spawned
void AUnveilPlatform::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFocusSBCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(PlayerCharacter != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("VisionPlatform:: Found PlayerCharacter"));
	}

	PlayerCharacter->OnPlayerTurnEnd.AddDynamic(this, &AUnveilPlatform::ResetVisibility);
	mStaticMeshComponent->SetMaterial(0, mMaterialInstance);
}

// Called every frame
void AUnveilPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

