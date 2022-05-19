#pragma once

#include "json.h"

#include <memory>
#include <utility>

namespace json {

	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;

	// Builder

	class Builder {
	private:
		node root_;
		std::vector<node*> nodes_stack_;
		bool is_finished_ = false;

	public:
		Builder();
		virtual KeyItemContext Key(std::string key);
		Builder& Value(node value);
		virtual DictItemContext StartDict();
		virtual ArrayItemContext StartArray();
		virtual Builder& EndDict();
		virtual Builder& EndArray();
		virtual Node Build();

	};

	// DictItemContext

	class DictItemContext final : Builder {
	private:
		Builder& builder_;

	public:
		DictItemContext(Builder&& builder);

		Builder& EndDict() override;
		KeyItemContext Key(std::string key) override;

	};

	// ArrayItemContext

	class ArrayItemContext final : Builder {
	private:
		Builder& builder_;

	public:
		ArrayItemContext(Builder&& builder);

		ArrayItemContext& Value(node value);
		DictItemContext StartDict() override;
		ArrayItemContext StartArray() override;
		Builder& EndArray() override;

	};

	// KeyItemContext

	class KeyItemContext final : Builder {
	private:
		Builder& builder_;

	public:
		KeyItemContext(Builder&& builder);

		DictItemContext Value(node value);
		DictItemContext StartDict() override;
		ArrayItemContext StartArray() override;

	};

}
