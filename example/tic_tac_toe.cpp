#include "../src/game_state.h"
#include "../src/monte_carlo_tree_search.h"

#include <iostream>
#include <cstring>

union Move
{
	int value;
	struct{
		unsigned char x,y,i,j;
	}pos;
	bool operator == (const Move& move)const{return value == move.value;}
};

typedef pai::GameState<Move,2> State;

class TicTacToe : public State
{
private:
	unsigned char player;
	char s[3][3][3][3];
	char win[3][3];
public:
	TicTacToe()
	{
		player = 0;
		memset(s,' ',sizeof(s));
		memset(win,-1,sizeof(win));
	}
	State* Clone() const
	{
		return new TicTacToe(*this);
	}
	void Play(Move move)
	{
		int x = move.pos.x, y = move.pos.y, i = move.pos.i, j = move.pos.j;
		s[x][y][i][j] = player ? 'X' : 'O';
		if(s[x][y][i][0] != ' ' && s[x][y][i][0] == s[x][y][i][1] && s[x][y][i][1] == s[x][y][i][2])
			win[x][y] = player;
		if(s[x][y][0][j] != ' ' && s[x][y][0][j] == s[x][y][1][j] && s[x][y][1][j] == s[x][y][2][j])
			win[x][y] = player;
		if(i == j && s[x][y][0][0] != ' ' && s[x][y][0][0] == s[x][y][1][1] && s[x][y][1][1] == s[x][y][2][2])
			win[x][y] = player;
		if(i + j == 2 && s[x][y][0][2] != ' ' && s[x][y][0][2] == s[x][y][1][1] && s[x][y][1][1] == s[x][y][2][0])
			win[x][y] = player;
		int cnt = 0;
		for(int a = 0;a < 3;++a)
			for(int b = 0;b < 3;++b)
				if(s[x][y][a][b] != ' ')
					++cnt;
		if(cnt == 9) win[x][y] = -2;
		player = 1 - player;
	}
	std::queue<Move>* GenerateAllLegalMoves() const
	{
		std::queue<Move> *q = new std::queue<Move>();
		typedef unsigned char byte;
		for(byte x = 0;x < 3;++x)
			for(byte y = 0;y < 3;++y)
				if(win[x][y] == -1)
					for(byte i = 0;i < 3;++i)
						for(byte j = 0;j < 3;++j)
							if(s[x][y][i][j] == ' ')
								q->push({.pos.x = x,.pos.y = y,.pos.i = i,.pos.j = j});
		return q;
	}
	bool IsEnd() const
	{
		for(int i = 0;i < 3;++i)
		{
			if(win[i][0] >= 0 && win[i][0] == win[i][1] && win[i][1] == win[i][2])return true;
			if(win[0][i] >= 0 && win[0][i] == win[1][i] && win[1][i] == win[2][i])return true;
		}
		if(win[0][0] >= 0 && win[0][0] == win[1][1] && win[1][1] == win[2][2])return true;
		if(win[0][2] >= 0 && win[0][2] == win[1][1] && win[1][1] == win[2][0])return true;
		int cnt = 0;
		for(int i = 0;i < 3;++i)
			for(int j = 0;j < 3;++j)
				if(win[i][j] != -1)
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
			if(win[i][0] >= 0 && win[i][0] == win[i][1] && win[i][1] == win[i][2])x = win[i][0];
			if(win[0][i] >= 0 && win[0][i] == win[1][i] && win[1][i] == win[2][i])x = win[0][i];
		}
		if(win[0][0] >= 0 && win[0][0] == win[1][1] && win[1][1] == win[2][2])x = win[0][0];
		if(win[0][2] >= 0 && win[0][2] == win[1][1] && win[1][1] == win[2][0])x = win[0][2];
		if(x != -1)
		{
			a->at(x) = 1;
			a->at(1 - x) = -1;
		}
		return a;
	}
	size_t get_previous_player() const
	{
		return 1 - player;
	}
	void Draw()
	{
		auto draw_line = []()
		{
			for(int k = 0;k < 3;++k)
			{
				for(int l = 0;l < 7;++l)std::cout << '-';
				std::cout<<"    ";
			}
			std::cout << std::endl;
		};
		for(int x = 0;x < 3;++x)
		{
			for(int i = 0;i < 3;++i)
			{
				draw_line();
				for(int y = 0;y < 3;++y)
				{
					for(int j = 0;j < 3;++j)std::cout << '|' << s[x][y][i][j];
					std::cout << "|    ";
				}
				std::cout << std::endl;
			}
			draw_line();
			std::cout << std::endl;
		}
		std::cout << std::endl << std::endl;
	}
};


int main()
{
	auto game = new TicTacToe();
	auto ai = pai::MonteCarloTreeSearch<Move,2>(game);
	ai.set_max_num_of_iteration(pai::InfiniteNumOfIteration);
	while(!game->IsEnd())
	{
		Move move = ai.SearchNextMove();
		game->Play(move);
		ai.Play(move);
		game->Draw();
	}
	delete game;
	return 0;
}