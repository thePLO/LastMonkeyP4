// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "monkey.h"
#include "monkeyGameMode.h"
#include "monkeyCharacter.h"

AmonkeyGameMode::AmonkeyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<SWidget> hudCDs(TEXT("/Game/hud"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
