// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include "NetTPS.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UNetGameInstance::Init()
{
	Super::Init();
	
	if ( auto subsys = IOnlineSubsystem::Get() )
	{
		// 서브시스템으로부터 세션인터페이스를 가져온다.
		sessionInterface = subsys->GetSessionInterface();
		
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
		
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle,
			FTimerDelegate::CreateLambda([&]
			{
				CreateSession(mySessionName, 10);
			}			
			), 2, false	);
	}
}

void UNetGameInstance::CreateSession(FString roomName, int32 playerCount)
{
	// 세션설정변수
	FOnlineSessionSettings sessionSettings;
	
	// 1. Dedicated Server 접속여부
	sessionSettings.bIsDedicated = false;
	
	// 2. 랜선(로컬)매칭을 할지 Steam 매칭을 사용할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = subsysName == "NULL";
	
	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false 이면 초대를 통해서만 입장이 가능하다.
	sessionSettings.bShouldAdvertise = true;
	
	// 4. 온라인 상태(presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;
	
	// 5. 게임진행중에 참여를 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;
	
	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	sessionSettings.NumPublicConnections = playerCount;
	
	// 7. 커스텀 룸네임 설정
	sessionSettings.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	// 8. 호스트네임 설정	
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	
	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
		
	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
	
	
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName,
	bool bWasSuccessful)
{
	PRINTLOG(TEXT("Session Name : %s, bWasSuccessful : %d"), *mySessionName, bWasSuccessful);
}

















