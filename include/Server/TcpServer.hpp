/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:50:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/20 14:09:47 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class TcpServer
{
	private:
		int	m_Port;
		int	m_ListenFd;

	public:
		TcpServer();
		TcpServer(int port);
		~TcpServer();

		bool	Setup();
		int		GetPort() const;
		int		GetListenFd() const;
		
};
