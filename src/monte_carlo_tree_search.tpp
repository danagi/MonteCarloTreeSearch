#include <stack>
#include <algorithm>
#include <set>
#include <type_traits>

namespace pai
{
	namespace
	{
		template<class Move,size_t player_num> struct MctsNode;

		template<class Move,size_t player_num>
		struct MctsEdge
		{
			double value;
			Move move;
			MctsNode<Move,player_num> *son;

			bool operator<(const MctsEdge& rhs)const
			{
				return value < rhs.value;
			}
		};

		template<class Move,size_t player_num>
		struct MctsNode
		{
			typedef GameState<Move,player_num> State;
			typedef MctsNode<Move,player_num> Node;

			State *game_state;
			std::queue<Move> *legal_moves;
			std::multiset<MctsEdge<Move,player_num>> edges;
			double winning_score;
			uint simulation_num;

			MctsNode(const State *game_state)
			{
				Init(game_state);
			}

			void Recycle()
			{
				delete game_state;
				delete legal_moves;
			}

			void Init(const State *game_state)
			{
				this->game_state = game_state->Clone();
				legal_moves = this->game_state->GenerateAllLegalMoves();
				if(!game_state->IsEnd() && legal_moves->empty())
				{
					throw CouldNotMoveExpection();
				}
				edges.clear();
				winning_score = 0.0;
				simulation_num = 0;
			}

			void Update(const std::array<double,player_num> *scores)
			{
				++simulation_num;
				winning_score += scores->at(game_state->get_previous_player());
			}
		};

		template<class Move,size_t player_num>
		class Allocator
		{
		private:
			typedef MctsNode<Move,player_num> Node;

			std::stack<Node*> pool;

		public:
			Node* NewNode(const GameState<Move,player_num> *game_state)
			{
				if(pool.empty())return new Node(game_state);
				auto p = pool.top();
				pool.pop();
				for(auto& edge:p->edges)
				{
					Recycle(edge.son);
				}
				p->Init(game_state);
				return p;
			}
			void Recycle(Node *node)
			{
				node->Recycle();
				pool.push(node);
			}
			~Allocator()
			{
				while(!pool.empty())
				{
					delete pool.top();
					pool.pop();
				}
			}
		};

		template<class T>
		struct has_operator_equal
		{
		    template<class U>
		    static auto test(U*) -> decltype(std::declval<U>() == std::declval<U>());
		    template<typename>
		    static auto test(...) -> std::false_type;

		    enum{ value = std::is_same<bool, decltype(test<T>(0))>::type::value };
		};
	}

	template<class Move,size_t player_num>
	MonteCarloTreeSearch<Move,player_num>::MonteCarloTreeSearch(const State *game_state)
	{
		root = allocator.NewNode(game_state);
		max_num_of_iteration_ = InitMaxNumOfIteration;
		max_search_time_ = InitMaxSearchTime;
		selection_policy_ = ucb1_selection_policy;
		next_move_policy_ = max_visit_count_policy;
		simulation_policy_ = random_simulation_policy<Move,player_num>;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_game_state(const State *game_state)
	{
		allocator.Recycle(root);
		root = allocator.NewNode(game_state);
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_max_num_of_iteration(int max_num_of_iteration)
	{
		max_num_of_iteration_ = max_num_of_iteration;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_max_search_time(double max_search_time)
	{
		max_search_time_ = max_search_time;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_selection_policy(Func selection_policy)
	{
		selection_policy_ = selection_policy;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_simulation_policy(SFunc simulation_policy)
	{
		simulation_policy_ = simulation_policy;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::set_next_move_policy(Func next_move_policy)
	{
		next_move_policy_ = next_move_policy;
	}

	template<class Move,size_t player_num>
	void MonteCarloTreeSearch<Move,player_num>::Play(Move move)
	{
		static_assert(has_operator_equal<Move>::value,"class Move should has bool operator== ");
		if(root->game_state->IsEnd())
		{
			throw ShouldNotMoveExpection();
		}
		auto iter = std::find_if(root->edges.begin(),root->edges.end(),
			[&](const MctsEdge<Move,player_num>& son){return son.move == move;});
		if(iter == root->edges.end())
		{
			auto game_state = root->game_state->Clone();
			game_state->Play(move);
			set_game_state(game_state);
			return;
		}
		auto p = (*iter).son;
		root->edges.erase(iter);
		allocator.Recycle(root);
		root = p;
	}

	template<class Move,size_t player_num>
	std::array<double,player_num>* MonteCarloTreeSearch<Move,player_num>::Simulate(Node *node) const
	{
		auto game_state = node->game_state->Clone();
		auto scores = simulation_policy_(game_state);
		delete game_state;
		node->Update(scores);
		return scores;
	}

	template<class Move,size_t player_num>
	std::array<double,player_num>* MonteCarloTreeSearch<Move,player_num>::Search(Node *node)
	{
		if(node->game_state->IsEnd())
		{
			return Simulate(node);
		}
		Node *next_node = nullptr;
		Move move;
		std::array<double,player_num>* scores = nullptr;
		if(node->legal_moves->empty())
		{
			auto iter = prev(node->edges.end());
			next_node = (*iter).son;
			move = (*iter).move;
			node->edges.erase(iter);
			scores = Search(next_node);
		}
		else
		{
			auto game_state = node->game_state->Clone();
			move = node->legal_moves->front();
			node->legal_moves->pop();
			game_state->Play(move);
			next_node = allocator.NewNode(game_state);
			scores = Simulate(next_node);
		}
		node->Update(scores);
		MctsEdge<Move,player_num> edge;
		edge.value = selection_policy_(next_node->winning_score,next_node->simulation_num,node->simulation_num);
		edge.move = move;
		edge.son = next_node;
		node->edges.insert(edge);
		return scores;
	}

	template<class Move,size_t player_num>
	Move MonteCarloTreeSearch<Move,player_num>::SearchNextMove()
	{
		if(root->game_state->IsEnd())
		{
			throw ShouldNotMoveExpection();
		}
		using namespace std::chrono;
		auto start = system_clock::now();
		for(size_t i = 0;;++i)
		{
			if(max_search_time_ > 0.0)
			{
				auto end = system_clock::now();
				auto duration = duration_cast<milliseconds>(end-start);
				if(duration.count() >= max_search_time_)
					break;
			}
			if(max_num_of_iteration_ > 0 && i >= max_num_of_iteration_)
				break;
			delete Search(root);
		}
		typedef MctsEdge<Move,player_num> Edge;
		return (*std::max_element(root->edges.begin(),root->edges.end(),
			[&](const Edge& l,const Edge& r)
			{
				auto lvalue = next_move_policy_(l.son->winning_score,l.son->simulation_num,root->simulation_num);
				auto rvalue = next_move_policy_(r.son->winning_score,r.son->simulation_num,root->simulation_num);
				return lvalue < rvalue;
			})).move;
	}
}