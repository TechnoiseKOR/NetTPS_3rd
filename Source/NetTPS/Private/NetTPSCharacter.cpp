// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthBar.h"
#include "InputActionValue.h"
#include "MainUI.h"
#include "NetPlayerAnimInstance.h"
#include "NetTPS.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ANetTPSCharacter::ANetTPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	//GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 40.0f, 60.0f));
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	GunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	GunComp->SetupAttachment(GetMesh(), TEXT("HandGrip_R"));
	GunComp->SetRelativeLocation(FVector(0.824356f,8.975311f,3.870210f));
	GunComp->SetRelativeRotation(FRotator(0.0f, 0.0f, 12.399401f));


	// HealthBar Component
	hpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	hpUIComp->SetupAttachment(GetMesh());








	
	
	
}

void ANetTPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 총 검색
	TArray<AActor*> allActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
	for ( auto tempPistol : allActors )
	{
		if ( tempPistol->GetName().Contains("BP_Pistol") )
		{
			pistolActors.Add(tempPistol);
		}
	}
	
	InitUIWidget();
	
	
	
	
	
	
	
	
}

void ANetTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);
		
		// TakePistol
		EnhancedInputComponent->BindAction(TakePistolAction, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);
		
		// ReleasePistol
		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &ANetTPSCharacter::ReleasePistol);
		
		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);
		
		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANetTPSCharacter::ReloadPistol);
		
		
	}
	else
	{
		UE_LOG(LogNetTPS, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ANetTPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ANetTPSCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ANetTPSCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ANetTPSCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ANetTPSCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ANetTPSCharacter::TakePistol(const FInputActionValue& Value)
{
	// 총을 소유하지 않고 있다면 범위 안에 있는 총을 잡는다.
	
	// 필요속성 : 총 소유 여부, 소유중인 총, 총 검색 범위
	// 1. 총을 잡고 있지 않다면
	if ( bHasPistol )
	{
		return;
	}
	
	ServerRPC_TakePistol();	
}

void ANetTPSCharacter::AttachPistol(AActor* pistolActor)
{
	if ( pistolActor == nullptr )
	{
		return;
	}
	
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(false);
	meshComp->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
	if ( IsLocallyControlled() && mainUI )
	{
		mainUI->ShowCrosshair(true);		
	}
}

void ANetTPSCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡고 있지 않다면 처리하지 않는다.
	if ( !bHasPistol )
	{
		return;
	}
	
	ServerRPC_ReleasePistol();	
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	
	if ( IsLocallyControlled() && mainUI )
	{
		mainUI->ShowCrosshair(false);		
	}
}

void ANetTPSCharacter::Fire(const FInputActionValue& Value)
{	
	// 총을 들고 있지 않다면 처리하지 않는다.
	// 총알이 0개면 처리하지 않는다.
	if ( !bHasPistol || BulletCount <= 0 || IsReloading )
	{
		return;
	}		
	
	ServerRPC_Fire();
	
	/*
	// 총쏘기
	FHitResult hitInfo;
	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 10000.0f;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);	
	
	if ( bHit )
	{
		// 맞은 부위에 파티클 표시
		if ( GunEffect )
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, hitInfo.Location, FRotator(), true);
		}
		
		// 맞은 대상이 상대방일 경우 데미지 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if ( otherPlayer )
		{
			otherPlayer->DamageProcess();
		}
		
	}
	*/
	
	
	
	
	
	
	
	
	
	
	
}

void ANetTPSCharacter::InitUIWidget()
{
	// Player 가 제어중이 아니라면 처리하지 않는다.
	auto pc = Cast<APlayerController>(Controller);
	if ( pc == nullptr )
	{
		return;
	}
	
	if ( mainUIWidget )
	{
		mainUI = Cast<UMainUI>(CreateWidget(GetWorld(), mainUIWidget));
		mainUI->AddToViewport();
		mainUI->ShowCrosshair(false);
		
		BulletCount = MaxBulletCount;
		// 총알추가
		for ( int i = 0; i < BulletCount; ++i )
		{
			mainUI->AddBullet();
		}
		
		// mainUI 가 있기 때문에 해당 컴포넌트는 비활성화
		if ( hpUIComp )
		{
			hpUIComp->SetVisibility(false);
		}
	}	
}

void ANetTPSCharacter::OnRep_BulletCount()
{
	if ( mainUI )
	{
		mainUI->PopBullet(BulletCount);			
	}
}

void ANetTPSCharacter::ReloadPistol(const FInputActionValue& Value)
{
	// 총 소지중이 아니라면 아무 처리하지 않는다.
	if ( !bHasPistol || IsReloading )
	{
		return;
	}
	
	// 재장전 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayReloadAnimation();
	
	IsReloading = true;
}

void ANetTPSCharacter::InitAmmoUI()
{
	ServerRPC_Reload();	
}

void ANetTPSCharacter::OnRep_HP()
{
	// UI 에 할당할 퍼센트 계산
	float percent = hp / MaxHP;
	
	if ( mainUI )
	{
		mainUI->HP = percent;
		// 피격효과 처리
		mainUI->PlayDamageAnimation();
		
		if ( damageCameraShake )
		{
			auto pc = Cast<APlayerController>(Controller);
			if ( pc )
			{
				pc->ClientStartCameraShake(damageCameraShake);
			}
		}
	}
	else
	{
		auto hpUI = Cast<UHealthBar>(hpUIComp->GetWidget());
		if ( hpUI )
		{
			hpUI->HP = percent;			
		}
	}	
}

float ANetTPSCharacter::GetHP()
{
	return hp;
}

void ANetTPSCharacter::SetHP(float value)
{
	hp = value;	
	OnRep_HP();
}

void ANetTPSCharacter::DamageProcess()
{
	// 체력을 감소시킨다.
	HP -= 1.0f;	
	
	// 사망처리 체크
	if ( HP <= 0.0f )
	{
		isDead = true;
	}
	
	
}

void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	PrintNetLog();	
	
	// 빌보드
	if ( hpUIComp && hpUIComp->GetVisibleFlag() )
	{
		FVector CamLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		FVector Direction = CamLoc - hpUIComp->GetComponentLocation();
		Direction.Z = 0.0f;
		hpUIComp->SetWorldRotation(Direction.GetSafeNormal().ToOrientationRotator());		
	}
}

