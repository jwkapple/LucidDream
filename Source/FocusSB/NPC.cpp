// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"

#include "FocusSBCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANPC::ANPC()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerCharacter = Cast<AFocusSBCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(PlayerCharacter != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ECSentry:: Found PlayerCharacter"));
	}
	
	mStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCSM"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NPC_SM(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule"));
	if(NPC_SM.Succeeded())
	{
		mStaticMeshComponent->SetStaticMesh(NPC_SM.Object);
	}

	mStaticMeshComponent->SetCollisionProfileName(FName("BlockAll"));
	SetRootComponent(mStaticMeshComponent);
	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> NPC_MI(TEXT("/Game/UnveilPlatform_MI.UnveilPlatform_MI"));
	if(NPC_MI.Succeeded())
	{
		mMaterialInstance = NPC_MI.Object;	
	}

	mHitBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HITBOX"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HitBox_SM(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if(HitBox_SM.Succeeded())
	{
		mHitBox->SetStaticMesh(HitBox_SM.Object);
		mHitBox->SetVisibility(false);
		mHitBox->SetCollisionProfileName(FName("NPC"));
		mHitBox->SetupAttachment(GetRootComponent());
		mHitBox->OnComponentBeginOverlap.AddDynamic(this, &ANPC::OnBeginOverlap);
		mHitBox->OnComponentEndOverlap.AddDynamic(this, &ANPC::OnEndOverlap);
	}

}

void ANPC::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Cast<AFocusSBCharacter>(OtherActor)->SetActionUIVisible(true);
}

void ANPC::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	Cast<AFocusSBCharacter>(OtherActor)->SetActionUIVisible(false);
}

void ANPC::Interact()
{
	
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

