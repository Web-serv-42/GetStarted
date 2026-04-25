/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:53 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 18:29:36 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Network/Client.hpp"

Client::Client()
{
}

Client::Client(int clientFd) : m_SocketFd(clientFd)
{
}

Client::~Client()
{
}

int	Client::GetClientFd() const
{
	return (this->m_SocketFd);
}

