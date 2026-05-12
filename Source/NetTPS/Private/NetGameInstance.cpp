// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include <string>

#include "NetTPS.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Compression/lz4.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();
	
	if ( auto subsys = IOnlineSubsystem::Get() )
	{
		// 서브시스템으로부터 세션인터페이스를 가져온다.
		sessionInterface = subsys->GetSessionInterface();
		
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
		sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnJoinSessionComplete);
		sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnDestroySessionComplete);
		
		/*
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle,
			FTimerDelegate::CreateLambda([&]
			{
				//CreateSession(mySessionName, 10);
				FindOtherSessions();
			}			
			), 2, false	);
			*/
		
	}
	
	if ( GEngine )
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UNetGameInstance::OnNetworkFailure);
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
	sessionSettings.Set(FName("ROOM_NAME"), StringBase64Encode(roomName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	// 8. 호스트네임 설정	
	sessionSettings.Set(FName("HOST_NAME"), StringBase64Encode(mySessionName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	
	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
		
	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
	
	
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName,
	bool bWasSuccessful)
{
	PRINTLOG(TEXT("Session Name : %s, bWasSuccessful : %d"), *mySessionName, bWasSuccessful);
	if ( bWasSuccessful )
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/Net/Maps/LobbyMap")), true, TEXT("listen?port=7777"));		
	}
}

void UNetGameInstance::GameToStart()
{
	GetWorld()->ServerTravel(TEXT("/Game/Net/Maps/BattleMap?listen?port=7777"));
}

void UNetGameInstance::FindOtherSessions()
{
	onSearchState.Broadcast(true);
	
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	
	// 1. 세션 검색 조건 설정
	sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	// 2. Lan 여부
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	
	// 3. 최대 검색 세션 수
	sessionSearch->MaxSearchResults = 10;
	
	// 4. 세션검색	
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 찾기 실패시
	if (bWasSuccessful == false)
	{
		PRINTLOG(TEXT("Session search failed..."));
		return;
	}
	
	//  세션검색결과 배열
	auto results = sessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());
	
	// 유효성 체크
	for ( int i = 0; i < results.Num(); i++ )	
	//for ( auto sr : results )
	{
		auto sr = results[i];
		if ( sr.IsValid() == false )
		{
			continue;
		}
		
		// 세션정보 구조체선언
		FSessionInfo sessionInfo;
		sessionInfo.index = i;
		
		FString roomName;
		FString hostName;				
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);		
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		
		sessionInfo.roomName = StringBase64Decode(roomName);
		sessionInfo.hostName = StringBase64Decode(hostName);
		
		// 세션주인(방장) 이름
		//FString userName = sr.Session.OwningUserName;
		
		// 입장가능한 플레이어 수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;
		// 현재 입장한 플레이어 수 ( 최대인원수 - 현재입장가능한 수 )
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
		
		sessionInfo.playerCount = FString::Printf(TEXT("(%d/%d)"), currentPlayerCount, maxPlayerCount);;
		
		// 핑 정보
		sessionInfo.pingSpeed = sr.PingInMs;
		
		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());
		//PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
		
		// 델리게이트로 위젯에 알려주기		
		onSearchCompleted.Broadcast(sessionInfo);		
	}	
	
	onSearchState.Broadcast(false);
}

void UNetGameInstance::JoinSelectedSession(int32 index)
{
	auto sr = sessionSearch->SearchResults;
	sessionInterface->JoinSession(0, FName(mySessionName), sr[index]); 
}

void UNetGameInstance::OnJoinSessionComplete(FName sessionName,
	EOnJoinSessionCompleteResult::Type result)
{
	if ( result == EOnJoinSessionCompleteResult::Type::Success )
	{
		auto pc = GetWorld()->GetFirstPlayerController();
		FString url;
		sessionInterface->GetResolvedConnectString(sessionName, url);
		
		PRINTLOG(TEXT("Join URL : %s"), *url);
		if ( url.IsEmpty() == false )
		{
			pc->ClientTravel(url, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		PRINTLOG(TEXT("Join Session Failed : %d"), result);
	}
}

void UNetGameInstance::ExitRoom()
{
	sessionInterface->DestroySession(FName(*mySessionName));
}

void UNetGameInstance::OnDestroySessionComplete(FName sessionName,
	bool bWasSuccessful)
{
	auto* pc = GetWorld()->GetFirstPlayerController();
	FString url = TEXT("/Game/Net/Maps/LobbyMap");
	pc->ClientTravel(url, TRAVEL_Absolute);
}

void UNetGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	if ( FailureType == ENetworkFailure::Type::ConnectionLost )
	{
		ExitRoom();
	}
}

bool UNetGameInstance::IsInRoom()
{
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	return sessionInterface->IsPlayerInSession(FName(*mySessionName), *netID);
}

FString UNetGameInstance::StringBase64Encode(const FString& str)
{
	// Set 할 때 :: FString -> UTF8(std::string) -> TArray<uint8> -> base64 로 Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)(utf8String.c_str()), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetGameInstance::StringBase64Decode(const FString& str)
{
	// Get 할 때 :: base64 로 Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)(arrayData.GetData()), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

















