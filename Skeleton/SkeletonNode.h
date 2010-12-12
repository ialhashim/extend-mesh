#ifndef SKELETONNODE_H
#define SKELETONNODE_H

class SkeletonNode
{
public:

	int index;
	float x,y,z;

	SkeletonNode()
	{
		x = y = z = 0;
	}

	SkeletonNode(float X, float Y, float Z, int newIndex)
	{
		this->x = X;
		this->y = Y;
		this->z = Z;

		this->index = newIndex;
	}

	SkeletonNode(SkeletonNode * n, int newIndex)
	{
		this->x = n->x;
		this->y = n->y;
		this->z = n->z;

		this->index = newIndex;
	}

	SkeletonNode(const SkeletonNode & from)
	{
		this->x = from.x;
		this->y = from.y;
		this->z = from.z;
		this->index = from.index;
	}

	static SkeletonNode * Midpoint(SkeletonNode * n1, SkeletonNode * n2, int index)
	{
		float x = (n1->x + n2->x) / 2;
		float y = (n1->y + n2->y) / 2;
		float z = (n1->z + n2->z) / 2;

		return new SkeletonNode(x, y, z, index);
	}

	void set(float X, float Y, float Z)
	{
		this->x = X;
		this->y = Y;
		this->z = Z;
	}

	void set(float * pos)
	{
		this->x = pos[0];
		this->y = pos[1];
		this->z = pos[2];
	}

	float distanceTo(SkeletonNode * n2)
	{
		float X = x - n2->x;
		float Y = y - n2->y;
		float Z = z - n2->z;

		return sqrt(X * X + Y * Y + Z * Z);
	}
	
	operator const Vec() const 
	{
		return Vec(x,y,z);
	}

	Vec v() const
	{
		return Vec(x,y,z);
	}
};

class Color3
{
public:
	float r, g, b;
	Color3(float R, float G, float B):r(R),g(G),b(B) {	}
	Color3(Color3& c, char channel, float step)
	{
		r = c.r;	g = c.g;	b = c.b;

		switch(channel){
			case 'r': r = step; break;
			case 'g': g = step; break;
			case 'b': b = step; break;
		}

		r = Min(r, 1.0f);
		g = Min(g, 1.0f);
		b = Min(b, 1.0f);
	}
};

#endif // SKELETONNODE_H
