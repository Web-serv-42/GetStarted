/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIManager.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 18:37:29 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 11:47:01 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once


#include "Client/Client.hpp"
#include "Network/Multiplexer.hpp"
#include "Utils/utils.hpp"

#include <map>

class CGIManager
{
	private:
		std::map<int, Client*>	m_CgiFdToClient;
		Multiplexer&			m_Polling;
	public:
		CGIManager(Multiplexer& poller);
		~CGIManager();

		HttpStatusCode	AttachCGI(Client* client);
		void			HandleCGI(int pipeFd, int eventIndex);
		void			DetachPipe(int pipeFd);
		void			DetachCGI(CGI* cgi);
		bool			IsCGIPipe(int triggeredFd);
};
