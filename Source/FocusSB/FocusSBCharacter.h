// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "FocusSBGameInstance.h"
#include "Components/SphereComponent.h"
#include "EnemyCharacter.h"
#include "Sound/SoundCue.h"
#include "FocusSBCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerTurnEndDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEnemyTurnEndDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMPChangeDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHPChangeDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPotionChangeDelegate);

UENUM()
enum class EDirection
{
	VERTICAL,
	HORIZONTAL
};
UCLASS(config=Game)
class AFocusSBCharacter : public ACharacter
{
	GENERATED_BODY()

	enum MANA{MP_MIN = 0, MP_MAX = 3};
	enum POTION{PO_MIN = 0, PO_MAX = 3};
	
public:
	AFocusSBCharacter();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float mTargetarmLength;

	UPROPERTY(BlueprintAssignable, Category = Delegate)
	FPlayerTurnEndDelegate OnPlayerTurnEnd;

	UPROPERTY(BlueprintAssignable, Category = Delegate)
	FEnemyTurnEndDelegate OnEnemyTurnEnd;

	UPROPERTY(VisibleAnywhere, Category = Skill)
	UStaticMeshComponent* mShield;

	UPROPERTY(VisibleAnywhere, Category = Skill)
	UMaterialInstance* Shield_M;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
protected:

	void OnResetVR();

	void MoveForward(float Value);

	void MoveRight(float Value);

	void OnAction();

	void OnShield();

	void OnPotion();

	void OnDustExplosion();
	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
// ---------------------- Timer Functions & Variables ----------------------
// -------------------------------------------------------------------------
public:
	UFUNCTION()
	void PauseTimer();

	UPROPERTY(BlueprintReadWrite, Category = Time)
	float RemainTime = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Time)
	ETurn CurrentTurn = ETurn::Enemy;
	
private:
	UFUNCTION()
	void OnEnemyUpdate(float value);
	
	UFUNCTION()
	void OnPlayerUpdate(float value);
private:
	UPROPERTY(VisibleAnywhere, Category = CurveFloat)
	UCurveFloat* EnemyCurveFloat;
	UPROPERTY(VisibleAnywhere, Category = CurveFloat)
	UCurveFloat* PlayerCurveFloat;

	UFUNCTION()
	void OnEnemyEnd();
	UFUNCTION()
	void OnPlayerEnd();
	
	UPROPERTY()
	FTimeline EnemyTL;
	UPROPERTY()
	FTimeline PlayerTL;
	
	UPROPERTY(VisibleAnywhere, Category = Direction)
	EDirection CurrentDirection;

// ---------------------- Stat(HP & MP) related ----------------------
// -------------------------------------------------------------------

public:
	UPROPERTY(BlueprintAssignable)
	FMPChangeDelegate OnMPChange;

	UPROPERTY(BlueprintAssignable)
	FHPChangeDelegate OnHPChange;

	UPROPERTY(BlueprintAssignable)
	FPotionChangeDelegate OnPotionChange;
	
	UFUNCTION(BlueprintCallable)
	const uint8& GetMP() const { UE_LOG(LogTemp, Warning, TEXT("Returning MP")); return MP; };

	UFUNCTION(BlueprintCallable)
	const float& GetHP() const { UE_LOG(LogTemp, Warning, TEXT("Returning HP")); return HP; };

	UFUNCTION(BlueprintCallable)
	const uint8& GetPotion() const { UE_LOG(LogTemp, Warning, TEXT("Returning POTION")); return mPotion; };

	UFUNCTION(BlueprintCallable)
	void UseMP(const uint8& value);

	UFUNCTION(BlueprintCallable)
	void UseHP(const float& value);

	UPROPERTY()
	bool isShield = false;

	UPROPERTY()
	bool isPotionAvailable = true;
private:
	uint8 MP = MP_MAX;
	uint8 mPotion = PO_MAX;
	float HP = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* UseMPAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* UseMPCue;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* PotionAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* PotionCue;
	
	UPROPERTY(VisibleAnywhere)
	AEnemyCharacter* pEnemyCharacter;
	FTimerHandle CharacterTimer;
};

