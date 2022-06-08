// Copyright Epic Games, Inc. All Rights Reserved.

#include "FocusSBCharacter.h"


#include "HeadMountedDisplayFunctionLibrary.h"
#include "MainCamera.h"
#include "ToolBuilderUtil.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AFocusSBCharacter

AFocusSBCharacter::AFocusSBCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
 
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, -1.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
	
	mTargetarmLength = 1000.0f;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = mTargetarmLength; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 300.0f);
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	mShield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldComponent"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SPSM(TEXT("/Game/Skill/Meshes/Shield_SM.Shield_SM"));
	if(SPSM.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Sphere Object"));
		mShield->SetStaticMesh(SPSM.Object);
	}

	mShield->SetVisibility(false);
	mShield->SetCollisionProfileName(FName("NoCollision"));
	mShield->SetupAttachment(GetRootComponent());
	mShield->SetWorldLocation(FVector::ZeroVector);
	
	
	Shield_M = CreateDefaultSubobject<UMaterialInstance>(TEXT("ShieldMI"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> SMI(TEXT("/Game/Skill/DivineShield_Inst.DivineShield_Inst"));
	if(SMI.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Sphere Material"));
		Shield_M = SMI.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UCurveFloat> ECF(TEXT("/Game/GameTurn/EnemyCurveFloat.EnemyCurveFloat"));
	if(ECF.Succeeded())
	{
		EnemyCurveFloat = ECF.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UCurveFloat> PCF(TEXT("/Game/GameTurn/PlayerCurveFloat.PlayerCurveFloat"));
	if(PCF.Succeeded())
	{
		PlayerCurveFloat = PCF.Object;
	}
	
	if(EnemyCurveFloat)
	{
		FOnTimelineFloat EnemyTimelineProgress;
		FOnTimelineEvent EnemyTLEndCallback;
		
		EnemyTimelineProgress.BindUFunction(this, FName("OnEnemyUpdate"));
		EnemyTLEndCallback.BindUFunction(this, FName("OnEnemyEnd"));
		EnemyTL.AddInterpFloat(EnemyCurveFloat, EnemyTimelineProgress);
		EnemyTL.SetLooping(false);
		EnemyTL.SetTimelineFinishedFunc(EnemyTLEndCallback);
	}

	if(PlayerCurveFloat)
	{
		FOnTimelineFloat PlayerTimelineProgress;
		FOnTimelineEvent PlayerTLEndCallback;
		
		PlayerTimelineProgress.BindUFunction(this, FName("OnPlayerUpdate"));
		PlayerTLEndCallback.BindUFunction(this, FName("OnPlayerEnd"));
		PlayerTL.AddInterpFloat(PlayerCurveFloat, PlayerTimelineProgress);
		PlayerTL.SetLooping(false);
		PlayerTL.SetTimelineFinishedFunc(PlayerTLEndCallback);
	}

	// Set Local Variables value
	CurrentDirection = EDirection::VERTICAL;
	CurrentTurn = ETurn::Enemy;
	RemainTime = 10.0f;

	// ----------------- SFX --------------------
	UseMPAC = CreateDefaultSubobject<UAudioComponent>(TEXT("MPUSESFX"));
	static ConstructorHelpers::FObjectFinder<USoundCue> UMC(TEXT("/Game/Sound/SFX/FB-EnergyUse_2_Cue.FB-EnergyUse_2_Cue"));
	if(UMC.Succeeded())
	{
		UseMPCue = UMC.Object;
		UseMPAC->SetSound(UseMPCue);
		UseMPAC->SetAutoActivate(false);
	}

	PotionAC = CreateDefaultSubobject<UAudioComponent>(TEXT("POTIONSFX"));
	static ConstructorHelpers::FObjectFinder<USoundCue> PTC(TEXT("/Game/Sound/SFX/PlayerBuff-PotionDrink1_Cue.PlayerBuff-PotionDrink1_Cue"));
	if(PTC.Succeeded())
	{
		PotionCue = PTC.Object;
		PotionAC->SetSound(PotionCue);
		PotionAC->SetAutoActivate(false);
	}

	CountDownAC = CreateDefaultSubobject<UAudioComponent>(TEXT("COUNTDOWNSFX"));
	static ConstructorHelpers::FObjectFinder<USoundCue> CDC(TEXT("/Game/Sound/SFX/UI-Countdown_1sec_Cue.UI-Countdown_1sec_Cue"));
	if(CDC.Succeeded())
	{
		CountDownCue = CDC.Object;
		CountDownAC->SetSound(CountDownCue);
		CountDownAC->SetAutoActivate(false);
	}

	EnemyTurnStartAC = CreateDefaultSubobject<UAudioComponent>(TEXT("ENEMYTURNSTARTSFX"));
	static ConstructorHelpers::FObjectFinder<USoundCue> ETSC(TEXT("/Game/Sound/SFX/UI-BossTurn1_Cue.UI-BossTurn1_Cue"));
	if(ETSC.Succeeded())
	{
		EnemyTurnStartCue = ETSC.Object;
		EnemyTurnStartAC->SetSound(EnemyTurnStartCue);
		EnemyTurnStartAC->SetAutoActivate(false);
	}

	PlayerTurnStartAC = CreateDefaultSubobject<UAudioComponent>(TEXT("PLAYERTURNSTARTSFX"));
	static ConstructorHelpers::FObjectFinder<USoundCue> PTSC(TEXT("/Game/Sound/SFX/UI-PlayerTurn1_Cue.UI-PlayerTurn1_Cue"));
	if(PTSC.Succeeded())
	{
		PlayerTurnStartCue = PTSC.Object;
		PlayerTurnStartAC->SetSound(PlayerTurnStartCue);
		PlayerTurnStartAC->SetAutoActivate(false);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFocusSBCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFocusSBCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFocusSBCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFocusSBCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AFocusSBCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFocusSBCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFocusSBCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFocusSBCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AFocusSBCharacter::OnAction);
	PlayerInputComponent->BindAction("Shield", IE_Pressed, this, &AFocusSBCharacter::OnShield);
	PlayerInputComponent->BindAction("Potion", IE_Pressed, this, &AFocusSBCharacter::OnPotion);
	PlayerInputComponent->BindAction("DustExplosion", IE_Pressed, this, &AFocusSBCharacter::OnDustExplosion);
	
}

void AFocusSBCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Cameras;

	UGameplayStatics::GetAllActorsWithTag(GetWorld(),FName(TEXT("FrontCamera")), Cameras);
	for(auto p : Cameras)
	{
		//APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		mFrontCamera = p;
	}
	
	UGameplayStatics::GetAllActorsWithTag(GetWorld(),FName(TEXT("SideCamera")), Cameras);
	for(auto p : Cameras)
	{
		//APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		mSideCamera = p;
	}
	
	mShield->SetMaterial(0, Shield_M);
	
	EnemyTL.Play();
}

void AFocusSBCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	EnemyTL.TickTimeline(DeltaSeconds);
	PlayerTL.TickTimeline(DeltaSeconds);
}

void AFocusSBCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFocusSBCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AFocusSBCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AFocusSBCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFocusSBCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFocusSBCharacter::MoveForward(float Value)
{
	FRotator CameraForward = FRotator(0.0f, 1.0f, 0.0f);
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = CameraForward;
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector

		auto Axis = CurrentDirection == EDirection::VERTICAL ? EAxis::X : EAxis::Y;
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
		AddMovementInput(Direction, Value);
	}
}

