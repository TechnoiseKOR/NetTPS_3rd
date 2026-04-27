// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"

#include "NetTPSGameMode.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if ( HasAuthority() )
	{
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
	}
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	// 현재 폰을 기억한다.
	auto player = GetPawn();
	
	UnPossess();
	
	player->Destroy();
	
	gm->RestartPlayer(this);
	
}














