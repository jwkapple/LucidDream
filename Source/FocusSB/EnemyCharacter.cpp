// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyCharacter::InitSkill(const char* SMPathName, const char* SMPath, const char* MIPathName, const char* MIPath)
{
	FString SMPathNameStr = FString(SMPathName);
	FString StaticMeshStr = FString(SMPath);
	
	UStaticMeshComponent* StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*SMPathNameStr);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMesh(*StaticMeshStr);
	if(StaticMesh.Succeeded())
	{
		StaticMeshComponent->SetStaticMesh(StaticMesh.Object);
	}

	StaticMeshComponent->SetWorldLocation(FVector(0.0f, 0.0f, 30.0f));
	StaticMeshComponent->SetWorldRotation(FRotator(30.0f, 00.0f, 0.0f));
	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetCollisionProfileName(FName("NoCollision"));
	
	SkillRangeList.push_back(StaticMeshComponent);

	UMaterialInstance* MaterialInstance;

	FString MIPathNameStr = FString(MIPathName);
	FString MaterialInstancePath = FString(MIPath);
	
	MaterialInstance = CreateDefaultSubobject<UMaterialInstance>(*MIPathNameStr);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MI(*MaterialInstancePath);
	if(MI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter:: Found Material Instance"));
		MaterialInstance = MI.Object;
		StaticMeshComponent->SetMaterial(0, MaterialInstance);
	}

	SkillRangeList.push_back(StaticMeshComponent);
}

