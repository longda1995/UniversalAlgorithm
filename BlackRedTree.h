#pragma once
#include <cstdint>
#include <utility>
#include <xhash>

template<
		typename Key,
		typename Value = int32_t,
        typename Comparator = std::hash_compare<Key, std::less<Key>>
		>
class blackredtree
{
public:
	using nodepair = std::pair<Key, Value>;
	struct node
	{
		friend class blackredtree;

		constexpr node(bool isblack,const nodepair pair)
			:isBlack(isblack),val(pair)
		{}

		decltype(auto) getkey()const 
		{
			return val.first;
		}
		
		decltype(auto) getval()const
		{
			return val.second;
		}

	private:
		void setVal(Value value)
		{
			val.second = value;
		};
		
		uint8_t isBlack:1;
		node* parentNode = nullptr;
		node* leftNode = nullptr;
		node* rightNode = nullptr;
		nodepair val;
	};

	node* find(const Key&& key)const;
	void insert(node* _node);
	node* insert(const Key&& key,const Value&& value);
	void remove(node* _node);
	
private:
	void rotateNode(node* node,bool isLeft);
	void fixAfterAddNode(node* _node);
	void fixAfterDeleteNode(node* _node);
	static node* successor(node* _node);
	
	node* rootnode = nullptr;
	size_t size = 0;
};
#define LeftBrotherNode(node) node -> parentNode -> leftNode 
#define RightBrotherNode(node) node -> parentNode -> rightNode 
#define LeftUncleNode(node) node -> parentNode -> parentNode -> leftNode
#define RightUncleNode(node) node -> parentNode -> parentNode -> rightNode
#define IsLeftOfParent(node) (node == node -> parentNode -> leftNode)
#define IsRightOfParent(node) (node == node -> parentNode -> rightNode)

template<typename Key, typename Value, typename Comparator>
inline typename blackredtree<Key, Value, Comparator>::node * 
blackredtree<Key, Value, Comparator>::find(const Key && key) const
{
	node* curNode = rootnode;
	while (curNode != nullptr)
	{
		if (curNode->getkey() > key)
			curNode = curNode->leftNode;
		else if (curNode->getkey() < key)
			curNode = curNode->rightNode;
		else
			break; 
	}
	return curNode;
}

template<typename Key, typename Value, typename Comparator>
inline void blackredtree<Key, Value, Comparator>::insert(node * _node)
{
	if(_node != nullptr)
	{
		node* curNode = rootnode;
		node* targetParentNode = nullptr;
		while(curNode != nullptr)
		{
			targetParentNode = curNode;
			if(curNode->getkey() > _node->getkey())
				curNode = curNode->leftNode;
			else
				curNode = curNode->rightNode;
		}
		_node->parentNode = targetParentNode;
		if (targetParentNode == nullptr)
		{
			rootnode = _node;
			size++;
		}
		else
		{
			if (_node->getkey() < targetParentNode->getkey())
			{
				targetParentNode->leftNode = _node;
				size++;
			}
			else if (_node->getkey() > targetParentNode->getkey())
			{
				targetParentNode->rightNode = _node;
				size++;
			}
			else
			{
				targetParentNode->setVal(_node->getval());
			}
		}
		fixAfterAddNode(_node);
	}
}

template<typename Key, typename Value, typename Comparator>
inline typename blackredtree<Key, Value, Comparator>::node *
blackredtree<Key, Value, Comparator>::insert(const Key && key, const Value && value)
{
	node* node = new typename blackredtree<Key, Value, Comparator>::node(false,{key,value});
	insert(node);
	return node;
}


template<typename Key, typename Value, typename Comparator>
inline void blackredtree<Key, Value, Comparator>::remove(node * _node)
{
	size--;
	if (_node->leftNode != nullptr && _node->rightNode != nullptr) {
		_node = successor(_node);
	}
	node* replacement = (_node->leftNode != nullptr ? _node->leftNode : _node->rightNode);
	if (replacement != nullptr) {
		replacement->parentNode = _node->parentNode;
		if (_node->parentNode == nullptr)
			rootnode = replacement;
		else if (IsLeftOfParent(_node))
			LeftBrotherNode(_node) = replacement;
		else
			RightBrotherNode(_node) = replacement;
		_node->leftNode = _node->rightNode = _node->parentNode = nullptr;
		if (_node->isBlack)
			fixAfterDeleteNode(replacement);
	}
	else if (_node->parentNode == nullptr) 
	{
		rootnode = nullptr;
	}
	else { 
		if (_node->isBlack)
			fixAfterDeleteNode(_node);
		if (_node->parentNode != nullptr) 
		{
			if (IsLeftOfParent(_node))
				LeftBrotherNode(_node) = nullptr;
			else if (IsRightOfParent(_node))
				RightBrotherNode(_node) = nullptr;
			_node->parentNode = nullptr;
		}
	}
}

