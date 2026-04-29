// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;	
	
	
public:
	IOnlineSessionPtr sessionInterface;
	
	void CreateSession(FString roomName, int32 playerCount);
	
	// 세션(호스트) 이름
	FString mySessionName = "Technoise";
	
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	
	
public:	// ------------- 방검색 -------------
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSessions();
	
	void OnFindSessionsComplete(bool bWasSuccessful);
	
	
	
};











