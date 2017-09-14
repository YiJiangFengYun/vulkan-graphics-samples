
#include <plog/Log.h>
#include <framework/global.hpp>
#include <foundation/gemo.hpp>

int main()
{
	gfw::init(); //init framework module, and it also init foundation and graphics modules at the same time.

	fd::Bounds<glm::vec3> bounds(glm::vec3(0, 0, 0), glm::vec3(10, 10, 10));
	fd::Ray<glm::vec3> ray(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
	float result = bounds.intersectRay(ray);

	glm::vec3 min = bounds.getMin();
	glm::vec3 max = bounds.getMax();
	glm::vec3 origin = ray.getOrigin();
	glm::vec3 direction = ray.getDirection();
	LOG(plog::debug) << "Bounds: " << "min: (" << min.x << ", " << min.y << ", " << min.z << ") max: ("
		<< max.x << ", " << max.y << ", " << max.z << ")." << std::endl;
	LOG(plog::debug) << "Ray: " << "origin: (" << origin.x << ", " << origin.y << ", " << origin.z << ") dir: ("
		<< direction.x << ", " << direction.y << ", " << direction.z << ")." << std::endl;
	if (result < 0)
	{
		LOG(plog::debug) << "The bounds don't intersect with the ray." << std::endl;
	}
	else
	{
		LOG(plog::debug) << "The bounds intersect with the ray, the distance to the ray origin is: " << result << std::endl;
	}
	return 0;
}