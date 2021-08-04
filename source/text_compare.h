#pragma once

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void FileInput(const std::string& filename, std::stringstream& input) {
	std::ifstream in(filename);
	
	if (in.is_open())
	{
		for(std::string s; getline(in, s);) {
			input << s;
		}
	}
	in.close();     // закрываем файл
}

/*void FileOutput(json::Document& document, json_reader::TransportCatalogueJSON& tcj, const std::string& filename) {
	
	std::ofstream out(filename);
	
	if (out.is_open())
	{
		json_reader::Output(document, tcj, out);
	}
	out.close();     // закрываем файл
}*/


void TextCompare(const std::string& left_filename, const std::string& right_filename) {
	std::fstream left_file(left_filename);
	std::fstream right_file(right_filename);
	string left_str;
	string right_str;
	char left_ch;
	char right_ch;
	uint ch_count = 1;
	uint line_count = 1;
	int error_count = 0;
	int errors_quantity = 50;
	while(getline(left_file, left_str)) {
		getline(right_file, right_str);
		stringstream l(left_str);
		stringstream r(right_str);
		while (l.get(left_ch)) {
			r.get(right_ch);
			if (left_ch != right_ch) {
				cerr << "Error: " << left_ch << " != " << right_ch << " in line " << line_count << " char " << ch_count
				     << endl;
				++error_count;
				if (error_count > errors_quantity) {
					break;
				}
			}
			++ch_count;
		}
		if (error_count > errors_quantity) {
			break;
		}
		++line_count;
		ch_count = 1;
	}
	if(error_count == 0) {
		cerr << "Files are equal" << endl;
	}
}


void Compare(string& l, string& r) {
	//std::string common_directory = "/Users/Evgenij/Desktop/С++/Я.Практикум/Элегантность и массштабируемость/Итоговый проект/"s;

	//string one = common_directory + "ex4etalon.json";
	//string two = common_directory + "output1.json";
	TextCompare(l, r);
}


