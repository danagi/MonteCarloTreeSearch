#ifndef MONTE_CARLO_TREE_SEARCH_H
#define MONTE_CARLO_TREE_SEARCH_H

#include "game_state.h"
#include "mcts_node.h"

#include <functional>
#include <exception>
#include <chrono>

struct CouldNotMoveExpection : std::exception
{
	const char* what() const noexcept { return "Game is not end,but has no legal moves.";}
};

struct ShouldNotMoveExpection : std::exception
{
	const char* what() const noexcept { return "Game is end,but try to move.";}
};

const int kInfiniteNumOfIteration = 0;
const double kInfiniteSearchTime = 0.0;
const int kInitMaxNumOfIteration = 1<<15;
const int kInitMaxSearchTime = 1000.0;

const auto ucb1_selection_policy = [](double w,uint n,uint N){return w/n+sqrt(2*log(N)/n);};
const auto max_visit_count_policy = [](double w,uint n,uint N){return double(n);};

template<class Move,size_t player_num>
std::array<double,player_num>* random_simulation_policy(GameState<Move,player_num> *game_state)
{
	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	while(!game_state->IsEnd())
	{
		auto legal_moves = game_state->GenerateAllLegalMoves();
		if(legal_moves->empty())
		{
			throw CouldNotMoveExpection();
		}
		std::uniform_int_distribution<size_t> distribution(0,legal_moves->size()-1);
		size_t k = distribution(generator),i = 0;
		while(i < k)
		{
			legal_moves->pop();
			++i;
		}
		game_state->Play(legal_moves->front());
	}
	return game_state->CaculateScore();
}

namespace
{
	typedef unsigned int uint;
	typedef std::function<double(double,uint,uint)> Func;

	template<class Move,size_t player_num> struct MctsNode;
	template<class Move,size_t player_num> class Allocator;
}

template<class Move,size_t player_num>
class MonteCarloTreeSearch
{
public:
	typedef GameState<Move,player_num> State;
	typedef MctsNode<Move,player_num> Node;
	typedef function<std::array<double,player_num>*(State *game_state)> SFunc;

	MonteCarloTreeSearch(const State *game_state);

	void set_game_state(const State *game_state);
	void set_max_num_of_iteration(int max_num_of_iteration);
	void set_max_search_time(double max_search_time);
	void set_selection_policy(Func selection_policy);
	void set_simulation_policy(SFunc simulation_policy);
	void set_next_move_policy(Func next_move_policy);

	void Play(Move move);
	Move SearchNextMove() const;

private:
	Node *root;
	Allocator<Move,player_num> allocator;
	int max_num_of_iteration_;
	double max_search_time_second_;
	Func selection_policy_;
	SFunc simulation_policy_;
	Func next_move_policy_;

	std::array<double,player_num>* Simulate(Node *node) const;
	std::array<double,player_num>* Search(Node *node) const;
};

#include "monte_carlo_tree_search.tpp"

#endif