#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <format>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
	using runtime_error::runtime_error;
};

class Node {
public:
	using Value = std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string>;

	Node();
	Node(Array v);
	Node(Dict v);
	Node(int v);
	Node(double v);
	Node(bool v);
	Node(std::string v);
	Node(const char* v);
	Node([[maybe_unused]]std::nullptr_t v);
	
	bool IsInt() const;
	bool IsDouble() const;
	bool IsPureDouble() const;
	bool IsBool() const;
	bool IsString() const;
	bool IsNull() const;
	bool IsArray() const;
	bool IsMap() const;
	
	int AsInt() const;
	bool AsBool() const;
	double AsDouble() const;
	const std::string& AsString() const;
	const Array& AsArray() const;
	const Dict& AsMap() const;
	
	bool operator==(const Node &node) const;
	bool operator!=(const Node &node) const;
	
	void Print(std::ostream &out) const;

	private:
	Value value_{};
};

struct PrintValue {
	PrintValue(std::ostream &output) : out(output) {}

	std::ostream &out;

	void operator() (int value) {
		out << value;
	}

	void operator() (double value) {
		out << value;
	}

	void operator() (bool value) {
		std::string result;

		switch(value) {
			case true:
				result = "true";
				break;
			case false:
				result = "false";
				break;
		}

		out << result;
	}

	void operator() (const std::string &value) {
		std::string str = "\"";

		for(char ch : value) {
			switch(ch) {
				case '\n':
					str += "\\n";
					break;
				case '\r':
					str += "\\r";
					break;
				case '\"':
					str += "\\\"";
					break;
				case '\t':
					str += "\\t";
					break;
				case '\\':
					str += "\\\\";
					break;
				default:
					str += ch;
			}
		}

		str += "\"";
		out << str;
	}

	void operator() ([[maybe_unused]]std::nullptr_t value) {
		out << "null";
	}

	void operator() (const Array &value) {
		bool is_first = true;
		out << "[";

		for(const auto &i : value) {
			out << (is_first ? "" : ", ");
			is_first = false;
			i.Print(out);
		}

		out << "]";
	}

	void operator() (const Dict &dict) {
		bool is_first = true;
		out << "{";
		for(const auto &[key, value] : dict) {
			out << (is_first ? "" : ", ") << std::format("\"{}\": ", key);
			is_first = false;
			value.Print(out);
		}
		out << "}";
	}
};

class Document {
public:
	explicit Document(Node root);

	const Node& GetRoot() const;

	bool operator==(const Document &v) const;
	bool operator!=(const Document &v) const;

private:
	Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json