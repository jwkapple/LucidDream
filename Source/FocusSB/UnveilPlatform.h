// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnveilPlatform.generated.h"

UCLASS()
class FOCUSSB_API AUnveilPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUnveilPlatform();

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* mStaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstance* mMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = HitBox)
	UStaticMeshComponent* mHitBox;

	UPROPERTY(VisibleAnywhere, Category = Player)
	class AFocusSBCharacter* PlayerCharacter;

	UFUNCTION(BlueprintCallable)
	void ResetVisibility() { mStaticMeshComponent->SetVisibility(true); }
private:
	TArray<FVector> RandPos = {FVector{40, -290, 20},FVector{40, 310, 20},
		                       FVector{-250, -310, -140},FVector{-250, 20, -140},FVector{-250, 310, -140},
							   FVector{290, -320, 170}, FVector{290, -10, 170}, FVector{290, 320, 170}};
};
