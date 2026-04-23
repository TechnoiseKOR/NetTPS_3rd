// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class NETTPS_API ANetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* meshComp;
	
	void PrintNetLog();
	
	// Onwer 검출 영역
	UPROPERTY(EditAnywhere)
	float searchDistance = 200.0f;
	
	// Owner설정
	void FindOwner();
	
	
	// 회전 값 동기화 변수
	UPROPERTY(ReplicatedUsing=OnRep_RotYaw)
	float RotYaw = 0.0f;
	
	UFUNCTION()
	void OnRep_RotYaw();
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

};
