#include "json_builder.h"

using namespace std::literals;

namespace json {
	
	//  ------------------ Builder --------------------
	
	
	KeyContext Builder::Key(std::string key) {
		node_stack_.emplace_back(Node(std::move(key)));
		return *this ;
	}
	
	Builder& Builder::Value(json::Node::Value value) {
			if (std::holds_alternative<std::nullptr_t>(value)) {
				node_stack_.emplace_back(std::get<std::nullptr_t>(value));
			}
			else if (std::holds_alternative<json::Array>(value)) {
				node_stack_.emplace_back(std::get<json::Array>(std::move(value)));
			}
			else if (std::holds_alternative<json::Dict>(value)) {
				node_stack_.emplace_back(std::get<json::Dict>(std::move(value)));
			}
			else if (std::holds_alternative<bool>(value)) {
				node_stack_.emplace_back(std::get<bool>(value));
			}
			else if (std::holds_alternative<int>(value)) {
				node_stack_.emplace_back(std::get<int>(value));
			}
			else if (std::holds_alternative<double>(value)) {
				node_stack_.emplace_back(std::get<double>(value));
			}
			else if (std::holds_alternative<std::string>(value)) {
				node_stack_.emplace_back(std::get<std::string>(std::move(value)));
			}
			return *this;
	}
	
	StartDictContext Builder::StartDict() {
			node_stack_.emplace_back("{"s);
			return *this;
	}
	
	StartArrayContext Builder::StartArray() {
			node_stack_.emplace_back("["s);
			return *this;
	}
	
	Builder& Builder::EndDict() {
		Dict dict;
		int elements_to_erase_count = 0;
		for(auto it = node_stack_.rbegin(); it != node_stack_.rend(); ++it) {
			if(it->IsString() && it->AsString() == "{"s) {
				++elements_to_erase_count;
				break;
			}
			Node value = *it;
			++elements_to_erase_count;
			std::string key = next(it)->AsString();
			++elements_to_erase_count;
			dict.emplace(std::move(key), std::move(value));
			++it;
		}
		node_stack_.erase(next(node_stack_.begin(), node_stack_.size() - elements_to_erase_count), node_stack_.end());
		node_stack_.emplace_back(std::move(dict));
		return *this;
	}
	
	Builder& Builder::EndArray() {
		Array array;
		int elements_to_erase_count = 0;
		for(auto it = node_stack_.rbegin(); it != node_stack_.rend(); ++it) {
			if(it->IsString() && it->AsString() == "["s) {
				++elements_to_erase_count;
				break;
			}
			Node value = *it;
			++elements_to_erase_count;
			array.emplace_back(std::move(value));
		}
		node_stack_.erase(next(node_stack_.begin(), node_stack_.size() - elements_to_erase_count), node_stack_.end());
		reverse(array.begin(), array.end());
		node_stack_.emplace_back(std::move(array));
		return *this;
	}
	
	json::Node Builder::Build() {
		return node_stack_.front();
	}
	
	
	//  ------------------ Context --------------------
	
	
	ValueAfterKeyContext::ValueAfterKeyContext(json::Builder& builder_ref)
			: Context(builder_ref)
	{}
	
	json::KeyContext ValueAfterKeyContext::Key(std::string key) {
		return GetBuilderRef().Key(std::move(key));
	}
	Builder& ValueAfterKeyContext::EndDict() {
		return GetBuilderRef().EndDict();
	}
	
	
	ValueAfterStartArrayContext::ValueAfterStartArrayContext(json::Builder& builder_ref)
			: Context(builder_ref)
	{}
	
	ValueAfterStartArrayContext ValueAfterStartArrayContext::Value(json::Node::Value value) {
		return GetBuilderRef().Value(std::move(value));
	}
	StartDictContext ValueAfterStartArrayContext::StartDict() {
		return GetBuilderRef().StartDict();
	}
	StartArrayContext ValueAfterStartArrayContext::StartArray() {
		return GetBuilderRef().StartArray();
	}
	Builder& ValueAfterStartArrayContext::EndArray() {
		return GetBuilderRef().EndArray();
	}
	
	
	KeyContext::KeyContext(json::Builder& builder_ref)
			: Context(builder_ref)
	{}
	
	ValueAfterKeyContext KeyContext::Value(json::Node::Value value) {
		return GetBuilderRef().Value(std::move(value));
	}
	StartDictContext KeyContext::StartDict() {
		return GetBuilderRef().StartDict();
	}
	StartArrayContext KeyContext::StartArray() {
		return GetBuilderRef().StartArray();
	}
	
	
	StartArrayContext::StartArrayContext(json::Builder& builder_ref)
			: Context(builder_ref)
	{}
	
	ValueAfterStartArrayContext StartArrayContext::Value(json::Node::Value value) {
		return GetBuilderRef().Value(std::move(value));
	}
	StartDictContext StartArrayContext::StartDict() {
		return GetBuilderRef().StartDict();
	}
	StartArrayContext StartArrayContext::StartArray() {
		return GetBuilderRef().StartArray();
	}
	Builder& StartArrayContext::EndArray() {
		return GetBuilderRef().EndArray();
	}
	
	
	StartDictContext::StartDictContext(json::Builder& builder_ref)
			: Context(builder_ref)
	{}
	
	KeyContext StartDictContext::Key(std::string key) {
		return GetBuilderRef().Key(std::move(key));
	}
	Builder& StartDictContext::EndDict() {
		return GetBuilderRef().EndDict();
	}
	
	

} // end namespace json