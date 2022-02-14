#include "Connection.hpp"

#include <iostream>
#include <conio.h>

using namespace conn;

std::ostream& operator<< (std::ostream& out, GameConnection::State state)
{
	switch (state)
	{
	case GameConnection::State::NotConnected:
		out << "Not connected";
		break;
	case GameConnection::State::Registration:
		out << "Registration";
		break;
	case GameConnection::State::Searching:
		out << "Searching";
		break;
	case GameConnection::State::InGame:
		out << "In game";
		break;
	}
	return out;
}

int main()
{
	try
	{
		GameConnection* con = new WebSocketAsyncGameConnection();

		std::cout << "Enter nickname: ";
		String nickname; std::cin >> nickname;

		con->Register(nickname);
		std::cerr << "registration complete" << std::endl;

		while (true)
		{
			std::cout << "Enter command(-1 - exit, 1 - state, 2 - list, 3 - offer, 4 - message, 5 - messages, 6 - remove message, 7 - end game): ";
			
			std::string csid; std::cin >> csid;
			int id;
			try
			{
				id = std::stoi(csid);
			}
			catch (...)
			{
				std::cout << "Command must be an integer" << std::endl;
				continue;
			}

			if (id == -1)
			{
				break;
			}
			else if (id == 1)
			{
				std::cout << con->GetState() << std::endl;
			}
			else if (id == 2)
			{
				auto players = con->GetPlayers();
				std::cerr << "players:" << std::endl;
				for (auto& p : players)
					std::cout << p.id << ":" << p.nickname << std::endl;
			}
			else if (id == 3)
			{
				PlayerID pid; std::cin >> pid;
				con->SendOffer(pid);
			}
			else if (id == 4)
			{
				String m; std::cin >> m;
				con->SendMessage(m);
			}
			else if (id == 5)
			{
				auto messages = con->GetMessages();
				for (auto& p : messages)
					std::cout << p.messageId << ":" << p.message << std::endl;
			}
			else if (id == 6)
			{
				MessageID mid; std::cin >> mid;
				con->RemoveMessage(mid);
			}
			else if (id == 7)
			{
				con->EndGame();
			}
			else {
				std::cout << "Unknown command" << std::endl;
			}
		}

	}
	catch (ConnectionException const& e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "error" << std::endl;
	}
	std::cout << "Press any key to exit...";
	_getch();
	return 0;
}