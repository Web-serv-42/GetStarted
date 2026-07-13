/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 14:49:43 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 17:52:27 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Session/SessionManager.hpp"

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
	// Clean up session memory allocations
	for (std::map<std::string, Session*>::iterator it = m_ActiveSessions.begin(); it != m_ActiveSessions.end(); ++it) {
		delete it->second;
	}
}

std::string	 SessionManager::GenerateRandomId()
{
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string	result = "";

	for (int i = 0; i < 16; i++)
	{
		result += charset[std::rand() % charset.length()];
	}
	return (result);
}

Session*	SessionManager::CreateSession()
{
	Session* session = new Session();
	session->sessionId = this->GenerateRandomId();
	session->createdAt = time(NULL);
	session->lastAccessed = time(NULL);
	this->m_ActiveSessions[session->sessionId] = session;
	return (session);
}

Session*	SessionManager::GetSession(const std::string& sessionId)
{
	std::map<std::string, Session*>::iterator it = m_ActiveSessions.find(sessionId);
	if (it != m_ActiveSessions.end())
	{
		it->second->lastAccessed = time(NULL);
		return it->second;
	}
	return (NULL);
}

void	SessionManager::DestroySession(const std::string& sessionId)
{
	std::map<std::string, Session*>::iterator it = m_ActiveSessions.find(sessionId);
	if (it != m_ActiveSessions.end())
	{
		delete it->second;
		this->m_ActiveSessions.erase(it);
	}
}
