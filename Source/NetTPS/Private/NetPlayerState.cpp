// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerState.h"

#include "NetGameInstance.h"

void ANetPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if ( GetPlayerController() && GetPlayerController()->IsLocalController() )
	{
		auto gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
		ServerRPC_SetUserName(gi->mySessionName);			
	}
}

void ANetPlayerState::ServerRPC_SetUserName_Implementation(const FString& name)
{
	SetPlayerName(name);
}










