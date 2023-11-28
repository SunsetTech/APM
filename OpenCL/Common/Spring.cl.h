#pragma once

typedef struct {
	float RestLength;
	float Stiffness;
} Spring_SpringParameters;

typedef struct {
	float Mass, Damping;
	bool Fixed;
} Spring_NodeParameters;

typedef struct {
	float Position;
	float Velocity;
} Spring_NodeState;

Spring_NodeState Spring_NextState(
	const Spring_SpringParameters* SpringParameters, //0
	const Spring_NodeParameters* NodeParameters, 
	const Spring_NodeState* Spacetime,
	const unsigned int* SpacetimeBounds,
	unsigned int MassID,
	unsigned int Timestep, //4
	float TimeDelta //5
);
