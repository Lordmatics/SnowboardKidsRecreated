// Lordmatics Games


#include "SnowboardKids/Controllers/SnowboardPlayerController.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include "../CustomCameras/SnowboardPlayerCamera.h"
#include <Kismet/GameplayStatics.h>

ASnowboardPlayerController::ASnowboardPlayerController() :
	PossessedPawn(nullptr),
	bRotationDisabled(false)
{

}

void ASnowboardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	check(InputComponent);

	InputComponent->BindAction("North", IE_Pressed, this, &ASnowboardPlayerController::OnNorthPressed);
	InputComponent->BindAction("North", IE_Released, this, &ASnowboardPlayerController::OnNorthReleased);

	InputComponent->BindAction("East", IE_Pressed, this, &ASnowboardPlayerController::OnEastPressed);
	InputComponent->BindAction("East", IE_Released, this, &ASnowboardPlayerController::OnEastReleased);

	InputComponent->BindAction("South", IE_Pressed, this, &ASnowboardPlayerController::OnSouthPressed);
	InputComponent->BindAction("South", IE_Released, this, &ASnowboardPlayerController::OnSouthReleased);

	InputComponent->BindAction("West", IE_Pressed, this, &ASnowboardPlayerController::OnWestPressed);
	InputComponent->BindAction("West", IE_Released, this, &ASnowboardPlayerController::OnWestReleased);

	InputComponent->BindAction("TrickUp", IE_Pressed, this, &ASnowboardPlayerController::OnDPadNorthPressed);
	InputComponent->BindAction("TrickUp", IE_Released, this, &ASnowboardPlayerController::OnDPadNorthReleased);
	InputComponent->BindAction("TrickRight", IE_Pressed, this, &ASnowboardPlayerController::OnDPadEastPressed);
	InputComponent->BindAction("TrickRight", IE_Released, this, &ASnowboardPlayerController::OnDPadEastReleased);
	InputComponent->BindAction("TrickDown", IE_Pressed, this, &ASnowboardPlayerController::OnDPadSouthPressed);
	InputComponent->BindAction("TrickDown", IE_Released, this, &ASnowboardPlayerController::OnDPadSouthReleased);
	InputComponent->BindAction("TrickLeft", IE_Pressed, this, &ASnowboardPlayerController::OnDPadWestPressed);
	InputComponent->BindAction("TrickLeft", IE_Released, this, &ASnowboardPlayerController::OnDPadWestReleased);

	InputComponent->BindAxis("MoveForward", this, &ASnowboardPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASnowboardPlayerController::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &ASnowboardPlayerController::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ASnowboardPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ASnowboardPlayerController::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ASnowboardPlayerController::LookUpAtRate);
}

void ASnowboardPlayerController::ConstrainYawToPlayer(bool Value)
{
	if (PlayerCamera)
	{
		PlayerCamera->ConstrainYawToPlayer(Value);
	}
}

void ASnowboardPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());
	
	if (ASnowboardPlayerCamera* Camera = Cast<ASnowboardPlayerCamera>(UGameplayStatics::GetActorOfClass(this, CameraClass)))
	{
		PlayerCamera = Camera;
		
		if (PossessedPawn)
		{
			Camera->UpdateCamera(*PossessedPawn);
		}		
	}
}

void ASnowboardPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DeltaSeconds <= 0.0f)
	{
		return;
	}

	if (!PossessedPawn)
	{
		PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());
	}
}

void ASnowboardPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	ProcessCamera(DeltaTime);
}

void ASnowboardPlayerController::ProcessCamera(float DeltaTime)
{
	ASnowboardPlayerCamera* Camera = PlayerCamera;
	if (!PlayerCamera)
	{
		return;
	}

	ASnowboardCharacterBase* PossessedPlayer = PossessedPawn;
	if (!PossessedPlayer)
	{
		return;
	}


	AActor* ViewTarget = GetViewTarget();

	Camera->UpdateCamera(*PossessedPlayer);
}

void ASnowboardPlayerController::OnNorthPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnNorthPressed();
}

void ASnowboardPlayerController::OnNorthReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnNorthReleased();
}

void ASnowboardPlayerController::OnEastPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnEastPressed();
}

void ASnowboardPlayerController::OnEastReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnEastReleased();
}

void ASnowboardPlayerController::OnSouthPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnSouthPressed();
}

void ASnowboardPlayerController::OnSouthReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnSouthReleased();
}

void ASnowboardPlayerController::OnWestPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnWestPressed();
}

void ASnowboardPlayerController::OnWestReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnWestReleased();
}

void ASnowboardPlayerController::OnDPadNorthPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadNorthPressed();
}

void ASnowboardPlayerController::OnDPadNorthReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadNorthReleased();
}

void ASnowboardPlayerController::OnDPadEastPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadEastPressed();
}

void ASnowboardPlayerController::OnDPadEastReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadEastReleased();
}

void ASnowboardPlayerController::OnDPadSouthPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadSouthPressed();
}

void ASnowboardPlayerController::OnDPadSouthReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadSouthReleased();
}

void ASnowboardPlayerController::OnDPadWestPressed()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadWestPressed();
}

void ASnowboardPlayerController::OnDPadWestReleased()
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->OnDPadWestReleased();
}

void ASnowboardPlayerController::MoveForward(float Value)
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->MoveForward(Value);
}

void ASnowboardPlayerController::MoveRight(float Value)
{
	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->MoveRight(Value);
	//if(Value != 0.0f)
	//	UE_LOG(LogTemp, Log, TEXT("MoveRight: %.1f"), Value);
}

void ASnowboardPlayerController::TurnAtRate(float Rate)
{
	if (bRotationDisabled)
	{
		return;
	}

	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->TurnAtRate(Rate);
}

void ASnowboardPlayerController::LookUpAtRate(float Rate)
{
	if (bRotationDisabled)
	{
		return;
	}

	if (!PossessedPawn)
	{
		return;
	}

	PossessedPawn->LookUpAtRate(Rate);
}

void ASnowboardPlayerController::AddControllerYawInput(float Value)
{
	if (bRotationDisabled)
	{
		return;
	}

	if (!PossessedPawn)
	{
		return;
	}
	
	AddYawInput(Value);
}

void ASnowboardPlayerController::AddControllerPitchInput(float Value)
{
	if (bRotationDisabled)
	{
		return;
	}

	if (!PossessedPawn)
	{
		return;
	}
	
	Value *= -1.0f;
	AddPitchInput(Value);
}
