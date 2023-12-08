#include "Spring.cl.h"
#include "Math.cl.h"

Spring_NodeState Spring_NextState( //make not a ring
	const Spring_SpringParameters* SpringParameters, //0
	const Spring_NodeParameters* NodeParameters, //1
	const Spring_NodeState* Spacetime, //2
	const unsigned int* SpacetimeBounds, //3
	unsigned int MassID,
	unsigned int Timestep, //4
	Spring_PrecisionType TimeDelta //5
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
	
	if (!CurrentNodeParameters.Fixed) {
		Spring_SpringParameters LeftSpringParameters = SpringParameters[MassID];
		Spring_SpringParameters RightSpringParameters = SpringParameters[Wrap(MassID+1, SpatialBounds)];
		
		Spring_PrecisionType DistanceToLeft = CurrentNode.Position - LeftNode.Position;
		Spring_PrecisionType LeftSpringDisplacement = LeftSpringParameters.RestLength - fabs(DistanceToLeft);
		Spring_PrecisionType DistanceToRight = RightNode.Position - CurrentNode.Position;
		Spring_PrecisionType RightSpringDisplacement = RightSpringParameters.RestLength - fabs(DistanceToRight);
		Spring_PrecisionType CurrentPosition = CurrentNode.Position;
		
		Cursor[0] += 1;
		
		Spring_PrecisionType LeftAcceleration = LeftSpringDisplacement * LeftSpringParameters.Stiffness / CurrentNodeParameters.Mass;
		Spring_PrecisionType RightAcceleration = -RightSpringDisplacement * RightSpringParameters.Stiffness / CurrentNodeParameters.Mass;
		Spring_PrecisionType TotalAcceleration = (LeftAcceleration + RightAcceleration);
		Spring_PrecisionType NewVelocity = (CurrentNodeParameters.Damping * CurrentNode.Velocity) + TotalAcceleration;
		Spring_PrecisionType NewPosition = CurrentPosition + NewVelocity * TimeDelta;
		return (Spring_NodeState){NewPosition, NewVelocity};
	} else {
		Spring_PrecisionType CurrentPosition = CurrentNode.Position;
		return (Spring_NodeState){CurrentPosition, 0.0f};
	}
}
