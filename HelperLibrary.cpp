// 2017 WindyDarian (RuoyuF). MIT License

#include "thegame.h"
#include "HelperLibrary.h"

#include <limits>

constexpr auto InfFloat = std::numeric_limits<float>::max();

float CalculateProjectileDirectionOneDimensionHelper(float Target, float Gravity, float Speed, bool * bWillHit, float * Time)
{
	// Helper function for 1D special case
	// Gravity should always be positive
	if (Target >= 0)
	{
		if (bWillHit) { *bWillHit = true; }
		if (Time)
		{
			*Time = (-Speed + FMath::Sqrt(Speed*Speed + 2 * Gravity * Target)) / Gravity;
		}
		return 1.0f; // Same direction as Gravity
	}
	else
	{
		auto Delta = Speed*Speed + 2 * Gravity * Target;
		if (Delta < 0)
		{
			if (bWillHit) { *bWillHit = false; }
			if (Time)
			{
				*Time = InfFloat;
			}
		}
		else
		{
			if (bWillHit) { *bWillHit = true; }
			if (Time)
			{
				auto SqrtDelta = FMath::Sqrt(Delta);
				if (-SqrtDelta + Speed >= 0)
				{
					*Time = (-SqrtDelta + Speed) / Gravity;
				}
				else
				{
					*Time = (SqrtDelta + Speed) / Gravity;
				}
			}
		}
		return -1.0f;
	}

}

FVector CalculateProjectileDirectionHelper(FVector Target, FVector Origin, FVector Gravity, float Speed, bool * bWillHit, float * Time)
{
	// TODO: Maybe changing zero related comparisons to an epsilon
	if (Speed < 0)
	{
		Speed *= -1.0f;
	}

	auto RelativeTarget = Target - Origin;
	if (RelativeTarget.IsNearlyZero())
	{
		// Special case: same location
		if (bWillHit) { *bWillHit = true; }
		if (Time) { *Time = 0.0f; }
		return FVector::ForwardVector;
	}
	else if (FMath::IsNearlyZero(Speed))
	{
		// Special case: zero speed
		if (bWillHit) { *bWillHit = false; }
		if (Time) { *Time = InfFloat; }
		return FVector::ForwardVector;
	}
	else if (Gravity.IsNearlyZero())
	{
		// Special case: no gravity
		FVector Direction; float Length;
		RelativeTarget.ToDirectionAndLength(Direction, Length);
		if (bWillHit) { *bWillHit = true; }
		if (Time)
		{
			*Time = Length / Speed;
		}
		return Direction;
	}

	FVector GravityAxis;
	float GravitySize;
	Gravity.ToDirectionAndLength(GravityAxis, GravitySize);
	auto FrontAxis = FVector::CrossProduct(Gravity, FVector::CrossProduct(RelativeTarget, Gravity));

	bool bIsDifferentDirectionFromGravity = FrontAxis.Normalize();
	if (!bIsDifferentDirectionFromGravity)
	{
		// Special case: X and gravity facing the same direction
		auto DirectionFactor = CalculateProjectileDirectionOneDimensionHelper(
			FVector::DotProduct(GravityAxis, RelativeTarget),
			GravitySize,
			Speed,
			bWillHit,
			Time
		);
		return DirectionFactor * GravityAxis;
	}

	auto Distance_X = FVector::DotProduct(FrontAxis, RelativeTarget);
	auto Distance_Y = FVector::DotProduct(GravityAxis, RelativeTarget);

	auto Distance_X_2 = Distance_X * Distance_X;
	auto Distance = RelativeTarget.Size();
	auto GravitySize_2 = GravitySize * GravitySize;
	auto Speed_2 = Speed * Speed;
	auto Speed_4 = Speed_2 * Speed_2;

	auto DeltaA = -Distance_X_2 * GravitySize_2
		+ 2 * Distance_Y * GravitySize * Speed_2
		+ Speed_4;

	// To make sure archers can fire to a close position even if cannot hit;
	auto AlteredSqrtDeltaA = DeltaA >= 0 ? FMath::Sqrt(DeltaA) : 0;
	auto DeltaB = Distance_Y * GravitySize + Speed_2 + AlteredSqrtDeltaA;

	constexpr auto OneOverSqrtTwo = 0.707107f;
	auto X = (DeltaB > 0) ? (FMath::Sqrt(DeltaB) * Distance_X / Distance * OneOverSqrtTwo) : 0;
	auto Y = (DeltaB > 0) ? (Distance_Y * X / Distance_X - 0.5f * Distance_X * GravitySize / X) : -Speed;

	if (DeltaB < 0 || DeltaA < 0)
	{
		if (bWillHit) { *bWillHit = false; }
		if (Time) { *Time = X > 0 ? Distance_X / X : InfFloat; }
	}
	else
	{
		if (bWillHit) { *bWillHit = true; }
		if (Time) { *Time = Distance_X / X; }
	}

	auto result = GravityAxis * Y + FrontAxis * X;

	return result;
}

FVector UHelperLibrary::CalculateProjectileDirection(FVector Target, FVector Origin, FVector Gravity, float Speed, bool & bWillHit, float & Time)
{
	return CalculateProjectileDirectionHelper(Target, Origin, Gravity, Speed, &bWillHit, &Time);
}

FVector UHelperLibrary::CalculateProjectileDirectionForMovingTarget(FVector Target, FVector TargetVelocity, FVector Origin, FVector Gravity, float Speed, bool & bWillHit, float & Time, int Iterations, float EpsilonTime)
{
	if (TargetVelocity.IsNearlyZero())
	{
		return CalculateProjectileDirectionHelper(Target, Origin, Gravity, Speed, &bWillHit, &Time);
	}
	if (Iterations <= 0)
	{
		UE_LOG(LnHLog, Warning, TEXT("Recursion Limit should be positve!"));
		Iterations = 10;
	}

	// TODO: enhance convergence

	FVector PrevDirection;
	float PrevTime = 0.0f;
	float NewTime = 0.0f;
	bool bHit = false;
	constexpr float Alpha = 0.8f;

	for (auto i = 0; i < Iterations; i++)
	{
		PrevTime = FMath::Lerp(PrevTime, NewTime, Alpha);
		PrevDirection = CalculateProjectileDirectionHelper(Target + TargetVelocity * PrevTime, Origin, Gravity, Speed, &bHit, &NewTime);
		if (NewTime == InfFloat)
		{
			bHit = false;
			break;
		}
		if (FMath::IsNearlyEqual(NewTime, PrevTime, EpsilonTime))
		{
			break;
		}
	}

	Time = NewTime;
	bWillHit = bHit;
	return PrevDirection;
}
