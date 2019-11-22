/*
Cis29
Lab3 - Graphing
Name: Jiayan Dong
Last Modified: 11/21/2019
Description:

Purpose: 

Data Files: PageRank.html
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <memory>
#include <algorithm>
#include <functional>
#include <numeric> 

using namespace std;

//Node to store html data
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
	string getData()
	{
		return data;
	}

	string getName()
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
				HTMLData.push_back(data);
			}
		}
		infile.close();
	}

	vector<Node> getHTMLData()
	{
		return HTMLData;
	}
};

class AdjacencyMatrix
{
private:
	vector<string> names;
	vector<vector<int>> matrix;
public:
	int getSize()
	{
		return names.size();
	}

	vector<string> getNames()
	{
		return names;
	}

	vector<vector<int>> getMatrix()
	{
		return matrix;
	}

	void insertAll(vector<Node> htmlData)
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

class PageRanker
{
private:
	vector<pair<string, double>> pageRanks;
	double tolerance;
	double pageRank(vector<pair<shared_ptr<double>, int>> r_ob)
	{
		return accumulate(r_ob.begin(), r_ob.end(), 0.0, [&](double sum, pair<shared_ptr<double>, int> page) {return sum + *(page.first) / static_cast<double>(page.second); });
	}
	double _calc(vector<vector<pair<shared_ptr<double>, int>>>& r_ob)
	{
		double diff = pageRanks.size();
		for (int i = 0; i < pageRanks.size(); i++)
		{
			double newRank = pageRank(r_ob[i]);
			double newDiff = abs(pageRanks[i].second - pageRank(r_ob[i]));
			if (abs(newDiff) < diff)
				diff = abs(newDiff);
			pageRanks[i].second = newRank;	 
		}
		return diff;
	}
public:
	PageRanker(vector<string> names)
	{
		for_each(names.begin(), names.end(), [&](string n) {pageRanks.push_back(pair<string, double>(n, 1)); });
		tolerance = 0.001;
	}
	
	int getSize()
	{
		return pageRanks.size();
	}

	vector<pair<string, double>> getPageRanks()
	{
		return pageRanks;
	}

	void calcRanks(vector<string> names, vector<vector<int>> matrix)
	{
		vector<vector<pair<shared_ptr<double>, int>>> ranks_outbounds(names.size());
		vector<int> outbounds;
		for_each(matrix.begin(), matrix.end(), [&](auto v) {
			outbounds.push_back(accumulate(v.begin(), v.end(), 0));
			});
		for (int col = 0; col < matrix.size(); col++)
		{
			for (int row = 0; row < matrix.size(); row++)
			{
				if (matrix[row][col])
					ranks_outbounds[col].push_back(pair<shared_ptr<double>, int>(make_shared<double>(pageRanks[row].second), outbounds[row]));
			}
		}
		while (tolerance < _calc(ranks_outbounds));
	}
};

class PageRankOutput
{
private:
	string outfilename;
public:
	void outputOnScreen(vector<pair<string, double>> prs)
	{
		for (pair<string, double> p : prs)
			cout << setw(20) << setiosflags(ios::left) << p.first << "PageRank: " << setprecision(3) << setiosflags(ios::fixed)<< p.second << endl;
	}
};

int main()
{
	HTMLProcessor htmlProcessor("PageRank.html");
	htmlProcessor.process();
	AdjacencyMatrix matrix;
	matrix.insertAll(htmlProcessor.getHTMLData());
	PageRanker pageRanker(matrix.getNames());
	pageRanker.calcRanks(matrix.getNames(), matrix.getMatrix());
	PageRankOutput output;
	output.outputOnScreen(pageRanker.getPageRanks());
	system("pause");
	return 0;
}