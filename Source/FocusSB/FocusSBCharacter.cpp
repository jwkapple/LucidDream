// Copyright Epic Games, Inc. All Rights Reserved.

#include "FocusSBCharacter.h"


#include "HeadMountedDisplayFunctionLibrary.h"
#include "MainCamera.h"
#include "ToolBuilderUtil.h"
#include "Camera/CameraComponent.h"
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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SPSM(TEXT("/Game/Geometry/Meshes/ShieldMesh.ShieldMesh"));
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
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> SMI(TEXT("/Game/Skill/DivineShield_MI.DivineShield_MI"));
	if(SMI.Succeeded())
	{
		Shield_M = SMI.Object;
	}
	
	mShield->SetMaterial(0, Shield_M);
	
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

}


void AFocusSBCharacter::BeginPlay()
{
	Super::BeginPlay();

	mShield->SetVisibility(false);
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
	TArray<AActor*> Cameras;

	switch (CurrentTurn) {
	case ETurn::Enemy:
			{
				UE_LOG(LogTemp, Warning, TEXT("Current : Enemy Camera"));
				UGameplayStatics::GetAllActorsWithTag(GetWorld(),FName(TEXT("PlayerTurn")), Cameras);
				for(auto p : Cameras)
				{
					UE_LOG(LogTemp, Warning, TEXT("Change to Player Camera"));
					APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
					OurPlayerController->SetViewTarget(p);
				}
				CurrentDirection = EDirection::HORIZONTAL;
				break;
			}
	case ETurn::Player:
			{
				UE_LOG(LogTemp, Warning, TEXT("Current : Player Camera"));
				UGameplayStatics::GetAllActorsWithTag(GetWorld(),FName(TEXT("EnemyTurn")), Cameras);
				for(auto p : Cameras)
				{
					UE_LOG(LogTemp, Warning, TEXT("Change to Enemy Camera"));
					APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
					OurPlayerController->SetViewTarget(p);
				}
				
				CurrentDirection = EDirection::VERTICAL;
				break;
			}
	default: ;
	}
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



Overflow










general


general

연봉보다는 자산




21
Loading history...


Vint S. Lee  1:14 PM
요즘 누가 개발자 구하기 어렵데~?


고수창:cooking:  1:15 PM
오졌다
1:15
ㅋㅋㅋㅋㅋㅋㅋ
1:15
저걸샀엌ㅋㅋㅋㅋㅋ


강웅석  1:23 PM
ㄷㄸㄸㄸㄷ
1:23
근데 저희 회사도 지금 저거 열풍이에요
1:23
1 사토시에 사서 2 사토시에 팔기
1:23
ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ


선명 이  1:24 PM
이러다가 0.1달러까지 회귀하면?!
1:24
이라는 상상을
1:24
여기 코인형님들이 하고있습니다


Vint S. Lee  1:24 PM
ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ
1:25
제친구중에 테라 레버리지
1:25
한친구있던데
1:25
1달러가즈앙


강웅석  1:27 PM
ㄹㅇ
1:27
근데 뭐 불가능한 이야기는 아니죠
1:27
튤립도 가격이 뛸 수 있으니


Vint S. Lee  1:30 PM
ㄹㅇ


강웅석  1:31 PM
수요와 공급!!


Vint S. Lee  1:49 PM
정기 수익 보고드립니다^^
Screenshot_20220513-134804_ mPOP.jpg 
Screenshot_20220513-134804_ mPOP.jpg




선명 이  1:49 PM
ㄷㄷ


정진호  1:49 PM
오졌다


강웅석  1:58 PM
ㄷㄷ
1:59
무서운 주식


박종엽  2:10 PM
vgroup 파산위기;;; ㄷㄷ


강웅석  2:12 PM
ㅋㅋㅋㅋ
2:12
cgroup 이런건줄 알았네요


강웅석  3:22 PM
와 루나로 부자 된사람 나왔네요
3:23
새벽에 테라 노드 다시 올라왔을때
3:23
1 UST를 다시 1$ 어치의 루나로 바꿔주는 기존 스왑이
3:23
여전히 1$에 연동돼있어서
3:23
페깅 깨진 UST 왕창 사서 테라 스테이션 보내고
3:23
그걸로 루나 존나 사서 DeX 보내서 스왑하고
3:23
업비트 입금 ㄷㄷ
3:23
150으로 60억 만들었다네요


고수창:cooking:  3:23 PM
ㅁㅊ


강웅석  3:24 PM
지금은 막혀서 안되가지고
3:24
새벽에 열심히 하고
3:24
지금까지 하다
3:24
인증글 올린거래요
3:24
ㅋㅋㅋㅋㅋㅋ


고수창:cooking:  3:24 PM
아 잠깐


강웅석  3:24 PM
150이 아니고 1500


고수창:cooking:  3:24 PM
1UST를 1$ LUNA였으면


강웅석  3:24 PM
ㄹㅇ


고수창:cooking:  3:24 PM
아


강웅석  3:24 PM
지금 1UST에 0.2$ 이러는데


고수창:cooking:  3:24 PM
아 그러네


강웅석  3:24 PM
1$로 고정가치로
3:24
교환해주니까
3:24
무조건 실행하면 5배씩


고수창:cooking:  3:24 PM
무조건 5배


강웅석  3:24 PM
이득이죠
3:25
ㅋㅋㅋ


고수창:cooking:  3:25 PM
이득보는거였네
3:25
ㅁㅊ


강웅석  3:25 PM
5배 넣고 빼고
3:25
다시 돌리고 돌리고
3:25
무한 돈복사~~!!


고수창:cooking:  3:25 PM
근데 루나 사토시가
3:25
떨어져서
3:25
불가능하지않았나
3:25
어케했지


강웅석  3:25 PM
아니에요
3:25
루나 막 4사토시
3:25
이런식으로까지
3:25
계속 펌핑했어요
3:25
던지기 충분했을듯


고수창:cooking:  3:25 PM
ㅇㅎ


강웅석  3:25 PM
바낸은 저점대비 60배 올랐때요
3:25
대요


고수창:cooking:  3:25 PM
순간적 펌핑에
3:25
했네요
3:25
그럼


강웅석  3:25 PM
네
3:25
계속 내다판듯


고수창:cooking:  3:25 PM
승부사 맞네
3:25
그럼 먹을 가치가 있는 사람 맞는듯요


강웅석  3:25 PM
ㄹㅇ
3:25
솔직히 개똑똑하네요


고수창:cooking:  3:25 PM
어제 체인 정지해서
3:25
스왑 안돌아갔을텐데


강웅석  3:25 PM
ㄹㅇ
3:26
근데 새벽에 다시 켜졌다고 하더라고요


고수창:cooking:  3:26 PM
타이밍 잘먹었네


강웅석  3:26 PM
저희회사 대표도 그러던데


고수창:cooking:  3:26 PM
흠


강웅석  3:26 PM
난 왜 그런생각 못했지~
3:26
테라스테이션 나도 깔려있는데~~
3:26
그리고 뭐 15만원으로 2억 7천 만든놈도나왔는데
3:26
테라스테이션에서 업비트로
3:26
루나 계속보내는데
3:26
안보내진것처럼 나오고
3:26
실제로는 계속 갔어서
3:26
업비트에 들오자마자 계속 던져가지고
3:26
돈벌었대요
3:26
ㅋㅋㅋㅋㅋㅋㅋ
3:26
돈복사 버그 제대로 걸렸네
3:27
자산 가치가 떨어지면
3:27
진짜 돈버는 사람들은
3:27
매매하는 사람들이 아니고
3:27
거래소와 버그맨들이네요


고수창:cooking:  3:28 PM
루나는
3:28
위로도
3:28
아래로도 벌게해준다..


강웅석  3:28 PM
ㄹㅇ..


고수창:cooking:  3:28 PM
흠


강웅석  3:28 PM
모든 자산이 그렇겠지만..


고수창:cooking:  3:28 PM
아 근데 왜
3:28
그생각을 못했지


강웅석  3:28 PM
ㄹㅇ
3:28
어차피 진짜 테라스테이션 스왑은
3:28
코드에 하드코딩 되어있을텐데


고수창:cooking:  3:28 PM
근데 솔직히 전


강웅석  3:28 PM
1달러어치로


고수창:cooking:  3:28 PM
할까 하다가


강웅석  3:28 PM
해주는걸로


고수창:cooking:  3:29 PM
말았거든요


강웅석  3:29 PM
ㄷ ㄷ ㄷ


고수창:cooking:  3:29 PM
왜냐면


강웅석  3:29 PM
코인왕


고수창:cooking:  3:29 PM
커널랩스
3:29
앞에
3:29
내 눈앞에서


강웅석  3:29 PM
그럼 저만 알려주시지


고수창:cooking:  3:29 PM
계시는데


강웅석  3:29 PM
ㅋㅋ;;


고수창:cooking:  3:29 PM
사무실에서 그지랄
3:29
하기엔


강웅석  3:29 PM
하핫~!!


고수창:cooking:  3:29 PM
좀 그랬음
3:29
그리고
3:29
사무실에
3:29
루나로 8억
3:29
잃은 분
3:29
3억 잃은분
3:29
있고 해서


강웅석  3:29 PM
ㅠㅠ


고수창:cooking:  3:29 PM
사무실에서 일하다가
3:29
스왑 던지기에
3:29
양심이 찔렸음..


강웅석  3:29 PM
저도 아까 저얘기 듣고


고수창:cooking:  3:29 PM
좀 많이..


강웅석  3:29 PM
헐레벌떡
3:29
오스모 들어가봤는데
3:29
역시 오스모는 정상이라
3:29
ㅠㅠ
3:29
테라스테이션 ~~~!!!!


이종찬  4:43 PM
실시간 루나 호가창 ㅋㅋㅋ
Image from iOS 
Image from iOS


4:43
저거보다 더 낮게 팔수 없나봄 ㅋㅋㅋ


강웅석  4:47 PM
와 그래도 2 사토시네요
4:47
1 사토시에 샀으면 2배 떡상 ㄸㄸㄸㄸㄸㄸㄸㄷ
4:47
10억 샀으면 바로 20억 ㄸㄸㄸㄸㄸㄸㄸㄸㄸㄸㄸㄸㄸㄸ


Vint S. Lee  4:55 PM
@channel 새벽에 같이 패티구으실분 구합니다



4 replies
Last reply today at 5:38 PMView thread


지윤  4:56 PM
ㄴㄴㅋㅋ


선엽 Lee  4:57 PM
와 아까보다 2배 올랐네


Vint S. Lee  5:02 PM
이야~


정진호  5:07 PM
replied to a thread:
@channel 새벽에 같이 패티구으실분 구합니다
노트북 들고올걸
View newer replies


이종찬  5:27 PM
나도 조의금 10만원어치만 사볼까^^


강웅석  5:32 PM
그생각으로 삿다가
5:32
바로 조의금 날렸어요
5:32
ㅋㅋ;;


지윤  5:36 PM
아 ㅋㅋ
5:36
회사에서 자리 이름표 해시태그 정해서 발주 넣엇는데
5:36
#회계인 이렇개한걸
5:36
업체에서
5:36
#외계인으로
5:37
뽑아줌
5:37
ㅇㅋㅋ 아


정진호  5:37 PM
외계인 하지윤


지윤  5:37 PM
재건 아니지만
5:37
ㅎ
5:37
제거는 어케됫우려나..


Vint S. Lee  5:38 PM
ㅋㅋㅋ


이종찬  5:39 PM
외계인 ㅋㅋㅋㅋ


혁규 권  5:40 PM
ㅋㅋㅋㅋㅋ


이종찬  6:04 PM
업비트 빗썸 결국 루나 거래 종료 발표 ㄷ ㄷ


강웅석  6:06 PM
ㄷ ㄷ


혁규 권  6:09 PM
지금까지 "LUNA"였습니다


강웅석  6:09 PM
THANK YOU.


지윤  6:12 PM
ㅎㄷ


이종찬  6:16 PM
달나라 가겠다는 의미로 루나였겠지?
6:16
달나라가 아니라 달동네로 가겠누 ㄷ ㄷ


지윤  6:17 PM
ㅋㅋㅋㅋ달동내


선명 이  6:18 PM
엌ㅋㅋ 막히기전에 못나왓다


Vint S. Lee  7:22 PM
ㅋㅋㅋㅋㅋ











Message general








Shift + Enter to add a new line
Thread
general



void AFocusSBCharacter::OnPotion()
{
	if(mPotion == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("NO POTION LEFT!!"));
		return;
	}

	mPotion--;
	UseHP(HP * -1);
	
	if(OnPotionChange.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPotionChange Broadcast"));
		OnPotionChange.Broadcast();
	}
}


void AFocusSBCharacter::PauseTimer()
{

}

void AFocusSBCharacter::OnEnemyUpdate(float value)
{
	RemainTime = value;
}

void AFocusSBCharacter::OnPlayerUpdate(float value)
{
	 RemainTime = value;
}

void AFocusSBCharacter::OnEnemyEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("--------------------------Enemy Turn End-------------------------"));
	PlayerTL.PlayFromStart();
	CurrentTurn = ETurn::Player;
};
void AFocusSBCharacter::OnPlayerEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("-------------------------Player Turn End-------------------------"));
	EnemyTL.PlayFromStart();
	CurrentTurn = ETurn::Enemy;
	
	if(OnPlayerTurnEnd.IsBound())
	{
		OnPlayerTurnEnd.Broadcast();
	}

	MP = MP_MAX;
}

void AFocusSBCharacter::UseMP(const uint8& value)
{
	MP -= value;

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