template<typename Key, typename Value, typename Comparator>
inline void blackredtree<Key, Value, Comparator>::rotateNode(node* _node,bool isLeft)
{
	if (_node != nullptr)
	{
		node* targetChild = isLeft ? _node->rightNode : _node->leftNode;
		if (isLeft)
		{
			_node->rightNode = targetChild->leftNode;
			if (targetChild->leftNode != nullptr)
				targetChild->leftNode->parentNode = _node;
			targetChild->parentNode = _node->parentNode;
		}
		else
		{
			_node->leftNode = targetChild->rightNode;
			if (targetChild->rightNode != nullptr)
				targetChild->rightNode->parentNode = _node;
			targetChild->parentNode = _node->parentNode;
		}
		if(_node->parentNode == nullptr)
		{
			rootnode = targetChild;
		}
		else
		{
			if (IsLeftOfParent(_node))
				_node->parentNode->leftNode = targetChild;
			else
				_node->parentNode->rightNode = targetChild;
		}
		isLeft ? targetChild->leftNode = _node : targetChild->rightNode = _node;
		_node->parentNode = targetChild;
	}
}

template<typename Key, typename Value, typename Comparator>
inline void blackredtree<Key, Value, Comparator>::fixAfterAddNode(node * _node)
{
	_node->isBlack = false;
	node* grandParent = nullptr;

	auto doFix = [&](bool isLeftChild)                                                         
	{
		node* uncle = isLeftChild ? RightUncleNode(_node) : LeftUncleNode(_node);
		if (uncle != nullptr && !uncle->isBlack)											   
		{
			_node->parentNode->isBlack = true;
			uncle->isBlack = true;
			grandParent->isBlack = false;													   
			_node = grandParent;
		}
		else
		{
			if (isLeftChild ? IsRightOfParent(_node) : IsLeftOfParent(_node))
			{
				_node = _node->parentNode;
				rotateNode(_node, isLeftChild);
			}
			_node->parentNode->isBlack = true;
			grandParent->isBlack = false;
			rotateNode(grandParent,!isLeftChild);
		}
	};																						   
																								
	while (_node->parentNode != nullptr && !_node->parentNode->isBlack)						   
	{																						   
		grandParent = _node->parentNode->parentNode;
		doFix(IsLeftOfParent(_node->parentNode));
    }
	rootnode->isBlack = true;
}

template<typename Key, typename Value, typename Comparator>
inline void blackredtree<Key, Value, Comparator>::fixAfterDeleteNode(node * _node)
{
	auto doFix = [&](bool isLeftChild)
	{
		node* brotherNode = isLeftChild ? RightBrotherNode(_node) : LeftBrotherNode(_node);
		if (!brotherNode->isBlack)
		{
			brotherNode->isBlack = true;
			_node->parentNode->isBlack = false;
			rotateNode(_node->parentNode, isLeftChild);
			brotherNode = isLeftChild ? RightBrotherNode(_node) : LeftBrotherNode(_node);
		}
		if ((brotherNode->rightNode == nullptr || brotherNode->leftNode->isBlack)
			&& (brotherNode->leftNode == nullptr || brotherNode->rightNode->isBlack))
		{
			brotherNode->isBlack = false;
			_node = _node->parentNode;
		}
		else
		{
			if(isLeftChild ? brotherNode->rightNode->isBlack : brotherNode->leftNode->isBlack)
			{
				isLeftChild ? brotherNode->leftNode->isBlack = true
					: brotherNode->rightNode->isBlack = true;
				brotherNode->isBlack = false;
				rotateNode(brotherNode,!isLeftChild);
				brotherNode = isLeftChild ? RightUncleNode(_node) : LeftUncleNode(_node);
			}
			brotherNode->isBlack = _node->parentNode->isBlack;
			_node->parentNode->isBlack = true;
			isLeftChild ? brotherNode->rightNode->isBlack = true
				: brotherNode->leftNode->isBlack = true;
			rotateNode(_node->parentNode, isLeftChild);
			_node = rootnode;
		}
	};
	while (_node!= nullptr && _node != rootnode && _node->isBlack) 
	{
		doFix(IsLeftOfParent(_node));
	}
	if(_node != nullptr) _node->isBlack = true;
}

template<typename Key, typename Value, typename Comparator>
inline typename blackredtree<Key, Value, Comparator>::node *
blackredtree<Key, Value, Comparator>::successor(node * _node)
{
	if (_node == nullptr)
		return nullptr;
	else if (_node->rightNode != nullptr) 
	{
		node* rightNode = _node->rightNode;
		while (rightNode->leftNode != nullptr)
			rightNode = rightNode->leftNode;
		return rightNode;
	}
	else 
	{
		node* parentNode = _node->parentNode;
		node* ch = _node;
		while (parentNode != nullptr && ch == parentNode->rightNode) 
		{
			ch = parentNode;
			parentNode = parentNode->parentNode;
		}
		return parentNode;
	}
}

