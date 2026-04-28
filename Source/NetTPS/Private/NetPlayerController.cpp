// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"

#include "NetTPSGameMode.h"
#include "GameFramework/SpectatorPawn.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if ( HasAuthority() )
	{
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
	}
}

void ANetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	// 관전자가 플레이어의 위치에서 생성될 수 있도록 플레이어 정보를 가져온다.
	APawn* player = GetPawn();
	if ( player )
	{
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// 관전자 생성
		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform(), params);
		
		Possess(spectator);
		
		// 이전 플레이어는 제거
		player->Destroy();
		
		
		// 5초후에 리스폰시키기
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &ANetPlayerController::ServerRPC_RespawnPlayer_Implementation, 5.0f, false);		
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














