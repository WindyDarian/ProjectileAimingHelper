// 2017 WindyDarian (RuoyuF). MIT License

#pragma once


#include "Kismet/BlueprintFunctionLibrary.h"
#include "HelperLibrary.generated.h"

/**
 *
 */
UCLASS()
class WILDHUNT_API UHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HelperLibrary")
	static FVector CalculateProjectileDirection(FVector Target, FVector Origin, FVector Gravity, float Speed, bool& bWillHit, float& Time);


	/**
	* Calculate a projectile direction to hit a target with constant velocity
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HelperLibrary")
	static FVector CalculateProjectileDirectionForMovingTarget(FVector Target, FVector TargetVelocity, FVector Origin, FVector Gravity, float Speed, bool& bWillHit, float& Time, int Iterations = 5, float EpsilonTime = 0.01f);

};
