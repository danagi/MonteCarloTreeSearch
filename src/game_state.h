#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <queue>
#include <array>

namespace pai
{
	template<class Move,size_t player_num>
	class GameState
	{
	public:
		virtual ~GameState(){};
		virtual GameState<Move,player_num>* Clone() const = 0;
		virtual void Play(Move move) = 0;
		virtual std::queue<Move>* GenerateAllLegalMoves() const = 0;
		virtual bool IsEnd() const = 0;
		virtual std::array<double,player_num>* CaculateScore() const = 0;
		virtual size_t get_previous_player() const = 0;
	};
}

#endif