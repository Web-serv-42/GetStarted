#pragma once

#include <map>
#include <string>
#include <cstdlib>
#include <ctime>
#include "Core/Timer.hpp"

struct Session
{
	std::string							sessionId;
	time_t								createdAt;
	time_t								lastAccessed;
	std::map<std::string, std::string>	data;
};

class SessionManager
{
	private:
		std::map<std::string, Session*>	m_ActiveSessions;

	public:
		SessionManager();
		~SessionManager();

		std::string	GenerateRandomId();
		Session*	CreateSession();
		Session*	GetSession(const std::string& sessionId);
		void		DestroySession(const std::string& sessionId);
};
