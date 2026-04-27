// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	bool bHasPistol = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float myDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float mySpeed;
		
	UPROPERTY()
	class ANetTPSCharacter* player;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


public:
	// 총쏘기에서 사용할 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* FireMontage;
	
	// 총쏘기 애니메이션 재생
	void PlayFireAnimation();
	
	// 회전값 기억변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float pitchAngle;
	
	// 재장전 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* ReloadMontage;
	// 재장전 애니메이션 재생
	void PlayReloadAnimation();
	
	// 재장전 애니메이션 노티파이이벤트
	UFUNCTION()
	void AnimNotify_OnReloadFinish();
	
	UFUNCTION()
	void AnimNotify_DieEnd();
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	bool isDead = false;












	
};
