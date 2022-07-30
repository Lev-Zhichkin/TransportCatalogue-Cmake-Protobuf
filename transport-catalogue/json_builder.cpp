#include "json_builder.h"


namespace json {

	// Builder

	Builder::Builder() {
		nodes_stack_.push_back(&root_);
	}

	KeyItemContext Builder::Key(std::string key) {

		if (nodes_stack_.size() == 1 || !std::holds_alternative<Dict>(*nodes_stack_.back())) {
			throw std::logic_error("Key call outside of dictionary");
		}
		if (nodes_stack_.size() != 1 && std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 2]) && std::holds_alternative<std::string>(*nodes_stack_[nodes_stack_.size() - 1])) {
			throw std::logic_error("Key call after previous Key call");
		}

		if (std::holds_alternative<Dict>(*nodes_stack_.back())) {
			node* val = new node(key);
			nodes_stack_.push_back(val);
		}
		return std::move(*this);
	}

	Builder& Builder::Value(node value) {

		if (is_finished_) {
			throw std::logic_error("Value call when object creation is in finished state");
		}
		if (nodes_stack_.size() > 1 && !std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 1]) && !std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 2])) {
			throw std::logic_error("Unproper Value call");
		}

		if (nodes_stack_.size() != 1 && std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 2])) {
			if (std::holds_alternative<nullptr_t>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<nullptr_t>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<Array>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<Array>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<Dict>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<Dict>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<bool>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<bool>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<int>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<int>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<double>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<double>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
			else if (std::holds_alternative<std::string>(value)) {
				std::get<Dict>(*nodes_stack_[nodes_stack_.size() - 2]).insert({ std::get<std::string>(*nodes_stack_.back()), Node(std::get<std::string>(value)) });
				delete nodes_stack_.back();
				nodes_stack_.pop_back();
			}
		}
		else if (std::holds_alternative<Array>(*nodes_stack_.back())) {
			if (std::holds_alternative<nullptr_t>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<nullptr_t>(value)));
			}
			else if (std::holds_alternative<Array>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<Array>(value)));
			}
			else if (std::holds_alternative<Dict>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<Dict>(value)));
			}
			else if (std::holds_alternative<bool>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<bool>(value)));
			}
			else if (std::holds_alternative<int>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<int>(value)));
			}
			else if (std::holds_alternative<double>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<double>(value)));
			}
			else if (std::holds_alternative<std::string>(value)) {
				std::get<Array>(*nodes_stack_.back()).push_back(Node(std::get<std::string>(value)));
			}
		}
		else if (nodes_stack_.size() == 1) {
			root_ = value;
			is_finished_ = true;
			//Node::Value* val = new Node::Value(value);
			//nodes_stack_.push_back(val);
		}
		return *this;
	}

	DictItemContext Builder::StartDict() {

		if (is_finished_) {
			throw std::logic_error("StartDict call when object creation is in finished state");
		}
		if (nodes_stack_.size() > 1 && !std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 1]) && !std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 2])) {
			throw std::logic_error("Unproper StartDict call");
		}

		if (std::holds_alternative<Array>(*nodes_stack_.back())) {
			std::get<Array>(*nodes_stack_.back()).push_back(Dict());
			Node* ptr = &std::get<Array>(*nodes_stack_.back()).back();
			nodes_stack_.push_back(&const_cast<node&>(ptr->GetNode()));
		}
		else {
			node* val = new node(Dict());
			nodes_stack_.push_back(val);
		}
		return std::move(*this);
	}

	ArrayItemContext Builder::StartArray() {

		if (is_finished_) {
			throw std::logic_error("StartArray call when object creation is in finished state");
		}
		if (nodes_stack_.size() > 1 && !std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 1]) && !std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 2])) {
			throw std::logic_error("Unproper StartArray call");
		}

		if (std::holds_alternative<Array>(*nodes_stack_.back())) {
			std::get<Array>(*nodes_stack_.back()).push_back(Array());
			Node* ptr = &std::get<Array>(*nodes_stack_.back()).back();
			nodes_stack_.push_back(&const_cast<node&>(ptr->GetNode()));
		}
		else {
			node* val = new node(Array());
			nodes_stack_.push_back(val);
		}
		return std::move(*this);
	}

	Builder& Builder::EndDict() {

		if (!std::holds_alternative<Dict>(*nodes_stack_[nodes_stack_.size() - 1])) {
			throw std::logic_error("Unproper EndDict call");
		}

		if (std::holds_alternative<Dict>(*nodes_stack_.back()) && nodes_stack_.size() == 2) {
			//return *this;
			root_ = *nodes_stack_.back();
			delete nodes_stack_.back();
			nodes_stack_.pop_back();
			is_finished_ = true;
		}
		else if (std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 2])) {
			nodes_stack_.pop_back();
		}
		else if (std::holds_alternative<Dict>(*nodes_stack_.back())) {
			Dict val = std::get<Dict>(*nodes_stack_.back());
			delete nodes_stack_.back();
			nodes_stack_.pop_back();
			this->Value(val);
		}
		return *this;
	}

	Builder& Builder::EndArray() {

		if (!std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 1])) {
			throw std::logic_error("Unproper EndArray call");
		}

		if (std::holds_alternative<Array>(*nodes_stack_.back()) && nodes_stack_.size() == 2) {
			//return *this;
			root_ = *nodes_stack_.back();
			delete nodes_stack_.back();
			nodes_stack_.pop_back();
			is_finished_ = true;
		}
		else if (std::holds_alternative<Array>(*nodes_stack_[nodes_stack_.size() - 2])) {
			nodes_stack_.pop_back();
		}
		else if (std::holds_alternative<Array>(*nodes_stack_.back())) {
			Array val = std::get<Array>(*nodes_stack_.back());
			delete nodes_stack_.back();
			nodes_stack_.pop_back();
			this->Value(val);
		}
		return *this;
	}

	Node Builder::Build() {

		if (!is_finished_ || nodes_stack_.size() != 1) {
			throw std::logic_error("Build call when object creation is in unfinished state");
		}

		//root_ = *nodes_stack_.back();
		//delete nodes_stack_.back();
		//nodes_stack_.pop_back();

		if (std::holds_alternative<nullptr_t>(root_)) {
			return Node(std::get<std::nullptr_t>(root_));
		}
		else if (std::holds_alternative<Array>(root_)) {
			return Node(std::get<Array>(root_));
		}
		else if (std::holds_alternative<Dict>(root_)) {
			return Node(std::get<Dict>(root_));
		}
		else if (std::holds_alternative<bool>(root_)) {
			return Node(std::get<bool>(root_));
		}
		else if (std::holds_alternative<int>(root_)) {
			return Node(std::get<int>(root_));
		}
		else if (std::holds_alternative<double>(root_)) {
			return Node(std::get<double>(root_));
		}
		else if (std::holds_alternative<std::string>(root_)) {
			return Node(std::get<std::string>(root_));
		}
		return Node();
	}

	// DictItemContext

	DictItemContext::DictItemContext(Builder&& builder)
		: builder_(builder)
	{
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	KeyItemContext DictItemContext::Key(std::string key) {
		return std::move(builder_.Key(std::move(key)));
	}

	// ArrayItemContext

	ArrayItemContext::ArrayItemContext(Builder&& builder)
		: builder_(builder)
	{
	}

	ArrayItemContext& ArrayItemContext::Value(node value) {
		builder_.Value(std::move(value));
		return *this;
	}

	DictItemContext ArrayItemContext::StartDict() {
		return std::move(builder_.StartDict());
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return std::move(builder_.StartArray());
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	// KeyItemContext

	KeyItemContext::KeyItemContext(Builder&& builder)
		: builder_(builder)
	{
	}

	DictItemContext KeyItemContext::Value(node value) {
		return std::move(builder_.Value(std::move(value)));
	}

	DictItemContext KeyItemContext::StartDict() {
		return std::move(builder_.StartDict());
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return std::move(builder_.StartArray());
	}

}