#include "Sequence.h"
#include <vector>
class triangle {
public:
	triangle() = default;
	triangle(int ass) :x(ass) {}
	int x = 0;
};
class Rectangle {
public:
	Rectangle() = default;
	Rectangle(const Rectangle& c) = delete;
	Rectangle& operator=(const Rectangle&) = delete;
};
class Rect2 {
public:
	Rect2() = delete;
	~Rect2() = default;
};
class Rect3 {
public:
	Rect3() = default;
	Rect3(Rect3&&)noexcept = delete;
	Rect3& operator=(Rect3&&)noexcept = delete;
	~Rect3() = default;
};
#include <iostream>
using namespace seq;
int main() {

	//Sequence<Rect2> mmmm;
	//mmmm.resize(0);//should work
	//mmmm.resize(1);//should fail


	//Sequence<Rect3> mmmm;
	//mmmm.resize(mmmm.capacity() + 1);//should fail, not movable

	Sequence<triangle> wow;
	Sequence<triangle> wow2 = wow;
	wow.reserve(200);
	wow.resize(100);

	for (int i = 0; i < wow.size(); i++) {
		wow[i] = {i};
	}
	wow.shrinkToFit();
	auto it = wow.begin() + 50;
	std::cout << it->x << "\n";

	//Sequence<Rectangle> mmmm2 = std::move(mmmm);
	//std::vector<Rect2> meme;
	//Sequence<Rect2> mmmm;
	//std::vector<Rectangle> meme = meme;;


	return 0;

}