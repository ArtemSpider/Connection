#pragma once

#include <exception>
#include <string>
#include <vector>

// ��������� �������������� � ���� �����������
#pragma warning(push, 0)
#pragma warning(disable: 26820 26812 26498 26495 26451 26439 26444)
#pragma warning(disable: 6388 6387 6385 6258 6255 6001)
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#pragma warning(pop)

#include <unordered_map>
#include <set>
#include <thread>
#include <mutex>

#undef SendMessage

namespace conn
{
	class ConnectionException : public std::exception
	{
		std::string _msg;
	public:
		ConnectionException(const std::string& msg) : _msg(msg) {}

		virtual const char* what() const noexcept override
		{
			return _msg.c_str();
		}
	};

	class UnhandledServerMessageException : public ConnectionException
	{
	public:
		UnhandledServerMessageException(const std::string& msg) : ConnectionException(msg) {}
	};

	class StateException : public ConnectionException
	{
	public:
		StateException(const std::string& msg) : ConnectionException(msg) {}
	};

	class NicknameException : public ConnectionException
	{
	public:
		NicknameException(const std::string& msg) : ConnectionException(msg) {}
	};
	class MessageException : public ConnectionException
	{
	public:
		MessageException(const std::string& msg) : ConnectionException(msg) {}
	};

	class UnknownPlayerIdException : public ConnectionException
	{
	public:
		UnknownPlayerIdException(const std::string& msg) : ConnectionException(msg) {}
	};
	class UnknownMessageIdException : public ConnectionException
	{
	public:
		UnknownMessageIdException(const std::string& msg) : ConnectionException(msg) {}
	};

	/*
	* ��������������� ������, ������������ ��� ��� � ���������
	* � ������� ��� ����� ����������
	*/
	typedef std::string String;

	/*
	* ���������� ������������� ������
	* � ������� ��� ����� ����������
	*/
	typedef String PlayerID;

	/*
	* ��������� ��� �������� ���� ����������� ���������� �� ������
	*/
	struct Player
	{
		PlayerID id;
		String nickname;

		Player(PlayerID id, String nickname) : id(id), nickname(nickname) {}
	};

	/*
	* ���������� ������������� ���������
	* � ������� ��� ����� ����������
	*/
	typedef int MessageID;

	/*
	* ��������� ��� �������� ���� ����������� ���������� � ���������
	*/
	struct Message
	{
		PlayerID senderId;
		MessageID messageId;
		String message;

		Message(PlayerID senderId, MessageID messageId, String message) : senderId(senderId), messageId(messageId), message(message) {}
	};

	namespace beast = boost::beast;         // from <boost/beast.hpp>
	namespace http = beast::http;           // from <boost/beast/http.hpp>
	namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
	namespace net = boost::asio;            // from <boost/asio.hpp>
	using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

	/*
	* ������� ����� ��� ���������� � ��������
	*/
	class GameConnection
	{
	public:
		/*
		* ��������� ��������� �� �������������� �����������
		* ��������� ����� ��������� ������ ����� �������� � ����������� ��������, �����, ������, ���� � ������ �������������
		*/
		static bool IsValidMessage(String message);

		static const size_t MAX_NICKNAME_SIZE = 16;
		/*
		* ��������� ��� �� �������������� �����������
		* ��� ����� ��������� ������ ����� �������� � ����������� ��������, �����, ������, ���� � ������ �������������
		* ����� ����� �� ������ ���� ������ MAX_NICKNAME_SIZE
		* ���� MAX_NICKNAME_SIZE == -1, �������� ����� ����� �� ������������
		*/
		static bool IsValidNickname(String nickname);

		/*
		* ������� ������ ����������
		*/
		enum class State
		{
			/*
			* ���������� �� �����������, ������������ ������ ��� ����� � �������� ��� ������������
			*/
			NotConnected,

			/*
			* ���������� �����������, ����� �� ������� �� ��������������
			* ��� ����������� ������������ ����� Register
			*/
			Registration,

			/*
			* ������������ ��������������� � ��������� � ������ ���������
			*/
			Searching,

			/*
			* ������������ ��������� � ����
			* � ���� ��������� ������ �������� ������������� ��� ��������� ���������, �� ����������� ��
			*/
			InGame,
		};

		// ����������� ����� �������
		static const std::string DEFAULT_URL;
		// ����������� ���� ��� �����������
		static const std::string DEFAULT_PORT;

		/*
		* ��������� ���������� � ��������
		* ��������� ���� � ������� �� ������ ���� ��� ����������
		*/
		virtual ~GameConnection();

		/*
		* ���������� ������� ������ ����������
		*/
		State GetState();

		/*
		* ���������� ID ������
		* ���� ������������ �� ���������������, ���������� StateException
		*/
		PlayerID GetID();


		/*
		* ������������ ������ �� ������� � �������� ������ �� ����� ���������
		* ���� ��� �� ������������� ����������� ��� ����� � ����� ������ ��� ���������������, ���������� NicknameException
		* ��������� ��� �� �������������� ����������� ����� � ������� ������� IsValidNickname
		*/
		void Register(String nickname);

		/*
		* ���������� ������ ���� �������, ����������� � ������ ����
		* ���� ������������ �� ���������������, ���������� StateException
		*/
		std::vector<Player> GetPlayers();

