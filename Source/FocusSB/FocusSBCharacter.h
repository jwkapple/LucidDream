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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPatternVisibleDelegate);
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

	void OnLullaby();
	
	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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

	UFUNCTION()
	void AttackStart();

	UFUNCTION()
	void AttackrEnd();
	
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

	UPROPERTY(BlueprintAssignable)
	FPatternVisibleDelegate OnPatternVisible;

	UFUNCTION(BlueprintCallable)
	const float& GetRemainTime() const { return RemainTime; };

	const ETurn& GetCurrentTurn() const { return CurrentTurn; };
	
	UFUNCTION(BlueprintCallable)
	const uint8& GetMP() const { UE_LOG(LogTemp, Warning, TEXT("Returning MP")); return MP; };

	UFUNCTION(BlueprintCallable)
	const float& GetHP() const { UE_LOG(LogTemp, Warning, TEXT("Returning HP")); return HP; };

	UFUNCTION(BlueprintCallable)
	const uint8& GetPotion() const { UE_LOG(LogTemp, Warning, TEXT("Returning POTION")); return mPotion; };

	UFUNCTION(BlueprintCallable)
	const bool& GetPatternVisible() const { return isPatternVisible; }
	
	UFUNCTION(BlueprintCallable)
	void SetPatternVisible(const bool& value);

	UFUNCTION(BlueprintCallable)
	void SetActionUIVisible(const bool& value)
	{
		isActionUIVisible = value;

		if(value)
		{
			UE_LOG(LogTemp, Warning, TEXT("Peppy:: Action ON"));
		}
		else UE_LOG(LogTemp, Warning, TEXT("Peppy:: Action OFF"));
	};
	UFUNCTION(BlueprintCallable)
	void UseMP(const uint8& value);

	UFUNCTION(BlueprintCallable)
	void UseHP(const float& value);

	UPROPERTY()
	bool isShield = false;

	UPROPERTY()
	bool isPotionAvailable = true;

	UPROPERTY()
	bool isPatternVisible = false;

	UPROPERTY()
	bool isActionUIVisible = false;
	
	FTimerHandle PlayerTimer;
private:
	uint8 MP = MP_MAX;
	uint8 mPotion = PO_MAX;
	float HP = 100.0f;
	
	UPROPERTY(VisibleAnywhere, Category = Viewpoint)
	AActor* mFrontCamera;

	UPROPERTY(VisibleAnywhere, Category = Viewpoint)
	AActor* mSideCamera;
	
	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* UseMPAC;
	
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* UseMPCue;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* PotionAC;
	
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* PotionCue;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* CountDownAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* CountDownCue;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* EnemyTurnStartAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* EnemyTurnStartCue;

	UPROPERTY(VisibleAnywhere, Category = SFX)
	class UAudioComponent* PlayerTurnStartAC;
	UPROPERTY(VisibleAnywhere, Category = SFX)
	USoundCue* PlayerTurnStartCue;
	
	UPROPERTY(VisibleAnywhere)
	AEnemyCharacter* pEnemyCharacter;
	FTimerHandle CharacterTimer;
};

