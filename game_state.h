#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <queue>
#include <array>

template<class Move,size_t player_num>
class GameState
{
public:
	virtual GameState<Move,player_num>* Clone() const = 0;
	virtual void Play(Move move) = 0;
	virtual std::queue<Move>* GenerateAllLegalMoves() = 0;
	virtual bool IsEnd() = 0;
	virtual std::array<double,player_num>* CaculateScore() = 0;
	virtual size_t get_current_player() = 0;
};

#endif