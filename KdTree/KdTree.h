#pragma once


#include <cstdlib>		// malloc and free
#include <cstring>		// memcpy


template<typename Payload, typename Type, unsigned int Dim = 3>
class KdTree final
{
	static_assert(std::numeric_limits<Type>::is_integer || std::is_same<float, Type>::value || std::is_same<double, Type>::value, "Type can be only integer, float or double");
	static_assert(Dim >= 2 && Dim <= 3 , "Dimension can be only 2 or 3");

public:
	struct Node
	{
		int m_dir;
		Type* m_pos;
		Payload* m_payload;

		Node* m_left;
		Node* m_right;
	};

	struct ResultNode
	{
		Node* m_item;
		ResultNode* m_next;
		Type m_squareDistance;
	};

	struct Result
	{
		ResultNode* m_list;
		ResultNode* m_iterator;
		int m_size;
	};

public:
	KdTree() : m_results(nullptr), m_root(nullptr), m_rect(nullptr)
	{
	}

	~KdTree()
	{
		Clear();
	}

	bool Insert(const Type* _pos, Payload* _payload)
	{
		if (Insert(&m_root, _pos, _payload, 0))
		{
			return false;
		}

		if (m_rect == nullptr)
		{
			m_rect = CreateHyperRect(_pos, _pos);
		}
		else
		{
			ExtendHyperRect(m_rect, _pos);
		}

		return true;
	}

	Result* Find(const Type* _pos, Type _range)
	{
		Result* resultSet;
		resultSet = static_cast<Result*>(malloc(sizeof(*resultSet)));
		resultSet->m_list = static_cast<ResultNode*>(malloc(sizeof(ResultNode)));
		resultSet->m_list->m_next = 0;

		int result = Find(m_root, _pos, _range, resultSet->m_list, 0);
		if (result == -1)
		{
			DestroyResults(resultSet);
			return nullptr;
		}

		resultSet->m_size = result;
		RewindResults(resultSet);
		return resultSet;
	}

	void DestroyResults(Result* _resultSet)
	{
		ClearResults(_resultSet);
		free(_resultSet->m_list);
		free(_resultSet);
	}

	void RewindResults(Result* _resultSet)
	{
		_resultSet->m_iterator = _resultSet->m_list->m_next;
	}

	bool ResultsEnd(Result* _resultSet)
	{
		return _resultSet->m_iterator == nullptr;
	}

	bool NextResult(Result* _resultSet)
	{
		_resultSet->m_iterator = _resultSet->m_iterator->m_next;
		return _resultSet->m_iterator != nullptr;
	}

	Payload* GetResult(Result* _resultSet)
	{
		if (_resultSet->m_iterator)
		{
			return _resultSet->m_iterator->m_item->m_payload;
		}
		return nullptr;
	}

private:
	struct HyperRect
	{
		Type* m_min;
		Type* m_max;
	};

private:
	void Clear()
	{
		Clear(m_root);
		m_root = nullptr;

		if (m_rect)
		{
			FreeHyperRect(m_rect);
			m_rect = nullptr;
		}
	}

	void Clear(Node* _node)
	{
		if (!_node)
		{
			return;
		}

		Clear(_node->m_left);
		Clear(_node->m_right);

		free(_node);
	}

	void FreeHyperRect(HyperRect* _rect)
	{
		free(_rect->m_min);
		free(_rect->m_max);
		free(_rect);
	}

	bool Insert(Node** _nodeList, const Type* _pos, Payload* _payload, int _dir)
	{
		if (!*_nodeList)
		{
			Node* node = static_cast<Node*>(malloc(sizeof(*node)));
			node->m_pos = static_cast<Type*>(malloc(Dim * sizeof(*node->m_pos)));
			memcpy(node->m_pos, _pos, Dim * sizeof(*node->m_pos));
			node->m_payload = _payload;
			node->m_dir = _dir;
			node->m_left = node->m_right = 0;

			*_nodeList = node;

			return true;
		}

		Node* node = *_nodeList;
		int updateDir = (node->m_dir + 1) % Dim;
		if (_pos[node->m_dir] < node->m_pos[node->m_dir])
		{
			return Insert(&(*_nodeList)->m_left, _pos, _payload, updateDir);
		}

		return Insert(&(*_nodeList)->m_right, _pos, _payload, updateDir);
	}

