#include <iostream>
#include <fstream>
#include <queue>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <locale.h>


#include <filesystem>
namespace fs = std::filesystem;


#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;


#include "KdTree.h"


//////////////////////////////////////////////////////////////////////////
// static parameters
static constexpr int kDim = 3;
static constexpr float kUnitsDistance = 8.0f;


//////////////////////////////////////////////////////////////////////////
// struct
struct Data
{
	float m_pos[kDim];
	int m_index;
	//bool m_isSpawner;
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// functions to read XYZ file
std::istream& operator>>(std::istream& is, Data& p)
{
	static int index = 0;
	double x, y, z;
	is >> x >> y >> z;
	p.m_pos[0] = static_cast<float>(x);
	p.m_pos[1] = static_cast<float>(y);
	p.m_pos[2] = static_cast<float>(z);
	p.m_index = index;
	++index;
	return is;
}

std::ostream& operator<<(std::ostream& os, const Data& p) {
	return os << p.m_pos[0] << ' ' << p.m_pos[1] << ' ' << p.m_pos[2];
}

struct custom_classification : std::ctype<char> {
	custom_classification() : ctype(make_table()) { }
private:
	static mask* make_table() {
		const mask* classic = classic_table();
		static std::vector<mask> v(classic, classic + table_size);
		v[','] |= space;
		return &v[0];
	}
};

std::vector<Data> read_points(std::istream& is) {
	auto old_locale = is.imbue(std::locale(is.getloc(), new custom_classification));
	is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	auto points = std::vector(std::istream_iterator<Data>(is), {});
	is.imbue(old_locale);
	return points;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Fill KdTree and Find the node
void FillTreeAndFindNode(std::vector<int>& _output, const Data& _dataToFind, std::vector<Data>& _nodes)
{
	std::cout << "CREATING AND POPULATING THE K-D TREE WITH " << _nodes.size() << " POINTS..." << std::endl;
	auto t1 = high_resolution_clock::now();

	KdTree<Data, float, kDim> tree;

	size_t count = _nodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		float* pos = (float*)malloc(kDim * sizeof(float));
		memcpy(pos, _nodes[i].m_pos, kDim * sizeof(float));
		tree.Insert(pos, &_nodes[i]);
		free(pos);
	}

	auto t2 = high_resolution_clock::now();
	duration<double, std::milli> ms_double = t2 - t1;

	std::cout << "CREATE AND POPULATE THE K-D TREE DONE IN " << ms_double.count() << "ms" << std::endl << std::endl;

	std::cout << "SEARCHING ALL THE RESULT AROUND (" << _dataToFind.m_pos[0] << ", " << _dataToFind.m_pos[1] << ", " << _dataToFind.m_pos[2] << ") FOR " << kUnitsDistance << " UNIT DISTANCE AND GET THE RESULT..." << std::endl;
	t1 = high_resolution_clock::now();
	KdTree<Data, float, kDim>::Result* results = tree.Find(_dataToFind.m_pos, kUnitsDistance);

	while (!tree.ResultsEnd(results))
	{
		Data* next = tree.GetResult(results);
		if (next != nullptr)
		{
			_output.push_back(next->m_index);
		}

		tree.NextResult(results);
	}

	tree.DestroyResults(results);

	t2 = high_resolution_clock::now();
	ms_double = t2 - t1;
	std::cout << "SEARCH DONE IN " << ms_double.count() << "ms" << std::endl << std::endl;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// main
int main()
{
	std::cout << "START" << std::endl;
	std::cout << "OPEN FILE" << std::endl;
	std::ifstream myfile(fs::current_path().string() + "\\swissALTI3D_rickenbach.txt");

	if (myfile)
	{
		std::cout << "READING FILE..." << std::endl;
		std::vector<Data> points = read_points(myfile);
		std::cout << "READ DONE" << std::endl;

		// Pick one value randomly
		int min = 0;
		int max = points.size() - 1;
		int index = min + (rand() % static_cast<int>(max - min + 1));

		Data pointToFind = points[index];

		myfile.close();
		std::cout << "CLOSE FILE" << std::endl << std::endl;

		std::vector<int> results;

		FillTreeAndFindNode(results, pointToFind, points);

		std::cout << "RESULTING COUNT: " << results.size() << std::endl;
		std::cout << "RESULTING LIST: " << std::endl;
		for (size_t i = 0; i < results.size(); ++i)
		{
			std::cout << results[i] << std::endl;
		}

	}
	else
	{
		std::cerr << "ERROR OPENING FILE!" << std::endl; // Report error

		size_t errmsglen = 1024;
		char errmsg[1024];

		std::cerr << "ERROR CODE: " << strerror_s(errmsg, errmsglen, errno) << std::endl; // Get some info as to why
	}

	std::cout << "END" << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