void AFocusSBCharacter::MoveRight(float Value)
{
	FRotator CameraForward = FRotator(0.0f, 1.0f, 0.0f);
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = CameraForward;
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		auto Axis = CurrentDirection == EDirection::VERTICAL ? EAxis::Y : EAxis::X;
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
		// add movement in that direction
		
		AddMovementInput(Direction, Value);
	}
}

void AFocusSBCharacter::OnAction()
{
	
}

void AFocusSBCharacter::OnShield()
{
	if(CurrentTurn == ETurn::Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnShield :: PlayerTurn"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("OnShield :: Activate"));

	UseMP(1);
	mShield->SetVisibility(true);
	isShield = true;

	GetWorldTimerManager().SetTimer(CharacterTimer, FTimerDelegate::CreateLambda([&]()
	{
		mShield->SetVisibility(false);
		isShield = false;
	}), 2.0f, false);
	
}

void AFocusSBCharacter::OnPotion()
{
	if(mPotion == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPotion:: NO MORE POTION LEFT!!"));
		return;
	}

	if(HP == 100.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPotion:: HP is already full!!"));
		return;
	}

	if(isPotionAvailable == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPotion:: Already used potion at this turn!!"));
		return;
	}
	
	mPotion--;

	if(PotionAC != nullptr)
	{
		PotionAC->Play();
	}
	int HealValue = HP - 100.0f;
	UseHP(HealValue);
	
	if(OnPotionChange.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPotionChange Broadcast"));
		OnPotionChange.Broadcast();
	}
}

void AFocusSBCharacter::OnDustExplosion()
{
	UseMP(1);

	PlayerTL.Stop();

	// Change to Side Camera
	AttackStart();
	
	// Change platform's angle
	// Play Niagara Effect

	if(pEnemyCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDustExplosion :: No enemy found"));
	}
	else
	{
		pEnemyCharacter->SetHP(5);
	}

	GetWorldTimerManager().SetTimer(PlayerTimer, this, &AFocusSBCharacter::AttackrEnd, 3.0f, false);

	
	// Return platform's angle
	
}

void AFocusSBCharacter::OnLullaby()
{
	
}

void AFocusSBCharacter::PauseTimer()
{
	
}

void AFocusSBCharacter::OnEnemyUpdate(float value)
{
	if(value == 2.0f || value == 3.0f || value == 4.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("CountDown play"));
		CountDownAC->Play();
	}
	RemainTime = value;
}

