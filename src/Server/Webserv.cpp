/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/19 15:57:20 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"

#include "Core/Timer.hpp"

Webserv::Webserv() : m_IsRunning(false) {}

Webserv::~Webserv()
{
}

bool	Webserv::Init()
{
	TRACE_LOG("Initializing Webserv Engine...");
	// Parse config file
	INFO_LOG("Webserv successfully initialized.");
	return (true);
}

void	Webserv::Run()
{
	Timer::Init();
	this->m_IsRunning = true;
	INFO_LOG("Start listening for events...");

	// Server Loop
	while (this->m_IsRunning)
	{
		if (Timer::GetServerUptime() > 4.0)
			break ;
	}
	
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
    this->m_IsRunning = false;
	// Clean recources
}
