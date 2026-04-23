// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"

#include "EngineUtils.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"
#include <Net/UnrealNetwork.h>

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = meshComp;
	meshComp->SetRelativeScale3D(FVector(0.5f));

}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PrintNetLog();
	
	FindOwner();
	
	
	// Server 일 경우
	if ( HasAuthority() )
	{
		AddActorLocalRotation(FRotator(0.0f, 50.0f * DeltaTime, 0.0f));
		RotYaw = GetActorRotation().Yaw;
	}
	else
	{
				
	}	
	
	
	
	DrawDebugSphere(GetWorld(), GetActorLocation(), searchDistance, 30, FColor::Yellow, false, 0, 0, 1);	
}

void ANetActor::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
		
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");
	
	const FString logStr = FString::Printf(TEXT("Connection : %s\nLocalRole : %s\nRemoteRole : %s\nOwnerName : %s"), *conStr, *LOCALROLE, *REMOTEROLE, *ownerName);
	
	DrawDebugString( GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::White, 0, true, 1);
	
	/*
	
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwnerName : %s"), *conStr, *ownerName);
	
	DrawDebugString( GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::White, 0, true, 1);
	*/
}

void ANetActor::FindOwner()
{
	if ( HasAuthority())
	{
		AActor* newOwner = nullptr;
		float minDist = searchDistance;
		
		for ( TActorIterator<ANetTPSCharacter> it(GetWorld()); it; ++it)
		{
			AActor* otherActor = *it;
			float dist = GetDistanceTo(otherActor);
			
			if ( dist < minDist )
			{
				minDist = dist;
				newOwner = otherActor;
			}
		}
		
		// Owner 설정
		if ( GetOwner() != newOwner )
		{
			SetOwner(newOwner);			
		}			
	}
}

void ANetActor::OnRep_RotYaw()
{
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = RotYaw;
	SetActorRotation(NewRot);
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANetActor, RotYaw);
}


















