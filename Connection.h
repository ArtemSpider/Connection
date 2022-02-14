#pragma once

#include <exception>
#include <string>
#include <vector>

// Отключить предупреждения в этих библиотеках
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
	* Переопределение строки, используемое для имён и сообщений
	* В будущем тип может измениться
	*/
	typedef std::string String;

	/*
	* Уникальный идентификатор игрока
	* В будущем тип может измениться
	*/
	typedef String PlayerID;

	/*
	* Структура для хранении всей необходимой информации об игроке
	*/
	struct Player
	{
		PlayerID id;
		String nickname;

		Player(PlayerID id, String nickname) : id(id), nickname(nickname) {}
	};

	/*
	* Уникальный идентификатор сообщения
	* В будущем тип может измениться
	*/
	typedef int MessageID;

	/*
	* Структура для хранении всей необходимой информации о сообщении
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
	* Базовый класс для соединения с сервером
	*/
	class GameConnection
	{
	public:
		/*
		* Проверяет сообщение на удовлетворение требованиям
		* Сообщение может содержать только буквы русского и английского алфавита, цифры, пробел, тире и нижнее подчёркивание
		*/
		static bool IsValidMessage(String message);

		static const size_t MAX_NICKNAME_SIZE = 16;
		/*
		* Проверяет имя на удовлетворение требованиям
		* Имя может содержать только буквы русского и английского алфавита, цифры, пробел, тире и нижнее подчёркивание
		* Длина имени не должна быть больше MAX_NICKNAME_SIZE
		* Если MAX_NICKNAME_SIZE == -1, проверка длины имени не производится
		*/
		static bool IsValidNickname(String nickname);

		/*
		* Текущий статус соединения
		*/
		enum class State
		{
			/*
			* Соединение не установлено, используется только при сбоях с сервером или подключением
			*/
			NotConnected,

			/*
			* Соединение установлено, игрок на сервере не зарегестирован
			* Для регистрации используется метод Register
			*/
			Registration,

			/*
			* Пользователь зарегестрирован и находится в поиске оппонента
			*/
			Searching,

			/*
			* Пользователь находится в игре
			* В этом состоянии сервер работает исключительно для пересылки сообщений, не обрабатывая их
			*/
			InGame,
		};

		// Стандартный адрес сервера
		static const std::string DEFAULT_URL;
		// Стандартный порт для подключения
		static const std::string DEFAULT_PORT;

		/*
		* Закрывает соединение с сервером
		* Завершает игру и выходит из поиска если это необходимо
		*/
		virtual ~GameConnection();

		/*
		* Возвращает текущий статус соединения
		*/
		State GetState();

		/*
		* Возвращает ID игрока
		* Если пользователь не зарегестрирован, генерирует StateException
		*/
		PlayerID GetID();


		/*
		* Регистрирует игрока на сервере и изменяет статус на поиск оппонента
		* Если имя не удовлетворяет требованиям или игрок с таким именем уже зарегестрирован, генерирует NicknameException
		* Проверить имя на удовлетворение требованиям можно с помощью функции IsValidNickname
		*/
		void Register(String nickname);

		/*
		* Возвращает список всех игроков, находящихся в поиске игры
		* Если пользователь не зарегестрирован, генерирует StateException
		*/
		std::vector<Player> GetPlayers();

		/*
		* Отправляет игроку в поиске приглашение в игру
		* Если пользователь не зарегестрирован, генерирует StateException
		* Если sendTo нет в списке игроков, генерирует UnknownPlayerIdException
		*/
		void SendOffer(PlayerID sendTo);

		/*
		* Возвращает список игроков, пригласивших пользователя в игру
		* Если пользователь не зарегестрирован, генерирует StateException
		*/
		std::vector<PlayerID> GetOffers();

		/*
		* Возвращает все необработанные сообщения от оппонента
		* Если пользователь не находится в игре, генерирует StateException
		*/
		std::vector<Message> GetMessages();

		/*
		* Обозначает сообщение как обработанное, оно более не будет возвращаться методом GetMessages
		* Если пользователь не находится в игре, генерирует StateException
		* Если id нет в списке необработанных сообщений, генерирует UnknownMessageIdException
		*/
		void RemoveMessage(MessageID id);

#ifdef _DEBUG
		/*
		* Возвращает все обработанные сообщения от оппонента
		* Если пользователь не находится в игре, генерирует StateException
		*/
		std::vector<Message> GetParsedMessages();

		/*
		* Возвращает все сообщения от оппонента
		* Если пользователь не находится в игре, генерирует StateException
		*/
		std::vector<Message> GetAllMessages();
#endif
		/*
		* Отправляет сообщение оппоненту
		* Если пользователь не находится в игре, генерирует StateException
		* Если сообщение не удовлетворяет требованиям, генерирует MessageException
		* Проверить сообщение на удовлетворение требованиям можно с помощью функции IsValidMessage
		*/
		void SendMessage(String message);

		/*
		* Уведомляет оппонента о закрытии игры и изменияет текущий статус на поиск оппонента
		* Если пользователь не находится в игре, генерирует StateException
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
	* Класс, поддерживающий соединение с сервером использую веб сокеты
	* Принимает сообщения асинхронно
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

		// Все приглашения игры от других игроков
		std::vector<PlayerID> offers;

		// ID оппонента
		PlayerID opponentID;

		// Возвращает уникальное ID для сообщения
		MessageID GetMessageID();

		// Все необработанные сообщения
		std::unordered_map<MessageID, Message> unparsedMessages;

#ifdef _DEBUG
		// Все обработанные сообщения
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
		// Стандартный адрес сервера
		static const std::string DEFAULT_URL;
		// Стандартный порт для подключения
		static const std::string DEFAULT_PORT;

		/*
		* Создаёт соединение между клиентом и сервером
		* При неудачном соединении генерирует ConnectionException
		* В качесте адреса будет использован DEFAULT_URL, в качестве порта - DEFAULT_PORT
		*/
		WebSocketAsyncGameConnection();

		/*
		* Создаёт соединение между клиентом и сервером
		* При неудачном соединении генерирует ConnectionException
		*/
		WebSocketAsyncGameConnection(std::string url, std::string port);

		/*
		* Закрывает соединение с сервером
		* Завершает игру и выходит из поиска если это необходимо
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