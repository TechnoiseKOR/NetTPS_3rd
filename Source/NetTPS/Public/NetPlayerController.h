// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetTPSPlayerController.h"
#include "NetPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ANetPlayerController : public ANetTPSPlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class ANetTPSGameMode* gm;
	
	
public:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeToSpectator();
	
	
	// 사용할 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UMainUI> mainUIWidget;
	// mainUIWidget 으로 부터 만들어진 인스턴스
	UPROPERTY()
	class UMainUI* mainUI;
	
};

















