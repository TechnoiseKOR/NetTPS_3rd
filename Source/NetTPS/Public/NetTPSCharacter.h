// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ANetTPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	ANetTPSCharacter();	
	
	virtual void BeginPlay() override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


public:
	// 총을 자식으로 붙일 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GunComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* TakePistolAction;
	
	// 필요속성 : 총 소유 여부, 소유중인 총, 총 검색 범위
	UPROPERTY(Replicated)
	bool bHasPistol = false;
	
	// 소유중인 총
	UPROPERTY()
	AActor* ownedPistol = nullptr;
	
	// 총 검색 범위
	UPROPERTY(EditAnywhere, Category="Gun")
	float DistanceToGun = 200.0f;
	
	// 월드에 배치된 총들
	UPROPERTY()
	TArray<AActor*> pistolActors;
	
	
	
	
	
	
public:	
	void TakePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에 붙이기
	void AttachPistol(AActor* pistolActor);
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReleaseAction;
	
	// 총 놓기입력처리 함수
	void ReleasePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에서 분리
	void DetachPistol(AActor* pistolActor);
	
	// 총쏘기 입력액션
	UPROPERTY(EditAnywhere, blueprintReadOnly, Category="Input")
	UInputAction* FireAction;
	
	// 총쏘기 처리 함수
	void Fire(const FInputActionValue& Value);
	
	// 피격 파티클
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	class UParticleSystem* GunEffect;
	
	// 사용할 위젯 클래스
	//UPROPERTY(EditDefaultsOnly, Category="UI")
	//TSubclassOf<class UMainUI> mainUIWidget;
	// mainUIWidget 으로 부터 만들어진 인스턴스
	UPROPERTY()
	class UMainUI* mainUI;
	
	// UI 초기화 함수
	void InitUIWidget();
	
	// 최대 총알개수
	UPROPERTY(EditAnywhere, Category="Bullet")
	int32 MaxBulletCount = 10;
	// 남은 총알개수
	UPROPERTY(ReplicatedUsing=OnRep_BulletCount)
	int32 BulletCount = MaxBulletCount;
	
	UFUNCTION()
	void OnRep_BulletCount();
	
	// 재장전에서 사용할 입력액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReloadAction;
	
	// 재장전 입력처리 함수
	void ReloadPistol(const FInputActionValue& Value);
	
	// 총알 UI 초기화 함수
	void InitAmmoUI();
	// 재장전 중인지 기억
	bool IsReloading = false;
	
	// 플레이어 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HP")
	float MaxHP = 3.0f;
	// 현재 체력
	UPROPERTY(ReplicatedUsing=OnRep_HP, BlueprintReadOnly, Category="HP")
	float hp = MaxHP;
	
	UFUNCTION()
	void OnRep_HP();
	
	
	__declspec(property(get=GetHP, put=SetHP)) float HP;
	float GetHP();
	void SetHP(float value);
	
	
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* hpUIComp;
	
	// 피격처리
	void DamageProcess();
	
	// 사망여부
	bool isDead = false;
	
	// 카메라셰이크
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UCameraShakeBase> damageCameraShake;
	
	// 죽음처리
	void DieProcess();
	
	
public:
	virtual void Tick(float DeltaSeconds) override;
	// 네트워크 상태로그 출력함수
	void PrintNetLog();
	
	
	virtual void PossessedBy(AController* NewController) override;
	
	
	
	
	
	//---------------------- Multiplayer 요소들 ----------------------
public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_TakePistol(AActor* pistolActor);
	
	// 총놓기 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ReleasePistol(AActor* pistolActor);
	
	// 총쏘기
	UFUNCTION(Server, Reliable)
	void ServerRPC_Fire();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_Fire(bool bHit, const FHitResult& hitInfo);
	
	// 재장전
	UFUNCTION(Server, Reliable)
	void ServerRPC_Reload();
	UFUNCTION(Client, Reliable)
	void ClientRPC_Reload();
	
	
	
	
	
	
	
	
};






























