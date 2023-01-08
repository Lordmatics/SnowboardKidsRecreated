// Lordmatics Games


#include "SnowboardKids/Data/BoardData.h"
#include <Engine/StaticMesh.h>

FBoardData::FBoardData() :
	ForwardSpeed(1000.0f),
	HorizontalSpeed(200.0f),
	Acceleration(100.0f),
	RecoverySpeed(250.0f),
	MinTurnSpeed(533.0f),
	MaxSpeed(1600.0f),
	MaxSpeedWhenCharged(2000.0f),
	JumpScale(600.0f),
	JumpForwardScale(125.0f),
	GravityScale(300.0f),
	TurnLimit(0.5f),
	TurnRateInterpSpeed(50.0f)
{
}

FBoardData::FBoardData(EBoardType BoardType)
{
	switch (BoardType)
	{
		case EBoardType::FreeStyle:
		{
			ForwardSpeed = 250.0f;// 900.0f;
			HorizontalSpeed = 400.0f;
			Acceleration = 320.0f;
			MaxSpeed = 1200.0f;
			MaxSpeedWhenCharged = 2000.0f;
			JumpScale = 800.0f;
			GravityScale = 450.0f;
			TurnLimit = 0.75f;
			TurnRateInterpSpeed =125.0f;
		}	break;
		case EBoardType::AllAround:
		{
			ForwardSpeed = 275.0f;// 1000.0f;
			HorizontalSpeed = 200.0f;
			Acceleration = 317.5f;
			MaxSpeed = 1300.0f;
			MaxSpeedWhenCharged = 2000.0f;
			JumpScale = 600.0f;
			GravityScale = 300.0f;
			TurnLimit = 0.5f;
			TurnRateInterpSpeed = 100.0f;
		}	break;
		case EBoardType::Alpine:
		{
			ForwardSpeed = 375.0f; // 1100.0f;
			HorizontalSpeed = 100.0f;
			Acceleration = 310.0f;
			MaxSpeed = 1400.0f;
			MaxSpeedWhenCharged = 2000.0f;
			JumpScale = 400.0f;
			GravityScale = 400.0f;
			TurnLimit = 0.33f;
			TurnRateInterpSpeed = 75.0f;
		}	break;
		case EBoardType::Special:
		{
			ForwardSpeed = 250.0f;// 1000.0f;
			HorizontalSpeed = 200.0f;
			Acceleration = 315.0f;
			MaxSpeed = 1300.0f;
			JumpScale = 600.0f;
			GravityScale = 300.0f;
			TurnLimit = 0.5f;
			TurnRateInterpSpeed = 100.0f;
		}	break;
		default:
			checkNoEntry();
			break;
	}
	JumpForwardScale = 125.0f;
	RecoverySpeed = 175.0f;
	MinTurnSpeed = MaxSpeed * 0.33f;
}

FBoardMeshes::FBoardMeshes() :
	FreeStyleMesh(nullptr),
	AllAroundMesh(nullptr),
	AlpineMesh(nullptr),
	SpecialMesh(nullptr)
{	
}

UStaticMesh* FBoardMeshes::GetMeshFromType(EBoardType BoardType) const
{
	switch (BoardType)
	{
	case EBoardType::FreeStyle:
		return FreeStyleMesh;
	case EBoardType::AllAround:
		return AllAroundMesh;
	case EBoardType::Alpine:
		return AlpineMesh;
	case EBoardType::Special:
		return SpecialMesh;
	default:
		checkNoEntry();
		break;
	}
	return nullptr;
}
