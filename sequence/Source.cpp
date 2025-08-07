#include "Sequence.h"
#include <vector>

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

using namespace seq;
int main() {

	//Sequence<Rect2> mmmm;
	//mmmm.resize(0);//should work
	//mmmm.resize(1);//should fail


	//Sequence<Rect3> mmmm;
	//mmmm.resize(mmmm.capacity() + 1);//should fail, not movable


	//Sequence<Rectangle> mmmm2 = std::move(mmmm);
	//std::vector<Rect2> meme;
	//Sequence<Rect2> mmmm;
	//std::vector<Rectangle> meme = meme;;


	return 0;

}