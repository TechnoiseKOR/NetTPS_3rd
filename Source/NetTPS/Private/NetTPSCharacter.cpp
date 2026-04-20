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
#include "InputActionValue.h"
#include "NetTPS.h"
#include "Kismet/GameplayStatics.h"

ANetTPSCharacter::ANetTPSCharacter()
{
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
		
		AttachPistol(pistolActor);
		
		break;
	}
	
	
	
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
}

void ANetTPSCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡고 있지 않다면 처리하지 않는다.
	if ( !bHasPistol )
	{
		return;
	}
	
	// 총 소유시
	if ( ownedPistol )
	{
		// 총 분리
		DetachPistol(ownedPistol);
		// 미소유로 설정
		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;		
	}
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);	
}





























































