// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowboardKidsEditorModePluginEdModeToolkit.h"
#include "SnowboardKidsEditorModePluginEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include <Widgets/Input/SNumericEntryBox.h>
#include "Slate/FloatWidget.h"

#define LOCTEXT_NAMESPACE "FSnowboardKidsEditorModePluginEdModeToolkit"

FSnowboardKidsEditorModePluginEdModeToolkit::FSnowboardKidsEditorModePluginEdModeToolkit()
{
}

void FSnowboardKidsEditorModePluginEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	struct Locals
	{
		static bool IsWidgetEnabled()
		{
			if (!GEditor)
			{
				return false;
			}

			USelection* CurrentSelection = GEditor->GetSelectedActors();
			if (!CurrentSelection)
			{
				return false;
			}
			
			const int NumSelected = CurrentSelection->Num();
			const bool bIsWidgetEnabled = NumSelected != 0;
			return bIsWidgetEnabled;
		}

		static bool GetSelectedObjects(TArray<AActor*>& SelectedObjects)
		{
			if (!GEditor)
			{
				return false;
			}

			USelection* CurrentSelection = GEditor->GetSelectedActors();
			if (!CurrentSelection)
			{
				return false;
			}

			CurrentSelection->GetSelectedObjects<AActor>(SelectedObjects);		
			return true;
		}

		enum class ERayDir : uint8
		{
			Up = 0,
			Forward,
			Right
		};

		static FReply OnDisplacementButtonClick(ERayDir RayDir, float Amount)
		{
			USelection* SelectedActors = GEditor->GetSelectedActors();

			// Let editor know that we're about to do something that we want to undo/redo
			GEditor->BeginTransaction(LOCTEXT("MoveActorsTransactionName", "MoveActors"));

			// For each selected actor
			for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
			{
				if (AActor* LevelActor = Cast<AActor>(*Iter))
				{
					// Register actor in opened transaction (undo/redo)
					LevelActor->Modify();
					// Move actor to given location
					FVector ActorLocation = LevelActor->GetActorLocation();
					FVector Dir = FVector::ZeroVector;
					switch (RayDir)
					{
					case ERayDir::Up:
						Dir = LevelActor->GetActorUpVector();
						break;
					case ERayDir::Forward:
						Dir = LevelActor->GetActorForwardVector();
						break;
					case ERayDir::Right:
						Dir = LevelActor->GetActorRightVector();
						break;
					default:
						checkNoEntry();
						break;
					}
					FVector FinalLocation = ActorLocation + Dir * Amount;
					const FRotator& ActorRotation = LevelActor->GetActorRotation();
					LevelActor->TeleportTo(FinalLocation, ActorRotation);
				}
			}

			// We're done moving actors so close transaction
			GEditor->EndTransaction();

			return FReply::Handled();
		}

		static bool ShootRay(AActor* LevelActor, FHitResult& OutHit, ERayDir RayDir, const float RayLength)
		{
			UWorld* World = LevelActor->GetWorld();
			if (!World)
			{
				return false;
			}

			FVector DirVec = FVector::ZeroVector;
			switch (RayDir)
			{
			case ERayDir::Up:
				DirVec = LevelActor->GetActorUpVector();
				break;
			case ERayDir::Forward:
				DirVec = LevelActor->GetActorForwardVector();
				break;
			case ERayDir::Right:
				DirVec = LevelActor->GetActorRightVector();
				break;
			default:
				checkNoEntry()
				return false;
			}
			const FVector& Start = LevelActor->GetActorLocation();
			const FVector End = Start + DirVec * RayLength;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(LevelActor);

			const bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_WorldStatic, QueryParams);
			if (!bHit)
			{
				return false;
			}

			return true;
		}

		static bool FindWalls(AActor* LevelActor, FHitResult& Left, FHitResult& Right)
		{
			const float RayLength = 5000.0f;
			bool bWallHit = ShootRay(LevelActor, Left, ERayDir::Right, -RayLength);
			if (!bWallHit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Centre-ing Actor No Left Wall Found!"));
				return false;
			}

			bWallHit = ShootRay(LevelActor, Right, ERayDir::Right, RayLength);
			if (!bWallHit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Centre-ing Actor No Right Wall Found!"));
				return false;
			}

			return true;
		}
		static bool FindGround(AActor* LevelActor, FHitResult& Floor)
		{
			const float RayLength = 5000.0f;
			bool bFloorHit = ShootRay(LevelActor, Floor, ERayDir::Up, -RayLength);
			if (!bFloorHit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Centre-ing Actor No Left Wall Found!"));
				return false;
			}

			return true;
		}

		static FReply OnCentreButtonClick()
		{
			UE_LOG(LogTemp, Log, TEXT("Centre-ing Actor Start!"));
			USelection* SelectedActors = GEditor->GetSelectedActors();
			GEditor->BeginTransaction(LOCTEXT("Centre Actors To Geometry", "CentreActors"));
			for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
			{
				if (AActor* LevelActor = Cast<AActor>(*Iter))
				{
					LevelActor->Modify();
					FHitResult Left, Right, Floor;					
					const bool bWallsFound = FindWalls(LevelActor, Left, Right);
					const bool bGroundFound = FindGround(LevelActor, Floor);
					if (bWallsFound)
					{
						const float LeftDistance = Left.Distance;
						const float RightDistance = Right.Distance;
						const float Difference = RightDistance - LeftDistance;
						const FVector& RightVec = LevelActor->GetActorRightVector();
						const FVector& LevelActorLocation = LevelActor->GetActorLocation();
						// Logic: If dist L = 70, Dist R = 30, Total Length = 100.
						// Centre = Total / 2 == 100 / 2 == 50
						// Difference = 70 - 30 = 40 OR 30 - 70 = -40
						// To reach centre, we need half the difference, 70 + (-40 / 2) = 50
						// Or 30 + (40 / 2) = 50
						FVector DisplacementLocation = RightVec * Difference * 0.5f;
						const FVector FinalLocation = LevelActorLocation + DisplacementLocation;
						const FRotator& ActorRotation = LevelActor->GetActorRotation();
						LevelActor->TeleportTo(FinalLocation, ActorRotation);
						UE_LOG(LogTemp, Log, TEXT("Centre-ing Actor Teleport!"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Centre-ing Actor No Walls Found!"));
					}
				}
			}
			GEditor->EndTransaction();
			UE_LOG(LogTemp, Log, TEXT("Centre-ing Actor End!"));
			return FReply::Handled();
		}

		static FReply OnGroundButtonClick()
		{
			UE_LOG(LogTemp, Log, TEXT("Floor-ing Actor Start!"));
			USelection* SelectedActors = GEditor->GetSelectedActors();
			GEditor->BeginTransaction(LOCTEXT("Floor Actors To Geometry", "FlooringActors"));
			for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
			{
				if (AActor* LevelActor = Cast<AActor>(*Iter))
				{
					LevelActor->Modify();
					FHitResult Floor;
					const bool bGroundFound = FindGround(LevelActor, Floor);
					if (bGroundFound)
					{
						const FVector& UpVec = LevelActor->GetActorUpVector();
						const FVector& LevelActorLocation = LevelActor->GetActorLocation();
	
						// Snap to floor.
						const float GroundDistance = Floor.Distance * 0.75f;
						FVector DisplacementLocation = -UpVec * GroundDistance;

						const FVector FinalLocation = LevelActorLocation + DisplacementLocation;
						const FRotator& ActorRotation = LevelActor->GetActorRotation();
						LevelActor->TeleportTo(FinalLocation, ActorRotation);
						UE_LOG(LogTemp, Log, TEXT("Floor-ing Actor Teleport!"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Floor-ing Actor No Floor Found!"));
					}
				}
			}
			GEditor->EndTransaction();
			UE_LOG(LogTemp, Log, TEXT("Floor-ing Actor End!"));
			return FReply::Handled();
		}

		static FReply OnCentreAndGroundButtonClick()
		{
			OnCentreButtonClick();
			OnGroundButtonClick();
			return FReply::Handled();
		}

		// NOTE: These can take extra parameters.
		static TSharedRef<SWidget> MakeDisplacementButton(FText InLabel, ERayDir Dir, float Amount)
		{
			return SNew(SButton)
				.Text(InLabel)
				.OnClicked_Static(&Locals::OnDisplacementButtonClick, Dir, Amount);
		}

		static TSharedRef<SWidget> MakeCentreButtonToGeometry(FText InLabel)
		{
			return SNew(SButton)
				.Text(InLabel)
				.OnClicked_Static(&Locals::OnCentreButtonClick);
		}

		static TSharedRef<SWidget> MakeGroundButtonToGeometry(FText InLabel)
		{
			return SNew(SButton)
				.Text(InLabel)
				.OnClicked_Static(&Locals::OnGroundButtonClick);
		}

		static TSharedRef<SWidget> MakeCentreGroundButtonToGeometry(FText InLabel)
		{
			return SNew(SButton)
				.Text(InLabel)
				.OnClicked_Static(&Locals::OnCentreAndGroundButtonClick);
		}
	};

	const float Factor = 256.0f;

	TArray<AActor*> SelectedObjects;
	Locals::GetSelectedObjects(SelectedObjects);

	const int NumSelected = SelectedObjects.Num();
	UE_LOG(LogTemp, Log, TEXT("FSnowboardKidsEditorModePluginEdModeToolkit::Init: %d"), NumSelected);

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		.IsEnabled_Static(&Locals::IsWidgetEnabled)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(25)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabel", "Select some actors and move them around using buttons below"))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(5)
			[
				SNew(SActorCollectionToTextWidget)
				.Collection(SelectedObjects)
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(5)
			[
				SNew(SFloatPropertyWidget)
				.Value(256.0f)
				.Min(0.0f)
				.Max(1024.0f)
				.Text(LOCTEXT("OffsetValue", "Offset Value"))
			]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(5)
				[
					Locals::MakeDisplacementButton(LOCTEXT("UpButtonLabel", "Up"), Locals::ERayDir::Up, Factor)
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(5)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5)
					[
						Locals::MakeDisplacementButton(LOCTEXT("LeftButtonLabel", "Left"), Locals::ERayDir::Right, -Factor)
					]
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(5)
						[
							Locals::MakeDisplacementButton(LOCTEXT("RightButtonLabel", "Right"), Locals::ERayDir::Right, Factor)
						]
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(10)
				[
					Locals::MakeDisplacementButton(LOCTEXT("DownButtonLabel", "Down"), Locals::ERayDir::Up, -Factor)
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabelCentre", "Centre Actor Will scan for static walls. Following the Green Local Arrow from the Actor."))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(5)
				[
					Locals::MakeCentreButtonToGeometry(LOCTEXT("CentreButtonLabel", "Centre Actor"))
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("HelperLabelGround", "Ground Actor Will scan for a static floor. Following the blue Local Arrow from the Actor."))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(5)
				[
					Locals::MakeGroundButtonToGeometry(LOCTEXT("GroundButtonLabel", "Ground Actor"))
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("HelperLabelCentreGround", "Centre + Ground Actor will centre the actor, then from the new centre position, ground the actor - green then blue arrows respectively."))
				]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(5)
			[
				Locals::MakeCentreGroundButtonToGeometry(LOCTEXT("GroundAndCentreButtonLabel", "Centre + Ground Actor"))
			]			
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FName FSnowboardKidsEditorModePluginEdModeToolkit::GetToolkitFName() const
{
	return FName("SnowboardKidsEditorModePluginEdMode");
}

FText FSnowboardKidsEditorModePluginEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("SnowboardKidsEditorModePluginEdModeToolkit", "DisplayName", "SnowboardKidsEditorModePluginEdMode Tool");
}

class FEdMode* FSnowboardKidsEditorModePluginEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FSnowboardKidsEditorModePluginEdMode::EM_SnowboardKidsEditorModePluginEdModeId);
}

#undef LOCTEXT_NAMESPACE


