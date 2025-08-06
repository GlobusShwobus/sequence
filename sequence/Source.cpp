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
	Rect2() = default;
	~Rect2() = delete;
};

using namespace seq;
int main() {

	Sequence<Rectangle> mmmm;
	//Sequence<Rectangle> mmmm2 = std::move(mmmm);
	//std::vector<Rect2> meme;
	//Sequence<Rect2> mmmm;
	//std::vector<Rectangle> meme = meme;;


	return 0;

}