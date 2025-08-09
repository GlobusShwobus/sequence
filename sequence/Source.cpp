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

	Sequence<triangle> wow(200, {7});
	Sequence<triangle> lols = wow;
	std::cout << wow.size() << " " << wow.capacity() << "\n";
	std::cout << lols.size() << " " << lols.capacity() << "\n";
	wow.reserve(100);
	wow.resize(50);
	wow.shrinkToFit();
	std::cout << wow.size() << " " << wow.capacity() << "\n";
	wow.erase(wow.begin(), wow.begin() + 11);
	std::cout << wow.size() << " " << wow.capacity() << "\n";
	auto it = wow.begin() + 5;
	it->x = 7;
	std::cout << wow[5].x << '\n';
	std::cout << wow.at(5).x << '\n';
	wow.push_back({77});
	int lol = 88;
	triangle fuck(9);
	wow.push_back(std::move(lol));
	wow.push_back(fuck);//ok good
	wow.clear();
	wow.shrinkToFit();
	std::cout << wow.size() << " " << wow.capacity() << "\n";
	std::cout << lols.size() << " " << lols.capacity() << "\n";

	return 0;

}