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

void AEnemyCharacter::InitSkill(UStaticMeshComponent* StaticMeshComponent)
{
	StaticMeshComponent->SetWorldLocation(FVector(0.0f, 0.0f, 30.0f));
	StaticMeshComponent->SetWorldRotation(FRotator(30.0f, 00.0f, 0.0f));
	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetCollisionProfileName(FName("NoCollision"));
}

