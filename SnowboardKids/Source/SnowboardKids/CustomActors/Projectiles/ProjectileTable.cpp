// Lordmatics Games


#include "SnowboardKids/CustomActors/Projectiles/ProjectileTable.h"
#include "ProjectileBase.h"

UProjectileTable::UProjectileTable() :
	Super()
{

}

FProjectileTableRow::FProjectileTableRow() :
	Super(),
	OffensiveItemType(EOffensiveType::Hands),
	OffensiveClass(nullptr)
{	
}
