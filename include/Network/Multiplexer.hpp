/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 15:40:26 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 18:31:58 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"

#include <unistd.h>
#include <sys/epoll.h>

#define MAX_QUEUE_EVENTS_LENGTH 1024

class Multiplexer
{
	private:
		int					m_EpollFd;
		struct epoll_event	m_Events[MAX_QUEUE_EVENTS_LENGTH];
	public:
		Multiplexer();
		~Multiplexer();

		bool	Init();
		bool	AddConnection(int fd, uint32_t events);
		bool	RemoveConnection(int fd);
		int		WaitEvents();

		int			GetEventFd(int index) const;
		uint32_t	GetEventFlags(int index) const;
};