	HyperRect* CreateHyperRect(const Type* _min, const Type* _max)
	{
		size_t size = Dim * sizeof(Type);

		HyperRect* rect = static_cast<HyperRect*>(malloc(sizeof(HyperRect)));
		rect->m_min = static_cast<Type*>(malloc(size));
		rect->m_max = static_cast<Type*>(malloc(size));

		memcpy(rect->m_min, _min, size);
		memcpy(rect->m_max, _max, size);

		return rect;
	}

	void DestroyHyperRect(HyperRect* _rect)
	{
		free(_rect->m_min);
		free(_rect->m_max);
		free(_rect);
	}

	void ExtendHyperRect(HyperRect* _rect, const Type* _pos)
	{
		for_<Dim>([&](auto i)
			{
				if (_pos[i.value] < _rect->m_min[i.value])
				{
					_rect->m_min[i.value] = _pos[i.value];
				}
				if (_pos[i.value] > _rect->m_max[i.value])
				{
					_rect->m_max[i.value] = _pos[i.value];
				}
			});
	}

	Type SquareDistance(HyperRect* _rect, const Type* _pos)
	{
		Type result = 0;

		for_<Dim>([&](auto i)
			{
				if (_pos[i.value] < _rect->m_min[i.value])
				{
					result += Square(_rect->m_min[i.value] - _pos[i.value]);
				}
				else if (_pos[i.value] > _rect->m_max[i.value])
				{
					result += Square(_rect->m_max[i.value] - _pos[i.value]);
				}
			});

		return result;
	}

	void ResultListInsert(ResultNode* _list, Node* _item, Type _squareDistance)
	{
		ResultNode* resultNode = static_cast<ResultNode*>(malloc(sizeof(ResultNode)));
		resultNode->m_item = _item;
		resultNode->m_squareDistance = _squareDistance;

		if (_squareDistance >= 0.0f)
		{
			while (_list->m_next && _list->m_next->m_squareDistance < _squareDistance)
			{
				_list = _list->m_next;
			}
		}
		resultNode->m_next = _list->m_next;
		_list->m_next = resultNode;
	}

	void ClearResults(Result* _resultSet)
	{
		ResultNode* tmp, * node = _resultSet->m_list->m_next;

		while (node)
		{
			tmp = node;
			node = node->m_next;
			free(tmp);
		}

		_resultSet->m_list->m_next = nullptr;
	}

	int Find(Node* _node, const Type* _pos, Type range, ResultNode* _list, int _ordered)
	{
		int accumulatorResult = 0;

		if (!_node)
		{
			return 0;
		}

		Type squareDistance = 0.0f;

		for_<Dim>([&](auto i)
			{
				squareDistance += Square(_node->m_pos[i.value] - _pos[i.value]);
			});

		if (squareDistance <= Square(range))
		{
			ResultListInsert(_list, _node, _ordered ? squareDistance : -1.0f);
			accumulatorResult = 1;
		}

		Type dx = _pos[_node->m_dir] - _node->m_pos[_node->m_dir];

		int ret = Find(dx <= 0.0f ? _node->m_left : _node->m_right, _pos, range, _list, _ordered);
		if (ret >= 0 && fabs(dx) < range)
		{
			accumulatorResult += ret;
			ret = Find(dx <= 0.0f ? _node->m_right : _node->m_left, _pos, range, _list, _ordered);
		}

		if (ret == -1)
		{
			return -1;
		}

		accumulatorResult += ret;

		return accumulatorResult;
	}

private:
	constexpr Type Square(Type _x) { return _x * _x; }

	template<std::size_t N>
	struct num { static const constexpr auto value = N; };

	template <class F, std::size_t... Is>
	void for_(F func, std::index_sequence<Is...>)
	{
		(func(num<Is>{}), ...);
	}

	template <std::size_t N, typename F>
	void for_(F func)
	{
		for_(func, std::make_index_sequence<N>());
	}

private:
	Result* m_results;
	Node* m_root;
	HyperRect* m_rect;
};