#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

void SkipSpace(std::istream &input) {
	while(std::isspace(input.peek())) {
		input.get();
	}
}

Node LoadArray(istream& input) {
	Array result;

	if(input.get() != '[') {
		throw ParsingError("Array should start with [");
	}

	SkipSpace(input);

	if(input.peek() == ']') {
		input.get();
		return Node(result);
	}

	while(true) {
		SkipSpace(input);
		result.push_back(LoadNode(input));
		SkipSpace(input);

		char c;
		c = input.get();
		
		if(c == ']') {
			break;
		}
		
		if(c != ',') {
			throw ParsingError("Array should have , or ]");
		}
	}

	return Node(std::move(result));
}

std::string ParseString(std::istream &input) {
	if(input.get() != '"') {
		throw ParsingError("String should start with \"");
	}

	std::string str;
	char c;
	bool escape = false;

	while(input.get(c)) {
		if(!escape && c == '"') {
			break;
		}
	
		switch(escape) {
			case true:
				switch(c) {
					case 'n':
						str += '\n';
						break;
					case 'r':
						str += '\r';
						break;
					case '"':
						str += '\"';
						break;
					case 't':
						str += '\t';
						break;
					case '\\':
						str += '\\';
						break;
				}
		
				escape = false;
				break;
			case false:
				switch(c) {
					case '\\':
						escape = true;
						break;
					default:
						str += c;
				}
		}
	}

	if(c != '"') {
		throw ParsingError("String should end with \"");
	}

	return str;
}

Node LoadString(istream& input) {
	return Node(ParseString(input));
}

Node LoadDict(istream& input) {
	Dict result;

	if(input.get() != '{') {
		throw ParsingError("Dictionary should start with {");
	}

	SkipSpace(input);

	if(input.peek() == '}') {
		input.get();
		return Node(result);
	}

	while(true) {
		SkipSpace(input);
		std::string key = ParseString(input);
		SkipSpace(input);

		if (input.get() != ':') {
			throw ParsingError("Expected ':' after dict key");
		}

		SkipSpace(input);
		Node value = LoadNode(input);
		result.emplace(std::move(key), std::move(value));
		SkipSpace(input);

		char c = input.get();

		if(c == '}') {
			break;
		}

		if(c != ',') {
			throw ParsingError("Dictionary should have , or }");
		}
	}

	return Node(std::move(result));
}

Node LoadBool(std::istream &input) {
	std::string str;
	bool result = false;

	while(std::isalpha(input.peek())) {
		str += input.get();
	}

	if(str == "true") {
		result = true;
	} else if(str == "false") {
		result = false;
	} else {
		throw ParsingError("Unknown token: " + str);
	}

	return Node(std::move(result));
}

Node LoadNull(std::istream &input) {
	std::string str;

	while(std::isalpha(input.peek())) {
		str += input.get();
	}

	if(str != "null") {
		throw ParsingError("Unknown token: " + str);
	}

	return Node(nullptr);
}

Node LoadNumber(std::istream &input) {
	using namespace std::literals;
	std::string str;

	auto read_char = [&str, &input] () {
		str += static_cast<char> (input.get());

		if (!input) {
			throw ParsingError("Failed to read number from stream"s);
		}
	};

	auto read_digits = [&input, read_char] () {
		if (!std::isdigit(input.peek())) {
			throw ParsingError("A digit is expected"s);
		}

		while (std::isdigit(input.peek())) {
			read_char();
		}
	};

	if(input.peek() == '-') {
		read_char();
	}

	if (input.peek() == '0') {
		read_char();
	} else {
		read_digits();
	}

	bool is_int = true;
	if(input.peek() == '.') {
		read_char();
		read_digits();
		is_int = false;
	}

	if (int ch = input.peek(); ch == 'e' || ch == 'E') {
		read_char();
		if (ch = input.peek(); ch == '+' || ch == '-') {
			 read_char();
		}
		read_digits();
		is_int = false;
	}

	try {
		if (is_int) {
			try {
				return Node(std::stoi(str));
			} catch (...) {}
		}

		return Node(std::stod(str));
	} catch (...) {
		throw ParsingError("Failed to convert "s + str + " to number"s);
	}
}

Node LoadNode(istream& input) {
	SkipSpace(input);
	char c;
	input >> c;

	switch(c) {
		case '[':
			input.putback(c);
			return LoadArray(input);
		case '{':
			input.putback(c);
			return LoadDict(input);
		case '"':
			input.putback(c);
			return LoadString(input);
		case 'n':
			input.putback(c);
			return LoadNull(input);
		case 't':
			input.putback(c);
			return LoadBool(input);
		case 'f':
			input.putback(c);
			return LoadBool(input);
		default:
			input.putback(c);
			return LoadNumber(input);
	}
}

}  // namespace

Node::Node() : value_{} {}
Node::Node(Array v) : value_(std::move(v)) {}
Node::Node(Dict v) : value_(std::move(v)) {}
Node::Node(int v) : value_(std::move(v)) {}
Node::Node(double v) : value_(std::move(v)) {}
Node::Node(bool v) : value_(std::move(v)) {}
Node::Node(std::string v) : value_(std::move(v)) {}
Node::Node(const char* v) : value_(std::string(v)) {}
Node::Node([[maybe_unused]]std::nullptr_t v) : value_(nullptr) {}

bool Node::IsInt() const {
	return std::holds_alternative<int> (value_);
}

bool Node::IsDouble() const {
	return std::holds_alternative<double> (value_) || std::holds_alternative<int> (value_);
}

bool Node::IsPureDouble() const {
	return std::holds_alternative<double> (value_);
}

bool Node::IsBool() const {
	return std::holds_alternative<bool> (value_);
}

bool Node::IsString() const {
	return std::holds_alternative<std::string> (value_);
}

bool Node::IsNull() const {
	return std::holds_alternative<std::nullptr_t> (value_);
}

bool Node::IsArray() const {
	return std::holds_alternative<Array> (value_);
}

bool Node::IsMap() const {
	return std::holds_alternative<Dict> (value_);
}

int Node::AsInt() const {
	if(!IsInt()) {
		throw std::logic_error("Not an int");
	}

	return std::get<int> (value_);
}

bool Node::AsBool() const {
	if(!IsBool()) {
		throw std::logic_error("Not an bool");
	}

	return std::get<bool> (value_);
}

double Node::AsDouble() const {
	if(!IsDouble()) {
		throw std::logic_error("Not a double");
	}

	return (IsInt() ? std::get<int> (value_) : std::get<double> (value_));
}

const std::string& Node::AsString() const {
	if(!IsString()) {
		throw std::logic_error("Not a string");
	}

	return std::get<std::string> (value_);
}

const Array& Node::AsArray() const {
	if(!IsArray()) {
		throw std::logic_error("Not an array");
	}

	return std::get<Array> (value_);
}

const Dict& Node::AsMap() const {
	if(!IsMap()) {
		throw std::logic_error("Not a map");
	}

	return std::get<Dict> (value_);
}

bool Node::operator==(const Node &node) const {
	return value_ == node.value_;
}

bool Node::operator!=(const Node &node) const {
	return !(*this == node);
}

void Node::Print(std::ostream &out) const {
	std::visit(PrintValue(out), value_);
}

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const {
	return root_;
}

Document Load(istream& input) {
	return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& out) {
	const Node &node = doc.GetRoot();
	node.Print(out);
}

bool Document::operator==(const Document &v) const {
	return root_ == v.root_;
}

bool Document::operator!=(const Document &v) const {
	return !(*this == v);
}

}  // namespace json