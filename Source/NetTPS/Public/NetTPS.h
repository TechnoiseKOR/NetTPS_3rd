// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogNetTPS, Log, All);

#define LOCALROLE (UEnum::GetValueAsString<ENetRole>(GetLocalRole()))
#define REMOTEROLE (UEnum::GetValueAsString<ENetRole>(GetRemoteRole()))
