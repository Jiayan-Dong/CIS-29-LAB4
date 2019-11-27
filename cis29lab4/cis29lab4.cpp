/*
Cis29
Lab3 - Graphing
Name: Jiayan Dong
Last Modified: 11/21/2019
Description:
PageRank is a numeric value that represents how important a page is on the web.
Google asserts that when one page links to another page - it "votes" for the other page.
The more votes for a page, the more important the page.  Also, the importance of the page casting
the vote determines how important the vote itself is.
Each page is represented by a Vertex in a graph. The web-page data (PageRank.html) in this
assignment is represented by an html document.
Algorithm: PR(A) = (1-d) + d(PR(t1)/C(t1) + ... + PR(tn)/C(tn))
Purpose: 
Use STL Algorithms, Containers and advanced C++ features
Data Files: PageRank.html
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <regex>
#include <memory>
#include <algorithm>
#include <numeric> 
#include <tuple>

using namespace std;

//Node Class to store html data
class Node
{
private:
	string name;	//name of the data
	string data;	//data
	vector<shared_ptr<Node>> subNodes;	//pointers point to subnodes
public:
	//Default constuctor that initalize the data
	Node()
	{
		name = "";
		data = "";
	}

	//overloaded constuctor that initalize the data
	Node(string n, string d)
	{
		name = n;
		data = d;
	}

	//Node(Node&& n)
	//{
	//	name = move(n.name);
	//	data = move(n.data);
	//	subNodes = move(n.subNodes);
	//	n.name.clear();
	//	n.name.shrink_to_fit();
	//	n.data.clear();
	//	n.data.shrink_to_fit();
	//	n.subNodes.clear();
	//	n.subNodes.shrink_to_fit();
	//}

	//Setter
	void setName(string n)
	{
		name = n;
	}

	void setData(string d)
	{
		data = d;
	}

	//Getter
	const string getData()
	{
		return data;
	}

	const string getName()
	{
		return name;
	}

	//Push a node as the target node's subnode
	void pushSubnode(string n, string d)
	{
		shared_ptr<Node> uPtr(new Node(n, d));
		subNodes.push_back(uPtr);
	}

	//Get vector of pointers that point to subnode
	vector<shared_ptr<Node>> getSubNodes()
	{
		return subNodes;
	}
};

//Generic Class HTMLProcessor to using regular expressions and use it to parse the PageRank.html file.
class HTMLProcessor
{
private:
	string infilename;	//xml filename
	vector<Node> HTMLData;	//vector contain xmlData node
	regex titlePattern;	//regex leaf pattern
	regex linkPattern;	//regex leaf pattern
	regex divBeginPattern;	//regex div begin pattern
	regex divEndPattern;	//regex div end pattern
public:
	//Overload constuctor that initalize the data with xml filename
	HTMLProcessor(string i)
	{
		infilename = i;
		titlePattern.assign(R"(<title>(.*)</title>)");
		linkPattern.assign(R"(<li><a href=\"(.*)\">(.*)</a></li>)");
		divBeginPattern.assign(R"(<div (.*)=\"(.*)\">)");
		divEndPattern.assign(R"(</div>)");
	}
	//Generic Public process function to process all xml noded
	void process()
	{
		string line;
		smatch match;
		ifstream infile;
		infile.open(infilename);
		if (!infile)
		{
			cout << "Error happened to open the input file!" << endl;
			system("pause");
			exit(EXIT_FAILURE);
		}
		while (!infile.eof() && infile.good())
		{
			getline(infile, line);
			if (regex_search(line, match, titlePattern))
			{
				Node data("title", match[1]);
				getline(infile, line);
				if (regex_search(line, match, divBeginPattern))
					data.pushSubnode(match[1], match[2]);
				do
				{
					getline(infile, line);
					if (regex_search(line, match, linkPattern))
						data.getSubNodes()[0]->pushSubnode(match[2], match[1]);
				} while (!regex_search(line, match, divEndPattern));
				HTMLData.push_back(move(data));
			}
		}
		infile.close();
	}

	//Getters
	const string getInfilename()
	{
		return infilename;
	}

	const vector<Node> getHTMLData()
	{
		return HTMLData;
	}
};

//AdjacencyMatrix class to store the adjacency matrix of the graph
class AdjacencyMatrix
{
private:
	vector<string> names;		// Names of Vertices 
	vector<vector<int>> matrix;	// Adjacency Matrix
public:
	// Getters
	const int getSize()
	{
		return names.size();
	}

	const vector<string> getNames()
	{
		return names;
	}

	const vector<vector<int>> getMatrix()
	{
		return matrix;
	}

	// Creating Adjacency Matrix from the htmlData
	void insertAll(const vector<Node>& htmlData)
	{
		for_each(htmlData.begin(), htmlData.end(), [&](Node i) {names.push_back(i.getData()); });
		matrix.resize(names.size());
		for_each(matrix.begin(), matrix.end(), [&](auto& i) {i.resize(names.size(), 0); });
		int row = 0;
		for_each(htmlData.begin(), htmlData.end(), [&](Node i) {
			vector<shared_ptr<Node>> adj = i.getSubNodes()[0]->getSubNodes();
			for_each(adj.begin(), adj.end(), [&](auto i) {
				vector<string>::iterator ite = find_if(names.begin(), names.end(), [=](auto j) {
					transform(j.begin(), j.end(), j.begin(), toupper);
					string n = i->getName();
					transform(n.begin(), n.end(), n.begin(), toupper);
					return j == n; });
				if ( ite != names.end())
				{
					int col = ite - names.begin();
					matrix[row][col] = 1;					
				}
			});
			row++;
		});
	}
};

// PageRanker class to calc the PageRank of each pages
class PageRanker
{
private:
	vector<tuple<string, double, int>> pageRanks;	// pageRanks vector, each pageRank contains name(string), pageRank(double), outbound(int)
	vector<vector<int>> matrix;						//  Adjacency Matrix
	double tolerance;			// tolerance(maximum error) 
	double dumpFactor;			// dumpFactor(the rate of person lose interest)
	// _calc Iteration 1 time, and return the sum of errors
	double _calc()	 
	{
		vector<double> diffs;
		int col = 0;
		for_each(pageRanks.begin(), pageRanks.end(), [&](tuple<string, double, int>& t) {
			double pre = get<1>(t);
			int row = 0;
			get<1>(t) = accumulate(pageRanks.begin(), pageRanks.end(), 1- dumpFactor, [&](double sum, tuple<string, double, int> tu) {
				if(matrix[row++][col])
					return sum + dumpFactor * get<1>(tu) / static_cast<double>(get<2>(tu));
				return sum;
				});
			col++;
			diffs.push_back(abs(pre - get<1>(t)));
			});
		return accumulate(diffs.begin(), diffs.end(), 0.0);
	}
public:
	// Overload constuctor that initalize the data
	PageRanker(const vector<string>& names, const vector<vector<int>>& m)
	{
		int i = 0;
		matrix = m;
		for_each(names.begin(), names.end(), [&](string n) {pageRanks.push_back(tuple<string, double, int>(n, 1.0, accumulate(matrix[i].begin(), matrix[i].end(), 0))); i++; });
		tolerance = 0.0001;
		dumpFactor = 0.85;
	}
	// Setter
	void setTolerance(double t)
	{
		tolerance = t;
	}

	void setDumpFactor(double d)
	{
		dumpFactor = d;
	}
	// Getter
	const int getSize()
	{
		return pageRanks.size();
	}

	const double getTolerance()
	{
		return tolerance;
	}

	const double getDumpFactor()
	{
		return dumpFactor;
	}

	vector<tuple<string, double, int>> getPageRanks()
	{
		return pageRanks;
	}

	// calcRanks function calculate the pageRanks until the sum of errors is equal or smaller than tolerance
	void calcRanks()
	{
		while (tolerance < _calc());
	}
};

// PageRankOutput class to output the PageRank result to the screen or file
class PageRankOutput
{
private:
	string outFilename;
public:
	// Screen output
	void outputOnScreen(const vector<tuple<string, double, int>> &tuples)
	{
		for (tuple<string, double, int> t : tuples)
			cout << setw(20) << setiosflags(ios::left) << get<0>(t) << "PageRank: " << setprecision(3) << setiosflags(ios::fixed)<< get<1>(t) << endl;
	}
	// File output
	void outputOnFile(const vector<tuple<string, double, int>>& tuples, string o)
	{
		outFilename = o;
		ofstream outfile;
		outfile.open(outFilename);
		for (tuple<string, double, int> t : tuples)
			outfile << setw(20) << setiosflags(ios::left) << get<0>(t) << "PageRank: " << setprecision(3) << setiosflags(ios::fixed) << get<1>(t) << endl;
		outfile.close();
	}
};

int main()
{
	string htmlFilename;
	
	cout << "Welcome to PageRank Program" << endl;
	cout << "Please enter the PageRank.html to calculate(with filename extension): ";
	getline(cin, htmlFilename);
	HTMLProcessor htmlProcessor(htmlFilename);	// HTMLProcessor initialize with htmlFilename
	htmlProcessor.process();		// processing htmldata
	AdjacencyMatrix matrix;			// Adjacency Matrix of graph of pages
	matrix.insertAll(htmlProcessor.getHTMLData());		// using htmldata create Adjacency Matrix
	PageRanker pageRanker(matrix.getNames(), matrix.getMatrix());	// PageRanker initialize with names of vetices in graph and Adjacency Matrix
	pageRanker.calcRanks();		// calculate the pageRanks
	PageRankOutput output;
	output.outputOnScreen(pageRanker.getPageRanks());	// output the pageranks on screen
	system("pause");
	return 0;
}