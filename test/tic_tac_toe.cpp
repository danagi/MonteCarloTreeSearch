#include "../src/game_state.h"
#include "../src/monte_carlo_tree_search.h"

#include <iostream>

typedef GameState<int,2> State;

class TicTacToe : public State
{
private:
	int player;
	char s[3][3];
public:
	TicTacToe()
	{
		player = 0;
		for(int i = 0;i < 3;++i)
			for(int j = 0;j < 3;++j)
				s[i][j] = ' ';
	}
	State* Clone() const
	{
		return new TicTacToe(*this);
	}
	void Play(int move)
	{
		int x = move / 3,y = move % 3;
		s[x][y] = player ? 'X' : 'O';
		player = 1 - player;
	}
	std::queue<int>* GenerateAllLegalMoves() const
	{
		std::queue<int> *q = new std::queue<int>();
		for(int i = 0;i < 3;++i)
			for(int j = 0;j < 3;++j)
				if(s[i][j] == ' ')
					q->push(i * 3 + j);
		return q;
	}
	bool IsEnd() const
	{
		for(int i = 0;i < 3;++i)
		{
			if(s[i][0] != ' ' && s[i][0] == s[i][1] && s[i][1] == s[i][2])return true;
			if(s[0][i] != ' ' && s[0][i] == s[1][i] && s[1][i] == s[2][i])return true;
		}
		if(s[0][0] != ' ' && s[0][0] == s[1][1] && s[1][1] == s[2][2])return true;
		if(s[0][2] != ' ' && s[0][2] == s[1][1] && s[1][1] == s[2][0])return true;
		int cnt = 0;
		for(int i = 0;i < 3;++i)
			for(int j = 0;j < 3;++j)
				if(s[i][j] != ' ')
					++cnt;
		if(cnt == 9)return true;
		return false;
	}
	std::array<double,2>* CaculateScore() const
	{
		std::array<double,2> *a = new std::array<double,2>();
		for(int i = 0;i < 2;++i) a->at(i) = 0;
		int x = -1;
		for(int i = 0;i < 3;++i)
		{
			if(s[i][0] != ' ' && s[i][0] == s[i][1] && s[i][1] == s[i][2])x = s[i][0] == 'X';
			if(s[0][i] != ' ' && s[0][i] == s[1][i] && s[1][i] == s[2][i])x = s[0][i] == 'X';
		}
		if(s[0][0] != ' ' && s[0][0] == s[1][1] && s[1][1] == s[2][2])x = s[0][0] == 'X';
		if(s[0][2] != ' ' && s[0][2] == s[1][1] && s[1][1] == s[2][0])x = s[0][2] == 'X';
		if(x != -1)
		{
			a->at(x) = 1;
			a->at(1-x) = -1;
		}
		return a;
	}
	size_t get_previous_player() const
	{
		return 1-player;
	}
	void Draw()
	{
		for(int i = 0;i < 3;++i)
		{
			std::cout << " - - -" << std::endl;
			for(int j = 0;j < 3;++j)
				std::cout << "|" << s[i][j];
			std::cout << "|" << std::endl;
		}
		std::cout << " - - -" << std::endl;
		std::cout << std::endl;
	}
};

int main()
{
	auto game = new TicTacToe();
	auto ai = MonteCarloTreeSearch<int,2>(game);
	while(!game->IsEnd())
	{
		int move = ai.SearchNextMove();
		game->Play(move);
		ai.Play(move);
		game->Draw();
	}
	return 0;
}