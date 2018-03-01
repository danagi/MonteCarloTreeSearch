# MonteCarloTreeSearch
---
## What is this?
This is a c++11 template which implements [Monte Carlo Tree Search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) algorithm.
## How to use？
1. implement interface ```GameState<>``` defines in ```game_state.h``` , it describes your game.
2. construt object ```MonteCarloTreeSearch<>``` defines in ```monte_carlo_tree_search.h```
3. use ```MonteCarloTreeSearch::SearchNextMove()``` to get next move.
### ```game_state.h```
#### interface
- ```GameState<Move,player_num>* Clone() const``` 
  copy itself and return the pointer of it's copy
-	```void Play(Move move)```
  the current player play a move to change the game state
-	```std::queue<Move>* GenerateAllLegalMoves() const``` 
  generate all legal moves in current game state
-	```bool IsEnd() const```
	check whether game is end
-	```std::array<double,player_num>* CaculateScore() const```
	caculate every player's score in current game state
-	```size_t get_previous_player() const```
	the current player is who is going to play next move ,so the previous player is who has played last move before
### ```monte_carlo_tree_search.h```
#### property
-	```int max_num_of_iteration```
	the maximum number of iteration , defult to ```kInitMaxNumOfIteration``` , you can set it to ```kInfiniteNumOfIteration``` and it will be infinite
-	```double max_search_time```
	the maximum search time in millisecond,default to ```kInitMaxSearchTime``` , you can set it to ```kInfiniteSearchTime``` and it will be infinite
-	```std::function<double(double,uint,uint)> selection_policy```
	the policy of selecting next node , default to ```ucb1_selection_policy``` 
-	```std::function<std::array<double,player_num>*(State *game_state)> simulation_policy```
	the policy of simulation , default to ```random_simulation_policy```
-	```std::function<double(double,uint,uint)> next_move_policy```
	the policy of selecting next move , default to ```max_visit_count_policy```
#### function
```cpp
	MonteCarloTreeSearch(const State *game_state);

	void set_game_state(const State *game_state);
	void set_max_num_of_iteration(int max_num_of_iteration);
	void set_max_search_time(double max_search_time);
	void set_selection_policy(Func selection_policy);
	void set_simulation_policy(SFunc simulation_policy);
	void set_next_move_policy(Func next_move_policy);

	void Play(Move move);
	Move SearchNextMove();
```