		/*
		* ���������� ������ � ������ ����������� � ����
		* ���� ������������ �� ���������������, ���������� StateException
		* ���� sendTo ��� � ������ �������, ���������� UnknownPlayerIdException
		*/
		void SendOffer(PlayerID sendTo);

		/*
		* ���������� ������ �������, ������������ ������������ � ����
		* ���� ������������ �� ���������������, ���������� StateException
		*/
		std::vector<PlayerID> GetOffers();

		/*
		* ���������� ��� �������������� ��������� �� ���������
		* ���� ������������ �� ��������� � ����, ���������� StateException
		*/
		std::vector<Message> GetMessages();

		/*
		* ���������� ��������� ��� ������������, ��� ����� �� ����� ������������ ������� GetMessages
		* ���� ������������ �� ��������� � ����, ���������� StateException
		* ���� id ��� � ������ �������������� ���������, ���������� UnknownMessageIdException
		*/
		void RemoveMessage(MessageID id);

#ifdef _DEBUG
		/*
		* ���������� ��� ������������ ��������� �� ���������
		* ���� ������������ �� ��������� � ����, ���������� StateException
		*/
		std::vector<Message> GetParsedMessages();

		/*
		* ���������� ��� ��������� �� ���������
		* ���� ������������ �� ��������� � ����, ���������� StateException
		*/
		std::vector<Message> GetAllMessages();
#endif
		/*
		* ���������� ��������� ���������
		* ���� ������������ �� ��������� � ����, ���������� StateException
		* ���� ��������� �� ������������� �����������, ���������� MessageException
		* ��������� ��������� �� �������������� ����������� ����� � ������� ������� IsValidMessage
		*/
		void SendMessage(String message);

		/*
		* ���������� ��������� � �������� ���� � ��������� ������� ������ �� ����� ���������
		* ���� ������������ �� ��������� � ����, ���������� StateException
		*/
		void EndGame();
	protected:
		State state;
		PlayerID id;

		virtual void					_Register(String nickname) = 0;
		virtual std::vector<Player>		_GetPlayers() = 0;
		virtual void					_SendOffer(PlayerID sendTo) = 0;
		virtual std::vector<PlayerID>	_GetOffers() = 0;
		virtual std::vector<Message>	_GetMessages() = 0;
		virtual void					_RemoveMessage(MessageID id) = 0;

#ifdef _DEBUG
		virtual std::vector<Message>	_GetParsedMessages() = 0;
		virtual std::vector<Message>	_GetAllMessages() = 0;
#endif

		virtual void					_SendMessage(String message) = 0;
		virtual void					_EndGame() = 0;
	};


	/*
	* �����, �������������� ���������� � �������� ��������� ��� ������
	* ��������� ��������� ����������
	*/
	class WebSocketAsyncGameConnection : public GameConnection
	{
	private:
		std::string url, port;

		net::io_context ioc;
		tcp::resolver resolver;
		websocket::stream<tcp::socket> ws;
		const net::ip::basic_resolver_results<tcp> results;

		std::mutex dataMutex;

		std::unordered_map<MessageID, String> responses;

		std::string listResponse;
		bool listUpdated;

		// ��� ����������� ���� �� ������ �������
		std::vector<PlayerID> offers;

		// ID ���������
		PlayerID opponentID;

		// ���������� ���������� ID ��� ���������
		MessageID GetMessageID();

		// ��� �������������� ���������
		std::unordered_map<MessageID, Message> unparsedMessages;

#ifdef _DEBUG
		// ��� ������������ ���������
		std::set<Message> parsedMessages;
#endif

		bool stopRecieving;
		std::thread recieverThread;


		void ParseServerMessage(std::string message);
		void ParseResponse(std::string id, std::string message);

		void ParseMessage(std::string message);

		beast::flat_buffer buffer;
		void MessageHandler(beast::error_code const& ec, std::size_t bytes_written);

		void RecieveMessages();

		void Connect();

		void Send(String message);
		void Send(String command, String data);
	public:
		// ����������� ����� �������
		static const std::string DEFAULT_URL;
		// ����������� ���� ��� �����������
		static const std::string DEFAULT_PORT;

		/*
		* ������ ���������� ����� �������� � ��������
		* ��� ��������� ���������� ���������� ConnectionException
		* � ������� ������ ����� ����������� DEFAULT_URL, � �������� ����� - DEFAULT_PORT
		*/
		WebSocketAsyncGameConnection();

		/*
		* ������ ���������� ����� �������� � ��������
		* ��� ��������� ���������� ���������� ConnectionException
		*/
		WebSocketAsyncGameConnection(std::string url, std::string port);

		/*
		* ��������� ���������� � ��������
		* ��������� ���� � ������� �� ������ ���� ��� ����������
		*/
		~WebSocketAsyncGameConnection();
	protected:
		void					_Register(String nickname);
		std::vector<Player>		_GetPlayers();
		void					_SendOffer(PlayerID sendTo);
		std::vector<PlayerID>	_GetOffers();
		std::vector<Message>	_GetMessages();
		void					_RemoveMessage(MessageID id);

#ifdef _DEBUG
		std::vector<Message>	_GetParsedMessages();
		std::vector<Message>	_GetAllMessages();
#endif

		void					_SendMessage(String message);
		void					_EndGame();
	};
}