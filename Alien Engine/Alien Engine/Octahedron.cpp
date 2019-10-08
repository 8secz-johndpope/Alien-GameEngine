#include "Octahedron.h"
#include "Primitive.h"
#include "Shapes.h"

Octahedron::Octahedron() : Primitive()
{
	shape = par_shapes_create_octahedron();
	SetPosition(0, 0, 0);
	MemCpy();
	InitBuffers();
}

Octahedron::Octahedron(const float& x, const float& y, const float& z) : Primitive()
{
	shape = par_shapes_create_octahedron();
	SetPosition(x, y, z);
	MemCpy();
	InitBuffers();
}

Octahedron::~Octahedron()
{
}