void ANetTPSCharacter::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
		
	const FString logStr = FString::Printf(TEXT("Connection : %s\nLocalRole : %s\nRemoteRole : %s"), *conStr, *LOCALROLE, *REMOTEROLE);
	
	DrawDebugString( GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::White, 0, true, 1);
	
	/*
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");
	
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwnerName : %s"), *conStr, *ownerName);
	
	DrawDebugString( GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::White, 0, true, 1);
	*/
}

void ANetTPSCharacter::ClientRPC_Reload_Implementation()
{
	if ( mainUI )
	{
		// 총알 UI 제거
		mainUI->RemoveAllAmmo();
		
		// 총알 UI 다시 세팅
		for ( int i = 0; i < MaxBulletCount; i++ )
		{
			mainUI->AddBullet();
		}		
	}
	// 재장전 완료상태로 처리
	IsReloading = false;
}

void ANetTPSCharacter::ServerRPC_Reload_Implementation()
{	
	// 총알 개수 초기화
	BulletCount = MaxBulletCount;
	ClientRPC_Reload();
}

void ANetTPSCharacter::MulticastRPC_Fire_Implementation(bool bHit,
                                                        const FHitResult& hitInfo)
{
	if ( bHit )
	{
		// 맞은 부위에 파티클 표시
		if ( GunEffect )
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, hitInfo.Location, FRotator(), true);
		}
				
		// Fire 애니메이션 재생
		auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
		if ( anim != nullptr )
		{
			anim->PlayFireAnimation();
		}
	}	
}

void ANetTPSCharacter::ServerRPC_Fire_Implementation()
{
	// 총쏘기
	FHitResult hitInfo;
	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 10000.0f;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);	
	
	if ( bHit )
	{	
		// 맞은 대상이 상대방일 경우 데미지 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if ( otherPlayer )
		{
			otherPlayer->DamageProcess();
		}		
	}
	
	// 총알 제거
	BulletCount--;
	OnRep_BulletCount();
	
	MulticastRPC_Fire(bHit, hitInfo);
}

void ANetTPSCharacter::MulticastRPC_ReleasePistol_Implementation(
	AActor* pistolActor)
{
	// 총 분리
	DetachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 총 소유시
	if ( ownedPistol )
	{
		MulticastRPC_ReleasePistol(ownedPistol);
		
		// 미소유로 설정
		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;		
	}
}

void ANetTPSCharacter::MulticastRPC_TakePistol_Implementation(
	AActor* pistolActor)
{
	// 총 붙이기
	AttachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_TakePistol_Implementation()
{	
	// 2. 월드에 있는 총을 모두 조사한다.
	for ( auto pistolActor : pistolActors )
	{
		// 3. 만약 총의 소유자가 있다면 검사하지 않는다.
		if ( pistolActor->GetOwner() != nullptr )
		{
			continue;
		}

		// 4. 총과의 거리를 구한다.
		float Distance = FVector::Dist(GetActorLocation(), pistolActor->GetActorLocation());
		
		// 5. 거리가 범위 안에 있다면
		if ( Distance > DistanceToGun )
		{
			continue;
		}
		
		// 6. 소유중인 총으로 등록
		ownedPistol = pistolActor;
		
		// 7. 총의 소유자를 자신으로 등록
		ownedPistol->SetOwner(this);
		
		// 8. 총 소유 상태로 변경
		bHasPistol = true;
		
		MulticastRPC_TakePistol(pistolActor);		
		
		break;
	}
}


void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANetTPSCharacter,bHasPistol);
	DOREPLIFETIME(ANetTPSCharacter,BulletCount);
	DOREPLIFETIME(ANetTPSCharacter,hp);
}


























































