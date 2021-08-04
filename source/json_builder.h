#pragma once

#include "json.h"

#include <string>
#include <memory>
#include <deque>
#include <algorithm>

namespace json {
	
	class ValueAfterStartArrayContext;
	class ValueAfterKeyContext;
	class KeyContext;
	class StartDictContext;
	class StartArrayContext;
	
	class Builder {
	public:
		KeyContext Key(std::string);
		Builder& Value(json::Node::Value);
		StartDictContext StartDict();
		StartArrayContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();
	
	private:
		std::vector<Node> node_stack_;
	};

	
// ------------------------- Context --------------------------------


	class Context {
	public:
		Context(json::Builder& builder_ref)
				: builder_ref_(builder_ref)
		{};
		
	protected:
		Builder& GetBuilderRef() {
			return builder_ref_;
		}
		
	private:
		json::Builder& builder_ref_;
	};
	
	class ValueAfterKeyContext : public Context{
	public:
		ValueAfterKeyContext(json::Builder& builder_ref);
		
		KeyContext Key(std::string key);
		Builder& EndDict();
	};
	
	class ValueAfterStartArrayContext : public Context{
	public:
		ValueAfterStartArrayContext(json::Builder& builder_ref);
		
		ValueAfterStartArrayContext Value(json::Node::Value value);
		StartDictContext StartDict();
		StartArrayContext StartArray();
		Builder& EndArray();
	};
	
	class KeyContext : public Context{
	public:
		KeyContext(json::Builder& builder_ref);
		
		ValueAfterKeyContext Value(json::Node::Value value);
		StartDictContext StartDict();
		StartArrayContext StartArray();
	};
	
	class StartArrayContext : public Context{
	public:
		StartArrayContext(json::Builder& builder_ref);
		
		ValueAfterStartArrayContext Value(json::Node::Value value);
		StartDictContext StartDict();
		StartArrayContext StartArray();
		Builder& EndArray();
	};
	
	class StartDictContext : public Context{
	public:
		StartDictContext(json::Builder& builder_ref);
		
		KeyContext Key(std::string key);
		Builder& EndDict();
	};

} // end namespace json
