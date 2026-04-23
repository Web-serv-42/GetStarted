/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 19:34:19 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/22 20:03:56 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Core/Timer.hpp"

// Initialize the static member
struct timeval	Timer::s_ServerStart;

Timer::Timer() {}

Timer::~Timer() {}

void	Timer::Init()
{
	gettimeofday(&s_ServerStart, NULL);
}

void	Timer::Reset()
{
	gettimeofday(&this->m_LocalStart, NULL);
}

// Global Uptime: Now - ServerStart
double	Timer::GetServerUptime()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - s_ServerStart.tv_sec) + 
		   (now.tv_usec - s_ServerStart.tv_usec) * 1e-6;
}

// Instance Elapsed: Now - LocalStart
double	Timer::Elapsed() const
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - m_LocalStart.tv_sec) + 
		   (now.tv_usec - m_LocalStart.tv_usec) * 1e-6;
}

double Timer::ElapsedMillis() const
{
	return (this->Elapsed() * 1000.0);
}

// Nginx-style timestamp: [day/month/year:hour:minute:second]
std::string	Timer::GetLogTime()
{
	char buf[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now); // Use gmtime for UTC or localtime for system time
	
	// Full format [%d/%b/%Y:%H:%M:%S +0000]
	strftime(buf, sizeof(buf), "[%d/%b/%Y:%H:%M:%S +0000]", &tm);
	return std::string(buf);
}
