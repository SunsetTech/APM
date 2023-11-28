#include "Spring.cl.h"
#include "Math.cl.h"

Spring_NodeState Spring_NextState( //make not a ring
	const Spring_SpringParameters* SpringParameters, //0
	const Spring_NodeParameters* NodeParameters, //1
	const Spring_NodeState* Spacetime, //2
	const unsigned int* SpacetimeBounds, //3
	unsigned int MassID,
	unsigned int Timestep, //4
	float TimeDelta //5
) {
	unsigned int SpatialBounds = SpacetimeBounds[1];
	int Cursor[2] = {Timestep-1, MassID};
	Spring_NodeState CurrentNode = Spacetime[MapIndex(2, Cursor, SpacetimeBounds)];
	Cursor[1] -= 1;
	Spring_NodeState LeftNode = Spacetime[MapIndex(2, Cursor, SpacetimeBounds)];
	Cursor[1] += 2;
	Spring_NodeState RightNode = Spacetime[MapIndex(2, Cursor, SpacetimeBounds)];
	Cursor[1] -= 1;
	
	Spring_NodeParameters CurrentNodeParameters = NodeParameters[MassID];
	Spring_SpringParameters LeftSpringParameters = SpringParameters[MassID];
	Spring_SpringParameters RightSpringParameters = SpringParameters[Wrap(MassID+1, SpatialBounds)];
	
	float DistanceToLeft = CurrentNode.Position - LeftNode.Position;
	float LeftSpringDisplacement = LeftSpringParameters.RestLength - fabs(DistanceToLeft);
	float DistanceToRight = RightNode.Position - CurrentNode.Position;
	float RightSpringDisplacement = RightSpringParameters.RestLength - fabs(DistanceToRight);
	
	if (!CurrentNodeParameters.Fixed) {
		float CurrentPosition = CurrentNode.Position;
		
		Cursor[0] += 1;
		
		float LeftAcceleration = LeftSpringDisplacement * LeftSpringParameters.Stiffness / CurrentNodeParameters.Mass;
		float RightAcceleration = -RightSpringDisplacement * RightSpringParameters.Stiffness / CurrentNodeParameters.Mass;
		float TotalAcceleration = (LeftAcceleration + RightAcceleration);
		float NewVelocity = (CurrentNodeParameters.Damping * CurrentNode.Velocity) + TotalAcceleration;
		float NewPosition = CurrentPosition + NewVelocity * TimeDelta;
		
		return (Spring_NodeState){NewPosition, NewVelocity};
	} else {
		float CurrentPosition = Spacetime[MapIndex(2, Cursor, SpacetimeBounds)].Position;
		return (Spring_NodeState){CurrentPosition, 0.0f};
	}
}
