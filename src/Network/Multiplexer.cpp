/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multiplexer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 15:40:24 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 18:33:08 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Network/Multiplexer.hpp"

// The struct epoll_event is defined as:

//            typedef union epoll_data {
//                void    *ptr;
//                int      fd;
//                uint32_t u32;
//                uint64_t u64;
//            } epoll_data_t;

//            struct epoll_event {
//                uint32_t     events;    /* Epoll events */
//                epoll_data_t data;      /* User data variable */
//            };

Multiplexer::Multiplexer()
{
}

Multiplexer::~Multiplexer()
{
	close(this->m_EpollFd);
}

bool	Multiplexer::Init()
{
	this->m_EpollFd = epoll_create(1337);
	if (this->m_EpollFd == -1)
	{
		ERROR_LOG("Failed to create an epoll instance");
		return (false);
	}
	return (true);
}

bool	Multiplexer::AddConnection(int fd, uint32_t events)
{
	struct epoll_event	event;
	int					status;

	event.data.fd = fd;
	event.events = events;

	if ((status = epoll_ctl(this->m_EpollFd, EPOLL_CTL_ADD, fd, &event)) == -1)
	{
		ERROR_LOG("Failed to add entry to the interest list of the epoll fd");
		return (false);
	}
	return (true);
}

bool	Multiplexer::ModifyConnection(int fd, uint32_t events)
{
	struct epoll_event	event;
	int					status;

	event.data.fd = fd;
	event.events = events;

	if ((status = epoll_ctl(this->m_EpollFd, EPOLL_CTL_MOD, fd, &event)) == -1)
	{
		ERROR_LOG("Failed to modify entry to the interest list of the epoll fd");
		return (false);
	}
	return (true);
}

bool	Multiplexer::RemoveConnection(int fd)
{
	int	status = 0;

	if ((status = epoll_ctl(this->m_EpollFd, EPOLL_CTL_DEL, fd, NULL)) == -1)
	{
		ERROR_LOG("Failed to remove entry from the interest list of the epoll fd");
		return (false);
	}
	return (true);
}

int	Multiplexer::WaitEvents()
{
	int	numEvents = 0;

	if ((numEvents = epoll_wait(this->m_EpollFd, this->m_Events, MAX_QUEUE_EVENTS_LENGTH, -1)) == -1)
	{
		ERROR_LOG("Failed to waits for events on the epoll instance");
	}
	return (numEvents);
}

int	Multiplexer::GetEventFd(int eventIndex) const
{
	return (this->m_Events[eventIndex].data.fd);
}

uint32_t	Multiplexer::GetEventFlags(int eventIndex) const
{
	return (this->m_Events[eventIndex].events);
}

bool	Multiplexer::IsReadReady(int eventIndex) const
{
	if ((this->m_Events[eventIndex].events & EPOLLIN) == EPOLLIN)
		return (true);
	return (false);
}

bool	Multiplexer::IsWriteReady(int eventIndex) const
{
	if ((this->m_Events[eventIndex].events & EPOLLOUT) == EPOLLOUT)
		return (true);
	return (false);
}