void AFocusSBCharacter::OnPlayerUpdate(float value)
{
	if(value == 2.0f || value == 3.0f || value == 4.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("CountDown play"));
		CountDownAC->Play();
	}
	RemainTime = value;
}

void AFocusSBCharacter::OnEnemyEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("--------------------------Enemy Turn End-------------------------"));
	PlayerTL.PlayFromStart();
	CurrentTurn = ETurn::Player;
	isPatternVisible = false;
	
	PlayerTurnStartAC->Play();
	if(OnEnemyTurnEnd.IsBound())
	{
		OnEnemyTurnEnd.Broadcast();
	}
}

void AFocusSBCharacter::OnPlayerEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("-------------------------Player Turn End-------------------------"));

	EnemyTL.PlayFromStart();
	
	CurrentTurn = ETurn::Enemy;
	isPotionAvailable = true;
	isPatternVisible = false;
	
	EnemyTurnStartAC->Play();
	if(OnPlayerTurnEnd.IsBound())
	{
		OnPlayerTurnEnd.Broadcast();
	}

	MP = MP_MAX;
}

void AFocusSBCharacter::AttackStart()
{
	APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	OurPlayerController->SetViewTarget(mSideCamera);
}

void AFocusSBCharacter::AttackrEnd()
{
	APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	OurPlayerController->SetViewTarget(mFrontCamera);
	PlayerTL.Play();
}

void AFocusSBCharacter::SetPatternVisible(const bool& value)
{
	if(isPatternVisible) return;

	isPatternVisible = true;
	
	if(CurrentTurn == ETurn::Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("FocusSBCharacter/SetEnemyPatternVisible:: It's Player's Turn!"));
		return;
	}

	if(OnPatternVisible.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("FocusSBCharacter/SetEnemyPatternVisible:: OnPatternVisible Broadcast"));
		OnPatternVisible.Broadcast();
	}
}

void AFocusSBCharacter::UseMP(const uint8& value)
{
	MP -= value;

	if(UseMPAC != nullptr)
	{
		UseMPAC->Play();
	}
	
	if(OnMPChange.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMPChange Broadcast"));
		OnMPChange.Broadcast();
	}
}

void AFocusSBCharacter::UseHP(const float& value)
{
	if(isShield) return;

	UE_LOG(LogTemp, Warning, TEXT("Player Damaged"));
	
	HP -= value;

	if(OnHPChange.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnHPChange Broadcast"));
		OnHPChange.Broadcast();
	}
};
